/* ************************************************************************
*  file: mapwld.c                                                        *
*  Usage: drawing ASCII maps of the world, zone by zone                  *
*  Originally by Neil (1995) for a single .wld file; rewritten in 2026   *
*  to read the whole world through lib/world/{wld,zon}/index.            *
************************************************************************* */

/*
 * mapwld [-l libdir] [-n] [-z zone]...
 *
 *   -l libdir   the lib directory (default "lib", i.e. run from the top
 *               of the source tree)
 *   -n          leave out the list of room names under each map
 *   -z zone     map only this zone (may be repeated); default: every zone
 *
 * The map goes to standard output.  Rooms have no coordinates in the world
 * files, so each zone is laid out by walking its exits from its lowest
 * room: north is up the page, east to the right, the diagonals at 45
 * degrees, and up/down to separate levels.  Where two rooms would land on
 * the same spot, the map is pushed apart and the exits across the gap are
 * drawn as longer lines.  Exits that still don't fit (bent corridors,
 * mazes, one-way loops) are listed under the map, as are exits that leave
 * the zone, so nothing is lost.  Groups of rooms with no way between them
 * are drawn as separate "parts"; rooms connected to nothing are listed.
 *
 * Only the standard C library is used; the game's headers aren't needed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NUM_DIRS   10        /* n e s w u d ne nw se sw (the game's order) */
#define CELL_W     8         /* characters per grid column                 */
#define CELL_H     2         /* lines per grid row                         */

static const char *dir_name[NUM_DIRS] =
  { "north", "east", "south", "west", "up", "down",
    "northeast", "northwest", "southeast", "southwest" };
/* grid offsets per direction: x east, y north, z up */
static const int dx[NUM_DIRS] = { 0, 1, 0, -1, 0, 0,  1, -1,  1, -1 };
static const int dy[NUM_DIRS] = { 1, 0, -1, 0, 0, 0,  1,  1, -1, -1 };
static const int dz[NUM_DIRS] = { 0, 0, 0, 0, 1, -1,  0,  0,  0,  0 };

struct exit_data {
  int to;                    /* vnum, or -1 for none                       */
  int door;                  /* nonzero for a door                         */
};

struct room {
  int  vnum;
  char *name;
  struct exit_data ex[NUM_DIRS];
  int  zone;                 /* index into zones[], or -1                  */
  int  part;                 /* 0 = not placed yet                         */
  int  x, y, z;
};

struct zone {
  int  num, bottom, top;
  char *name;
  int  selected;
};

static struct room *rooms;
static int nrooms, maxrooms;
static struct zone *zones;
static int nzones, maxzones;
static int show_names = 1;


/* ---------------------------------------------------------------- */
/* small helpers                                                    */

static void *xrealloc(void *p, size_t n)
{
  if (!(p = realloc(p, n))) {
    perror("mapwld");
    exit(1);
  }
  return p;
}

static char *xstrdup(const char *s)
{
  char *d = xrealloc(NULL, strlen(s) + 1);

  return strcpy(d, s);
}

/* Read a '~'-terminated string (possibly several lines); returns its first
   line, or NULL at end of file. */
static char *read_tilde_string(FILE *fp)
{
  static char first[256];
  char line[1024], *t;
  int got_first = 0;

  while (fgets(line, sizeof(line), fp)) {
    t = strchr(line, '~');
    if (!got_first) {
      if (t)
	*t = '\0';
      line[strcspn(line, "\r\n")] = '\0';
      strncpy(first, line, sizeof(first) - 1);
      first[sizeof(first) - 1] = '\0';
      got_first = 1;
    }
    if (t)
      return first;
  }
  return NULL;
}

static int cmp_room(const void *a, const void *b)
{
  return ((const struct room *) a)->vnum - ((const struct room *) b)->vnum;
}

static int cmp_zone(const void *a, const void *b)
{
  return ((const struct zone *) a)->top - ((const struct zone *) b)->top;
}

static struct room *find_room(int vnum)
{
  struct room key;

  key.vnum = vnum;
  return bsearch(&key, rooms, nrooms, sizeof(struct room), cmp_room);
}

/* the zone a vnum falls in, as the game decides it (by zone ranges) */
static int zone_of(int vnum)
{
  int i;

  for (i = 0; i < nzones; i++)
    if (vnum >= zones[i].bottom && vnum <= zones[i].top)
      return i;
  return -1;
}


/* ---------------------------------------------------------------- */
/* reading the world                                                */

/* The file names listed in <dir>/index, up to its "$" line. */
static char **read_index(const char *dir, int *count)
{
  char path[512], line[256], **names = NULL;
  FILE *fp;
  int n = 0;

  snprintf(path, sizeof(path), "%s/index", dir);
  if (!(fp = fopen(path, "r"))) {
    perror(path);
    exit(1);
  }
  while (fgets(line, sizeof(line), fp)) {
    line[strcspn(line, " \t\r\n")] = '\0';
    if (*line == '$')
      break;
    if (!*line)
      continue;
    names = xrealloc(names, sizeof(char *) * (n + 1));
    names[n++] = xstrdup(line);
  }
  fclose(fp);
  *count = n;
  return names;
}

static void read_zone_file(const char *path)
{
  char line[256], *name;
  int num, top;
  FILE *fp;

  if (!(fp = fopen(path, "r"))) {
    perror(path);
    return;
  }
  if (!fgets(line, sizeof(line), fp) || sscanf(line, "#%d", &num) != 1 ||
      !(name = read_tilde_string(fp))) {
    fprintf(stderr, "%s: not a zone file; skipped.\n", path);
    fclose(fp);
    return;
  }
  name = xstrdup(name);
  /* builder~, level recommendation~, "recall pkill", then "top lifespan reset" */
  if (!read_tilde_string(fp) || !read_tilde_string(fp) ||
      !fgets(line, sizeof(line), fp) || !fgets(line, sizeof(line), fp) ||
      sscanf(line, " %d", &top) != 1) {
    fprintf(stderr, "%s: can't find the zone's top room; skipped.\n", path);
    free(name);
    fclose(fp);
    return;
  }
  fclose(fp);

  if (nzones == maxzones)
    zones = xrealloc(zones, sizeof(struct zone) * (maxzones += 64));
  zones[nzones].num = num;
  zones[nzones].top = top;
  zones[nzones].name = name;
  zones[nzones].selected = 0;
  nzones++;
}

static void read_world_file(const char *path)
{
  char line[1024], *name;
  struct room *r = NULL;
  int vnum, dir, type, key, to;
  FILE *fp;

  if (!(fp = fopen(path, "r"))) {
    perror(path);
    return;
  }
  while (fgets(line, sizeof(line), fp)) {
    if (*line == '$')
      break;
    if (*line == '#' && sscanf(line, "#%d", &vnum) == 1) {
      if (!(name = read_tilde_string(fp)) ||        /* name        */
	  (name = xstrdup(name), !read_tilde_string(fp)) ||  /* description */
	  !fgets(line, sizeof(line), fp)) {         /* zone flags sector */
	fprintf(stderr, "%s: room #%d is cut short.\n", path, vnum);
	break;
      }
      if (nrooms == maxrooms)
	rooms = xrealloc(rooms, sizeof(struct room) * (maxrooms += 1024));
      r = &rooms[nrooms++];
      memset(r, 0, sizeof(*r));
      r->vnum = vnum;
      r->name = name;
      for (dir = 0; dir < NUM_DIRS; dir++)
	r->ex[dir].to = -1;
    } else if (r && *line == 'D' && sscanf(line, "D%d", &dir) == 1) {
      /* exit description~, keywords~, then "type key destination" */
      if (!read_tilde_string(fp) || !read_tilde_string(fp) ||
	  !fgets(line, sizeof(line), fp) ||
	  sscanf(line, " %d %d %d", &type, &key, &to) != 3) {
	fprintf(stderr, "%s: bad exit in room #%d.\n", path, r->vnum);
	continue;
      }
      if (dir >= 0 && dir < NUM_DIRS) {
	r->ex[dir].to = to;
	r->ex[dir].door = type;
      }
    } else if (r && *line == 'E' && (line[1] == '\n' || line[1] == '\r')) {
      read_tilde_string(fp);                        /* keywords    */
      read_tilde_string(fp);                        /* description */
    }
  }
  fclose(fp);
}

static void read_world(const char *lib)
{
  char dir[512], path[1024], **files;
  int n, i;

  snprintf(dir, sizeof(dir), "%s/world/zon", lib);
  files = read_index(dir, &n);
  for (i = 0; i < n; i++) {
    snprintf(path, sizeof(path), "%s/%s", dir, files[i]);
    read_zone_file(path);
  }
  qsort(zones, nzones, sizeof(struct zone), cmp_zone);
  for (i = 0; i < nzones; i++)                       /* as the game does */
    zones[i].bottom = i ? zones[i - 1].top + 1 : 0;

  snprintf(dir, sizeof(dir), "%s/world/wld", lib);
  files = read_index(dir, &n);
  for (i = 0; i < n; i++) {
    snprintf(path, sizeof(path), "%s/%s", dir, files[i]);
    read_world_file(path);
  }
  qsort(rooms, nrooms, sizeof(struct room), cmp_room);
  for (i = 0; i < nrooms; i++)
    rooms[i].zone = zone_of(rooms[i].vnum);
}


/* ---------------------------------------------------------------- */
/* laying a zone out on a grid                                      */

static struct room **zrooms;      /* the rooms of the zone being drawn */
static int nz;

static struct room *at(int part, int x, int y, int z)
{
  int i;

  for (i = 0; i < nz; i++)
    if (zrooms[i]->part == part && zrooms[i]->x == x &&
	zrooms[i]->y == y && zrooms[i]->z == z)
      return zrooms[i];
  return NULL;
}

/* Make room at the cell next to r in direction dir by pushing every room
   of the part that lies beyond r along that direction one step further.
   Exits that cross the gap are then drawn as longer lines. */
static void stretch(const struct room *r, int part, int dir)
{
  int i;
  struct room *t;

  for (i = 0; i < nz; i++) {
    t = zrooms[i];
    if (t->part != part)
      continue;
    if (dx[dir] > 0 && t->x > r->x) t->x++;
    if (dx[dir] < 0 && t->x < r->x) t->x--;
    if (dy[dir] > 0 && t->y > r->y) t->y++;
    if (dy[dir] < 0 && t->y < r->y) t->y--;
    if (dz[dir] > 0 && t->z > r->z) t->z++;
    if (dz[dir] < 0 && t->z < r->z) t->z--;
  }
}

static void place(struct room *r, struct room *t, int dir, int part,
		  struct room **queue, int *tail)
{
  if (at(part, r->x + dx[dir], r->y + dy[dir], r->z + dz[dir]))
    stretch(r, part, dir);
  t->part = part;
  t->x = r->x + dx[dir];
  t->y = r->y + dy[dir];
  t->z = r->z + dz[dir];
  queue[(*tail)++] = t;
}

/* Place every room connected to start inside the zone, by exits in either
   direction; returns how many were placed. */
static int place_part(struct room *start, int part)
{
  static const int rev[NUM_DIRS] = { 2, 3, 0, 1, 5, 4, 9, 8, 7, 6 };
  struct room **queue = xrealloc(NULL, sizeof(struct room *) * nz), *r, *t;
  int head = 0, tail = 0, dir, i;

  start->part = part;
  start->x = start->y = start->z = 0;
  queue[tail++] = start;
  while (head < tail) {
    r = queue[head++];
    /* the exits out of r ... */
    for (dir = 0; dir < NUM_DIRS; dir++)
      if (r->ex[dir].to >= 0 && (t = find_room(r->ex[dir].to)) &&
	  t->zone == r->zone && !t->part)
	place(r, t, dir, part, queue, &tail);
    /* ... and the one-way exits into it */
    for (i = 0; i < nz; i++) {
      t = zrooms[i];
      if (t->part)
	continue;
      for (dir = 0; dir < NUM_DIRS; dir++)
	if (t->ex[dir].to == r->vnum) {
	  place(r, t, rev[dir], part, queue, &tail);
	  break;
	}
    }
  }
  free(queue);
  return tail;
}

/* If the exit r -> dir is drawn on the grid, the number of steps to its
   target (straight along dir, through empty cells only); else 0. */
static int drawn(const struct room *r, int dir)
{
  const struct room *t = find_room(r->ex[dir].to);
  int k, s, ddx, ddy;

  if (!t || t->zone != r->zone || t->part != r->part || t->z != r->z ||
      dir == 4 || dir == 5)
    return 0;
  ddx = t->x - r->x;
  ddy = t->y - r->y;
  k = dx[dir] ? ddx / dx[dir] : ddy / dy[dir];
  if (k < 1 || ddx != k * dx[dir] || ddy != k * dy[dir])
    return 0;
  for (s = 1; s < k; s++)
    if (at(r->part, r->x + s * dx[dir], r->y + s * dy[dir], r->z))
      return 0;
  return k;
}

/* Is the up/down exit r -> dir on the grid (straight above or below)? */
static int stacked(const struct room *r, int dir)
{
  const struct room *t = find_room(r->ex[dir].to);

  return t && t->zone == r->zone && t->part == r->part &&
    t->x == r->x && t->y == r->y &&
    (dir == 4 ? t->z > r->z : t->z < r->z);
}

static void put(char *canvas, int w, int h, int row, int col, char c)
{
  char *p;

  if (row < 0 || row >= h || col < 0 || col >= w)
    return;
  p = canvas + row * (w + 1) + col;
  if ((*p == '/' && c == '\\') || (*p == '\\' && c == '/'))
    *p = 'X';                     /* two diagonals crossing */
  else
    *p = c;
}

/* Draw one level of one part. */
static void draw_level(int part, int z)
{
  int minx = 0, maxx = 0, miny = 0, maxy = 0, first = 1, i, w, h, row, col, dir;
  char *canvas, label[16], mark;
  struct room *r;

  /* the whole part's extent, so that its levels line up */
  for (i = 0; i < nz; i++) {
    r = zrooms[i];
    if (r->part != part)
      continue;
    if (first || r->x < minx) minx = r->x;
    if (first || r->x > maxx) maxx = r->x;
    if (first || r->y < miny) miny = r->y;
    if (first || r->y > maxy) maxy = r->y;
    first = 0;
  }
  w = (maxx - minx + 1) * CELL_W;
  h = (maxy - miny) * CELL_H + 1;
  canvas = xrealloc(NULL, (size_t) (w + 1) * h + 1);
  for (row = 0; row < h; row++) {
    memset(canvas + row * (w + 1), ' ', w);
    canvas[row * (w + 1) + w] = '\n';
  }
  canvas[(w + 1) * h] = '\0';

  for (i = 0; i < nz; i++) {
    r = zrooms[i];
    if (r->part != part || r->z != z)
      continue;
    row = (maxy - r->y) * CELL_H;
    col = (r->x - minx) * CELL_W;

    /* the room: its number, then ^ v or % for exits up, down or both */
    mark = ' ';
    if (r->ex[4].to >= 0 && r->ex[5].to >= 0)
      mark = '%';
    else if (r->ex[4].to >= 0)
      mark = '^';
    else if (r->ex[5].to >= 0)
      mark = 'v';
    snprintf(label, sizeof(label), "%5d%c", r->vnum, mark);
    memcpy(canvas + row * (w + 1) + col, label, 6);

    /* the exits, as long as the gap to their target; doors as = and : */
    for (dir = 0; dir < NUM_DIRS; dir++) {
      int k = r->ex[dir].to >= 0 ? drawn(r, dir) : 0, step, rr, cc;
      char hc = r->ex[dir].door ? '=' : '-', vc = r->ex[dir].door ? ':' : '|';

      for (step = 0; step < k; step++) {
	rr = row - step * dy[dir] * CELL_H;        /* the cell the line leaves */
	cc = col + step * dx[dir] * CELL_W;
	if (step) {                                 /* crossing an empty cell */
	  if (dir == 1 || dir == 3)
	    memset(canvas + rr * (w + 1) + cc, hc, 6);
	  else if (dir == 0 || dir == 2)
	    put(canvas, w, h, rr, cc + 2, vc);
	  else
	    put(canvas, w, h, rr, cc + 3, (dir == 6 || dir == 9) ? '/' : '\\');
	}
	switch (dir) {
	case 1: put(canvas, w, h, rr, cc + 6, hc); put(canvas, w, h, rr, cc + 7, hc); break;
	case 3: put(canvas, w, h, rr, cc - 2, hc); put(canvas, w, h, rr, cc - 1, hc); break;
	case 0: put(canvas, w, h, rr - 1, cc + 2, vc); break;
	case 2: put(canvas, w, h, rr + 1, cc + 2, vc); break;
	case 6: put(canvas, w, h, rr - 1, cc + 7, '/'); break;
	case 9: put(canvas, w, h, rr + 1, cc - 1, '/'); break;
	case 7: put(canvas, w, h, rr - 1, cc - 1, '\\'); break;
	case 8: put(canvas, w, h, rr + 1, cc + 7, '\\'); break;
	}
      }
    }
  }

  /* trim trailing blanks; leave out blank lines (nothing crosses them) */
  for (row = 0; row < h; row++) {
    char *line = canvas + row * (w + 1);

    for (col = w; col > 0 && line[col - 1] == ' '; col--)
      ;
    if (col)
      printf("%.*s\n", col, line);
  }
  free(canvas);
}


static void map_zone(int zi)
{
  struct zone *zn = &zones[zi];
  struct room *r, *t;
  int i, j, dir, part = 0, placed, z, minz, maxz, nleave = 0, nother = 0;
  int nalone = 0, shown = 0;

  nz = 0;
  for (i = 0; i < nrooms; i++)
    if (rooms[i].zone == zi)
      zrooms[nz++] = &rooms[i];
  if (!nz)
    return;

  printf("==============================================================================\n");
  printf("Zone %d: %s  (rooms %d-%d, %d rooms)\n", zn->num, zn->name,
	 zn->bottom, zn->top, nz);
  printf("==============================================================================\n");

  for (i = 0; i < nz; i++) {
    if (zrooms[i]->part)
      continue;
    placed = place_part(zrooms[i], ++part);
    if (placed == 1) {                /* drawn as a list after the maps */
      nalone++;
      continue;
    }
    shown++;

    minz = maxz = 0;
    for (j = 0; j < nz; j++)
      if (zrooms[j]->part == part) {
	if (zrooms[j]->z < minz) minz = zrooms[j]->z;
	if (zrooms[j]->z > maxz) maxz = zrooms[j]->z;
      }
    for (z = maxz; z >= minz; z--) {
      printf("\nPart %d (%d room%s from %d)", shown, placed,
	     placed == 1 ? "" : "s", zrooms[i]->vnum);
      if (minz != maxz)
	printf(", level %+d", z);
      printf(":\n\n");
      draw_level(part, z);
    }
  }

  /* rooms connected to nothing else in the zone */
  if (nalone) {
    printf("\nRooms connected to nothing else in this zone (%d):\n", nalone);
    for (i = 0, j = 0; i < nz; i++) {
      int alone = 1;

      for (dir = 0; dir < nz && alone; dir++)
	if (dir != i && zrooms[dir]->part == zrooms[i]->part)
	  alone = 0;
      if (!alone)
	continue;
      printf("%s%6d", j % 12 ? "" : (j ? "\n " : " "), zrooms[i]->vnum);
      j++;
    }
    printf("\n");
  }

  /* exits that aren't on the grid */
  for (i = 0; i < nz; i++) {
    r = zrooms[i];
    for (dir = 0; dir < NUM_DIRS; dir++) {
      if (r->ex[dir].to < 0 || drawn(r, dir))
	continue;
      t = find_room(r->ex[dir].to);
      if ((dir == 4 || dir == 5) && stacked(r, dir))
	continue;               /* up/down to the level above/below */
      if (!t || t->zone != r->zone) {
	if (!nleave++)
	  printf("\nExits out of the zone:\n");
	if (!t)
	  printf("  %5d %-9s -> %d (no such room)\n", r->vnum, dir_name[dir],
		 r->ex[dir].to);
	else
	  printf("  %5d %-9s -> %d in zone %d, %s\n", r->vnum, dir_name[dir],
		 t->vnum, t->zone >= 0 ? zones[t->zone].num : -1,
		 t->zone >= 0 ? zones[t->zone].name : "(none)");
      }
    }
  }
  for (i = 0; i < nz; i++) {
    r = zrooms[i];
    for (dir = 0; dir < NUM_DIRS; dir++) {
      if (r->ex[dir].to < 0 || drawn(r, dir))
	continue;
      t = find_room(r->ex[dir].to);
      if (!t || t->zone != r->zone)
	continue;
      if ((dir == 4 || dir == 5) && stacked(r, dir))
	continue;
      if (!nother++)
	printf("\nExits inside the zone that don't fit the grid:\n");
      if (t->part != r->part)
	printf("  %5d %-9s -> %d (in another part)\n", r->vnum, dir_name[dir],
	       t->vnum);
      else
	printf("  %5d %-9s -> %d\n", r->vnum, dir_name[dir], t->vnum);
    }
  }

  if (show_names) {
    printf("\nRooms:\n");
    for (i = 0; i < nz; i++)
      printf("  %5d  %s\n", zrooms[i]->vnum, zrooms[i]->name);
  }
  printf("\n");
}


static void usage(const char *prog)
{
  fprintf(stderr, "Usage: %s [-l libdir] [-n] [-z zone]...\n"
	  "  Draws an ASCII map of every zone (or the -z zones) on standard output.\n"
	  "  -l libdir  the lib directory (default: lib)\n"
	  "  -n         no list of room names under each map\n", prog);
  exit(1);
}


int main(int argc, char **argv)
{
  const char *lib = "lib";
  int *want = NULL, nwant = 0, i, j, total = 0;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-l") && i + 1 < argc)
      lib = argv[++i];
    else if (!strcmp(argv[i], "-n"))
      show_names = 0;
    else if (!strcmp(argv[i], "-z") && i + 1 < argc && isdigit((unsigned char) *argv[i + 1])) {
      want = xrealloc(want, sizeof(int) * (nwant + 1));
      want[nwant++] = atoi(argv[++i]);
    } else
      usage(argv[0]);
  }

  read_world(lib);
  zrooms = xrealloc(NULL, sizeof(struct room *) * (nrooms + 1));

  for (i = 0; i < nzones; i++) {
    zones[i].selected = !nwant;
    for (j = 0; j < nwant; j++)
      if (zones[i].num == want[j])
	zones[i].selected = 1;
  }

  for (i = 0; i < nzones; i++)
    total += zones[i].selected;
  if (!total) {
    fprintf(stderr, "No such zone.\n");
    return 1;
  }
  total = 0;

  printf("DominionMUD world map: %d rooms in %d zones.\n\n", nrooms, nzones);
  printf("Each room is its number; north is up the page.  After the number,\n"
	 "^ = an exit up, v = down, %% = both.  - | / \\ are exits (X where two\n"
	 "diagonals cross), = and : are doors.  Each part is laid out by walking\n"
	 "the exits from its first room, pushing the map apart where rooms would\n"
	 "overlap; exits that still don't fit are listed under the map.\n\n");

  for (i = 0; i < nzones; i++)
    if (zones[i].selected)
      map_zone(i);
  return 0;
}
