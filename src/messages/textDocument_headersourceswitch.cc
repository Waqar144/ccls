// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#include "message_handler.hh"
#include "query.hh"

#include <llvm/ADT/iterator_range.h>

#include <llvm/Support/Path.h>
#include <llvm/Support/VirtualFileSystem.h>

using namespace llvm;

namespace ccls {

using PathRef = llvm::StringRef;

void MessageHandler::textDocument_switchHeaderSource(TextDocumentParam &doc, ReplyOnce &reply)
{
    auto path = doc.textDocument.uri.getPath();

    llvm::StringRef SourceExtensions[] = {".cpp", ".c", ".cc", ".cxx",
                                          ".c++", ".m", ".mm"};
    llvm::StringRef HeaderExtensions[] = {".h", ".hh", ".hpp", ".hxx", ".inc"};


    llvm::StringRef PathExt = llvm::sys::path::extension(path);


    // Lookup in a list of known extensions.
    auto SourceIter =
        llvm::find_if(SourceExtensions, [&PathExt](PathRef SourceExt) {
          return SourceExt.equals_lower(PathExt);
        });
    bool IsSource = SourceIter != std::end(SourceExtensions);
    auto HeaderIter =
        llvm::find_if(HeaderExtensions, [&PathExt](PathRef HeaderExt) {
          return HeaderExt.equals_lower(PathExt);
        });
    bool IsHeader = HeaderIter != std::end(HeaderExtensions);
    // We can only switch between the known extensions.
    if (!IsSource && !IsHeader)
      return;
    // Array to lookup extensions for the switch. An opposite of where original
    // extension was found.
    llvm::ArrayRef<llvm::StringRef> NewExts;
    if (IsSource)
      NewExts = HeaderExtensions;
    else
      NewExts = SourceExtensions;

    std::unique_ptr<llvm::vfs::FileSystem> VFS = llvm::vfs::createPhysicalFileSystem();

    // Storage for the new path.
    llvm::SmallString<128> NewPath = llvm::StringRef(path);
    // Loop through switched extension candidates.
    for (llvm::StringRef NewExt : NewExts) {
      llvm::sys::path::replace_extension(NewPath, NewExt);
      if (VFS->exists(NewPath))
//        return NewPath.str().str(); // First str() to convert from SmallString to
                                    // StringRef, second to convert from StringRef
                                    // to std::string
      // Also check NewExt in upper-case, just in case.
      llvm::sys::path::replace_extension(NewPath, NewExt.upper());
      if (VFS->exists(NewPath))
          ;
//        return NewPath.str().str();
    }
    return;

}

}
