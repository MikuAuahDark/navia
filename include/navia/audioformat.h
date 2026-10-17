#ifndef _NAVIA_AUDIOFORMAT_H_
#define _NAVIA_AUDIOFORMAT_H_

#include <stdint.h>

/**
 * @brief Audio format flags.
 *
 * (Copied straight from SDL)
 * 
 * These are what the 16 bits in SDL_AudioFormat currently mean...
 * (Unspecified bits are always zero).
 *
 * ```
 * ++-----------------------sample is signed if set
 * ||
 * ||       ++-----------sample is bigendian if set
 * ||       ||
 * ||       ||          ++---sample is float if set
 * ||       ||          ||
 * ||       ||          || +=--sample bit size--++
 * ||       ||          || ||                   ||
 * 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
 * ```
 */
typedef uint16_t navia_audioformat;

/**
 * @brief Retrieve the size, in bits, from a navia_audioformat.
 * @param x a navia_audioformat value
 * @returns data size in bits
 */
#define NAVIA_AUDIOFORMAT_BITSIZE(x) ((x) & (0xFFu))

/**
 * @brief Retrieve the size, in bytes, from a navia_audioformat.
 * @param x a navia_audioformat value
 * @returns data size in bytes
 */
#define NAVIA_AUDIOFORMAT_BYTESIZE(x) (NAVIA_AUDIOFORMAT_BITSIZE(x) / 8)

/**
 * @brief Determine if a navia_audioformat represents floating point data.
 * @param x a navia_audioformat value
 * @returns non-zero if format is floating point, zero otherwise.
 */
#define NAVIA_AUDIOFORMAT_ISFLOAT(x) ((x) & (1u<<8))

/**
 * @brief Determine if a navia_audioformat represents bigendian data.
 * @param x a navia_audioformat value
 * @returns non-zero if format is bigendian, zero otherwise.
 */
#define NAVIA_AUDIOFORMAT_ISBIGENDIAN(x) ((x) & (1u<<12))

/**
 * @brief Determine if a navia_audioformat represents littleendian data.
 * @param x a navia_audioformat value
 * @returns non-zero if format is littleendian, zero otherwise.
 */
#define NAVIA_AUDIOFORMAT_ISLITTLEENDIAN(x) (!NAVIA_AUDIOFORMAT_ISBIGENDIAN(x))

/**
 * @brief Determine if a navia_audioformat represents signed data.
 * @param x a navia_audioformat value
 * @returns non-zero if format is signed, zero otherwise.
 */
#define NAVIA_AUDIOFORMAT_ISSIGNED(x) ((x) & (1u<<15))

/**
 * @brief Determine if a navia_audioformat represents integer data.
 * @param x a navia_audioformat value
 * @returns non-zero if format is integer, zero otherwise.
 */
#define NAVIA_AUDIOFORMAT_ISINT(x) (!NAVIA_AUDIOFORMAT_ISFLOAT(x))

/**
 * @brief Determine if a navia_audioformat represents unsigned data.
 * @param x a navia_audioformat value
 * @returns non-zero if format is unsigned, zero otherwise.
 */
#define NAVIA_AUDIOFORMAT_ISUNSIGNED(x) (!NAVIA_AUDIOFORMAT_ISSIGNED(x))

#endif /* _NAVIA_AUDIOFORMAT_H_ */
