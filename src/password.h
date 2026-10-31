/* ***********************************************************************\
*  _____ _            ____                  _       _                     *
* |_   _| |__   ___  |  _ \  ___  _ __ ___ (_)_ __ (_) ___  _ __          *
*   | | | '_ \ / _ \ | | | |/ _ \| '_ ` _ \| | '_ \| |/ _ \| '_ \         *
*   | | | | | |  __/ | |_| | (_) | | | | | | | | | | | (_) | | | |        *
*   |_| |_| |_|\___| |____/ \___/|_| |_| |_|_|_| |_|_|\___/|_| |_|        *
*                                                                         *
*  File:  PASSWORD.H                                                      *
*  Usage: Password hashing (PBKDF2-HMAC-SHA256) for player passwords      *
*  Programmer(s): Sean Mountcastle (Glasgian)                             *
\*********************************************************************** */

#ifndef __PASSWORD_H__
#define __PASSWORD_H__

#include <stddef.h>

/*
 * Stored hashes look like
 *
 *     $pbkdf2-sha256$<rounds>$<salt>$<hash>
 *
 * with the salt and hash in "adapted base64" (standard base64 with '.'
 * instead of '+', no padding).  This is the same format Python's passlib
 * uses for pbkdf2_sha256, so hashes can be checked with other tools.
 *
 * PASSWORD_ROUNDS is the PBKDF2 iteration count for new hashes.  Every
 * check runs that many rounds inside the single-threaded game loop, so it
 * trades login cost against brute-force resistance.  The count is stored
 * in each hash, so raising it later upgrades players as they log in.
 */
#define PASSWORD_ROUNDS         100000
#define PASSWORD_SALT_BYTES     16
#define PASSWORD_HASH_LENGTH    127   /* max stored length, without NUL     */

/*
 * Old player files stored the password itself, truncated to 12 characters,
 * and compared only the first 12 characters typed.
 */
#define LEGACY_PASSWORD_LENGTH  12

/* results of password_verify() */
#define PASSWORD_BAD            0   /* wrong password (or unusable hash)   */
#define PASSWORD_OK             1   /* correct, hash is current            */
#define PASSWORD_OK_REHASH      2   /* correct, but stored in an old form: */
                                    /* hash it again and save              */

/*
 * Hash 'plain' into 'out' (at least PASSWORD_HASH_LENGTH + 1 bytes).
 * Returns 0 on success, -1 if 'out' is too small.
 */
int  password_hash( const char * plain, char * out, size_t outlen );

/* Check 'plain' against a stored hash (or a legacy plaintext password). */
int  password_verify( const char * plain, const char * stored );

/* Raw PBKDF2-HMAC-SHA256, exposed for testing. */
void pbkdf2_sha256( const unsigned char * pass, size_t passlen,
                    const unsigned char * salt, size_t saltlen,
                    unsigned long rounds,
                    unsigned char * out, size_t outlen );

#endif /* __PASSWORD_H__ */
