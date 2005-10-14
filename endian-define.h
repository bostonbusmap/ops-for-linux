#ifndef ENDIAN_DEFINE_H
#define ENDIAN_DEFINE_H

////////////////////////////////////////////////////////
// big-endian and little-endian

#define bswap16(x) ((unsigned short)( (((x)&0xffu)<<8u) | (((x)>>8u)&0xffu) ))

#define bswap32(x) (\
  ((x)&0xff000000u) >> 24 \
  |                       \
  ((x)&0x00ff0000u) >>  8 \
  |                       \
  ((x)&0x0000ff00u) <<  8 \
  |                       \
  ((x)&0x000000ffu) << 24 \
)

// these are compiler-defined, so we put this section BEFORE the includes

#if defined(_WIN32) || defined(_WIN64)
#define __BYTE_ORDER 1
#define __BIG_ENDIAN 0
#define __LITTLE_ENDIAN 1
#elif defined(__APPLE__) && defined(__MACH__)
#include <machine/endian.h>
#else
#include <endian.h>
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
# define cpu_to_le32(x) bswap32(x)
# define le32_to_cpu(x) bswap32(x)
# define cpu_to_le16(x) bswap16(x)
# define le16_to_cpu(x) bswap16(x)
# define cpu_to_be32(x) (x)
# define be32_to_cpu(x) (x)
# define cpu_to_be16(x) (x)
# define be16_to_cpu(x) (x)
#else
# define cpu_to_le32(x) (x)
# define le32_to_cpu(x) (x)
# define cpu_to_le16(x) (x)
# define le16_to_cpu(x) (x)
# define cpu_to_be32(x) bswap32(x)
# define be32_to_cpu(x) bswap32(x)
# define cpu_to_be16(x) bswap16(x)
# define be16_to_cpu(x) bswap16(x)
#endif

/////////////////////////////////////////////////////////
#endif
