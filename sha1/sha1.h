// $Id: sha1.h 14 2000-02-09 19:32:15Z jkf $

/*
 * SHA-1 in C
 * By Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 */

#ifndef _SHA1_H
#define _SHA1_H

typedef struct {
    u_int32_t state[5];
    u_int32_t count[2];  
    u_char buffer[64];
} SHA1_CTX;
  
void SHA1Transform (u_int32_t state[5], const u_char buffer[64]);
void SHA1Init (SHA1_CTX *context);
void SHA1Update (SHA1_CTX *context, const u_char *data, u_int len);
void SHA1Final (u_char digest[20], SHA1_CTX *context);
char *SHA1End (SHA1_CTX *, char *);
char *SHA1File (char *, char *);
char *SHA1Data (const u_char *, size_t, char *);

#define SHA1_DIGESTSIZE       20
#define SHA1_BLOCKSIZE        64
#define HTONDIGEST(x) { \
      x[0] = htonl(x[0]); \
      x[1] = htonl(x[1]); \
      x[2] = htonl(x[2]); \
      x[3] = htonl(x[3]); \
      x[4] = htonl(x[4]); }

#define NTOHDIGEST(x) { \
      x[0] = ntohl(x[0]); \
      x[1] = ntohl(x[1]); \
      x[2] = ntohl(x[2]); \
      x[3] = ntohl(x[3]); \
      x[4] = ntohl(x[4]); }

#endif /* _SHA1_H */
