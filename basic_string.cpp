/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * basic_string.cpp
 *
 * This file defines an STL-compatible string container
 */

#include "basic_string.h"

namespace thor
{

namespace
{
    const size_type utf8len[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // invalid
        2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 5, 6
    };
}

bool utf8_to_wide(const char* src, wstring& out)
{
    out.clear();
    if (src == 0) return false;

    while (*src)
    {
        wstring::value_type c;
        const byte b = *(byte*)src;
        switch (utf8len[b >> 2])
        {
        default:
        case 0:
        case 5:
        case 6:
            THOR_DEBUG_ASSERT(0);
            return false;

        case 1:
            out.push_back(wstring::value_type(*src++));
            break;

        case 2:
            c = (wstring::value_type)((*src++ & 0x1f) << 6);
            c |= (*src++ & 0x3f);
            out.push_back(c);
            break;

        case 3:
            c = (wstring::value_type)((b & 0xf) << 12); ++src;
            c |= ((*src++ & 0x3f) << 6);
            c |=  (*src++ & 0x3f);
            out.push_back(c);
            break;

        case 4:
            {
                size_type ch = (size_type)((*src++ & 0x7) << 18);
                ch |= ((*src++ & 0x3f) << 12);
                ch |= ((*src++ & 0x3f) << 6);
                ch |=  (*src++ & 0x3f);

                if (THOR_SUPPRESS_WARNING(sizeof(wchar_t) >= 4))
                {
                    out.push_back(wstring::value_type(ch));
                }
                else
                {
                    // UTF-16 encode
                    ch -= 0x10000;
                    out.push_back(wstring::value_type(0xd800 | (ch >> 10)));
                    out.push_back(wstring::value_type(0xdc00 | (ch & 0x3ff)));
                }
            }
            break;
        }
    }
    return true;
}

bool wide_to_utf8(const wchar_t* src, string& out)
{
    out.clear();

    while (*src)
    {
        size_type c(*src++);

        if (c <= 0x7f)
        {
            out.push_back(string::value_type(c));
        }
        else if (c <= 0x7ff)
        {
            out.push_back(string::value_type(0xc0 + ((c >>  6) & 0x3f)));
            out.push_back(string::value_type(0x80 + ((c      ) & 0x3f)));
        }
        else if (c >= 0xd800 && c <= 0xdfff)
        {
            // UTF-16; translate into UTF-8

            c = (c & 0x3ff) << 10;
            size_type n(*src++);
            if (n < 0xdc00 || n > 0xdfff) return false; // Invalid sequence.

            c |= (n & 0x3ff);
            c += 0x10000;

            out.push_back(string::value_type(0xf0 + ((c >> 18) & 0x07)));
            out.push_back(string::value_type(0x80 + ((c >> 12) & 0x3f)));
            out.push_back(string::value_type(0x80 + ((c >>  6) & 0x3f)));
            out.push_back(string::value_type(0x80 + ((c      ) & 0x3f)));
        }
        else if (c <= 0xffff)
        {
            out.push_back(string::value_type(0xe0 + ((c >> 12) & 0x0f)));
            out.push_back(string::value_type(0x80 + ((c >>  6) & 0x3f)));
            out.push_back(string::value_type(0x80 + ((c      ) & 0x3f)));
        }
        else if (c <= 0x10ffff)
        {
            out.push_back(string::value_type(0xf0 + ((c >> 18) & 0x07)));
            out.push_back(string::value_type(0x80 + ((c >> 12) & 0x3f)));
            out.push_back(string::value_type(0x80 + ((c >>  6) & 0x3f)));
            out.push_back(string::value_type(0x80 + ((c      ) & 0x3f)));
        }
        else
        {
            // Unsupported value
            return false;
        }
    }
    return true;
}

bool utf8_is_valid(const char* s)
{
    if (s == 0) return false;

    const byte* p = (const byte*)s;

    while (*p)
    {
        switch (utf8len[*p >> 2])
        {
        default:
        case 0:
        case 5:
        case 6:
            return false;
        
        case 4:
            if ((*++p >> 6) != 2) return false;
            // fall through
        case 3:
            if ((*++p >> 6) != 2) return false;
            // fall through
        case 2:
            if ((*++p >> 6) != 2) return false;
            // fall through
        case 1:
            ++p;
            break;
        }
    }
    return true;
}

size_type wide_length(const char* s)
{
    if (s == 0) return 0;

    size_type len = 0;
    const byte* p = (const byte*)s;

    while (*p)
    {
        ++len;
        switch (utf8len[*p >> 2])
        {
        default:
        case 0:
        case 5:
        case 6:
            return size_type(-1);

        case 1: ++p; break;
        case 2: p += 2; break;
        case 3: p += 3; break;
        case 4: p += 4; break;
        }
    }
    return len;
}

size_type utf8_length(const wchar_t* src)
{
    size_type len = 0;
    while (*src)
    {
        size_type c(*src++);

        if (c <= 0x7f)
        {
            ++len;
        }
        else if (c <= 0x7ff)
        {
            len += 2;
        }
        else if (c >= 0xd800 && c <= 0xdfff)
        {
            // UTF-16; translate into UTF-8

            size_type n(*src++);
            if (n < 0xdc00 || n > 0xdfff) return size_type(-1); // Invalid sequence.

            len += 4;
        }
        else if (c <= 0xffff)
        {
            len += 3;
        }
        else if (c <= 0x10ffff)
        {
            len += 4;
        }
        else
        {
            // Unsupported value
            return size_type(-1);
        }
    }
    return true;
}

}