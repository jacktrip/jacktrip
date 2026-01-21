//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2024 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file WebTransportSession.cpp
 * \author JackTrip Contributors
 * \date 2026
 */

#include "WebTransportSession.h"

#include <msquic.h>

#include <QMap>
#include <QUrl>
#include <QUrlQuery>
#include <algorithm>
#include <bitset>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>

#include "../jacktrip_globals.h"
#include "WebTransportDataProtocol.h"

using std::cerr;
using std::cout;
using std::endl;

// Default maximum datagram size (conservative estimate)
static constexpr size_t DEFAULT_MAX_DATAGRAM_SIZE = 1200;

// WebTransport ALPN
// ALPN for WebTransport is "h3" (HTTP/3)

//*******************************************************************************
// HTTP/3 and QPACK decoding
//*******************************************************************************

// HTTP/3 frame types
enum Http3FrameType {
    HTTP3_FRAME_DATA = 0x00,
    HTTP3_FRAME_HEADERS = 0x01,
    HTTP3_FRAME_CANCEL_PUSH = 0x03,
    HTTP3_FRAME_SETTINGS = 0x04,
    HTTP3_FRAME_PUSH_PROMISE = 0x05,
    HTTP3_FRAME_GOAWAY = 0x07,
    HTTP3_FRAME_MAX_PUSH_ID = 0x0D,
};

/** \brief Read a QUIC variable-length integer (QUIC varint, 2-bit prefix)
 * 
 * Returns the value and updates pos to point after the integer.
 * Returns -1 on error.
 */
static int64_t readVarint(const uint8_t* data, size_t len, size_t& pos)
{
    if (pos >= len) {
        return -1;
    }

    uint8_t firstByte = data[pos];
    uint8_t prefix = firstByte >> 6;  // Top 2 bits indicate length
    int64_t value = firstByte & 0x3F; // Bottom 6 bits are part of value

    size_t numBytes = 1 << prefix;  // 1, 2, 4, or 8 bytes

    if (pos + numBytes > len) {
        return -1;
    }

    pos++;  // Move past first byte

    // Read remaining bytes
    for (size_t i = 1; i < numBytes; i++) {
        value = (value << 8) | data[pos++];
    }

    return value;
}

/** \brief Read a QPACK integer with specified prefix length
 * 
 * Used for QPACK field encodings where the prefix length varies.
 * prefixBits: number of bits in the first byte used for the value (e.g., 7, 6, 5, 4)
 * Returns the value and updates pos to point after the integer.
 * Returns -1 on error.
 */
static int64_t readQpackInt(const uint8_t* data, size_t len, size_t& pos, int prefixBits)
{
    if (pos >= len || prefixBits < 1 || prefixBits > 8) {
        return -1;
    }

    uint8_t mask = (1 << prefixBits) - 1;  // Mask for prefix bits
    int64_t value = data[pos++] & mask;
    
    // Check if this is a multi-byte integer
    if (value == mask) {
        // Read continuation bytes
        int m = 0;
        uint8_t b;
        do {
            if (pos >= len) {
                return -1;
            }
            b = data[pos++];
            value += (b & 0x7F) * (1 << m);
            m += 7;
        } while ((b & 0x80) != 0);
    }
    
    return value;
}

/** \brief Parse HTTP/3 frame and extract payload
 *
 * Returns true if a HEADERS frame was found and extracted.
 * Sets payloadData and payloadLen to point to the QPACK-encoded headers.
 */
static bool parseHttp3Frame(const uint8_t* data, size_t len,
                           const uint8_t*& payloadData, size_t& payloadLen)
{
    size_t pos = 0;

    while (pos < len) {
        // Read frame type
        int64_t frameType = readVarint(data, len, pos);
        if (frameType < 0) {
            cerr << "WebTransportSession: Failed to read frame type at pos " << pos << endl;
            return false;
        }

        // Read frame length
        int64_t frameLength = readVarint(data, len, pos);
        if (frameLength < 0) {
            cerr << "WebTransportSession: Failed to read frame length at pos " << pos << endl;
            return false;
        }

        if (pos + frameLength > len) {
            cerr << "WebTransportSession: Frame length exceeds buffer" << endl;
            return false;
        }

        // Check if this is a HEADERS frame
        if (frameType == HTTP3_FRAME_HEADERS) {
            payloadData = &data[pos];
            payloadLen = static_cast<size_t>(frameLength);
            return true;
        }

        // Skip to next frame
        pos += static_cast<size_t>(frameLength);
    }

    cerr << "WebTransportSession: No HEADERS frame found in stream data" << endl;
    return false;
}

// QPACK static table entries for WebTransport
// See: https://www.rfc-editor.org/rfc/rfc9204.html#name-static-table
// Each entry is (name, value) pair. Access with: name = TABLE[index*2], value = TABLE[index*2+1]
static const char* QPACK_STATIC_TABLE[] = {
    ":authority", "",                    // 0
    ":path", "/",                        // 1
    "age", "0",                          // 2
    "content-disposition", "",           // 3
    "content-length", "0",               // 4
    "cookie", "",                        // 5
    "date", "",                          // 6
    "etag", "",                          // 7
    "if-modified-since", "",             // 8
    "if-none-match", "",                 // 9
    "last-modified", "",                 // 10
    "link", "",                          // 11
    "location", "",                      // 12
    "referer", "",                       // 13
    "set-cookie", "",                    // 14
    ":method", "CONNECT",                // 15
    ":method", "DELETE",                 // 16
    ":method", "GET",                    // 17
    ":method", "HEAD",                   // 18
    ":method", "OPTIONS",                // 19
    ":method", "POST",                   // 20
    ":method", "PUT",                    // 21
    ":scheme", "http",                   // 22
    ":scheme", "https",                  // 23
    ":status", "103",                    // 24
    ":status", "200",                    // 25
    ":status", "304",                    // 26
    ":status", "404",                    // 27
    ":status", "503",                    // 28
    "accept", "*/*",                     // 29
    "accept", "application/dns-message", // 30
    "accept-encoding", "gzip, deflate, br", // 31
    "accept-ranges", "bytes",            // 32
    "access-control-allow-headers", "cache-control", // 33
    "access-control-allow-headers", "content-type",  // 34
    "access-control-allow-origin", "*",  // 35
    "cache-control", "max-age=0",        // 36
    "cache-control", "max-age=2592000",  // 37
    "cache-control", "max-age=604800",   // 38
    "cache-control", "no-cache",         // 39
    "cache-control", "no-store",         // 40
    "cache-control", "public, max-age=31536000", // 41
    "content-encoding", "br",            // 42
    "content-encoding", "gzip",          // 43
    "content-type", "application/dns-message", // 44
    "content-type", "application/javascript",  // 45
    "content-type", "application/json",  // 46
    "content-type", "application/x-www-form-urlencoded", // 47
    "content-type", "image/gif",         // 48
    "content-type", "image/jpeg",        // 49
    "content-type", "image/png",         // 50
    "content-type", "text/css",          // 51
    "content-type", "text/html; charset=utf-8", // 52
    "content-type", "text/plain",        // 53
    "content-type", "text/plain;charset=utf-8", // 54
    "range", "bytes=0-",                 // 55
    "strict-transport-security", "max-age=31536000", // 56
    "strict-transport-security", "max-age=31536000; includesubdomains", // 57
    "strict-transport-security", "max-age=31536000; includesubdomains; preload", // 58
    "vary", "accept-encoding",           // 59
    "vary", "origin",                    // 60
    "x-content-type-options", "nosniff", // 61
    "x-xss-protection", "1; mode=block", // 62
    ":status", "100",                    // 63
    ":status", "204",                    // 64
    ":status", "206",                    // 65
    ":status", "302",                    // 66
    ":status", "400",                    // 67
    ":status", "403",                    // 68
    ":status", "421",                    // 69
    ":status", "425",                    // 70
    ":status", "500",                    // 71
    "accept-language", "",               // 72
    "access-control-allow-credentials", "FALSE", // 73
    "access-control-allow-credentials", "TRUE",  // 74
    "access-control-allow-headers", "*", // 75
    "access-control-allow-methods", "get", // 76
    "access-control-allow-methods", "get, post, options", // 77
    "access-control-allow-methods", "options", // 78
    "access-control-expose-headers", "content-length", // 79
    "access-control-request-headers", "content-type", // 80
    "access-control-request-method", "get", // 81
    "access-control-request-method", "post", // 82
    "alt-svc", "clear",                  // 83
    "authorization", "",                 // 84
    "content-security-policy", "script-src 'none'; object-src 'none'; base-uri 'none'", // 85
    "early-data", "1",                   // 86
    "expect-ct", "",                     // 87
    "forwarded", "",                     // 88
    "if-range", "",                      // 89
    "origin", "",                        // 90
    "purpose", "prefetch",               // 91
    "server", "",                        // 92
    "timing-allow-origin", "*",          // 93
    "upgrade-insecure-requests", "1",    // 94
    "user-agent", "",                    // 95
    "x-forwarded-for", "",               // 96
    "x-frame-options", "deny",           // 97
    "x-frame-options", "sameorigin",     // 98
};

/** \brief HPACK/QPACK Huffman decoding
 *
 * Complete decode table from RFC 7541 Appendix B
 * Each entry: {code, bit_length, symbol}
 */
struct HuffmanCode {
    uint32_t code;
    uint8_t bits;
    uint8_t sym;
};

// Complete HPACK Huffman table (RFC 7541 Appendix B)
// Sorted by code for efficient decoding
static const HuffmanCode HUFFMAN_CODES[] = {
    // 5-bit codes (0x00-0x09)
    {0x00, 5, '0'}, {0x01, 5, '1'}, {0x02, 5, '2'}, {0x03, 5, 'a'},
    {0x04, 5, 'c'}, {0x05, 5, 'e'}, {0x06, 5, 'i'}, {0x07, 5, 'o'},
    {0x08, 5, 's'}, {0x09, 5, 't'},
    // 6-bit codes (0x14-0x2d)
    {0x14, 6, ' '}, {0x15, 6, '%'}, {0x16, 6, '-'}, {0x17, 6, '.'},
    {0x18, 6, '/'}, {0x19, 6, '3'}, {0x1a, 6, '4'}, {0x1b, 6, '5'},
    {0x1c, 6, '6'}, {0x1d, 6, '7'}, {0x1e, 6, '8'}, {0x1f, 6, '9'},
    {0x20, 6, '='}, {0x21, 6, 'A'}, {0x22, 6, '_'}, {0x23, 6, 'b'},
    {0x24, 6, 'd'}, {0x25, 6, 'f'}, {0x26, 6, 'g'}, {0x27, 6, 'h'},
    {0x28, 6, 'l'}, {0x29, 6, 'm'}, {0x2a, 6, 'n'}, {0x2b, 6, 'p'},
    {0x2c, 6, 'r'}, {0x2d, 6, 'u'},
    // 7-bit codes (0x5c-0x7b)
    {0x5c, 7, ':'}, {0x5d, 7, 'B'}, {0x5e, 7, 'C'}, {0x5f, 7, 'D'},
    {0x60, 7, 'E'}, {0x61, 7, 'F'}, {0x62, 7, 'G'}, {0x63, 7, 'H'},
    {0x64, 7, 'I'}, {0x65, 7, 'J'}, {0x66, 7, 'K'}, {0x67, 7, 'L'},
    {0x68, 7, 'M'}, {0x69, 7, 'N'}, {0x6a, 7, 'O'}, {0x6b, 7, 'P'},
    {0x6c, 7, 'Q'}, {0x6d, 7, 'R'}, {0x6e, 7, 'S'}, {0x6f, 7, 'T'},
    {0x70, 7, 'U'}, {0x71, 7, 'V'}, {0x72, 7, 'W'}, {0x73, 7, 'Y'},
    {0x74, 7, 'j'}, {0x75, 7, 'k'}, {0x76, 7, 'q'}, {0x77, 7, 'v'},
    {0x78, 7, 'w'}, {0x79, 7, 'x'}, {0x7a, 7, 'y'}, {0x7b, 7, 'z'},
    // 8-bit codes (0xf8-0xff)
    {0xf8, 8, '&'}, {0xf9, 8, '*'}, {0xfa, 8, ','}, {0xfb, 8, ';'},
    {0xfc, 8, 'X'}, {0xfd, 8, 'Z'},
    // 10-bit codes
    {0x3f8, 10, '!'}, {0x3f9, 10, '"'}, {0x3fa, 10, '('}, {0x3fb, 10, ')'},
    {0x3fc, 10, '?'},
    // 11-bit codes
    {0x7fa, 11, '\''}, {0x7fb, 11, '+'}, {0x7fc, 11, '|'},
    // 12-bit codes
    {0xffa, 12, '#'}, {0xffb, 12, '>'},
    // 13-bit codes
    {0x1ff8, 13, '\x00'}, {0x1ff9, 13, '$'}, {0x1ffa, 13, '@'}, {0x1ffb, 13, '['},
    {0x1ffc, 13, ']'}, {0x1ffd, 13, '~'},
    // 14-bit codes
    {0x3ffc, 14, '^'}, {0x3ffd, 14, '}'}, 
    // 15-bit codes
    {0x7ffc, 15, '<'}, {0x7ffd, 15, '`'}, {0x7ffe, 15, '{'},
    // 19-bit codes
    {0x7fff0, 19, '\\'}, 
    // 20-bit codes
    {0xfffe6, 20, 0xc3}, {0xfffe7, 20, 0xd0}, {0xfffe8, 20, 0x80},
    {0xfffe9, 20, 0x82}, {0xfffea, 20, 0x83}, {0xfffeb, 20, 0xa2},
    {0xfffec, 20, 0xb8}, {0xfffed, 20, 0xc2}, {0xfffee, 20, 0xe0},
    {0xfffef, 20, 0xe2},
    // 21-bit codes  
    {0x1fffdc, 21, 0x99}, {0x1fffdd, 21, 0xa1}, {0x1fffde, 21, 0xa7},
    {0x1fffdf, 21, 0xac}, {0x1fffe0, 21, 0xb0}, {0x1fffe1, 21, 0xb1},
    {0x1fffe2, 21, 0xb3}, {0x1fffe3, 21, 0xd1}, {0x1fffe4, 21, 0xd8},
    {0x1fffe5, 21, 0xd9}, {0x1fffe6, 21, 0xe3}, {0x1fffe7, 21, 0xe5},
    {0x1fffe8, 21, 0xe6},
    // 22-bit codes
    {0x3fffd2, 22, 0x81}, {0x3fffd3, 22, 0x84}, {0x3fffd4, 22, 0x85},
    {0x3fffd5, 22, 0x86}, {0x3fffd6, 22, 0x88}, {0x3fffd7, 22, 0x92},
    {0x3fffd8, 22, 0x9a}, {0x3fffd9, 22, 0x9c}, {0x3fffda, 22, 0xa0},
    {0x3fffdb, 22, 0xa3}, {0x3fffdc, 22, 0xa4}, {0x3fffdd, 22, 0xa9},
    {0x3fffde, 22, 0xaa}, {0x3fffdf, 22, 0xad}, {0x3fffe0, 22, 0xb2},
    {0x3fffe1, 22, 0xb5}, {0x3fffe2, 22, 0xb9}, {0x3fffe3, 22, 0xba},
    {0x3fffe4, 22, 0xbb}, {0x3fffe5, 22, 0xbd}, {0x3fffe6, 22, 0xbe},
    {0x3fffe7, 22, 0xc4}, {0x3fffe8, 22, 0xc6}, {0x3fffe9, 22, 0xe4},
    {0x3fffea, 22, 0xe8}, {0x3fffeb, 22, 0xe9},
    // 23-bit codes
    {0x7fffd8, 23, 0x01}, {0x7fffd9, 23, 0x87}, {0x7fffda, 23, 0x89},
    {0x7fffdb, 23, 0x8a}, {0x7fffdc, 23, 0x8b}, {0x7fffdd, 23, 0x8c},
    {0x7fffde, 23, 0x8d}, {0x7fffdf, 23, 0x8f}, {0x7fffe0, 23, 0x93},
    {0x7fffe1, 23, 0x95}, {0x7fffe2, 23, 0x96}, {0x7fffe3, 23, 0x97},
    {0x7fffe4, 23, 0x98}, {0x7fffe5, 23, 0x9b}, {0x7fffe6, 23, 0x9d},
    {0x7fffe7, 23, 0x9e}, {0x7fffe8, 23, 0xa5}, {0x7fffe9, 23, 0xa6},
    {0x7fffea, 23, 0xa8}, {0x7fffeb, 23, 0xae}, {0x7fffec, 23, 0xaf},
    {0x7fffed, 23, 0xb4}, {0x7fffee, 23, 0xb6}, {0x7fffef, 23, 0xb7},
    {0x7ffff0, 23, 0xbc}, {0x7ffff1, 23, 0xbf}, {0x7ffff2, 23, 0xc5},
    {0x7ffff3, 23, 0xe7}, {0x7ffff4, 23, 0xef},
    // 24-bit codes
    {0xffffea, 24, 0x09}, {0xffffeb, 24, 0x8e}, {0xffffec, 24, 0x90},
    {0xffffed, 24, 0x91}, {0xffffee, 24, 0x94}, {0xffffef, 24, 0x9f},
    {0xfffff0, 24, 0xab}, {0xfffff1, 24, 0xce}, {0xfffff2, 24, 0xd7},
    {0xfffff3, 24, 0xe1}, {0xfffff4, 24, 0xec}, {0xfffff5, 24, 0xed},
    // 25-bit codes
    {0x1ffffec, 25, 0xc7}, {0x1ffffed, 25, 0xcf}, {0x1ffffee, 25, 0xea},
    {0x1ffffef, 25, 0xeb},
    // 26-bit codes
    {0x3ffffe0, 26, 0xc0}, {0x3ffffe1, 26, 0xc1}, {0x3ffffe2, 26, 0xc8},
    {0x3ffffe3, 26, 0xc9}, {0x3ffffe4, 26, 0xca}, {0x3ffffe5, 26, 0xcd},
    {0x3ffffe6, 26, 0xd2}, {0x3ffffe7, 26, 0xd5}, {0x3ffffe8, 26, 0xda},
    {0x3ffffe9, 26, 0xdb}, {0x3ffffea, 26, 0xee}, {0x3ffffeb, 26, 0xf0},
    {0x3ffffec, 26, 0xf2}, {0x3ffffed, 26, 0xf3}, {0x3ffffee, 26, 0xff},
    // 27-bit codes
    {0x7ffffde, 27, 0xcb}, {0x7ffffdf, 27, 0xcc}, {0x7ffffe0, 27, 0xd3},
    {0x7ffffe1, 27, 0xd4}, {0x7ffffe2, 27, 0xd6}, {0x7ffffe3, 27, 0xdd},
    {0x7ffffe4, 27, 0xde}, {0x7ffffe5, 27, 0xdf}, {0x7ffffe6, 27, 0xf1},
    {0x7ffffe7, 27, 0xf4}, {0x7ffffe8, 27, 0xf5}, {0x7ffffe9, 27, 0xf6},
    {0x7ffffea, 27, 0xf7}, {0x7ffffeb, 27, 0xf8}, {0x7ffffec, 27, 0xfa},
    {0x7ffffed, 27, 0xfb}, {0x7ffffee, 27, 0xfc}, {0x7ffffef, 27, 0xfd},
    {0x7fffff0, 27, 0xfe},
    // 28-bit codes
    {0xfffffe2, 28, 0x02}, {0xfffffe3, 28, 0x03}, {0xfffffe4, 28, 0x04},
    {0xfffffe5, 28, 0x05}, {0xfffffe6, 28, 0x06}, {0xfffffe7, 28, 0x07},
    {0xfffffe8, 28, 0x08}, {0xfffffe9, 28, 0x0b}, {0xfffffea, 28, 0x0c},
    {0xfffffeb, 28, 0x0e}, {0xfffffec, 28, 0x0f}, {0xfffffed, 28, 0x10},
    {0xfffffee, 28, 0x11}, {0xfffffef, 28, 0x12}, {0xffffff0, 28, 0x13},
    {0xffffff1, 28, 0x14}, {0xffffff2, 28, 0x15}, {0xffffff3, 28, 0x17},
    {0xffffff4, 28, 0x18}, {0xffffff5, 28, 0x19}, {0xffffff6, 28, 0x1a},
    {0xffffff7, 28, 0x1b}, {0xffffff8, 28, 0x1c}, {0xffffff9, 28, 0x1d},
    {0xffffffa, 28, 0x1e}, {0xffffffb, 28, 0x1f}, {0xffffffc, 28, 0x7f},
    {0xffffffd, 28, 0xdc}, {0xffffffe, 28, 0xf9},
    // Note: 30-bit EOS marker (0x3fffffff) is handled via padding check
};

static const size_t HUFFMAN_CODES_SIZE = sizeof(HUFFMAN_CODES) / sizeof(HUFFMAN_CODES[0]);

/** \brief Decode Huffman-encoded string
 *
 * Proper implementation using bit accumulator and the RFC 7541 table.
 */
static QString decodeHuffman(const uint8_t* data, size_t len)
{
    QString result;
    uint64_t accum = 0;  // Use 64-bit to handle up to 30-bit codes
    int bits = 0;
    
    size_t bytePos = 0;
    while (bytePos < len || bits >= 5) {
        // Fill accumulator with more bytes
        while (bits < 32 && bytePos < len) {
            accum = (accum << 8) | data[bytePos++];
            bits += 8;
        }
        
        if (bits < 5) break;  // Not enough bits for shortest code
        
        // Try to find a matching code, starting with shortest codes
        bool found = false;
        for (size_t i = 0; i < HUFFMAN_CODES_SIZE && !found; i++) {
            int codeBits = HUFFMAN_CODES[i].bits;
            if (codeBits > bits) continue;
            
            // Extract the top 'codeBits' bits from accumulator
            uint32_t code = static_cast<uint32_t>(accum >> (bits - codeBits));
            uint32_t mask = (1u << codeBits) - 1;
            code &= mask;
            
            if (code == HUFFMAN_CODES[i].code) {
                result += QChar(HUFFMAN_CODES[i].sym);
                bits -= codeBits;
                accum &= (1ULL << bits) - 1;  // Clear consumed bits
                found = true;
            }
        }
        
        if (!found) {
            // Check if remaining bits are EOS padding (all 1s)
            if (bits <= 7) {
                uint32_t remaining = static_cast<uint32_t>(accum & ((1ULL << bits) - 1));
                uint32_t eosPadding = (1u << bits) - 1;
                if (remaining == eosPadding) {
                    break;  // Valid EOS padding
                }
            }
            // Decoding error - skip a bit and try again (shouldn't happen with valid input)
            bits--;
            if (bits > 0) {
                accum &= (1ULL << bits) - 1;
            }
        }
    }
    
    return result;
}

/** \brief Simple QPACK decoder for HTTP/3 headers
 *
 * This is a minimal decoder that handles the most common cases:
 * - Indexed header fields from static table
 * - Literal header fields with literal names (including Huffman-encoded)
 *
 * It's sufficient for parsing WebTransport CONNECT requests.
 */
static bool decodeQPackHeaders(const uint8_t* data, size_t len,
                                QMap<QString, QString>& headers)
{
    if (len < 2) {
        return false;
    }

    size_t pos = 0;

    // Read QPACK prefix:
    // 1. Required Insert Count (8-bit prefix integer)
    // 2. Delta Base with S bit (7-bit prefix integer)
    
    // Read Required Insert Count (8-bit prefix)
    int64_t requiredInsertCount = readQpackInt(data, len, pos, 8);
    if (requiredInsertCount < 0) {
        cerr << "WebTransportSession: Failed to read Required Insert Count" << endl;
        return false;
    }
    
    // Read Delta Base with sign bit
    if (pos >= len) {
        cerr << "WebTransportSession: Buffer too short for Delta Base" << endl;
        return false;
    }
    
    int64_t deltaBase = readQpackInt(data, len, pos, 7);  // 7-bit prefix (after S bit)
    if (deltaBase < 0) {
        cerr << "WebTransportSession: Failed to read Delta Base" << endl;
        return false;
    }
    
    // Parse header fields
    while (pos < len) {
        uint8_t byte = data[pos];

        // Indexed Header Field (pattern: 1T)
        if (byte & 0x80) {
            // Static table (T=1) or dynamic table (T=0)
            bool isStatic = (byte & 0x40) != 0;
            int64_t index = readQpackInt(data, len, pos, 6);  // 6-bit prefix
            
            if (index < 0) {
                cerr << "    Failed to read index" << endl;
                return false;
            }

            if (isStatic && static_cast<size_t>(index) * 2 + 1 < sizeof(QPACK_STATIC_TABLE) / sizeof(char*)) {
                const char* name = QPACK_STATIC_TABLE[index * 2];
                const char* value = QPACK_STATIC_TABLE[index * 2 + 1];
                if (name && value) {
                    headers[QString::fromUtf8(name)] = QString::fromUtf8(value);
                }
            }
        }
        // Literal Header Field with Name Reference (pattern: 01NT)
        else if (byte & 0x40) {
            bool isStatic = (byte & 0x10) != 0;    // T bit
            int64_t nameIndex = readQpackInt(data, len, pos, 4);  // 4-bit prefix

            if (nameIndex < 0) {
                cerr << "    Failed to read name index" << endl;
                return false;
            }

            // Get the name from static table
            QString name;
            if (isStatic && static_cast<size_t>(nameIndex) * 2 < sizeof(QPACK_STATIC_TABLE) / sizeof(char*)) {
                const char* nameStr = QPACK_STATIC_TABLE[nameIndex * 2];
                if (nameStr) {
                    name = QString::fromUtf8(nameStr);
                }
            }

            // Read the value (length-prefixed string)
            if (pos >= len) {
                cerr << "    Buffer too short for value" << endl;
                return false;
            }

            bool huffman = (data[pos] & 0x80) != 0;  // H bit
            int64_t valueLen = readQpackInt(data, len, pos, 7);  // 7-bit prefix

            if (valueLen < 0) {
                cerr << "    Failed to read value length" << endl;
                return false;
            }

            QString value;
            if (huffman) {
                if (pos + valueLen > len) {
                    cerr << "    Huffman value length exceeds buffer" << endl;
                    return false;
                }
                value = decodeHuffman(&data[pos], static_cast<size_t>(valueLen));
                pos += static_cast<size_t>(valueLen);
            } else if (pos + valueLen <= len) {
                value = QString::fromUtf8(
                    reinterpret_cast<const char*>(&data[pos]), 
                    static_cast<int>(valueLen));
                pos += static_cast<size_t>(valueLen);
            } else {
                cerr << "    Value length exceeds buffer" << endl;
                return false;
            }

            if (!name.isEmpty() && !value.isEmpty()) {
                headers[name] = value;
            }
        }
        // Literal Header Field with Literal Name (pattern: 001N)
        else if (byte & 0x20) {
            // Read name length and name
            bool huffmanName = (data[pos] & 0x08) != 0;  // H bit for name
            int64_t nameLen = readQpackInt(data, len, pos, 3);  // 3-bit prefix
            
            if (nameLen < 0) {
                cerr << "    Failed to read name length" << endl;
                return false;
            }
            
            QString name;
            if (huffmanName) {
                if (pos + nameLen > len) {
                    cerr << "    Huffman name length exceeds buffer" << endl;
                    return false;
                }
                name = decodeHuffman(&data[pos], static_cast<size_t>(nameLen));
                pos += static_cast<size_t>(nameLen);
            } else if (pos + nameLen <= len) {
                name = QString::fromUtf8(
                    reinterpret_cast<const char*>(&data[pos]), 
                    static_cast<int>(nameLen));
                pos += static_cast<size_t>(nameLen);
            } else {
                cerr << "    Name length exceeds buffer" << endl;
                return false;
            }

            // Read value length and value
            if (pos >= len) {
                cerr << "    Buffer too short for value" << endl;
                return false;
            }

            bool huffmanValue = (data[pos] & 0x80) != 0;  // H bit for value
            int64_t valueLen = readQpackInt(data, len, pos, 7);  // 7-bit prefix
            
            if (valueLen < 0) {
                cerr << "    Failed to read value length" << endl;
                return false;
            }

            QString value;
            if (huffmanValue) {
                if (pos + valueLen > len) {
                    cerr << "    Huffman value length exceeds buffer" << endl;
                    return false;
                }
                value = decodeHuffman(&data[pos], static_cast<size_t>(valueLen));
                pos += static_cast<size_t>(valueLen);
            } else if (pos + valueLen <= len) {
                value = QString::fromUtf8(
                    reinterpret_cast<const char*>(&data[pos]), 
                    static_cast<int>(valueLen));
                pos += static_cast<size_t>(valueLen);
            } else {
                cerr << "    Value length exceeds buffer" << endl;
                return false;
            }

            if (!name.isEmpty()) {
                headers[name] = value;
            }
        } else {
            // Unknown encoding or other field types we don't handle yet
            pos++;
        }
    }

    return !headers.isEmpty();
}

//*******************************************************************************
// Static callback handlers for msquic
//*******************************************************************************

static QUIC_STATUS QUIC_API ConnectionCallback(HQUIC /* connection */, void* context,
                                                QUIC_CONNECTION_EVENT* event)
{
    WebTransportSession* session = static_cast<WebTransportSession*>(context);
    if (session) {
        return session->handleConnectionEvent(event);
    }
    return QUIC_STATUS_INVALID_STATE;
}

static QUIC_STATUS QUIC_API StreamCallback(HQUIC stream, void* context,
                                           QUIC_STREAM_EVENT* event)
{
    WebTransportSession* session = static_cast<WebTransportSession*>(context);
    if (session) {
        QUIC_STATUS result = session->handleStreamEvent(stream, event);
        return result;
    }
    return QUIC_STATUS_INVALID_STATE;
}

// Callback for server-initiated infrastructure streams (control, QPACK)
static QUIC_STATUS QUIC_API InfraStreamCallback(HQUIC stream, void* context,
                                                QUIC_STREAM_EVENT* event)
{
    WebTransportSession* session = static_cast<WebTransportSession*>(context);
    if (session) {
        QUIC_STATUS result = session->handleInfraStreamEvent(stream, event);
        return result;
    }
    return QUIC_STATUS_INVALID_STATE;
}

//*******************************************************************************
WebTransportSession::WebTransportSession(const QUIC_API_TABLE* api, HQUIC connection,
                                         const QHostAddress& peerAddress,
                                         quint16 peerPort, QObject* parent)
    : QObject(parent)
    , mApi(api)
    , mConnection(connection)
    , mControlStream(nullptr)
    , mQpackEncoderStream(nullptr)
    , mQpackDecoderStream(nullptr)
    , mConnectStream(nullptr)
    , mConnectStreamId(0)
    , mState(STATE_NEW)
    , mPeerAddress(peerAddress)
    , mPeerPort(peerPort)
    , mSessionAccepted(false)
    , mControlStreamReady(false)
    , mQpackEncoderStreamReady(false)
    , mQpackDecoderStreamReady(false)
    , mClientSettingsReceived(false)
    , mServerSettingsSent(false)
    , mMaxDatagramSize(DEFAULT_MAX_DATAGRAM_SIZE)
{
    if (mConnection && mApi) {
        // Set the callback context to this session
        mApi->SetCallbackHandler(mConnection, (void*)ConnectionCallback, this);

        // Enable datagram support
        QUIC_SETTINGS settings{};
        std::memset(&settings, 0, sizeof(settings));
        settings.DatagramReceiveEnabled = TRUE;
        settings.IsSet.DatagramReceiveEnabled = TRUE;

        QUIC_STATUS status = mApi->SetParam(mConnection, QUIC_PARAM_CONN_SETTINGS, 
                                            sizeof(settings), &settings);
        if (QUIC_FAILED(status)) {
            cerr << "WebTransportSession: Failed to enable datagram support, status: 0x"
                 << std::hex << status << std::dec << endl;
        } else {
            setState(STATE_CONNECTING);
        }
    } else {
        cerr << "WebTransportSession: ERROR - Connection or API is null!" << endl;
    }
}

//*******************************************************************************
WebTransportSession::~WebTransportSession()
{
    // If connection is still active, initiate shutdown and wait for it to complete
    std::unique_lock<std::mutex> lock(mMutex);
    
    if (mConnection && mApi && !mShutdownComplete) {
        // Mark as disconnected so SHUTDOWN_COMPLETE callback won't emit signals
        mState = STATE_DISCONNECTED;
        
        mApi->ConnectionShutdown(mConnection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
        
        // Wait for SHUTDOWN_COMPLETE callback (releases mMutex while waiting)
        bool completed = false;
        do {
            completed = mShutdownCv.wait_for(lock, std::chrono::seconds(5), [this] {
                return mShutdownComplete;  // Protected by mMutex (held when predicate is checked)
            });
        } while (!completed);
    }
    
    // Now safe to close handles - msquic is done with them (still holding mMutex)
    if (mControlStream && mApi) {
        mApi->StreamClose(mControlStream);
        mControlStream = nullptr;
    }
    
    if (mQpackEncoderStream && mApi) {
        mApi->StreamClose(mQpackEncoderStream);
        mQpackEncoderStream = nullptr;
    }
    
    if (mQpackDecoderStream && mApi) {
        mApi->StreamClose(mQpackDecoderStream);
        mQpackDecoderStream = nullptr;
    }

    if (mConnection && mApi) {
        mApi->ConnectionClose(mConnection);
        mConnection = nullptr;
    }
}

//*******************************************************************************
bool WebTransportSession::processConnectRequest(const QString& path)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mState != STATE_CONNECTING) {
        cerr << "WebTransportSession: Invalid state for CONNECT request (state=" 
             << mState << ", expected=" << STATE_CONNECTING << ")" << endl;
        return false;
    }

    // Parse client name from path
    parseClientNameFromPath(path);

    // Mark session as accepted - will send response
    mSessionAccepted = true;
    return true;
}

//*******************************************************************************
bool WebTransportSession::sendConnectResponse(int statusCode)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (!mConnection || !mApi) {
        cerr << "WebTransportSession: Cannot send response - connection or API is null" << endl;
        return false;
    }

    // Send the actual HTTP/3 response to the client
    if (!sendHttp3Response(statusCode)) {
        cerr << "WebTransportSession: Failed to send HTTP/3 response" << endl;
        setState(STATE_FAILED);
        emit sessionFailed(QStringLiteral("Failed to send HTTP/3 response"));
        return false;
    }

    if (statusCode == 200) {
        // Session accepted - transition to connected state
        setState(STATE_CONNECTED);
        emit sessionEstablished();
        return true;
    } else {
        // Session rejected
        cerr << "WebTransportSession: Rejecting session with status " << statusCode << endl;
        setState(STATE_FAILED);
        emit sessionFailed(
            QStringLiteral("CONNECT request rejected with status %1").arg(statusCode));
        return false;
    }
}

//*******************************************************************************
// Helper to encode a QUIC variable-length integer
// Returns the number of bytes written
static size_t encodeQuicVarint(uint64_t value, uint8_t* output)
{
    if (value <= 63) {
        // 1-byte encoding: 00xxxxxx
        output[0] = static_cast<uint8_t>(value);
        return 1;
    } else if (value <= 16383) {
        // 2-byte encoding: 01xxxxxx xxxxxxxx (big-endian)
        output[0] = static_cast<uint8_t>(0x40 | (value >> 8));
        output[1] = static_cast<uint8_t>(value & 0xFF);
        return 2;
    } else if (value <= 1073741823) {
        // 4-byte encoding: 10xxxxxx ... (big-endian)
        output[0] = static_cast<uint8_t>(0x80 | (value >> 24));
        output[1] = static_cast<uint8_t>((value >> 16) & 0xFF);
        output[2] = static_cast<uint8_t>((value >> 8) & 0xFF);
        output[3] = static_cast<uint8_t>(value & 0xFF);
        return 4;
    } else {
        // 8-byte encoding: 11xxxxxx ... (big-endian)
        output[0] = static_cast<uint8_t>(0xC0 | (value >> 56));
        output[1] = static_cast<uint8_t>((value >> 48) & 0xFF);
        output[2] = static_cast<uint8_t>((value >> 40) & 0xFF);
        output[3] = static_cast<uint8_t>((value >> 32) & 0xFF);
        output[4] = static_cast<uint8_t>((value >> 24) & 0xFF);
        output[5] = static_cast<uint8_t>((value >> 16) & 0xFF);
        output[6] = static_cast<uint8_t>((value >> 8) & 0xFF);
        output[7] = static_cast<uint8_t>(value & 0xFF);
        return 8;
    }
}

//*******************************************************************************
bool WebTransportSession::sendDatagram(uint8_t* buffer, size_t length, QUIC_BUFFER* quicBuf, void* owner)
{
    // Lock-free check - safe to call from real-time audio thread
    if (!isConnected() || !mConnection || !mApi || !buffer || !owner || !quicBuf) {
        return false;
    }

    // Calculate total size needed (varint prefix + payload)
    uint64_t quarterStreamId = mConnectStreamId / 4;
    uint8_t varintBuf[8];
    size_t varintLen = encodeQuicVarint(quarterStreamId, varintBuf);
    size_t totalLen = varintLen + length;
    
    if (totalLen > mMaxDatagramSize) {
        return false;
    }
    
    // Shift data to make room for varint prefix
    // Move from right to left to avoid overwriting
    std::memmove(buffer + varintLen, buffer, length);
    
    // Write varint prefix at the beginning
    std::memcpy(buffer, varintBuf, varintLen);
    
    // Setup QUIC buffer in the persistent context (not on stack!)
    // This must stay alive until MsQuic's worker thread processes it
    quicBuf->Buffer = buffer;
    quicBuf->Length = static_cast<uint32_t>(totalLen);
    
    // Send the datagram - pass owner as context for cleanup in callback
    QUIC_STATUS status = mApi->DatagramSend(mConnection, quicBuf, 1,
                                            QUIC_SEND_FLAG_NONE, owner);
    
    if (QUIC_FAILED(status)) {
        // Shift data back on failure
        std::memmove(buffer, buffer + varintLen, length);
        return false;
    }
    
    return true;
}

//*******************************************************************************
void WebTransportSession::close()
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mState == STATE_SHUTTING_DOWN || mState == STATE_DISCONNECTED) {
        return;
    }

    setState(STATE_SHUTTING_DOWN);

    if (mConnection && mApi && !mShutdownComplete) {
        // Initiate graceful shutdown - SHUTDOWN_COMPLETE callback will emit sessionClosed
        mApi->ConnectionShutdown(mConnection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    }
    
}

//*******************************************************************************
void WebTransportSession::setState(SessionState state)
{
    if (mState != state) {
        mState = state;
        emit stateChanged(state);
    }
}

//*******************************************************************************
void WebTransportSession::parseClientNameFromPath(const QString& path)
{
    QUrl url(QStringLiteral("http://localhost") + path);
    QUrlQuery query(url);

    if (query.hasQueryItem(QStringLiteral("name"))) {
        mClientName = query.queryItemValue(QStringLiteral("name"));
    } else if (query.hasQueryItem(QStringLiteral("client"))) {
        mClientName = query.queryItemValue(QStringLiteral("client"));
    }
}

//*******************************************************************************
void WebTransportSession::createInfrastructureStreams()
{
    // Called from QUIC_CONNECTION_EVENT_CONNECTED
    // Create all three HTTP/3 infrastructure streams:
    // 1. Control stream (type 0x00) - required for HTTP/3
    // 2. QPACK encoder stream (type 0x02) - required by browsers
    // 3. QPACK decoder stream (type 0x03) - required by browsers
    
    if (!createInfraStream(H3_STREAM_CONTROL, &mControlStream)) {
        cerr << "WebTransportSession: Failed to create control stream" << endl;
        return;
    }
    
    if (!createInfraStream(H3_STREAM_QPACK_ENCODER, &mQpackEncoderStream)) {
        cerr << "WebTransportSession: Failed to create QPACK encoder stream" << endl;
        return;
    }
    
    if (!createInfraStream(H3_STREAM_QPACK_DECODER, &mQpackDecoderStream)) {
        cerr << "WebTransportSession: Failed to create QPACK decoder stream" << endl;
        return;
    }
}

//*******************************************************************************
bool WebTransportSession::createInfraStream(H3StreamType type, HQUIC* streamHandle)
{
    if (!mConnection || !mApi || !streamHandle) {
        return false;
    }
    
    const char* streamNames[] = {"control", "push", "QPACK encoder", "QPACK decoder"};
    
    // Open a unidirectional stream with our callback handler
    // The callback will be invoked when START_COMPLETE fires
    QUIC_STATUS status = mApi->StreamOpen(
        mConnection, 
        QUIC_STREAM_OPEN_FLAG_UNIDIRECTIONAL,
        InfraStreamCallback,  // Use infrastructure stream callback
        this, 
        streamHandle);
        
    if (QUIC_FAILED(status)) {
        cerr << "WebTransportSession: Failed to open " << streamNames[type] 
             << " stream, status: 0x" << std::hex << status << std::dec << endl;
        return false;
    }
    
    // Start the stream - data will be sent when START_COMPLETE fires
    status = mApi->StreamStart(*streamHandle, QUIC_STREAM_START_FLAG_NONE);
    if (QUIC_FAILED(status)) {
        cerr << "WebTransportSession: Failed to start " << streamNames[type]
             << " stream, status: 0x" << std::hex << status << std::dec << endl;
        mApi->StreamClose(*streamHandle);
        *streamHandle = nullptr;
        return false;
    }
    
    return true;
}

//*******************************************************************************
void WebTransportSession::sendStreamType(HQUIC stream, H3StreamType type)
{
    if (!stream || !mApi) {
        return;
    }
    
    // Allocate buffer that will persist until SEND_COMPLETE
    // msquic requires the buffer to remain valid until the send completes
    uint8_t* typeData = new uint8_t[1];
    typeData[0] = static_cast<uint8_t>(type);
    
    QUIC_BUFFER* buffer = new QUIC_BUFFER;
    buffer->Buffer = typeData;
    buffer->Length = 1;
    
    // Send the stream type byte
    // Context is used to free the buffer in SEND_COMPLETE
    QUIC_STATUS status = mApi->StreamSend(stream, buffer, 1, 
                                          QUIC_SEND_FLAG_NONE, buffer);
    if (QUIC_FAILED(status)) {
        cerr << "WebTransportSession: Failed to send stream type, status: 0x"
             << std::hex << status << std::dec << endl;
        delete[] typeData;
        delete buffer;
    }
}

//*******************************************************************************
std::vector<uint8_t> WebTransportSession::buildSettingsFrame()
{
    // Build HTTP/3 SETTINGS frame
    std::vector<uint8_t> frame;
    
    // Build SETTINGS payload first to calculate length
    std::vector<uint8_t> settingsPayload;
    
    // SETTINGS_QPACK_MAX_TABLE_CAPACITY (0x01) = 0 (we don't use dynamic table)
    settingsPayload.push_back(0x01);
    settingsPayload.push_back(0x00);
    
    // SETTINGS_QPACK_BLOCKED_STREAMS (0x07) = 0
    settingsPayload.push_back(0x07);
    settingsPayload.push_back(0x00);
    
    // SETTINGS_ENABLE_CONNECT_PROTOCOL (0x08) = 1
    // Required for extended CONNECT (WebTransport)
    settingsPayload.push_back(0x08);
    settingsPayload.push_back(0x01);
    
    // SETTINGS_H3_DATAGRAM (0x33 = 51 decimal) = 1
    // This enables datagrams (HTTP Datagrams RFC 9297)
    settingsPayload.push_back(0x33);
    settingsPayload.push_back(0x01);
    
    // SETTINGS_ENABLE_WEBTRANSPORT (0x2b603742 = 729713730 decimal) = 1
    // Encoded as 4-byte varint: top 2 bits = 0b10 (4 byte)
    settingsPayload.push_back(0xab);  // 0b10101011 (2-bit prefix 10 = 4 bytes)
    settingsPayload.push_back(0x60);
    settingsPayload.push_back(0x37);
    settingsPayload.push_back(0x42);
    settingsPayload.push_back(0x01);  // value = 1 (enabled)
    
    // Frame type: 0x04 (SETTINGS)
    frame.push_back(0x04);
    
    // Frame length (varint)
    size_t payloadLen = settingsPayload.size();
    if (payloadLen < 64) {
        frame.push_back(static_cast<uint8_t>(payloadLen));
    } else {
        // Multi-byte varint (shouldn't happen for our small payload)
        frame.push_back(0x40 | static_cast<uint8_t>(payloadLen >> 8));
        frame.push_back(static_cast<uint8_t>(payloadLen & 0xFF));
    }
    
    // Append payload
    frame.insert(frame.end(), settingsPayload.begin(), settingsPayload.end());
    
    return frame;
}

//*******************************************************************************
void WebTransportSession::sendSettingsFrame()
{
    // Called after receiving client's SETTINGS and our control stream is ready
    if (mServerSettingsSent) {
        return;
    }
    
    if (!mControlStream || !mApi) {
        cerr << "WebTransportSession: Cannot send SETTINGS - control stream not available" << endl;
        return;
    }
    
    if (!mControlStreamReady) {
        return;
    }
    
    if (!mClientSettingsReceived) {
        return;
    }
    
    mServerSettingsSent = true;
    
    // Build the SETTINGS frame
    std::vector<uint8_t> settingsFrame = buildSettingsFrame();
    
    // Allocate buffer that persists until SEND_COMPLETE
    size_t dataSize = settingsFrame.size();
    uint8_t* frameData = new uint8_t[dataSize];
    std::memcpy(frameData, settingsFrame.data(), dataSize);
    
    QUIC_BUFFER* buffer = new QUIC_BUFFER;
    buffer->Buffer = frameData;
    buffer->Length = static_cast<uint32_t>(dataSize);
        
    QUIC_STATUS status = mApi->StreamSend(mControlStream, buffer, 1, 
                                          QUIC_SEND_FLAG_NONE, buffer);
    if (QUIC_FAILED(status)) {
        cerr << "WebTransportSession: Failed to send SETTINGS, status: 0x"
             << std::hex << status << std::dec << endl;
        mServerSettingsSent = false;
        delete[] frameData;
        delete buffer;
        return;
    }
}

//*******************************************************************************
std::vector<uint8_t> WebTransportSession::buildResponseFrame(int statusCode)
{
    // Build HTTP/3 HEADERS frame with :status response
    // Using QPACK static table encoding
    std::vector<uint8_t> frame;
    
    // Build QPACK-encoded headers
    std::vector<uint8_t> qpackPayload;
    
    // QPACK prefix: Required Insert Count = 0, Delta Base = 0
    qpackPayload.push_back(0x00);  // Required Insert Count (8-bit prefix)
    qpackPayload.push_back(0x00);  // Delta Base with S=0 (7-bit prefix)
    
    // Encode :status header using QPACK static table
    // Format: Indexed Header Field (1T pattern, T=1 for static table)
    // QPACK static table indices for :status:
    //   25 = :status: 200
    //   63 = :status: 100
    //   64 = :status: 204
    //   67 = :status: 400
    //   68 = :status: 403
    //   27 = :status: 404
    //   71 = :status: 500
    //   28 = :status: 503
    
    int staticIndex = -1;
    switch (statusCode) {
        case 200: staticIndex = 25; break;
        case 100: staticIndex = 63; break;
        case 204: staticIndex = 64; break;
        case 400: staticIndex = 67; break;
        case 403: staticIndex = 68; break;
        case 404: staticIndex = 27; break;
        case 500: staticIndex = 71; break;
        case 503: staticIndex = 28; break;
        default:
            // For other status codes, use literal encoding
            staticIndex = -1;
            break;
    }
    
    if (staticIndex >= 0 && staticIndex < 64) {
        // Indexed Header Field: 1 1 index (6-bit prefix)
        // Pattern: 11xxxxxx for static table index
        qpackPayload.push_back(0xC0 | static_cast<uint8_t>(staticIndex));
    } else {
        // Literal Header Field With Name Reference
        // Use static table index 24 for :status name (":status", "103")
        // Then provide literal value
        // Pattern: 0101 NNNN for static table name reference
        qpackPayload.push_back(0x5F);  // 01011111 = literal with name ref, static, index needs continuation
        qpackPayload.push_back(0x09);  // Index 24 encoded with 4-bit prefix (24 - 15 = 9)
        
        // Status value as literal string
        std::string statusStr = std::to_string(statusCode);
        qpackPayload.push_back(static_cast<uint8_t>(statusStr.length()));  // Length (no huffman)
        for (char c : statusStr) {
            qpackPayload.push_back(static_cast<uint8_t>(c));
        }
    }
    
    // Add sec-webtransport-http3-draft header for WebTransport compatibility
    // This is a literal header with literal name and value
    // QPACK literal with literal name format: 0010 NHLL
    //   N = never index (1), H = huffman for name (0), LL = 3-bit length prefix
    if (statusCode == 200) {
        const char* headerName = "sec-webtransport-http3-draft";
        const char* headerValue = "draft02";
        size_t nameLen = strlen(headerName);
        size_t valueLen = strlen(headerValue);
        
        // First byte: 0010 N H LL (N=1 never index, H=0 no huffman)
        // 0x2? where ? encodes the length with 3-bit prefix
        if (nameLen < 8) {
            qpackPayload.push_back(0x20 | static_cast<uint8_t>(nameLen));
        } else {
            // Length >= 8, need continuation
            qpackPayload.push_back(0x27);  // 0010 0111 = max 3-bit prefix value
            // Continuation: nameLen - 7 encoded as varint
            size_t remaining = nameLen - 7;
            while (remaining >= 128) {
                qpackPayload.push_back(0x80 | (remaining & 0x7F));
                remaining >>= 7;
            }
            qpackPayload.push_back(static_cast<uint8_t>(remaining));
        }
        
        // Name bytes (lowercase, no huffman)
        for (size_t i = 0; i < nameLen; i++) {
            qpackPayload.push_back(static_cast<uint8_t>(headerName[i]));
        }
        
        // Value length (7-bit prefix, H=0 no huffman)
        if (valueLen < 128) {
            qpackPayload.push_back(static_cast<uint8_t>(valueLen));
        } else {
            qpackPayload.push_back(0x7F);
            size_t remaining = valueLen - 127;
            while (remaining >= 128) {
                qpackPayload.push_back(0x80 | (remaining & 0x7F));
                remaining >>= 7;
            }
            qpackPayload.push_back(static_cast<uint8_t>(remaining));
        }
        
        // Value bytes
        for (size_t i = 0; i < valueLen; i++) {
            qpackPayload.push_back(static_cast<uint8_t>(headerValue[i]));
        }
    }
    
    // Build the HEADERS frame
    // Frame type: 0x01 (HEADERS)
    frame.push_back(0x01);
    
    // Frame length (varint)
    size_t payloadLen = qpackPayload.size();
    if (payloadLen < 64) {
        frame.push_back(static_cast<uint8_t>(payloadLen));
    } else {
        frame.push_back(0x40 | static_cast<uint8_t>(payloadLen >> 8));
        frame.push_back(static_cast<uint8_t>(payloadLen & 0xFF));
    }
    
    // Append QPACK payload
    frame.insert(frame.end(), qpackPayload.begin(), qpackPayload.end());
    
    return frame;
}

//*******************************************************************************
bool WebTransportSession::sendHttp3Response(int statusCode)
{
    if (!mConnectStream || !mApi) {
        cerr << "WebTransportSession: Cannot send HTTP/3 response - no CONNECT stream" << endl;
        return false;
    }
    
    // Build the response frame
    std::vector<uint8_t> responseFrame = buildResponseFrame(statusCode);
    
    // Allocate buffer that persists until SEND_COMPLETE
    size_t dataSize = responseFrame.size();
    uint8_t* frameData = new uint8_t[dataSize];
    std::memcpy(frameData, responseFrame.data(), dataSize);
    
    QUIC_BUFFER* buffer = new QUIC_BUFFER;
    buffer->Buffer = frameData;
    buffer->Length = static_cast<uint32_t>(dataSize);
    
    // Send the response - use the peer's CONNECT stream
    // The buffer context is used to free memory in the stream callback
    QUIC_STATUS status = mApi->StreamSend(mConnectStream, buffer, 1, 
                                          QUIC_SEND_FLAG_NONE, buffer);
    if (QUIC_FAILED(status)) {
        cerr << "WebTransportSession: Failed to send HTTP/3 response, status: 0x"
             << std::hex << status << std::dec << endl;
        delete[] frameData;
        delete buffer;
        return false;
    }
    
    return true;
}


//*******************************************************************************
unsigned int WebTransportSession::handleConnectionEvent(void* eventPtr)
{
    QUIC_CONNECTION_EVENT* event = static_cast<QUIC_CONNECTION_EVENT*>(eventPtr);

    switch (event->Type) {
        case QUIC_CONNECTION_EVENT_CONNECTED:
            if (gVerboseFlag) {
                cout << "WebTransportSession: QUIC connection established from "
                     << mPeerAddress.toString().toStdString() << ":" << mPeerPort << endl;
            }
            
            // Create HTTP/3 infrastructure streams immediately on connection
            // This follows the libwtf pattern - create control + QPACK streams right away
            // The streams will send their type byte when START_COMPLETE fires
            createInfrastructureStreams();

            // Connection is ready, but we wait for HTTP/3 CONNECT before declaring
            // session established
            break;

        case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
            // Common QUIC error codes for debugging:
            // 0x65 (101) = Connection refused or protocol error
            // 0x0A (10)  = No application protocol
            // 0x01 (1)   = Internal error
            setState(STATE_FAILED);
            // Cleanup will happen in SHUTDOWN_COMPLETE
            emit sessionFailed(QStringLiteral("Transport shutdown"));
            break;

        case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
            // Peer initiated shutdown - transition to SHUTTING_DOWN and wait for SHUTDOWN_COMPLETE
            // to emit the sessionClosed signal
            if (mState == STATE_CONNECTED) {
                setState(STATE_SHUTTING_DOWN);
            }
            break;

        case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE: {
            bool shouldEmitClosed = false;
            {
                // Set shutdown complete flag, check/update state, all protected by mutex
                std::lock_guard<std::mutex> lock(mMutex);
                mShutdownComplete = true;
                
                // Emit sessionClosed signal only if we're in SHUTTING_DOWN state
                // (i.e., close() was called but destructor hasn't run yet)
                // If state is DISCONNECTED, the destructor is running and we shouldn't emit
                if (mState == STATE_SHUTTING_DOWN) {
                    setState(STATE_DISCONNECTED);
                    shouldEmitClosed = true;
                }
            }  // mMutex released here
            
            // Notify outside the lock (standard pattern - avoids waking waiting thread while we hold lock)
            mShutdownCv.notify_all();
            
            // Emit signal outside the lock to avoid holding mutex during potentially slow signal/slot calls
            if (shouldEmitClosed) {
                emit sessionClosed();
            }

            break;
        }

        case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
            // A new stream was started by the peer
            // This could be the HTTP/3 control stream or the CONNECT stream
            if (mApi) {
                mApi->SetCallbackHandler(event->PEER_STREAM_STARTED.Stream,
                                         (void*)StreamCallback, this);
            }
            break;

        case QUIC_CONNECTION_EVENT_DATAGRAM_RECEIVED: {
            // Received a QUIC datagram - strip the quarter stream ID prefix
            const QUIC_BUFFER* buffer = event->DATAGRAM_RECEIVED.Buffer;
            if (buffer && buffer->Length > 0) {
                const uint8_t* data = buffer->Buffer;
                size_t len = buffer->Length;
                
                // Decode QUIC varint to get prefix length
                size_t varintLen = 0;
                if (len > 0) {
                    uint8_t firstByte = data[0];
                    uint8_t prefix = firstByte >> 6;
                    varintLen = 1 << prefix;  // 1, 2, 4, or 8 bytes
                }
                
                // Skip the quarter stream ID prefix and deliver directly (zero-copy)
                if (len > varintLen) {
                    const uint8_t* payload = data + varintLen;
                    size_t payloadLen = len - varintLen;
                    
                    // Direct callback invocation (audio hot path, no Qt overhead)
                    if (mDatagramCallback) {
                        mDatagramCallback(payload, payloadLen);
                    }
                }
            }
            break;
        }

        case QUIC_CONNECTION_EVENT_DATAGRAM_STATE_CHANGED:
            // Update maximum datagram size based on path MTU
            if (event->DATAGRAM_STATE_CHANGED.SendEnabled) {
                mMaxDatagramSize = event->DATAGRAM_STATE_CHANGED.MaxSendLength;
            } else {
                cerr << "WebTransportSession: WARNING - Datagrams NOT enabled by peer!" << endl;
            }
            break;

        case QUIC_CONNECTION_EVENT_DATAGRAM_SEND_STATE_CHANGED:
            // Datagram send state changed - only release buffer on final states
            // States: 0=Sent, 1=LostSuspect, 2=LostDiscarded, 3=Acknowledged, 
            //         4=AcknowledgedSpurious, 5=Canceled
            // Final states are >= 2 (LostDiscarded, Acknowledged, AcknowledgedSpurious, Canceled)
            if (event->DATAGRAM_SEND_STATE_CHANGED.State >= QUIC_DATAGRAM_SEND_LOST_DISCARDED) {
                void* ctx = event->DATAGRAM_SEND_STATE_CHANGED.ClientContext;
                if (ctx && reinterpret_cast<uintptr_t>(ctx) > 0x10000) {
                    // This is a SendContext from the pool - release it
                    WebTransportDataProtocol::releaseSendContext(
                        static_cast<WebTransportDataProtocol::SendContext*>(ctx));
                }
            }
            break;

        case QUIC_CONNECTION_EVENT_IDEAL_PROCESSOR_CHANGED:
            // CPU affinity hint - can be safely ignored
            break;

        case QUIC_CONNECTION_EVENT_STREAMS_AVAILABLE:
            if (gVerboseFlag) {
                cout << "WebTransportSession: STREAMS_AVAILABLE - Bidi: " 
                     << event->STREAMS_AVAILABLE.BidirectionalCount
                     << ", Unidi: " << event->STREAMS_AVAILABLE.UnidirectionalCount << endl;
            }
            break;

        case QUIC_CONNECTION_EVENT_PEER_NEEDS_STREAMS:
            if (gVerboseFlag) {
                cout << "WebTransportSession: PEER_NEEDS_STREAMS - Bidi: "
                     << (event->PEER_NEEDS_STREAMS.Bidirectional ? "yes" : "no") << endl;
            }
            break;

        case QUIC_CONNECTION_EVENT_RESUMED:
            if (gVerboseFlag) {
                cout << "WebTransportSession: CONNECTION_RESUMED" << endl;
            }
            break;

        case QUIC_CONNECTION_EVENT_RESUMPTION_TICKET_RECEIVED:
            if (gVerboseFlag) {
                cout << "WebTransportSession: RESUMPTION_TICKET_RECEIVED" << endl;
            }
            break;

        case QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED:
            if (gVerboseFlag) {
                cout << "WebTransportSession: PEER_CERTIFICATE_RECEIVED" << endl;
            }
            break;

        default:
            if (gVerboseFlag) {
                cout << "WebTransportSession: Connection event type: " 
                     << event->Type << " (unhandled)" << endl;
            }
            break;
    }

    return QUIC_STATUS_SUCCESS;
}

//*******************************************************************************
unsigned int WebTransportSession::handleStreamEvent(HQUIC stream, void* eventPtr)
{
    QUIC_STREAM_EVENT* event = static_cast<QUIC_STREAM_EVENT*>(eventPtr);

    switch (event->Type) {
        case QUIC_STREAM_EVENT_START_COMPLETE:
            break;

        case QUIC_STREAM_EVENT_RECEIVE: {
            // Received data on stream - could be HTTP/3 frames
            // For WebTransport, we primarily use datagrams for audio
            // but streams are used for signaling (CONNECT request)

            // Check for FIN flag indicating stream closure
            bool finReceived = (event->RECEIVE.Flags & QUIC_RECEIVE_FLAG_FIN) != 0;
            
            // First, determine if this is a unidirectional or bidirectional stream
            // In QUIC: stream_id bit 1 (0x02) = 1 means unidirectional
            uint64_t streamId = 0;
            uint32_t streamIdSize = sizeof(streamId);
            bool isUnidirectional = false;
            
            if (mApi) {
                QUIC_STATUS status = mApi->GetParam(stream, QUIC_PARAM_STREAM_ID, 
                                                    &streamIdSize, &streamId);
                if (QUIC_SUCCEEDED(status)) {
                    isUnidirectional = (streamId & 0x02) != 0;
                }
            }

            // If this is the CONNECT stream and we received FIN after being connected,
            // treat it as a client disconnect
            if (finReceived && stream == mConnectStream && mState == STATE_CONNECTED) {
                if (gVerboseFlag) {
                    cout << "WebTransportSession: Client closing CONNECT stream (FIN received)" << endl;
                }
                // Client is disconnecting - close the connection gracefully
                close();
                break;
            }

            // Parse the HTTP/3 CONNECT request from the stream data
            if (event->RECEIVE.TotalBufferLength > 0) {
                QByteArray data;
                for (uint32_t i = 0; i < event->RECEIVE.BufferCount; i++) {
                    data.append(reinterpret_cast<const char*>(
                                    event->RECEIVE.Buffers[i].Buffer),
                                event->RECEIVE.Buffers[i].Length);
                }

                // Handle unidirectional streams (control, QPACK encoder/decoder)
                // These start with a stream type byte
                if (isUnidirectional && data.size() > 0) {
                    uint8_t streamType = static_cast<uint8_t>(data[0]);
                    
                    // Check for HTTP/3 unidirectional stream types
                    // 0x00 = Control, 0x02 = QPACK Encoder, 0x03 = QPACK Decoder
                    if (streamType == 0x00 || streamType == 0x02 || streamType == 0x03) {
                        // If this is the client's control stream, look for SETTINGS frame
                        if (streamType == 0x00 && data.size() > 1) {
                            // Parse SETTINGS frame after stream type byte
                            size_t pos = 1;  // Skip stream type byte
                            int64_t frameType = readVarint(
                                reinterpret_cast<const uint8_t*>(data.constData()), 
                                data.size(), pos);
                            
                            if (frameType == HTTP3_FRAME_SETTINGS) {
                                // Read frame length
                                int64_t frameLen = readVarint(
                                    reinterpret_cast<const uint8_t*>(data.constData()),
                                    data.size(), pos);
                                    
                                if (frameLen >= 0) {
                                    // Mark client settings as received
                                    mClientSettingsReceived = true;
                                    
                                    // Now we can send our SETTINGS in response
                                    // (if control stream is ready)
                                    sendSettingsFrame();
                                }
                            }
                        }
                        
                        // For infrastructure streams, don't process as CONNECT request
                        break;
                    }
                    
                    // Unknown unidirectional stream type - log and ignore
                    if (gVerboseFlag) {
                        cout << "WebTransportSession: Unknown unidirectional stream type: 0x" 
                             << std::hex << static_cast<int>(streamType) << std::dec << endl;
                    }
                    break;
                }

                // Bidirectional stream - this should be the CONNECT request
                // These start directly with HTTP/3 frames (no stream type byte)

                // Parse HTTP/3 frame to extract QPACK payload
                const uint8_t* qpackPayload = nullptr;
                size_t qpackLen = 0;
                
                if (!parseHttp3Frame(reinterpret_cast<const uint8_t*>(data.constData()),
                                    data.size(), qpackPayload, qpackLen)) {
                    cerr << "WebTransportSession: Failed to parse HTTP/3 frame (not a request stream?)" << endl;
                    break;
                }

                // Decode QPACK-encoded HTTP/3 headers
                QMap<QString, QString> headers;
                
                if (decodeQPackHeaders(qpackPayload, qpackLen, headers)) {
                    // Check if this is a CONNECT request for WebTransport
                    QString method = headers.value(QStringLiteral(":method"));
                    QString protocol = headers.value(QStringLiteral(":protocol"));
                    QString path = headers.value(QStringLiteral(":path"), 
                                                 QStringLiteral("/webtransport"));
                    QString authority = headers.value(QStringLiteral(":authority"));
                    
                    if (gVerboseFlag) {
                        cout << "WebTransportSession: Request headers:" << endl;
                        cout << "  :method = " << method.toStdString() << endl;
                        cout << "  :protocol = " << protocol.toStdString() << endl;
                        cout << "  :path = " << path.toStdString() << endl;
                        cout << "  :authority = " << authority.toStdString() << endl;
                    }
                    
                    if (method == QStringLiteral("CONNECT") && 
                        protocol == QStringLiteral("webtransport")) {
                        
                        // Store the CONNECT stream handle and ID for sending the response
                        // and for the quarter stream ID in datagrams
                        mConnectStream = stream;
                        mConnectStreamId = streamId;
                        
                        if (processConnectRequest(path)) {
                            sendConnectResponse(200);
                        } else {
                            sendConnectResponse(400);
                        }
                    } else {
                        cerr << "WebTransportSession: Invalid request - expected CONNECT with webtransport protocol" << endl;
                        cerr << "  Got method=" << method.toStdString() 
                             << ", protocol=" << protocol.toStdString() << endl;
                        // Store stream for error response too
                        mConnectStream = stream;
                        sendConnectResponse(400);
                    }
                } else {
                    cerr << "WebTransportSession: Failed to decode QPACK headers" << endl;
                }
            }
            break;
        }

        case QUIC_STREAM_EVENT_SEND_COMPLETE:
            // Stream send completed - free the buffer we allocated
            if (event->SEND_COMPLETE.ClientContext) {
                QUIC_BUFFER* buffer = static_cast<QUIC_BUFFER*>(
                    event->SEND_COMPLETE.ClientContext);
                if (buffer) {
                    delete[] buffer->Buffer;
                    delete buffer;
                }
            }
            break;

        case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
            if (gVerboseFlag) {
                cout << "WebTransportSession: PEER_SEND_SHUTDOWN on stream " << stream;
                if (stream == mConnectStream) cout << " (CONNECT stream)";
                cout << endl;
            }
            // If the CONNECT stream is shut down by peer, treat as disconnect
            if (stream == mConnectStream && mState == STATE_CONNECTED) {
                if (gVerboseFlag) {
                    cout << "WebTransportSession: Client disconnected (CONNECT stream shutdown)" << endl;
                }
                close();
            }
            break;

        case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
            if (gVerboseFlag) {
                cout << "WebTransportSession: PEER_SEND_ABORTED on stream " << stream
                     << " (error code: " << event->PEER_SEND_ABORTED.ErrorCode << ")";
                if (stream == mConnectStream) cout << " (CONNECT stream)";
                cout << endl;
            }
            // If the CONNECT stream is aborted by peer, treat as disconnect
            if (stream == mConnectStream && mState == STATE_CONNECTED) {
                if (gVerboseFlag) {
                    cout << "WebTransportSession: Client disconnected (CONNECT stream aborted)" << endl;
                }
                close();
            }
            break;

        case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
            if (gVerboseFlag) {
                cout << "WebTransportSession: STREAM_SHUTDOWN_COMPLETE on stream " << stream;
                if (stream == mConnectStream) {
                    cout << " (CONNECT stream - THIS SHOULD NOT HAPPEN DURING ACTIVE SESSION!)";
                }
                cout << endl;
            }
            if (mApi) {
                mApi->StreamClose(stream);
            }
            break;

        default:
            if (gVerboseFlag) {
            // Log unhandled stream events for debugging
                cout << "WebTransportSession: Unhandled stream event type: " 
                     << event->Type << endl;
            }
            break;
    }

    return QUIC_STATUS_SUCCESS;
}

//*******************************************************************************
unsigned int WebTransportSession::handleInfraStreamEvent(HQUIC stream, void* eventPtr)
{
    QUIC_STREAM_EVENT* event = static_cast<QUIC_STREAM_EVENT*>(eventPtr);

    switch (event->Type) {
        case QUIC_STREAM_EVENT_START_COMPLETE: {
            // Stream is now ready - we can send data on it
            QUIC_STATUS status = event->START_COMPLETE.Status;
            
            if (QUIC_FAILED(status)) {
                cerr << "WebTransportSession: Infrastructure stream start failed, status: 0x"
                     << std::hex << status << std::dec << endl;
                return QUIC_STATUS_SUCCESS;
            }
            
            // Determine which stream this is and mark it ready
            if (stream == mControlStream) {
                mControlStreamReady = true;
                
                // Send stream type byte (0x00 for control)
                sendStreamType(stream, H3_STREAM_CONTROL);
                
                // Try to send SETTINGS if client settings already received
                if (mClientSettingsReceived && !mServerSettingsSent) {
                    sendSettingsFrame();
                }
            } else if (stream == mQpackEncoderStream) {
                mQpackEncoderStreamReady = true;
                
                // Send stream type byte (0x02 for QPACK encoder)
                sendStreamType(stream, H3_STREAM_QPACK_ENCODER);
            } else if (stream == mQpackDecoderStream) {
                mQpackDecoderStreamReady = true;
                
                // Send stream type byte (0x03 for QPACK decoder)
                sendStreamType(stream, H3_STREAM_QPACK_DECODER);
            }
            break;
        }
        
        case QUIC_STREAM_EVENT_SEND_COMPLETE: {
            // Free the buffer we allocated for sending
            if (event->SEND_COMPLETE.ClientContext) {
                QUIC_BUFFER* buffer = static_cast<QUIC_BUFFER*>(
                    event->SEND_COMPLETE.ClientContext);
                if (buffer) {
                    delete[] buffer->Buffer;
                    delete buffer;
                }
            }
            break;
        }
        
        case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
            break;
            
        case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
            cerr << "WebTransportSession: Infrastructure stream peer aborted (error: "
                 << event->PEER_SEND_ABORTED.ErrorCode << ")" << endl;
            break;
            
        case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
            // Don't close the stream handle here - cleanup() will handle it
            break;
            
        default:
            break;
    }
    
    return QUIC_STATUS_SUCCESS;
}
