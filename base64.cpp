/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * base64.h
 *
 * Base64 utility functions
 */

#include "base64.h"

#ifndef THOR_STRING_UTIL_H
#include "string_util.h"
#endif

namespace thor
{

static const char base64_character_map[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char base64_character_map_url[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static const byte base64_decode_map[] =
{
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 000 - 00F
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 010 - 01F
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  62,   0,  62,   0,  63, // 020 - 02F
     52,  53,  54,  55,  56,  57,  58,  59,  60,  61,   0,   0,   0,   0,   0,   0, // 030 - 03F
      0,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14, // 040 - 04F
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,   0,   0,   0,   0,  63, // 050 - 05F
      0,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40, // 060 - 06F
     41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,   0,   0,   0,   0,   0, // 070 - 07F
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 080 - 08F
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 090 - 09F
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0A0 - 0AF
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0B0 - 0BF
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0C0 - 0CF
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0D0 - 0DF
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0E0 - 0EF
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 0F0 - 0FF
};

inline byte decode_byte(char c)
{
    byte b = base64_decode_map[(byte)c];
    THOR_ASSERT(b < 64 && (b > 0 || c == 'A')); // Assert only valid characters
    return b;
}

size_type base64_encode(bool url, const byte* data, size_type len, char* output, size_type maxoutput) THOR_NOTHROW
{
    if (len == 0) return 0;

    const char* map = url ? base64_character_map_url : base64_character_map;
    const size_type required = base64_encode_size(data, len);
    if (maxoutput < required)
    {
        THOR_ASSERT(0);
        return size_type(-1);
    }

    const size_type reserve = len % 3;
    const byte* dataend = data + len - reserve;
    const char* outend = output + maxoutput; THOR_UNUSED(outend);
    char* start = output;

    while (data < dataend)
    {
        THOR_ASSERT((output + 4) < outend);
        *output++ = map[data[0] >> 2];
        *output++ = map[((data[0] & 0x3) << 4) | (data[1] >> 4)];
        *output++ = map[((data[1] & 0xf) << 2) | (data[2] >> 6)];
        *output++ = map[data[2] & 0x3f];
        data += 3;
    }

    if (reserve == 2)
    {
        THOR_ASSERT((output + 4) < outend);
        *output++ = map[data[0] >> 2];
        *output++ = map[((data[0] & 0x3) << 4) | (data[1] >> 4)];
        *output++ = map[((data[1] & 0xf) << 2)];
        *output++ = '=';
    }
    else if (reserve == 1)
    {
        THOR_ASSERT((output + 4) < outend);
        *output++ = map[data[0] >> 2];
        *output++ = map[((data[0] & 0x3) << 4)];
        *output++ = '=';
        *output++ = '=';
    }
    THOR_ASSERT(output < outend);
    *output = '\0';

    return output - start;
}

size_type base64_decode(byte* output, size_type maxlen, const char* p, size_type len) THOR_NOTHROW
{
    if (len == size_type(-1))
    {
        len = string_length(p);
    }

    if (len == 0) return 0;

    THOR_ASSERT((len % 4) == 0); // Should be a multiple of 4

    const size_type required = base64_decode_size(p, len);
    if (maxlen < required)
    {
        THOR_ASSERT(0);
        return size_type(-1);
    }

    const size_type padding = (p[len-1] == '=' ? 1 : 0) + (p[len-2] == '=' ? 1 : 0);
    byte* start = output;
    byte* outend = output + maxlen;
    const char* pend = p + len - (padding ? 4 : 0);

    while (p < pend)
    {
        THOR_ASSERT((output + 3) <= outend);
        *output++ = (byte)((decode_byte(p[0]) << 2) | (decode_byte(p[1]) >> 4));
        *output++ = (byte)((decode_byte(p[1]) << 4) | (decode_byte(p[2]) >> 2));
        *output++ = (byte)((decode_byte(p[2]) << 6) | (decode_byte(p[3]) >> 0));
        p += 4;
    }

    if (padding == 1)
    {
        THOR_ASSERT((output + 2) <= outend);
        *output++ = (byte)((decode_byte(p[0]) << 2) | (decode_byte(p[1]) >> 4));
        *output++ = (byte)((decode_byte(p[1]) << 4) | (decode_byte(p[2]) >> 2));
        p += 3;
    }
    else if (padding == 2)
    {
        THOR_ASSERT((output + 1) <= outend);
        *output++ = (byte)((decode_byte(p[0]) << 2) | (decode_byte(p[1]) >> 4));
        p += 2;
    }
    return output - start;
}




static const size_type remainder_sizes[] = { 0, 0, 1, 2 };

size_type base64_decode_size(const char* p, size_type len) THOR_NOTHROW
{
    if (len == size_type(-1))
    {
        len = string_length(p);
    }

    if (len == 0) return 0;

    if ((len % 4) != 0)
    {
        // Invalid; should always be a multiple of 4
        return size_type(-1);
    }

    // Remove filler
    if (p[len-1] == '=') --len;
    if (p[len-1] == '=') --len;

    size_type out = (len / 4) * 3;
    out += remainder_sizes[len % 4];
    return out;
}

}