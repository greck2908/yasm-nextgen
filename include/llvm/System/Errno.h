//===- llvm/System/Errno.h - Portable+convenient errno handling -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares some portable and convenient functions to deal with errno.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SYSTEM_ERRNO_H
#define LLVM_SYSTEM_ERRNO_H

#include "yasmx/Config/export.h"
#include <string>

namespace llvm {
namespace sys {

/// Returns a string representation of the errno value, using whatever
/// thread-safe variant of strerror() is available.  Be sure to call this
/// immediately after the function that set errno, or errno may have been
/// overwritten by an intervening call.
YASM_LIB_EXPORT
std::string StrError();

/// Like the no-argument version above, but uses \p errnum instead of errno.
YASM_LIB_EXPORT
std::string StrError(int errnum);

}  // namespace sys
}  // namespace llvm

#endif  // LLVM_SYSTEM_ERRNO_H
