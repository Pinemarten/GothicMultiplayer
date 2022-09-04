
/*
MIT License

Copyright (c) 2022 Gothic Multiplayer Team (pampi, skejt23, mecio)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _CCompression_H
#define _CCompression_H

#include <cstdint>

#include "singleton.h"
#include "zlib.h"

class CCompression : public TSingleton<CCompression> {
public:
  enum {
    DEFLATE = 0,
    DEFAULT,
  };
  enum {
    DT_BINARY = 0,
    DT_TEXT,
    DT_DEFAULT,
  };
  CCompression(void);
  ~CCompression(void);
  void ChangeType(std::uint8_t type);
  std::uint32_t Compress(std::uint8_t *src, std::uint8_t *dst, std::uint32_t src_len, std::uint32_t &dst_len,
                         std::uint8_t method = DEFLATE);
  std::uint32_t Decompress(std::uint8_t *src, std::uint8_t *dst, std::uint32_t src_len, std::uint32_t &dst_len,
                           std::uint8_t method = DEFLATE);

private:
  std::uint8_t ratio, data_type;
  z_stream strm;
};

#endif