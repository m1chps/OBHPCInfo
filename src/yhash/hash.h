#include "common.h"

//
extern void md5hash(const byte *restrict msg, const u_int64 len, byte *restrict hash);
extern void sha1hash(const byte *restrict msg, const u_int64 len, byte *restrict hash);
extern void sha224hash(const byte *restrict msg, const u_int64 len, byte *restrict hash);
extern void sha256hash(const byte *restrict msg, const u_int64 len, byte *restrict hash);
