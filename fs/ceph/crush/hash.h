#ifndef _CRUSH_HASH_H
#define _CRUSH_HASH_H

/*
 * Robert Jenkins' function for mixing 32-bit values
 * http://burtleburtle.net/bob/hash/evahash.html
 * a, b = random bits, c = input and output
 */
#define crush_hashmix(a, b, c) do {			\
		a = a-b;  a = a-c;  a = a^(c>>13);	\
		b = b-c;  b = b-a;  b = b^(a<<8);	\
		c = c-a;  c = c-b;  c = c^(b>>13);	\
		a = a-b;  a = a-c;  a = a^(c>>12);	\
		b = b-c;  b = b-a;  b = b^(a<<16);	\
		c = c-a;  c = c-b;  c = c^(b>>5);	\
		a = a-b;  a = a-c;  a = a^(c>>3);	\
		b = b-c;  b = b-a;  b = b^(a<<10);	\
		c = c-a;  c = c-b;  c = c^(b>>15);	\
	} while (0)

#define crush_hash_seed 1315423911

static inline __u32 crush_hash32(__u32 a)
{
	__u32 hash = crush_hash_seed ^ a;
	__u32 b = a;
	__u32 x = 231232;
	__u32 y = 1232;
	crush_hashmix(b, x, hash);
	crush_hashmix(y, a, hash);
	return hash;
}

static inline __u32 crush_hash32_2(__u32 a, __u32 b)
{
	__u32 hash = crush_hash_seed ^ a ^ b;
	__u32 x = 231232;
	__u32 y = 1232;
	crush_hashmix(a, b, hash);
	crush_hashmix(x, a, hash);
	crush_hashmix(b, y, hash);
	return hash;
}

static inline __u32 crush_hash32_3(__u32 a, __u32 b, __u32 c)
{
	__u32 hash = crush_hash_seed ^ a ^ b ^ c;
	__u32 x = 231232;
	__u32 y = 1232;
	crush_hashmix(a, b, hash);
	crush_hashmix(c, x, hash);
	crush_hashmix(y, a, hash);
	crush_hashmix(b, x, hash);
	crush_hashmix(y, c, hash);
	return hash;
}

static inline __u32 crush_hash32_4(__u32 a, __u32 b, __u32 c,
				   __u32 d)
{
	__u32 hash = crush_hash_seed ^ a ^ b ^ c ^ d;
	__u32 x = 231232;
	__u32 y = 1232;
	crush_hashmix(a, b, hash);
	crush_hashmix(c, d, hash);
	crush_hashmix(a, x, hash);
	crush_hashmix(y, b, hash);
	crush_hashmix(c, x, hash);
	crush_hashmix(y, d, hash);
	return hash;
}

static inline __u32 crush_hash32_5(__u32 a, __u32 b, __u32 c,
				   __u32 d, __u32 e)
{
	__u32 hash = crush_hash_seed ^ a ^ b ^ c ^ d ^ e;
	__u32 x = 231232;
	__u32 y = 1232;
	crush_hashmix(a, b, hash);
	crush_hashmix(c, d, hash);
	crush_hashmix(e, x, hash);
	crush_hashmix(y, a, hash);
	crush_hashmix(b, x, hash);
	crush_hashmix(y, c, hash);
	crush_hashmix(d, x, hash);
	crush_hashmix(y, e, hash);
	return hash;
}

#endif
