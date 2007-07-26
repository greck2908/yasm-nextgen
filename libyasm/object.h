#ifndef YASM_OBJECT_H
#define YASM_OBJECT_H
///
/// @file libyasm/object.h
/// @brief YASM object interface.
///
/// @license
///  Copyright (C) 2001-2007  Peter Johnson
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
#include <string>
#include <iostream>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace yasm {

class Arch;
class Section;
class Errwarns;
class ObjectFormat;
class DebugFormat;
class ObjectFormatModule;
class DebugFormatModule;

/// An object.  This is the internal representation of an object file.
class Object : private boost::noncopyable {
public:
    /// Constructor.  A default section is created as the first
    /// section.  An empty symbol table and line mapping are also
    /// automatically created.
    /// @param src_filename     source filename (e.g. "file.asm")
    /// @param obj_filename     object filename (e.g. "file.o")
    /// @param arch             architecture
    /// @param objfmt_module    object format module
    /// @param dbgfmt_module    debug format module
    Object(const std::string& src_filename,
           const std::string& obj_filename,
           std::auto_ptr<Arch> arch,
           const ObjectFormatModule* objfmt_module,
           const DebugFormatModule* dbgfmt_module);

    /// Destructor.
    ~Object();

    /// Print an object.  For debugging purposes.
    /// @param os           output stream
    /// @param indent_level indentation level
    void put(std::ostream& os, int indent_level) const;

    /// Add a new section.  Does /not/ check to see if there's already
    /// an existing section in the object with that name.  The caller
    /// should first call get_section_by_name() if only unique names
    /// are desired.
    /// @param sect         section
    void append_section(std::auto_ptr<Section> sect);

    /// Find a general section in an object, based on its name.
    /// @param name         section name
    /// @return Section matching name, or NULL if no match found.
    /*@null@*/ Section* get_section_by_name(const std::string& name);

    /// Finalize an object after parsing.
    /// @param errwarns     error/warning set
    /// @note Errors/warnings are stored into errwarns.
    void finalize(Errwarns& errwarns);

    /// Change the source filename for an object.
    /// @param src_filename new source filename (e.g. "file.asm")
    void set_source_fn(const std::string& src_filename);

    /// Optimize an object.  Takes the unoptimized object and optimizes it.
    /// If successful, the object is ready for output to an object file.
    /// @param errwarns     error/warning set
    /// @note Optimization failures are stored into errwarns.
    void optimize(Errwarns& errwarns);

    typedef boost::ptr_vector<Section>::iterator section_iterator;
    typedef boost::ptr_vector<Section>::const_iterator const_section_iterator;

    section_iterator sections_begin() { return m_sections.begin(); }
    const_section_iterator sections_begin() const
    { return m_sections.begin(); }

    section_iterator sections_end() { return m_sections.end(); }
    const_section_iterator sections_end() const { return m_sections.end(); }

private:
    std::string m_src_filename;         ///< Source filename
    std::string m_obj_filename;         ///< Object filename

    // /*@owned@*/ yasm_symtab *symtab;         ///< Symbol table
    boost::scoped_ptr<Arch> m_arch;             ///< Target architecture
    boost::scoped_ptr<ObjectFormat> m_objfmt;   ///< Object format
    boost::scoped_ptr<DebugFormat> m_dbgfmt;    ///< Debug format

    /// Currently active section.  Used by some directives.  NULL if no
    /// section active.
    /*@null@*/ Section* m_cur_section;

    /// Sections
    boost::ptr_vector<Section> m_sections;

    /// Directives, organized as two level HAMT; first level is parser,
    /// second level is directive name.
    // /*@owned@*/ struct HAMT *directives;
};

#if 0
/** Handle a directive.  Passed down to object format, debug format, or
 * architecture as appropriate.
 * \param object                object
 * \param name                  directive name
 * \param parser                parser keyword
 * \param valparams             value/parameters
 * \param objext_valparams      "object format-specific" value/parameters
 * \param line                  virtual line (from yasm_linemap)
 * \return 0 if directive recognized, nonzero if unrecognized.
 */
int yasm_object_directive(Object* object,
                          const std::string& name,
                          const std::string& parser,
                          yasm_valparamhead *valparams,
                          yasm_valparamhead *objext_valparams,
                          unsigned long line);

/** Traverses all sections in an object, calling a function on each section.
 * \param object        object
 * \param d             data pointer passed to func on each call
 * \param func          function
 * \return Stops early (and returns func's return value) if func returns a
 *         nonzero value; otherwise 0.
 */
int yasm_object_sections_traverse
    (yasm_object *object, /*@null@*/ void *d,
     int (*func) (yasm_section *sect, /*@null@*/ void *d));
#endif

} // namespace yasm

#endif