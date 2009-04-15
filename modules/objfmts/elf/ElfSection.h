#ifndef YASM_ELFSECTION_H
#define YASM_ELFSECTION_H
//
// ELF object format section
//
//  Copyright (C) 2003-2007  Michael Urman
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

#include <iosfwd>
#include <vector>

#include <yasmx/Support/marg_ostream_fwd.h>
#include <yasmx/AssocData.h>
#include <yasmx/IntNum.h>
#include <yasmx/Section.h>
#include <yasmx/SymbolRef.h>

#include "ElfTypes.h"


namespace yasm
{

class Bytes;
class Errwarns;
class Section;
class StringTable;

namespace objfmt
{
namespace elf
{

class ElfSection : public AssocData
{
public:
    static const char* key;

    // Constructor that reads from file.  Assumes input stream is already
    // positioned at the beginning of the section header.
    ElfSection(const ElfConfig&     config,
               std::istream&        is,
               ElfSectionIndex      index);

    ElfSection(const ElfConfig&     config,
               ElfSectionType       type,
               ElfSectionFlags      flags,
               bool                 symtab = false);

    ~ElfSection();

    void put(marg_ostream& os) const;

    unsigned long write(std::ostream& os, Bytes& scratch) const;

    std::auto_ptr<Section> create_section(const StringTable& shstrtab) const;
    void load_section_data(Section& sect, std::istream& is) const;

    ElfSectionType get_type() const { return m_type; }

    void set_name(ElfStringIndex index) { m_name_index = index; }
    ElfStringIndex get_name() const { return m_name_index; }

    void set_typeflags(ElfSectionType type, ElfSectionFlags flags)
    {
        m_type = type;
        m_flags = flags;
    }
    ElfSectionFlags get_flags() const { return m_flags; }

    bool is_empty() const { return m_size.is_zero(); }

    SymbolRef get_sym() const { return m_sym; }

    unsigned long get_align() const { return m_align; }
    void set_align(unsigned long align) { m_align = align; }

    ElfSectionIndex get_index() { return m_index; }

    void set_info(ElfSectionInfo info) { m_info = info; }
    ElfSectionInfo get_info() const { return m_info; }

    void set_index(ElfSectionIndex sectidx) { m_index = sectidx; }

    void set_link(ElfSectionIndex link) { m_link = link; }
    ElfSectionIndex get_link() const { return m_link; }

    void set_rel_index(ElfSectionIndex sectidx) { m_rel_index = sectidx; }
    void set_rel_name(ElfStringIndex nameidx) { m_rel_name_index = nameidx; }

    void set_entsize(ElfSize size) { m_entsize = size; }
    ElfSize get_entsize() const { return m_entsize; }

    void set_sym(SymbolRef sym) { m_sym = sym; }

    void add_size(const IntNum& size) { m_size += size; }
    void set_size(const IntNum& size) { m_size = size; }
    IntNum get_size() const { return m_size; }

    unsigned long write_rel(std::ostream& os,
                            ElfSectionIndex symtab,
                            Section& sect,
                            Bytes& scratch);
    unsigned long write_relocs(std::ostream& os,
                               Section& sect,
                               Errwarns& errwarns,
                               Bytes& scratch,
                               const ElfMachine& machine);
    bool read_relocs(std::istream& is,
                     Section& sect,
                     unsigned long size,
                     const ElfMachine& machine,
                     const ElfSymtab& symtab,
                     bool rela) const;

    unsigned long set_file_offset(unsigned long pos);
    unsigned long get_file_offset() const { return m_offset; }

private:
    const ElfConfig&    m_config;

    ElfSectionType      m_type;
    ElfSectionFlags     m_flags;
    IntNum              m_addr;
    ElfAddress          m_offset;
    IntNum              m_size;
    ElfSectionIndex     m_link;
    ElfSectionInfo      m_info;         // see note ESD1
    unsigned long       m_align;
    ElfSize             m_entsize;

    SymbolRef           m_sym;
    ElfStringIndex      m_name_index;
    ElfSectionIndex     m_index;

    ElfStringIndex      m_rel_name_index;
    ElfSectionIndex     m_rel_index;
    ElfAddress          m_rel_offset;
};

inline ElfSection*
get_elf(Section& sym)
{
    return static_cast<ElfSection*>(sym.get_assoc_data(ElfSection::key));
}

// Note ESD1:
//   for section types SHT_REL, SHT_RELA:
//     link -> index of associated symbol table
//     info -> index of relocated section
//   for section types SHT_SYMTAB, SHT_DYNSYM:
//     link -> index of associated string table
//     info -> 1+index of last "local symbol" (bind == STB_LOCAL)
//  (for section type SHT_DNAMIC:
//     link -> index of string table
//     info -> 0 )
//  (for section type SHT_HASH:
//     link -> index of symbol table to which hash applies
//     info -> 0 )
//   for all others:
//     link -> SHN_UNDEF
//     info -> 0

}}} // namespace yasm::objfmt::elf

#endif