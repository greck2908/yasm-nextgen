//
// String composition.
//
//  Copyright (C) 2007  Peter Johnson
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
#include "yasmx/Support/Compose.h"

#include <cctype>

using namespace String;

Composer::~Composer()
{
}

Composer::Composer(const Composer& rhs)
    : m_fmt(rhs.m_fmt),
      m_os(m_ss),
      m_arg(rhs.m_arg)
{
    m_os << rhs.m_os.str();
    for (int i=0; i<10; i++)
        m_arg_pos[i] = rhs.m_arg_pos[i];
}

Composer&
Composer::operator=(const Composer& rhs)
{
    m_fmt = rhs.m_fmt;
    m_os.flush();
    m_ss.clear();
    m_os << rhs.m_os.str();
    m_arg = rhs.m_arg;
    for (int i=0; i<10; i++)
        m_arg_pos[i] = rhs.m_arg_pos[i];
    return *this;
}

std::string
Composer::getStr() const
{
    llvm::StringRef args = m_os.str();

    std::string str;
    // Estimate the final string size
    str.reserve(m_fmt.size() + args.size());

    // fill in output with the strings between the %1 %2 %3 etc. and
    // fill in specs with the arg data
    size_t start = 0, stop;
    while ((stop = m_fmt.substr(start).find('%')) != llvm::StringRef::npos)
    {
        stop += start;  // offset from start of original string
        llvm::StringRef before = m_fmt.slice(start, stop);
        str.append(before.data(), before.size());
        if (m_fmt[stop + 1] == '%')     // catch %%
        {
            str += '%';
            start = stop + 2;
        }
        else if (std::isdigit(m_fmt[stop + 1]))
        {
            int spec_no = m_fmt[stop + 1] - '0';
            if (spec_no > 0 && spec_no < m_arg)
            {
                str.append(args, m_arg_pos[spec_no-1],
                           m_arg_pos[spec_no] - m_arg_pos[spec_no-1]);
            }
            else if (spec_no == m_arg)
            {
                str.append(args, m_arg_pos[spec_no-1],
                           args.size() - m_arg_pos[spec_no-1] + 1);
            }
            else
            {
                str += m_fmt[stop];
                str += m_fmt[stop+1];
            }
            start = stop + 2;
        }
        else
        {
            str += m_fmt[stop];
            start = stop + 1;
        }
    }

    // tail end of string
    llvm::StringRef after = m_fmt.substr(start);
    str.append(after.data(), after.size());

    return str;
}