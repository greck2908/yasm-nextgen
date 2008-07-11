#ifndef YASM_BYTES_H
#define YASM_BYTES_H
///
/// @file
/// @brief Bytes interface.
///
/// @license
///  Copyright (C) 2007  Peter Johnson
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions
/// are met:
///  - Redistributions of source code must retain the above copyright
///    notice, this list of conditions and the following disclaimer.
///  - Redistributions in binary form must reproduce the above copyright
///    notice, this list of conditions and the following disclaimer in the
///    documentation and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
/// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
/// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
/// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
/// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
/// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
/// POSSIBILITY OF SUCH DAMAGE.
/// @endlicense
///
#include <iosfwd>
#include <vector>

#include "export.h"
#include "marg_ostream_fwd.h"


namespace yasm
{

class YASM_LIB_EXPORT Bytes : public std::vector<unsigned char>
{
public:
    Bytes(bool bigendian=false);
    ~Bytes();

    void set_bigendian(bool bigendian) { m_bigendian = bigendian; }
    bool is_bigendian() const { return m_bigendian; }

    /// Copy from an input stream, appending the values to the end.
    /// @param  is  input stream
    /// @param  n   number of bytes
    void write(std::istream& is, size_type n);

    /// Copy from a byte array, appending the values to the end.
    /// @param  buf input buffer
    /// @param  n   number of bytes
    void write(const unsigned char* buf, size_type n);

    /// Append n bytes of value v.
    /// @param  n   number of bytes
    /// @param  v   byte value
    void write(size_type n, unsigned char v);

private:
    bool m_bigendian;
};

struct SetEndian
{
    bool m_bigendian;
};

inline SetEndian
set_endian(bool bigendian)
{
    SetEndian x;
    x.m_bigendian = bigendian;
    return x;
}

inline Bytes&
operator<< (Bytes& bytes, const SetEndian& sete)
{
    bytes.set_bigendian(sete.m_bigendian);
    return bytes;
}

inline Bytes&
big_endian(Bytes& bytes)
{
    bytes.set_bigendian(true);
    return bytes;
}

inline Bytes&
little_endian(Bytes& bytes)
{
    bytes.set_bigendian(false);
    return bytes;
}

/// Output the entire contents of a bytes container to an output stream.
/// @param os    output stream
/// @param bytes bytes
/// @return Output stream
YASM_LIB_EXPORT
std::ostream& operator<< (std::ostream& os, const Bytes& bytes);

/// Output a bytes container in user-readable format to an debug output stream.
/// @param os    debug stream
/// @param bytes bytes
/// @return Debug stream
YASM_LIB_EXPORT
marg_ostream& operator<< (marg_ostream& os, const Bytes& bytes);

} // namespace yasm

#endif
