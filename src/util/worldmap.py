#!/usr/bin/env python3
"""Draw a fantasy-style map of the Dominion as a PNG.

    python3 src/util/worldmap.py [-l lib] [-s scale] [-o out.png]

At scale 1 (the default) the map is 7680x4320 ("8K"); -s 0.25 gives a quick
1920x1080 preview.  Run it from the top of the tree, or pass -l.

Where each zone sits, its kind of icon, the land, seas, rivers, biomes,
mountain ranges and region labels are set by hand in worldmap_layout.py;
edit that file after adding or moving zones.  Roads come from the exits
between zones (worldmap_zones.py reads them from lib/world).  The drawing
is seeded, so the same layout always gives the same map.

Needs Python 3 with numpy, opencv-python (cv2) and Pillow, and about 4 GB
of memory at full size.  Labels use Lora if it is installed and fall back
to DejaVu Serif or FreeSerif.
"""
import sys, os, math, random, time, argparse
import numpy as np, cv2
from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from worldmap_layout import *
import worldmap_zones

ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
ap.add_argument("-l", "--lib", default="lib", help="path to the lib directory (default: lib)")
ap.add_argument("-s", "--scale", type=float, default=1.0, help="1 = 7680x4320 (default), 0.25 = preview")
ap.add_argument("-o", "--out", default="dominion-map.png", help="output PNG (default: dominion-map.png)")
args = ap.parse_args()
S, OUT = args.scale, args.out
W, H = int(7680 * S), int(4320 * S)
RNG = np.random.default_rng(1997)
random.seed(1997)
T0 = time.time()
def log(m): print(f"[{time.time()-T0:6.1f}s] {m}", flush=True)
def P(x, y): return x * W, y * H
def s(v): return v * S

# ---------------------------------------------------------------- noise
def fbm(h, w, base, octaves, seed, persist=0.5, lac=2.0):
    rng = np.random.default_rng(seed)
    out = np.zeros((h, w), np.float32); amp = 1.0; tot = 0; per = base
    for _ in range(octaves):
        gh, gw = max(2, int(h / per) + 3), max(2, int(w / per) + 3)
        g = rng.standard_normal((gh, gw)).astype(np.float32)
        big = cv2.resize(g, (int(gw * per), int(gh * per)), interpolation=cv2.INTER_CUBIC)
        oy, ox = rng.integers(0, max(1, big.shape[0] - h)), rng.integers(0, max(1, big.shape[1] - w))
        out += amp * big[oy:oy + h, ox:ox + w]; tot += amp; amp *= persist; per /= lac
        if per < 1.5: break
    return out / tot

FONT_DIRS = ["/usr/share/fonts", "/usr/local/share/fonts", os.path.expanduser("~/.fonts"),
             os.path.expanduser("~/.local/share/fonts"), "/Library/Fonts", "/System/Library/Fonts",
             os.path.expanduser("~/Library/Fonts"), "C:/Windows/Fonts"]
FONT_WANT = {
    "lab": ["Lora-Variable.ttf", "Lora[wght].ttf", "DejaVuSerif.ttf", "FreeSerif.ttf", "LiberationSerif-Regular.ttf"],
    "ital": ["Lora-Italic-Variable.ttf", "Lora-Italic[wght].ttf", "DejaVuSerif-Italic.ttf", "FreeSerifItalic.ttf", "LiberationSerif-Italic.ttf"],
    "sym": ["DejaVuSans.ttf", "FreeSerif.ttf", "Symbola.ttf"],
}
def find_font(names):
    found = {}
    for d in FONT_DIRS:
        for root, _, files in os.walk(d):
            for f in files:
                found.setdefault(f, os.path.join(root, f))
    for n in names:
        if n in found: return found[n]
    return None
FONTS = {k: find_font(v) for k, v in FONT_WANT.items()}
FONTS["cap"] = FONTS["lab"]
for k, v in FONTS.items():
    if v is None: print(f"warning: no font found for {k}; using Pillow's default", file=sys.stderr)
def font(k, size, bold=False):
    if FONTS[k] is None: return ImageFont.load_default(max(6, int(size)))
    f = ImageFont.truetype(FONTS[k], max(6, int(size)))
    if k in ("lab", "ital", "cap"):
        try: f.set_variation_by_axes([700 if bold else 500])
        except Exception: pass
    return f


def blend(alpha, col):
    """img = img*(1-alpha) + col*alpha, in bands; alpha is HxW float32."""
    col = np.array(col, np.float32)
    for y0 in range(0, H, 512):
        sl = slice(y0, min(H, y0 + 512))
        a = alpha[sl][..., None].astype(np.float32)
        img[sl] = np.clip(img[sl] * (1 - a) + col * a, 0, 255).astype(np.uint8)

# ---------------------------------------------------------------- land mask
log(f"canvas {W}x{H}")
lw, lh = W // 4, H // 4
yy, xx = np.mgrid[0:lh, 0:lw].astype(np.float32)
wx = fbm(lh, lw, lw / 6, 5, 11) * 0.035 * lw
wy = fbm(lh, lw, lw / 6, 5, 12) * 0.035 * lh
X = (xx + wx) / lw; Y = (yy + wy) / lh
def ell(cx, cy, rx, ry):
    return np.exp(-(((X - cx) / rx) ** 2 + ((Y - cy) / ry) ** 2))
field = np.zeros((lh, lw), np.float32)
for e in LAND: field = np.maximum(field, ell(*e))
for e in SEA: field = field * (1 - 0.95 * ell(*e))
# fractured northern ice
north = np.clip((0.16 - Y) / 0.10, 0, 1)
ice = fbm(lh, lw, lw / 40, 4, 13)
field = np.maximum(field, north * (0.55 + 0.9 * ice))
field = field * (1 - 0.6 * north * np.clip((fbm(lh, lw, lw / 90, 3, 14) - 0.25) * 5, 0, 1))
field += 0.22 * fbm(lh, lw, lw / 10, 6, 15)
field = cv2.resize(field, (W, H), interpolation=cv2.INTER_CUBIC)
field += 0.07 * fbm(H, W, s(120), 5, 16)
land = field > 0.45
# lakes
lakes = np.zeros((H, W), np.uint8)
lk = fbm(H, W, s(200), 4, 17)
Yf, Xf = None, None
for (cx, cy, rx, ry) in LAKES:
    x0, y0 = int((cx - rx * 2) * W), int((cy - ry * 2) * H)
    x1, y1 = int((cx + rx * 2) * W), int((cy + ry * 2) * H)
    gy, gx = np.mgrid[y0:y1, x0:x1].astype(np.float32)
    d = ((gx / W - cx) / rx) ** 2 + ((gy / H - cy) / ry) ** 2
    lakes[y0:y1, x0:x1] |= ((d + 0.5 * lk[y0:y1, x0:x1]) < 1).astype(np.uint8)
# keep every zone on land
for z, (n, x, y, k) in ZONES.items():
    cv2.circle(land.view(np.uint8), (int(x * W), int(y * H)), int(s(55)), 1, -1)
land = land.astype(np.uint8)
# tidy specks
land = cv2.morphologyEx(land, cv2.MORPH_OPEN, np.ones((3, 3), np.uint8))
land[lakes > 0] = 0
n, lab, stats, _ = cv2.connectedComponentsWithStats(land, 4)
small = stats[:, cv2.CC_STAT_AREA] < s(s(900))
small[0] = False
land[small[lab]] = 0
log("land mask")

water = (land == 0).astype(np.uint8)
dw = cv2.distanceTransform(water, cv2.DIST_L2, 5)
dl = cv2.distanceTransform(land, cv2.DIST_L2, 5)
log("distances")

# ---------------------------------------------------------------- biomes
COL = {
    "grass": (158, 166, 92), "forest": (104, 126, 62), "darkforest": (66, 84, 50),
    "desert": (220, 192, 136), "ash": (92, 80, 74), "waste": (156, 128, 92),
    "swamp": (96, 108, 72), "hills": (182, 166, 110), "plains": (184, 180, 108),
    "scar": (164, 138, 100), "snow": (236, 242, 246), "mount": (168, 152, 122),
}
names = list(COL)
BW = W // 4; BH = H // 4
by, bx = np.mgrid[0:BH, 0:BW].astype(np.float32)
bx = (bx + fbm(BH, BW, BW / 12, 4, 31) * BW * 0.02) / BW
by = (by + fbm(BH, BW, BW / 12, 4, 32) * BH * 0.02) / BH
wts = {k: np.zeros((BH, BW), np.float32) for k in names}
wts["grass"] += 0.35 + 0.15 * fbm(BH, BW, BW / 8, 3, 33)
for (b, cx, cy, rx, ry, st) in BIOMES:
    wts[b] += st * np.exp(-(((bx - cx) / rx) ** 2 + ((by - cy) / ry) ** 2) ** 1.6) * 2.0
for (cx, cy, rx, ry, dens, size, snowy) in MOUNTAINS:
    wts["mount"] += 1.6 * np.exp(-(((bx - cx) / rx / 1.15) ** 2 + ((by - cy) / ry / 1.15) ** 2) ** 1.6)
snowt = np.clip((0.165 - by + 0.03 * fbm(BH, BW, BW / 15, 4, 34)) / 0.03, 0, 1)
wts["snow"] += snowt * 6
for k in names: wts[k] = cv2.GaussianBlur(wts[k], (0, 0), BW / 400) ** 3
tot = sum(wts.values()) + 1e-6
base = np.zeros((BH, BW, 3), np.float32)
for k in names: base += (wts[k] / tot)[..., None] * np.array(COL[k], np.float32)
base = np.clip(cv2.resize(base, (W, H), interpolation=cv2.INTER_CUBIC), 0, 255).astype(np.uint8)
bio = {k: cv2.resize(wts[k] / tot, (W, H), interpolation=cv2.INTER_LINEAR).astype(np.float16) for k in
       ("forest", "darkforest", "desert", "ash", "swamp", "hills", "snow", "mount", "waste", "plains")}
log("biomes")

# relief: height rising inland, noise, hillshade
hgt = np.clip(dl / s(400), 0, 1) * 0.5 + 0.18 * fbm(H, W, s(180), 5, 41)
hgt += 1.2 * bio["mount"].astype(np.float32); hgt += 0.45 * bio["hills"].astype(np.float32)
hgt = cv2.GaussianBlur(hgt, (0, 0), max(1, s(6)))
shade = cv2.Sobel(hgt, cv2.CV_32F, 1, 0, ksize=3) * -0.7
shade -= cv2.Sobel(hgt, cv2.CV_32F, 0, 1, ksize=3)
del hgt
shade = np.clip(1 + shade * 16 * S, 0.86, 1.14)
shade *= 1.08 + 0.07 * fbm(H, W, s(90), 4, 42) + 0.035 * fbm(H, W, s(10), 2, 43)
wn = fbm(H, W, s(500), 5, 21)
dunen = 6 * fbm(H, W, s(300), 3, 44)
crackn = np.abs(fbm(H, W, s(90), 4, 45)) < 0.035
log("noise")

deep = np.array([18, 44, 56], np.float32); mid = np.array([34, 82, 96], np.float32)
shal = np.array([92, 158, 160], np.float32)
img = np.zeros((H, W, 3), np.uint8)
BAND = 256
for y0 in range(0, H, BAND):
    y1 = min(H, y0 + BAND); sl = slice(y0, y1)
    gy_ = np.linspace(0, 1, H, dtype=np.float32)[sl][:, None]
    gx_ = np.linspace(0, 1, W, dtype=np.float32)[None, :]
    dwb, dlb = dw[sl], dl[sl]
    # water
    t = np.exp(-dwb / s(170))[..., None]
    wimg = deep + (mid - deep) * np.clip(0.55 + 0.6 * wn[sl], 0, 1)[..., None] * 0.6
    wimg = wimg * (1 - t) + mid * t
    glow = np.exp(-dwb / s(28))[..., None]
    wimg = wimg * (1 - glow * 0.85) + shal * glow * 0.85
    for r in (s(34), s(64)):
        ring = (np.exp(-((dwb - r) / s(2.2)) ** 2) * (dwb > 0))[..., None]
        wimg = wimg * (1 - 0.18 * ring) + shal * 0.18 * ring
    vig = (gx_ - 0.5) ** 2 * 1.6 + (gy_ - 0.5) ** 2 * 1.6
    wimg *= (1 - 0.55 * np.clip(vig, 0, 1))[..., None]
    # land
    lc = base[sl].astype(np.float32) * shade[sl][..., None]
    dune = np.sin(gx_ * W / s(38) + dunen[sl] + gy_ * H / s(90))
    lc += (bio["desert"][sl].astype(np.float32) * 14 * dune)[..., None]
    ck = crackn[sl] & (bio["ash"][sl] > 0.35)
    lc[ck] = (150, 52, 30)
    beach = np.exp(-dlb / s(7))[..., None]
    lc = lc * (1 - beach * 0.35) + np.array([214, 200, 150], np.float32) * beach * 0.35
    out = np.where((land[sl] > 0)[..., None], lc, wimg)
    edge = np.exp(-((dlb + dwb) / max(0.9, s(2.6))) ** 2)[..., None]
    out = out * (1 - edge * 0.8) + np.array([34, 40, 34], np.float32) * edge * 0.8
    img[sl] = np.clip(out, 0, 255).astype(np.uint8)
del shade, wn, dunen, crackn, base
log("land colour")

# ---------------------------------------------------------------- compass rhumb lines (water only)
CX, CY = 0.585, 0.905
cxp, cyp = P(CX, CY)
rl = np.zeros((H, W), np.uint8)
for i in range(32):
    a = i * math.pi / 16
    cv2.line(rl, (int(cxp), int(cyp)), (int(cxp + 9000 * S * math.cos(a)), int(cyp + 9000 * S * math.sin(a))),
             255 if i % 4 == 0 else 150, max(1, int(s(4 if i % 4 == 0 else 2))), cv2.LINE_AA)
blend(rl.astype(np.float32) / 255 * 0.22 * (water > 0), (12, 26, 32))

# ---------------------------------------------------------------- rivers
def curve(pts, step):
    pts = [P(*p) for p in pts]
    pts = [pts[0]] + pts + [pts[-1]]
    out = []
    for i in range(1, len(pts) - 2):
        p0, p1, p2, p3 = map(np.array, pts[i - 1:i + 3])
        n = max(2, int(np.linalg.norm(p2 - p1) / step))
        for t in np.linspace(0, 1, n, endpoint=False):
            out.append(0.5 * ((2 * p1) + (-p0 + p2) * t + (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t + (-p0 + 3 * p1 - 3 * p2 + p3) * t ** 3))
    out.append(np.array(pts[-2]))
    out = np.array(out)
    # meander
    d = np.gradient(out, axis=0); nrm = np.stack([-d[:, 1], d[:, 0]], 1)
    nrm /= np.linalg.norm(nrm, axis=1, keepdims=True) + 1e-6
    k = np.arange(len(out))
    wig = np.sin(k * 0.22) * 0.6 + np.sin(k * 0.071 + 1) * 1.0 + np.sin(k * 0.53) * 0.25
    return out + nrm * wig[:, None] * s(9) * np.minimum(1, np.minimum(k, k[::-1]) / 4)[:, None]
riv_mask = np.zeros((H, W), np.uint8)
for pts, wmax in RIVERS:
    c = curve(pts, s(6))
    for i in range(len(c) - 1):
        w = s(6) + s(17) * wmax * (i / len(c))
        cv2.line(riv_mask, tuple(map(int, c[i])), tuple(map(int, c[i + 1])), 255, max(1, int(w)), cv2.LINE_AA)
riv_mask = np.maximum(riv_mask, lakes * 255)
rm = cv2.GaussianBlur(riv_mask, (0, 0), s(1.0)).astype(np.float32) / 255
rim = cv2.dilate(riv_mask, np.ones((max(3, int(s(5))),) * 2, np.uint8))
rim = cv2.GaussianBlur(rim, (0, 0), s(1.0)).astype(np.float32) / 255
blend(rim * 0.75 * (land > 0), (40, 54, 50))
blend(rm * (land > 0), (72, 132, 140))
del rl, rm, rim
log("rivers")

# ---------------------------------------------------------------- roads
links = {z["num"]: z["links"] for z in worldmap_zones.load(args.lib)}
roads = set()
for a_, ls in links.items():
    for b_, n_ in ls.items():
        b_ = int(b_)
        if a_ in ZONES and b_ in ZONES and a_ != b_:
            (_, x1, y1, _), (_, x2, y2, _) = ZONES[a_], ZONES[b_]
            if math.hypot(x1 - x2, (y1 - y2) * 0.5625) < 0.13:
                roads.add(tuple(sorted((a_, b_))))
road_layer = np.zeros((H, W), np.uint8)
for a_, b_ in sorted(roads):
    (_, x1, y1, _), (_, x2, y2, _) = ZONES[a_], ZONES[b_]
    mx, my = (x1 + x2) / 2, (y1 + y2) / 2
    off = (random.random() - 0.5) * 0.25
    mx += -(y2 - y1) * off; my += (x2 - x1) * off
    c = curve([(x1, y1), (mx, my), (x2, y2)], s(5))
    seg = int(s(26)); gap = int(s(16)); acc = 0; on = True
    for i in range(len(c) - 1):
        L_ = np.linalg.norm(c[i + 1] - c[i]); acc += L_
        if on: cv2.line(road_layer, tuple(map(int, c[i])), tuple(map(int, c[i + 1])), 255, max(1, int(s(5))), cv2.LINE_AA)
        if acc > (seg if on else gap): on = not on; acc = 0
blend(road_layer.astype(np.float32) / 255 * 0.8 * (land > 0) * (riv_mask == 0), (92, 62, 36))
del road_layer
log(f"roads {len(roads)}")

# ---------------------------------------------------------------- glyphs
glyphs = []  # (ybase, kind, x, y, size, extra)
occupied = np.zeros((H, W), np.uint8)  # keep glyphs off towns
for z, (n_, x, y, k) in ZONES.items():
    cv2.circle(occupied, tuple(map(int, P(x, y))), int(s(70)), 1, -1)
occupied = np.maximum(occupied, (riv_mask > 0).astype(np.uint8))
occupied = cv2.dilate(occupied, np.ones((max(3, int(s(9))),) * 2, np.uint8))
def free(px, py, pad=0):
    ix, iy = int(px), int(py)
    if not (0 <= ix < W and 0 <= iy < H): return False
    return land[iy, ix] and not occupied[iy, ix] and dl[iy, ix] > s(10 + pad)

def poisson(cx, cy, rx, ry, spacing, test, tries=12000):
    pts = []; cell = spacing / 1.42
    grid = {}
    x0, y0 = (cx - rx) * W, (cy - ry) * H
    for _ in range(tries):
        u, v = random.uniform(-1, 1), random.uniform(-1, 1)
        if u * u + v * v > 1: continue
        px, py = cx * W + u * rx * W, cy * H + v * ry * H
        if not test(px, py, u * u + v * v): continue
        gk = (int(px / cell), int(py / cell)); ok = True
        for dx in (-2, -1, 0, 1, 2):
            for dy in (-2, -1, 0, 1, 2):
                q = grid.get((gk[0] + dx, gk[1] + dy))
                if q and (q[0] - px) ** 2 + (q[1] - py) ** 2 < spacing ** 2: ok = False; break
            if not ok: break
        if ok: grid[gk] = (px, py); pts.append((px, py))
    return pts

for (cx, cy, rx, ry, dens, size, snowy) in MOUNTAINS:
    sp = s(64) * size / max(dens, 0.3) ** 0.5
    for px, py in poisson(cx, cy, rx, ry, sp, lambda px, py, r2: free(px, py, 30) and random.random() < 1.15 - r2):
        r2 = ((px / W - cx) / rx) ** 2 + ((py / H - cy) / ry) ** 2
        sz = s(95) * size * (1.15 - 0.55 * r2) * random.uniform(0.75, 1.15)
        glyphs.append((py, "mount", px, py, sz, snowy or py < 0.2 * H))
# cliff ring
cx, cy, rx, ry = CLIFF_RING
for i in range(0):
    a = i / 54 * 2 * math.pi
    px, py = (cx + rx * math.cos(a)) * W, (cy + ry * math.sin(a)) * H
    if land[int(py), int(px)]:
        glyphs.append((py, "cliff", px, py, s(34), a))
# hills
for (b, cx, cy, rx, ry, st) in BIOMES:
    if b != "hills": continue
    for px, py in poisson(cx, cy, rx * 1.2, ry * 1.2, s(46), lambda px, py, r2: free(px, py) and random.random() < 1.1 - r2):
        glyphs.append((py, "hill", px, py, s(44) * random.uniform(0.8, 1.2), None))
# forests by biome strength
step = int(s(17))
fr = (bio["forest"] + bio["darkforest"]) * (1 + 0.9 * fbm(H, W, s(140), 4, 51))
for iy in range(int(s(20)), H, step):
    for ix in range(0, W, step):
        jx, jy = ix + random.uniform(-0.5, 0.5) * step, iy + random.uniform(-0.5, 0.5) * step
        jx = min(W - 1, max(0, jx)); jy = min(H - 1, max(0, jy))
        f = fr[int(jy), int(jx)]
        if f > 0.30 and random.random() < (f - 0.22) * 2.4 and free(jx, jy, 2):
            dark = bio["darkforest"][int(jy), int(jx)] > bio["forest"][int(jy), int(jx)]
            north_ = jy < 0.33 * H
            glyphs.append((jy, "tree", jx, jy, s(22) * random.uniform(0.85, 1.2), (dark, north_)))
# swamp reeds, desert dunes, snow pines in the north, scattered grass trees
for iy in range(0, H, int(s(30))):
    for ix in range(0, W, int(s(30))):
        jx, jy = ix + random.uniform(0, s(30)), iy + random.uniform(0, s(30))
        if not free(jx, jy, 4): continue
        b_ = {k: bio[k][int(jy), int(jx)] for k in ("swamp", "desert", "snow", "ash", "waste", "plains")}
        if b_["swamp"] > 0.35 and random.random() < 0.6: glyphs.append((jy, "reed", jx, jy, s(16), None))
        elif b_["desert"] > 0.45 and random.random() < 0.12: glyphs.append((jy, "dune", jx, jy, s(46), None))
        elif b_["ash"] > 0.4 and random.random() < 0.08: glyphs.append((jy, "skullrock", jx, jy, s(18), None))
        elif b_["snow"] > 0.4 and jy > 0.05 * H and random.random() < 0.10: glyphs.append((jy, "tree", jx, jy, s(20), (False, True)))
        elif b_["waste"] > 0.4 and random.random() < 0.08: glyphs.append((jy, "deadtree", jx, jy, s(22), None))
        elif fr[int(jy), int(jx)] < 0.12 and b_["desert"] < 0.2 and b_["snow"] < 0.2 and b_["ash"] < 0.2 and random.random() < 0.035:
            glyphs.append((jy, "tree", jx, jy, s(20), (False, False)))
        elif b_["plains"] > 0.3 and random.random() < 0.18: glyphs.append((jy, "grass", jx, jy, s(12), None))
log(f"{len(glyphs)} glyphs placed")

AA = cv2.LINE_AA
def poly(pts, col, outline=None, th=1):
    p = np.array(pts, np.int32).reshape(-1, 1, 2)
    cv2.fillPoly(img, [p], col, AA)
    if outline is not None: cv2.polylines(img, [p], True, outline, max(1, int(th)), AA)

def draw_mount(x, y, sz, snowy, volcanic=False):
    w = sz * random.uniform(0.85, 1.25); h = sz * random.uniform(0.75, 1.0)
    px = x + random.uniform(-0.12, 0.12) * w
    left = [(x - w / 2, y)]; right = [(x + w / 2, y)]
    # jagged ridge
    lpts = [(x - w / 2, y)]
    for t in (0.25, 0.5, 0.75):
        lpts.append((x - w / 2 + (px - (x - w / 2)) * t + random.uniform(-1, 1) * s(2), y - h * t + random.uniform(-0.06, 0.06) * h))
    lpts.append((px, y - h))
    rpts = []
    for t in (0.3, 0.6, 0.85):
        rpts.append((px + ((x + w / 2) - px) * t + random.uniform(-1, 1) * s(2), y - h * (1 - t) + random.uniform(-0.06, 0.06) * h))
    rpts.append((x + w / 2, y))
    base_l = (200, 184, 152) if not volcanic else (110, 96, 88)
    base_d = (112, 96, 78) if not volcanic else (56, 46, 44)
    # shadow side (right) and lit side (left)
    mid = (px + random.uniform(-0.05, 0.12) * w, y)
    poly(lpts + [mid], base_l)
    poly([(px, y - h)] + rpts + [mid], base_d)
    # ridge texture lines on the dark side
    for i in range(3):
        t = random.uniform(0.2, 0.8)
        a = (px + (x + w / 2 - px) * t * 0.6, y - h * (1 - t * 0.6))
        cv2.line(img, (int(a[0]), int(a[1])), (int(a[0] + s(6)), int(a[1] + h * 0.25)), (80, 66, 54), max(1, int(s(1.6))), AA)
    if snowy:
        cap = [(px, y - h)]
        cap += [(px - (px - lpts[2][0]) * 0.75, y - h * 0.62), (px - w * 0.06, y - h * 0.72), (px + w * 0.02, y - h * 0.60),
                (px + (rpts[0][0] - px) * 0.9, y - h * 0.66)]
        poly(cap, (240, 244, 246))
        poly([(px, y - h), (px + w * 0.02, y - h * 0.60), (px + (rpts[0][0] - px) * 0.9, y - h * 0.66)], (196, 206, 214))
    if volcanic and random.random() < 0.35:
        cv2.circle(img, (int(px), int(y - h)), max(1, int(s(5))), (210, 70, 30), -1, AA)
    outl = lpts + [(px, y - h)] + rpts
    cv2.polylines(img, [np.array(outl, np.int32).reshape(-1, 1, 2)], False, (52, 42, 34), max(1, int(s(3))), AA)

def draw_tree(x, y, sz, dark, conifer):
    if conifer:
        c1, c2 = ((52, 82, 58), (34, 54, 40)) if not dark else ((44, 62, 46), (26, 38, 30))
        h = sz * 1.5; w = sz * 0.75
        poly([(x, y - h), (x - w / 2, y), (x, y - h * 0.05)], c1)
        poly([(x, y - h), (x + w / 2, y), (x, y - h * 0.05)], c2)
        cv2.polylines(img, [np.array([(x - w / 2, y), (x, y - h), (x + w / 2, y)], np.int32).reshape(-1, 1, 2)], False, (22, 30, 22), max(1, int(s(1.5))), AA)
        return
    v = random.uniform(0.82, 1.08)
    c1, c2, c3 = ((84, 116, 52), (54, 82, 38), (118, 146, 70)) if not dark else ((54, 74, 46), (34, 48, 32), (74, 94, 58))
    c1, c2, c3 = [tuple(int(c * v) for c in cc) for cc in (c1, c2, c3)]
    r = sz * 0.5
    cv2.line(img, (int(x), int(y)), (int(x), int(y - r * 0.6)), (70, 50, 34), max(1, int(s(3))), AA)
    cv2.circle(img, (int(x), int(y - r * 1.2)), int(r), (26, 36, 24), -1, AA)
    cv2.circle(img, (int(x), int(y - r * 1.2)), max(1, int(r - s(1.5))), c2, -1, AA)
    cv2.circle(img, (int(x - r * 0.2), int(y - r * 1.35)), max(1, int(r * 0.7)), c1, -1, AA)
    cv2.circle(img, (int(x - r * 0.35), int(y - r * 1.55)), max(1, int(r * 0.28)), c3, -1, AA)

def draw_hill(x, y, sz):
    pts = [(x - sz / 2 + sz * t, y - math.sin(math.pi * t) * sz * 0.38) for t in np.linspace(0, 1, 14)]
    poly(pts + [(x + sz / 2, y)], (172, 158, 104))
    poly(pts[6:] + [(x + sz / 2, y), (x + sz * 0.05, y)], (132, 118, 76))
    cv2.polylines(img, [np.array(pts, np.int32).reshape(-1, 1, 2)], False, (70, 58, 40), max(1, int(s(2.5))), AA)

glyphs.sort(key=lambda g: g[0])
for (_, k, x, y, sz, ex) in glyphs:
    if k == "mount": draw_mount(x, y, sz, ex, volcanic=bio["ash"][int(y), int(x)] > 0.3)
    elif k == "tree": draw_tree(x, y, sz, ex[0], ex[1])
    elif k == "hill": draw_hill(x, y, sz)
    elif k == "cliff":
        a = ex; dx, dy = math.cos(a), math.sin(a)
        p1 = (x - dy * sz / 2, y + dx * sz / 2); p2 = (x + dy * sz / 2, y - dx * sz / 2)
        p3 = (p2[0] - dx * sz * 0.5, p2[1] - dy * sz * 0.5 + s(6)); p4 = (p1[0] - dx * sz * 0.5, p1[1] - dy * sz * 0.5 + s(6))
        poly([p1, p2, p3, p4], (126, 108, 84), (60, 48, 36), s(2))
    elif k == "reed":
        for i in range(4):
            cv2.line(img, (int(x + i * s(3)), int(y)), (int(x + i * s(3) + random.uniform(-1, 1) * s(4)), int(y - sz * random.uniform(0.6, 1))), (48, 66, 40), max(1, int(s(1.6))), AA)
        cv2.ellipse(img, (int(x + s(16)), int(y + s(3))), (int(s(8)), int(s(3))), 0, 0, 360, (70, 110, 110), -1, AA)
    elif k == "dune":
        cv2.ellipse(img, (int(x), int(y)), (int(sz / 2), int(sz / 5)), 0, 190, 350, (168, 132, 84), max(1, int(s(2.5))), AA)
        cv2.ellipse(img, (int(x), int(y)), (int(sz / 2), int(sz / 5)), 0, 270, 350, (244, 224, 176), max(1, int(s(1.5))), AA)
    elif k == "skullrock":
        poly([(x - sz, y), (x - sz * 0.4, y - sz), (x + sz * 0.5, y - sz * 0.7), (x + sz, y)], (60, 50, 48), (24, 20, 20), s(1.5))
    elif k == "deadtree":
        cv2.line(img, (int(x), int(y)), (int(x), int(y - sz)), (70, 52, 40), max(1, int(s(2.5))), AA)
        cv2.line(img, (int(x), int(y - sz * 0.6)), (int(x - sz * 0.35), int(y - sz * 0.95)), (70, 52, 40), max(1, int(s(2))), AA)
        cv2.line(img, (int(x), int(y - sz * 0.45)), (int(x + sz * 0.3), int(y - sz * 0.8)), (70, 52, 40), max(1, int(s(2))), AA)
    elif k == "grass":
        for i in (-1, 0, 1):
            cv2.line(img, (int(x), int(y)), (int(x + i * sz * 0.5), int(y - sz)), (104, 116, 58), max(1, int(s(1.5))), AA)
log("glyphs drawn")

# ---------------------------------------------------------------- settlement icons
def rect(x0, y0, x1, y1, col, outl=(40, 30, 24), th=2):
    poly([(x0, y0), (x1, y0), (x1, y1), (x0, y1)], col, outl, s(th))
def roof(x0, x1, y, h, col=(160, 60, 44)):
    poly([(x0 - s(3), y), ((x0 + x1) / 2, y - h), (x1 + s(3), y)], col, (40, 26, 20), s(2))
def tower(x, y, w, h, roofc=(70, 76, 110), wall=(196, 184, 160)):
    rect(x - w / 2, y - h, x + w / 2, y, wall)
    for i in range(3):
        rect(x - w / 2 + i * w / 3 + s(1), y - h - s(6), x - w / 2 + i * w / 3 + w / 6, y - h, wall, th=1.5)
    poly([(x - w / 2 - s(4), y - h - s(5)), (x, y - h - w * 1.3), (x + w / 2 + s(4), y - h - s(5))], roofc, (30, 26, 30), s(2))
    rect(x - w * 0.12, y - h * 0.55, x + w * 0.12, y - h * 0.38, (40, 34, 30), None)
def house(x, y, w, h, rc=(160, 60, 44)):
    rect(x - w / 2, y - h, x + w / 2, y, (214, 196, 160)); roof(x - w / 2, x + w / 2, y - h, h * 0.8, rc)

def icon(kind, x, y, sc=1.35):
    k = s(sc)
    cv2.ellipse(img, (int(x), int(y + 4 * k)), (int(70 * k), int(18 * k)), 0, 0, 360, (60, 60, 40), -1, AA)
    if kind == "capital":
        cv2.ellipse(img, (int(x), int(y)), (int(92 * k), int(36 * k)), 0, 0, 360, (150, 140, 120), -1, AA)
        cv2.ellipse(img, (int(x), int(y)), (int(92 * k), int(36 * k)), 0, 0, 360, (40, 30, 24), max(1, int(3 * k)), AA)
        for dx, dy in ((-40, -6), (-14, 6), (14, -12), (42, 2), (-56, 12), (58, 14), (0, 18)):
            house(x + dx * k, y + dy * k, 24 * k, 18 * k, random.choice([(160, 60, 44), (110, 64, 46), (70, 80, 120)]))
        tower(x, y - 18 * k, 34 * k, 70 * k, (150, 34, 34))
        for dx in (-84, 84): tower(x + dx * k, y + 10 * k, 18 * k, 44 * k)
    elif kind == "city":
        for dx, dy in ((-36, -4), (-10, 8), (14, -8), (38, 4), (-24, 18), (24, 20)):
            house(x + dx * k, y + dy * k, 24 * k, 18 * k, random.choice([(160, 60, 44), (110, 64, 46), (70, 80, 120)]))
        tower(x + 2 * k, y - 4 * k, 24 * k, 52 * k)
    elif kind == "town":
        for dx, dy in ((-22, 0), (4, 8), (26, -4)):
            house(x + dx * k, y + dy * k, 22 * k, 17 * k, random.choice([(160, 60, 44), (110, 64, 46)]))
    elif kind == "keep":
        rect(x - 42 * k, y - 34 * k, x + 42 * k, y + 6 * k, (170, 160, 140))
        for i in range(7):
            rect(x - 42 * k + i * 12 * k, y - 42 * k, x - 36 * k + i * 12 * k, y - 34 * k, (170, 160, 140), th=1.5)
        tower(x - 42 * k, y + 6 * k, 22 * k, 62 * k); tower(x + 42 * k, y + 6 * k, 22 * k, 62 * k)
        tower(x, y - 20 * k, 28 * k, 56 * k, (150, 34, 34))
        rect(x - 8 * k, y - 16 * k, x + 8 * k, y + 6 * k, (50, 38, 30), None)
    elif kind == "tower":
        tower(x, y + 6 * k, 30 * k, 100 * k, (90, 50, 120))
    elif kind == "temple":
        rect(x - 38 * k, y - 26 * k, x + 38 * k, y + 6 * k, (222, 214, 190))
        for i in range(5): rect(x - 32 * k + i * 15 * k, y - 22 * k, x - 28 * k + i * 15 * k, y + 4 * k, (180, 170, 150), th=1)
        poly([(x - 44 * k, y - 26 * k), (x, y - 52 * k), (x + 44 * k, y - 26 * k)], (200, 170, 90), (50, 40, 24), s(2))
    elif kind == "cave":
        poly([(x - 48 * k, y + 6 * k), (x - 30 * k, y - 34 * k), (x - 4 * k, y - 50 * k), (x + 26 * k, y - 38 * k), (x + 50 * k, y + 6 * k)], (130, 116, 98), (40, 32, 26), s(3))
        cv2.ellipse(img, (int(x), int(y + 6 * k)), (int(20 * k), int(30 * k)), 0, 180, 360, (24, 18, 16), -1, AA)
    elif kind == "lair":
        poly([(x - 50 * k, y + 6 * k), (x - 20 * k, y - 50 * k), (x + 6 * k, y - 36 * k), (x + 24 * k, y - 60 * k), (x + 52 * k, y + 6 * k)], (92, 72, 66), (30, 20, 20), s(3))
        cv2.ellipse(img, (int(x), int(y + 6 * k)), (int(18 * k), int(26 * k)), 0, 180, 360, (130, 30, 20), -1, AA)
        cv2.ellipse(img, (int(x), int(y + 6 * k)), (int(10 * k), int(16 * k)), 0, 180, 360, (230, 120, 40), -1, AA)
    elif kind == "camp":
        for dx in (-20, 16):
            poly([(x + (dx - 16) * k, y + 4 * k), (x + dx * k, y - 26 * k), (x + (dx + 16) * k, y + 4 * k)], (200, 180, 140), (50, 40, 30), s(2))
        cv2.line(img, (int(x - 2 * k), int(y - 40 * k)), (int(x - 2 * k), int(y + 4 * k)), (60, 44, 30), max(1, int(3 * k)), AA)
        poly([(x - 2 * k, y - 40 * k), (x + 22 * k, y - 34 * k), (x - 2 * k, y - 28 * k)], (160, 40, 36))

for z, (n_, x, y, kind) in ZONES.items():
    icon(kind, *P(x, y))
log("icons")

# ---------------------------------------------------------------- text (PIL)
pim = Image.fromarray(img)
del img
dr = ImageDraw.Draw(pim, "RGBA")
boxes = []
def overlaps(b):
    return any(not (b[2] < o[0] or b[0] > o[2] or b[3] < o[1] or b[1] > o[3]) for o in boxes)

def plaque(cx, cy, text, sub=None, size=1.0, red=False):
    f = font("lab", s(40) * size, bold=True); fs = font("ital", s(29) * size)
    tw = dr.textlength(text, font=f); sw = dr.textlength(sub, font=fs) if sub else 0
    w = max(tw, sw + s(36)) + s(36); h = s(56) * size + (s(38) * size if sub else 0)
    best = None
    for dx, dy in ((0, s(60)), (0, -s(140) - h), (s(110) + w / 2, -h / 2), (-s(110) - w / 2, -h / 2),
                   (0, s(110)), (0, -s(190) - h), (s(150) + w / 2, 0), (-s(150) - w / 2, 0)):
        b = (cx + dx - w / 2, cy + dy, cx + dx + w / 2, cy + dy + h)
        if not overlaps(b) and b[0] > s(110) and b[2] < W - s(110): best = b; break
    if best is None: best = (cx - w / 2, cy + s(60), cx + w / 2, cy + s(60) + h)
    boxes.append(best)
    x0, y0, x1, y1 = best
    dr.rounded_rectangle((x0 + s(4), y0 + s(5), x1 + s(4), y1 + s(5)), radius=s(10), fill=(0, 0, 0, 70))
    dr.rounded_rectangle(best, radius=s(10), fill=(238, 224, 190, 235), outline=(92, 66, 40, 255), width=max(1, int(s(3))))
    dr.rounded_rectangle((x0 + s(5), y0 + s(5), x1 - s(5), y1 - s(5)), radius=s(7), outline=(150, 118, 76, 200), width=max(1, int(s(1.5))))
    dr.text(((x0 + x1) / 2, y0 + s(28) * size + s(2)), text, font=f, fill=(52, 34, 22), anchor="mm")
    if sub:
        sy = y0 + s(56) * size + s(15) * size
        sk = font("sym", s(26) * size); skw = dr.textlength("\u2620 ", font=sk)
        sx = (x0 + x1) / 2 - (skw + sw) / 2
        dr.text((sx, sy), "\u2620", font=sk, fill=(140, 30, 24), anchor="lm")
        dr.text((sx + skw, sy), sub, font=fs, fill=(140, 30, 24), anchor="lm")

def spaced(text, f, track):
    return sum(dr.textlength(c, font=f) for c in text) + track * (len(text) - 1)

def big_label(text, x, y, size, kind, rot):
    f = font("cap", s(110) * size) if kind == "land" else font("ital", s(100) * size)
    track = s(30) * size
    up = text if kind == "land" and text.isupper() else text
    tw = spaced(up, f, track); th = s(130) * size
    lay = Image.new("RGBA", (int(tw + th), int(th * 1.6)), (0, 0, 0, 0))
    ld = ImageDraw.Draw(lay)
    cx = th / 2
    col = (44, 30, 20, 245) if kind == "land" else (196, 220, 220, 225)
    halo = (236, 224, 188, 190) if kind == "land" else (10, 30, 40, 150)
    for c in up:
        ld.text((cx, th * 0.8), c, font=f, fill=col, anchor="lm", stroke_width=max(1, int(s(5) * size)), stroke_fill=halo)
        cx += ld.textlength(c, font=f) + track
    if rot: lay = lay.rotate(rot, expand=True, resample=Image.BICUBIC)
    px, py = P(x, y)
    pim.alpha_composite(lay, (int(px - lay.width / 2), int(py - lay.height / 2))) if pim.mode == "RGBA" else pim.paste(lay, (int(px - lay.width / 2), int(py - lay.height / 2)), lay)
    boxes.append((px - lay.width / 2, py - lay.height / 2 + th * 0.4, px + lay.width / 2, py + lay.height / 2 - th * 0.4))

for (t_, x, y, sz, kind, rot) in REGION_LABELS:
    big_label(t_, x, y, sz, kind, rot)
dr = ImageDraw.Draw(pim, "RGBA")
for z, (n_, x, y, kind) in sorted(ZONES.items(), key=lambda kv: (kv[1][3] != "capital", kv[1][2])):
    foe = FOES.get(z)
    plaque(*P(x, y), n_, foe[0] if foe else None, size=1.3 if kind == "capital" else 1.0)
log("labels")

# ---------------------------------------------------------------- compass rose
def compass(cx, cy, R):
    lay = Image.new("RGBA", (int(R * 2.6), int(R * 2.6)), (0, 0, 0, 0)); ld = ImageDraw.Draw(lay)
    c = R * 1.3
    ld.ellipse((c - R * 0.78, c - R * 0.78, c + R * 0.78, c + R * 0.78), outline=(220, 200, 150, 220), width=max(1, int(s(6))))
    ld.ellipse((c - R * 0.70, c - R * 0.70, c + R * 0.70, c + R * 0.70), outline=(220, 200, 150, 160), width=max(1, int(s(3))))
    for i in range(72):
        a = i * math.pi / 36; r0 = R * (0.70 if i % 2 else 0.66)
        ld.line((c + r0 * math.cos(a), c + r0 * math.sin(a), c + R * 0.78 * math.cos(a), c + R * 0.78 * math.sin(a)), fill=(220, 200, 150, 180), width=max(1, int(s(2))))
    for i, (L, wd) in enumerate([(R * 0.62, 0.11)] * 8):
        a = i * math.pi / 4 + math.pi / 8
        L = R * 0.55; wd = R * 0.07
        tip = (c + L * math.cos(a), c + L * math.sin(a))
        l = (c + wd * math.cos(a - math.pi / 2), c + wd * math.sin(a - math.pi / 2)); r = (c + wd * math.cos(a + math.pi / 2), c + wd * math.sin(a + math.pi / 2))
        ld.polygon([l, tip, (c, c)], fill=(60, 50, 40, 230)); ld.polygon([r, tip, (c, c)], fill=(200, 180, 130, 230))
    for i in range(4):
        a = i * math.pi / 2 - math.pi / 2
        L = R * (1.15 if i == 0 else 0.95); wd = R * 0.13
        tip = (c + L * math.cos(a), c + L * math.sin(a))
        l = (c + wd * math.cos(a - math.pi / 2), c + wd * math.sin(a - math.pi / 2)); r = (c + wd * math.cos(a + math.pi / 2), c + wd * math.sin(a + math.pi / 2))
        ld.polygon([l, tip, (c, c)], fill=(236, 222, 186, 245), outline=(40, 30, 24, 255))
        ld.polygon([r, tip, (c, c)], fill=(150, 40, 34, 245) if i == 0 else (52, 44, 38, 245), outline=(40, 30, 24, 255))
    ld.ellipse((c - R * 0.06, c - R * 0.06, c + R * 0.06, c + R * 0.06), fill=(220, 200, 150, 255), outline=(40, 30, 24, 255))
    f = font("cap", R * 0.24)
    for t, a in (("N", -math.pi / 2), ("E", 0), ("S", math.pi / 2), ("W", math.pi)):
        rr = R * (1.27 if t == "N" else 1.07)
        ld.text((c + rr * math.cos(a), c + rr * math.sin(a)), t, font=f, fill=(236, 222, 186, 255), anchor="mm", stroke_width=max(1, int(s(4))), stroke_fill=(20, 30, 36, 220))
    pim.paste(lay, (int(cx - c), int(cy - c)), lay)
compass(cxp, cyp, s(300))

# ---------------------------------------------------------------- frame, title, legend
fr_w = int(s(64))
dr.rectangle((0, 0, W, fr_w), fill=(30, 24, 20)); dr.rectangle((0, H - fr_w, W, H), fill=(30, 24, 20))
dr.rectangle((0, 0, fr_w, H), fill=(30, 24, 20)); dr.rectangle((W - fr_w, 0, W, H), fill=(30, 24, 20))
for i, (o, col, wd) in enumerate([(s(14), (150, 124, 82), 4), (s(34), (110, 88, 60), 2), (fr_w - s(4), (180, 150, 100), 5)]):
    dr.rectangle((o, o, W - o, H - o), outline=col, width=max(1, int(s(wd))))
for cx_, cy_ in ((fr_w, fr_w), (W - fr_w, fr_w), (fr_w, H - fr_w), (W - fr_w, H - fr_w)):
    r = s(90)
    dr.polygon([(cx_, cy_ - r), (cx_ + r * 0.35, cy_), (cx_, cy_ + r), (cx_ - r * 0.35, cy_)], fill=(40, 32, 26), outline=(190, 160, 108), width=max(1, int(s(4))))
    dr.polygon([(cx_ - r, cy_), (cx_, cy_ + r * 0.35), (cx_ + r, cy_), (cx_, cy_ - r * 0.35)], fill=(40, 32, 26), outline=(190, 160, 108), width=max(1, int(s(4))))
    dr.ellipse((cx_ - r * 0.22, cy_ - r * 0.22, cx_ + r * 0.22, cy_ + r * 0.22), fill=(150, 40, 34), outline=(190, 160, 108), width=max(1, int(s(3))))

def panel(x0, y0, x1, y1):
    dr.rectangle((x0 + s(10), y0 + s(12), x1 + s(10), y1 + s(12)), fill=(0, 0, 0, 110))
    dr.rectangle((x0, y0, x1, y1), fill=(30, 24, 20, 255))
    dr.rectangle((x0 + s(10), y0 + s(10), x1 - s(10), y1 - s(10)), fill=(236, 222, 186, 255), outline=(150, 124, 82), width=max(1, int(s(4))))
    dr.rectangle((x0 + s(22), y0 + s(22), x1 - s(22), y1 - s(22)), outline=(120, 92, 60), width=max(1, int(s(2))))

# title
ft = font("cap", s(150)); title = "The Dominion"
tw = dr.textlength(title, font=ft)
tx0, ty0 = W / 2 - tw / 2 - s(110), fr_w - s(8)
panel(tx0, ty0, W / 2 + tw / 2 + s(110), ty0 + s(250))
dr.text((W / 2, ty0 + s(112)), title, font=ft, fill=(44, 30, 20), anchor="mm")
dr.text((W / 2, ty0 + s(205)), "a map of the lands of DominionMUD", font=font("ital", s(46)), fill=(120, 34, 26), anchor="mm")

# legend (drawn onto a copy-area with icons rendered via numpy)
lx0, ly0, lx1, ly1 = W - fr_w - s(1300), H - fr_w - s(1120), W - fr_w - s(70), H - fr_w - s(70)
panel(lx0, ly0, lx1, ly1)
dr.text(((lx0 + lx1) / 2, ly0 + s(90)), "Legend", font=font("cap", s(80)), fill=(44, 30, 20), anchor="mm")
items = [("capital", "Capital"), ("city", "City"), ("town", "Town / village"), ("keep", "Castle / stronghold"),
         ("tower", "Tower"), ("temple", "Temple / monastery"), ("cave", "Cave / dungeon"), ("lair", "Monster lair"),
         ("camp", "Wild region / camp")]
arr = np.array(pim.convert("RGB")); img = arr
for i, (k, t_) in enumerate(items):
    col, row = i % 2, i // 2
    ix = lx0 + s(130) + col * s(560); iy = ly0 + s(250) + row * s(130)
    icon(k, ix, iy + s(22), 0.8)
pim = Image.fromarray(img); dr = ImageDraw.Draw(pim, "RGBA")
fl = font("lab", s(37))
for i, (k, t_) in enumerate(items):
    col, row = i % 2, i // 2
    ix = lx0 + s(130) + col * s(560); iy = ly0 + s(250) + row * s(130)
    dr.text((ix + s(105), iy - s(4)), t_, font=fl, fill=(52, 34, 22), anchor="lm")
yb = ly0 + s(250) + 5 * s(130) - s(20)
dr.text((lx0 + s(60), yb), "☠", font=font("sym", s(40)), fill=(140, 30, 24), anchor="lm")
dr.text((lx0 + s(110), yb), "Notable foe (strongest mob in the zone)", font=font("ital", s(32)), fill=(140, 30, 24), anchor="lm")
for j, (c1, t_) in enumerate((((92, 62, 36), "Road"), ((72, 132, 140), "River"))):
    x = lx0 + s(60) + j * s(420); y = yb + s(80)
    if j == 0:
        for d in range(4): dr.line((x + d * s(42), y, x + d * s(42) + s(26), y), fill=c1, width=max(1, int(s(6))))
    else:
        dr.line((x, y, x + s(160), y), fill=(40, 54, 50), width=max(1, int(s(14)))); dr.line((x, y, x + s(160), y), fill=c1, width=max(1, int(s(9))))
    dr.text((x + s(190), y), t_, font=fl, fill=(52, 34, 22), anchor="lm")
dr.text(((lx0 + lx1) / 2, ly1 - s(48)), "Drawn from the zone, room and mob files. Not to scale.", font=font("ital", s(26)), fill=(110, 84, 56), anchor="mm")

# paper grain over everything
g = (fbm(H // 2, W // 2, s(4) / 2 + 1, 2, 99) * 6).astype(np.float32)
g = cv2.resize(g, (W, H))
arr = np.array(pim.convert("RGB")); del pim
for y0 in range(0, H, 512):
    sl = slice(y0, min(H, y0 + 512))
    arr[sl] = np.clip(arr[sl].astype(np.float32) + g[sl][..., None], 0, 255).astype(np.uint8)
Image.fromarray(arr).save(OUT, optimize=False, compress_level=6)
log(f"saved {OUT}")
