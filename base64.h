/* THOR - THOR Template Library
 * Joshua M. Kriegshauser
 *
 * base64.h
 *
 * Base64 (RFC 4648) utility functions
 */

#ifndef THOR_BASE64_H
#define THOR_BASE64_H
#pragma once

#ifndef THOR_BASETYPES_H
#include "basetypes.h"
#endif

#ifndef THOR_BASIC_STRING_H
#include "basic_string.h"
#endif

#ifndef THOR_VECTOR_H
#include "vector.h"
#endif

namespace thor
{

// Encodes binary data using Base64. Specify url to use URL-safe encoding
size_type base64_encode(bool url, const byte* data, size_type len, char* output, size_type maxoutput) THOR_NOTHROW;
size_type base64_encode(bool url, const vector<byte>& data, char* output, size_type maxoutput) THOR_NOTHROW;
void      base64_encode(bool url, const byte* data, size_type len, string& output) THOR_NOTHROW;
void      base64_encode(bool url, const vector<byte>& data, string& output) THOR_NOTHROW;

// Decodes Base64 data to binary. Will handle both standard and URL
size_type base64_decode(byte* output, size_type maxlen, const char* p, size_type len = size_type(-1)) THOR_NOTHROW;
size_type base64_decode(byte* output, size_type maxlen, const string& str) THOR_NOTHROW;
void      base64_decode(vector<byte>& output, const string& str) THOR_NOTHROW;
void      base64_decode(vector<byte>& output, const char* p, size_type len = size_type(-1)) THOR_NOTHROW;

// Returns string size necessary to convert len bytes of data. Includes terminator.
size_type base64_encode_size(const byte* data, size_type len) THOR_NOTHROW;
size_type base64_encode_size(const vector<byte>& data) THOR_NOTHROW;

// Returns binary size necessary to convert string to data
size_type base64_decode_size(const char* p, size_type len = size_type(-1)) THOR_NOTHROW;
size_type base64_decode_size(const string& str) THOR_NOTHROW;

///////////////////////////////////////////////////////////////////////////////
// Inline implementations
///////////////////////////////////////////////////////////////////////////////

inline size_type base64_encode(bool url, const vector<byte>& data, char* output, size_type maxoutput) THOR_NOTHROW
{
    return base64_encode(url, &data[0], data.size(), output, maxoutput);
}

inline void base64_encode(bool url, const byte* data, size_type len, string& output) THOR_NOTHROW
{
    const size_type size = base64_encode_size(data, len);
    THOR_ASSERT(size >= 1);
    output.resize(size - 1); // remove null terminator
    base64_encode(url, data, len, &output[0], output.length() + 1);
}

inline void base64_encode(bool url, const vector<byte>& data, string& output) THOR_NOTHROW
{
    base64_encode(url, &data[0], data.size(), output);
}

inline size_type base64_decode(byte* output, size_type maxlen, const string& str) THOR_NOTHROW
{
    return base64_decode(output, maxlen, str.c_str(), str.length());
}

inline void base64_decode(vector<byte>& output, const char* p, size_type len)
{
    const size_type size = base64_decode_size(p, len);
    THOR_ASSERT(size != size_type(-1));
    output.resize(size);
    base64_decode(&output[0], output.size(), p, len);
}

inline void base64_decode(vector<byte>& output, const string& str) THOR_NOTHROW
{
    base64_decode(output, str.c_str(), str.length());
}

inline size_type base64_encode_size(const vector<byte>& data) THOR_NOTHROW
{
    return base64_encode_size(&data[0], data.size());
}

inline size_type base64_encode_size(const byte* data, size_type len) THOR_NOTHROW
{
    // Encodes 3 bytes in 4 characters
    THOR_UNUSED(data);
    size_type out = (len / 3) * 4;
    out += (len % 3) ? 4 : 0;
    ++out; // nul terminator
    return out;
}

inline size_type base64_decode_size(const string& str) THOR_NOTHROW
{
    return base64_decode_size(str.c_str(), str.length());
}

}

#endif