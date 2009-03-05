#ifndef PREP_HH_5MBTMC0H
#define PREP_HH_5MBTMC0H

#include <string>

#include <llvm/Config/config.h>

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/FileManager.h>

#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>

#include <clang/Driver/TextDiagnosticPrinter.h>


struct PrepCtxt {
  PrepCtxt(clang::DiagnosticClient* client = 0,
           const std::string& triple = LLVM_HOSTTRIPLE)
    : diagClient((client == 0) ? new clang::TextDiagnosticPrinter(llvm::errs(),
                                                                  false,
                                                                  false,
                                                                  false)
                               : client),
      diags(diagClient),
      target(clang::TargetInfo::CreateTargetInfo(triple)),
      headers(fm),
      prep(diags, opts, *target, sm, headers)
  {
    using namespace clang;
    diags.setDiagnosticMapping(diag::warn_pp_undef_identifier,diag::MAP_IGNORE);
  }
  ~PrepCtxt() {
    delete diagClient;
    delete target;
  }
  
  clang::DiagnosticClient* diagClient;
  clang::Diagnostic diags;
  clang::LangOptions opts;
  clang::TargetInfo* target;
  clang::SourceManager sm;
  clang::FileManager fm;
  clang::HeaderSearch headers;
  clang::Preprocessor prep;
};

#endif /* end of include guard: PREP_HH_5MBTMC0H */
