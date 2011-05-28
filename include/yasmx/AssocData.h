#ifndef YASM_ASSOCDATA_H
#define YASM_ASSOCDATA_H
///
/// @file
/// @brief Associated data interface.
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
#include <memory>

#include "llvm/ADT/SmallVector.h"
#include "yasmx/Config/export.h"
#include "yasmx/DebugDumper.h"


namespace yasm
{

/// Associated data interface (abstract base).
class YASM_LIB_EXPORT AssocData : public DebugDumper<AssocData>
{
public:
    AssocData() {}

    /// Destructor.
    virtual ~AssocData();

    /// Write a YAML representation.  For debugging purposes.
    /// @param out          YAML emitter
    virtual void Write(YAML::Emitter& out) const = 0;

private:
    AssocData(const AssocData&);                    // not implemented
    const AssocData& operator=(const AssocData&);   // not implemented
};

/// Dump a YAML representation of associated data.  For debugging purposes.
/// @param out          YAML emitter
/// @param data         associated data
/// @return Emitter.
inline YAML::Emitter&
operator<< (YAML::Emitter& out, const AssocData& data)
{
    data.Write(out);
    return out;
}

/// Associated data container.
class YASM_LIB_EXPORT AssocDataContainer
    : public DebugDumper<AssocDataContainer>
{
    struct AssocMapEntry
    {
        const void* key;
        AssocData* value;
    };
    typedef llvm::SmallVector<AssocMapEntry, 1> AssocMap;
    AssocMap m_assoc_map;

public:
    AssocDataContainer();
    virtual ~AssocDataContainer();

    std::auto_ptr<AssocData> AddAssocData(const void* key,
                                          std::auto_ptr<AssocData> data);

    AssocData* getAssocData(const void* key);
    const AssocData* getAssocData(const void* key) const;

    template <typename T>
    std::auto_ptr<AssocData> AddAssocData(std::auto_ptr<T> data)
    {
        return AddAssocData(T::key, std::auto_ptr<AssocData>(data.release()));
    }

    template <typename T>
    T* getAssocData()
    {
        return static_cast<T*>(getAssocData(T::key));
    }
    template <typename T>
    const T* getAssocData() const
    {
        return static_cast<const T*>(getAssocData(T::key));
    }

    /// Write a YAML representation.  For debugging purposes.
    /// @param out          YAML emitter
    virtual void Write(YAML::Emitter& out) const = 0;
};

/// Dump a YAML representation of associated data container.
/// For debugging purposes.
/// @param out          YAML emitter
/// @param container    associated data container
/// @return Emitter.
inline YAML::Emitter&
operator<< (YAML::Emitter& out, const AssocDataContainer& container)
{
    container.Write(out);
    return out;
}

} // namespace yasm

#endif
