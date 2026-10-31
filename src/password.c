/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  PASSWORD.C                                                      *
*  Usage: Password hashing (PBKDF2-HMAC-SHA256) for player passwords      *
*  Programmer(s): Sean Mountcastle (Glasgian)                             *
\*********************************************************************** */

/*
 * Self-contained on purpose: it uses only the standard C library (plus
 * getpid()), so it builds the same way on macOS and Linux and can be linked
 * into the offline utilities.  SHA-256 follows FIPS 180-4, HMAC follows
 * RFC 2104 and PBKDF2 follows RFC 8018; the implementation is checked
 * against the published test vectors.
 */

#define __PASSWORD_C__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "password.h"

#define PBKDF2_PREFIX     "$pbkdf2-sha256$"
#define PBKDF2_HASH_BYTES 32          /* SHA-256 output size */
#define PBKDF2_MAX_ROUNDS 10000000UL  /* refuse absurd stored counts */
#define PBKDF2_MAX_SALT   64

/* ---------------------------------------------------------------------- */
/* SHA-256                                                                 */
/* ---------------------------------------------------------------------- */

typedef unsigned int u32;    /* at least 32 bits on every platform we build for */

struct sha256_ctx {
  u32 h[8];
  unsigned char buf[64];
  unsigned long long len;    /* bytes hashed so far */
  size_t used;               /* bytes waiting in buf */
};

static const u32 K[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
  0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
  0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
  0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
  0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
  0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
  0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
  0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
  0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define ROR(x, n)  ((((x) & 0xffffffffU) >> (n)) | ((x) << (32 - (n))))

static void sha256_block( struct sha256_ctx * c, const unsigned char * p )
{
  u32 w[64], a, b, d, e, f, g, h, cc, t1, t2;
  int i;

  for ( i = 0; i < 16; i++ )
    w[ i ] = ((u32) p[ 4*i ] << 24) | ((u32) p[ 4*i+1 ] << 16) |
             ((u32) p[ 4*i+2 ] << 8) | (u32) p[ 4*i+3 ];
  for ( i = 16; i < 64; i++ ) {
    u32 s0 = ROR(w[i-15], 7) ^ ROR(w[i-15], 18) ^ ((w[i-15] & 0xffffffffU) >> 3);
    u32 s1 = ROR(w[i-2], 17) ^ ROR(w[i-2], 19) ^ ((w[i-2] & 0xffffffffU) >> 10);
    w[ i ] = (w[i-16] + s0 + w[i-7] + s1) & 0xffffffffU;
  }

  a = c->h[0]; b = c->h[1]; cc = c->h[2]; d = c->h[3];
  e = c->h[4]; f = c->h[5]; g = c->h[6]; h = c->h[7];

  for ( i = 0; i < 64; i++ ) {
    t1 = h + (ROR(e, 6) ^ ROR(e, 11) ^ ROR(e, 25)) + ((e & f) ^ (~e & g)) + K[ i ] + w[ i ];
    t2 = (ROR(a, 2) ^ ROR(a, 13) ^ ROR(a, 22)) + ((a & b) ^ (a & cc) ^ (b & cc));
    h = g; g = f; f = e;
    e = (d + t1) & 0xffffffffU;
    d = cc; cc = b; b = a;
    a = (t1 + t2) & 0xffffffffU;
  }

  c->h[0] = (c->h[0] + a) & 0xffffffffU;  c->h[1] = (c->h[1] + b) & 0xffffffffU;
  c->h[2] = (c->h[2] + cc) & 0xffffffffU; c->h[3] = (c->h[3] + d) & 0xffffffffU;
  c->h[4] = (c->h[4] + e) & 0xffffffffU;  c->h[5] = (c->h[5] + f) & 0xffffffffU;
  c->h[6] = (c->h[6] + g) & 0xffffffffU;  c->h[7] = (c->h[7] + h) & 0xffffffffU;
}

static void sha256_init( struct sha256_ctx * c )
{
  static const u32 iv[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
  };
  memcpy( c->h, iv, sizeof( iv ) );
  c->len = 0;
  c->used = 0;
}

static void sha256_update( struct sha256_ctx * c, const unsigned char * p, size_t n )
{
  c->len += n;
  while ( n > 0 ) {
    size_t take = 64 - c->used;
    if ( take > n )
      take = n;
    memcpy( c->buf + c->used, p, take );
    c->used += take; p += take; n -= take;
    if ( c->used == 64 ) {
      sha256_block( c, c->buf );
      c->used = 0;
    }
  }
}

static void sha256_final( struct sha256_ctx * c, unsigned char out[32] )
{
  unsigned long long bits = c->len * 8;
  unsigned char pad = 0x80, zero = 0, lenbuf[8];
  int i;

  sha256_update( c, &pad, 1 );
  while ( c->used != 56 )
    sha256_update( c, &zero, 1 );
  for ( i = 0; i < 8; i++ )
    lenbuf[ i ] = (unsigned char) (bits >> (56 - 8 * i));
  sha256_update( c, lenbuf, 8 );
  for ( i = 0; i < 8; i++ ) {
    out[ 4*i ]   = (unsigned char) (c->h[ i ] >> 24);
    out[ 4*i+1 ] = (unsigned char) (c->h[ i ] >> 16);
    out[ 4*i+2 ] = (unsigned char) (c->h[ i ] >> 8);
    out[ 4*i+3 ] = (unsigned char) c->h[ i ];
  }
}

/* ---------------------------------------------------------------------- */
/* HMAC-SHA256 and PBKDF2                                                  */
/* ---------------------------------------------------------------------- */

/* Pre-keyed inner/outer states, so each PBKDF2 round costs two blocks. */
struct hmac_key {
  struct sha256_ctx inner, outer;
};

static void hmac_setkey( struct hmac_key * hk, const unsigned char * key, size_t keylen )
{
  unsigned char k[64], ipad[64], opad[64];
  int i;

  memset( k, 0, sizeof( k ) );
  if ( keylen > 64 ) {
    struct sha256_ctx c;
    sha256_init( &c );
    sha256_update( &c, key, keylen );
    sha256_final( &c, k );
  } else
    memcpy( k, key, keylen );

  for ( i = 0; i < 64; i++ ) {
    ipad[ i ] = k[ i ] ^ 0x36;
    opad[ i ] = k[ i ] ^ 0x5c;
  }
  sha256_init( &hk->inner );
  sha256_update( &hk->inner, ipad, 64 );
  sha256_init( &hk->outer );
  sha256_update( &hk->outer, opad, 64 );
  memset( k, 0, sizeof( k ) );
}

static void hmac( const struct hmac_key * hk, const unsigned char * msg, size_t len,
                  unsigned char out[32] )
{
  struct sha256_ctx c = hk->inner;
  unsigned char ih[32];

  sha256_update( &c, msg, len );
  sha256_final( &c, ih );
  c = hk->outer;
  sha256_update( &c, ih, 32 );
  sha256_final( &c, out );
}

void pbkdf2_sha256( const unsigned char * pass, size_t passlen,
                    const unsigned char * salt, size_t saltlen,
                    unsigned long rounds,
                    unsigned char * out, size_t outlen )
{
  struct hmac_key hk;
  unsigned char u[32], t[32], block[PBKDF2_MAX_SALT + 4];
  unsigned long blocknum, r;
  size_t take;
  int i;

  hmac_setkey( &hk, pass, passlen );
  if ( saltlen > PBKDF2_MAX_SALT )
    saltlen = PBKDF2_MAX_SALT;

  for ( blocknum = 1; outlen > 0; blocknum++ ) {
    memcpy( block, salt, saltlen );
    block[ saltlen ]     = (unsigned char) (blocknum >> 24);
    block[ saltlen + 1 ] = (unsigned char) (blocknum >> 16);
    block[ saltlen + 2 ] = (unsigned char) (blocknum >> 8);
    block[ saltlen + 3 ] = (unsigned char) blocknum;

    hmac( &hk, block, saltlen + 4, u );
    memcpy( t, u, 32 );
    for ( r = 1; r < rounds; r++ ) {
      hmac( &hk, u, 32, u );
      for ( i = 0; i < 32; i++ )
        t[ i ] ^= u[ i ];
    }
    take = outlen < 32 ? outlen : 32;
    memcpy( out, t, take );
    out += take;
    outlen -= take;
  }
  memset( &hk, 0, sizeof( hk ) );
  memset( u, 0, sizeof( u ) );
  memset( t, 0, sizeof( t ) );
}

/* ---------------------------------------------------------------------- */
/* Encoding, salts and the public interface                                */
/* ---------------------------------------------------------------------- */

/* "adapted base64": standard alphabet with '.' for '+', no padding */
static const char ab64[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789./";

static void ab64_encode( const unsigned char * in, size_t len, char * out )
{
  size_t i;
  unsigned long v;

  for ( i = 0; i + 2 < len; i += 3 ) {
    v = ((unsigned long) in[i] << 16) | ((unsigned long) in[i+1] << 8) | in[i+2];
    *out++ = ab64[ (v >> 18) & 63 ]; *out++ = ab64[ (v >> 12) & 63 ];
    *out++ = ab64[ (v >> 6) & 63 ];  *out++ = ab64[ v & 63 ];
  }
  if ( len - i == 1 ) {
    v = (unsigned long) in[i] << 16;
    *out++ = ab64[ (v >> 18) & 63 ]; *out++ = ab64[ (v >> 12) & 63 ];
  } else if ( len - i == 2 ) {
    v = ((unsigned long) in[i] << 16) | ((unsigned long) in[i+1] << 8);
    *out++ = ab64[ (v >> 18) & 63 ]; *out++ = ab64[ (v >> 12) & 63 ];
    *out++ = ab64[ (v >> 6) & 63 ];
  }
  *out = '\0';
}

/* Decode 'len' characters; returns the number of bytes, or -1 if invalid. */
static int ab64_decode( const char * in, size_t len, unsigned char * out, size_t outmax )
{
  unsigned long v = 0;
  size_t i, n = 0;
  int bits = 0;
  const char * p;

  if ( len % 4 == 1 )
    return -1;
  for ( i = 0; i < len; i++ ) {
    if ( in[ i ] == '\0' || !(p = strchr( ab64, in[ i ] )) )
      return -1;
    v = (v << 6) | (unsigned long) (p - ab64);
    bits += 6;
    if ( bits >= 8 ) {
      bits -= 8;
      if ( n >= outmax )
        return -1;
      out[ n++ ] = (unsigned char) (v >> bits);
    }
  }
  return (int) n;
}

/*
 * Salts only need to be unique, not secret.  Use the system's random
 * device; if that is unavailable, hash together several varying values.
 */
static void make_salt( unsigned char * salt, size_t len )
{
  FILE * fl;
  size_t got = 0;

  if ( (fl = fopen( "/dev/urandom", "rb" )) != NULL ) {
    got = fread( salt, 1, len, fl );
    fclose( fl );
  }
  if ( got != len ) {
    static unsigned long counter = 0;
    struct sha256_ctx c;
    unsigned char digest[32];
    unsigned long mix[5];

    mix[0] = (unsigned long) time( NULL );
    mix[1] = (unsigned long) clock();
    mix[2] = (unsigned long) getpid();
    mix[3] = ++counter;
    mix[4] = (unsigned long) (size_t) &c;
    sha256_init( &c );
    sha256_update( &c, (const unsigned char *) mix, sizeof( mix ) );
    sha256_final( &c, digest );
    memcpy( salt, digest, len < 32 ? len : 32 );
  }
}

int password_hash( const char * plain, char * out, size_t outlen )
{
  unsigned char salt[ PASSWORD_SALT_BYTES ], dk[ PBKDF2_HASH_BYTES ];
  char salt64[ 2 * PASSWORD_SALT_BYTES ], dk64[ 2 * PBKDF2_HASH_BYTES ];
  int n;

  make_salt( salt, sizeof( salt ) );
  pbkdf2_sha256( (const unsigned char *) plain, strlen( plain ),
                 salt, sizeof( salt ), PASSWORD_ROUNDS, dk, sizeof( dk ) );
  ab64_encode( salt, sizeof( salt ), salt64 );
  ab64_encode( dk, sizeof( dk ), dk64 );
  memset( dk, 0, sizeof( dk ) );

  n = snprintf( out, outlen, "%s%lu$%s$%s", PBKDF2_PREFIX,
                (unsigned long) PASSWORD_ROUNDS, salt64, dk64 );
  if ( n < 0 || (size_t) n >= outlen || n > PASSWORD_HASH_LENGTH ) {
    if ( outlen > 0 )
      *out = '\0';
    return -1;
  }
  return 0;
}

int password_verify( const char * plain, const char * stored )
{
  const char * p, * salt_end, * hash;
  unsigned char salt[ PBKDF2_MAX_SALT ], want[ PBKDF2_HASH_BYTES + 3 ];
  unsigned char got[ PBKDF2_HASH_BYTES ], diff = 0;
  unsigned long rounds;
  char * end;
  int saltlen, i;

  if ( !plain || !stored || !*stored )
    return PASSWORD_BAD;

  if ( strncmp( stored, PBKDF2_PREFIX, strlen( PBKDF2_PREFIX ) ) ) {
    /* not a hash: an old plaintext password, compared the way the old
       code did (first LEGACY_PASSWORD_LENGTH characters) */
    if ( *stored == '$' )
      return PASSWORD_BAD;           /* some other hash format */
    return strncmp( plain, stored, LEGACY_PASSWORD_LENGTH ) ?
           PASSWORD_BAD : PASSWORD_OK_REHASH;
  }

  p = stored + strlen( PBKDF2_PREFIX );
  rounds = strtoul( p, &end, 10 );
  if ( end == p || *end != '$' || rounds < 1 || rounds > PBKDF2_MAX_ROUNDS )
    return PASSWORD_BAD;
  p = end + 1;
  if ( !(salt_end = strchr( p, '$' )) )
    return PASSWORD_BAD;
  hash = salt_end + 1;
  if ( (saltlen = ab64_decode( p, (size_t) (salt_end - p), salt, sizeof( salt ) )) < 1 ||
       ab64_decode( hash, strlen( hash ), want, sizeof( want ) ) != PBKDF2_HASH_BYTES )
    return PASSWORD_BAD;

  pbkdf2_sha256( (const unsigned char *) plain, strlen( plain ),
                 salt, (size_t) saltlen, rounds, got, sizeof( got ) );
  for ( i = 0; i < PBKDF2_HASH_BYTES; i++ )     /* constant-time compare */
    diff |= got[ i ] ^ want[ i ];
  memset( got, 0, sizeof( got ) );

  if ( diff )
    return PASSWORD_BAD;
  return rounds < PASSWORD_ROUNDS ? PASSWORD_OK_REHASH : PASSWORD_OK;
}
