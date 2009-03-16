// System headers
#include <stdlib.h>
#include <sysexits.h>

// STL headers
#include <iostream>
#include <llvm/Support/raw_ostream.h>

// LLVM headers
#include <llvm/Support/CommandLine.h>
#include <llvm/ADT/OwningPtr.h>

// Clang headers
#include <clang/Basic/FileManager.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Parse/Action.h>
#include <clang/Parse/Parser.h>
#include <clang/Sema/ParseAST.h>
#include "clang/AST/TranslationUnit.h"
#include <clang/AST/Decl.h>
#include "clang/AST/ASTContext.h"
#include <clang/Driver/InitHeaderSearch.h>
#include <clang/Driver/TextDiagnosticBuffer.h>
#include <clang/Driver/TextDiagnosticPrinter.h>


#include "scriptlang.hh"
#include "prep.hh"

// Command line options
static llvm::cl::list<std::string>
inputFilenames(llvm::cl::Positional, llvm::cl::desc("<input files>"));

static llvm::cl::list<std::string>
includeDirs("I", llvm::cl::value_desc("directory"), llvm::cl::Prefix,
            llvm::cl::desc("Add directory to include search path"));
          
static llvm::cl::opt<std::string>
isysroot("isysroot", llvm::cl::value_desc("dir"), llvm::cl::init("/"),
         llvm::cl::desc("Set the system root directory (usually /)"));

static llvm::cl::opt<std::string>
modName("modname", llvm::cl::value_desc("module"), llvm::cl::init("autowrap"),
                  llvm::cl::desc("Set the python module name"));


static llvm::cl::list<std::string>
sysIncDirs("isystem", llvm::cl::value_desc("directory"), llvm::cl::Prefix,
           llvm::cl::desc("Add directory to SYSTEM include search path"));

static llvm::cl::list<std::string>
prepDefines("D", llvm::cl::value_desc("macro"), llvm::cl::Prefix,
            llvm::cl::desc("Predefine the specified macro"));

static void DefineBuiltinMacro(std::vector<char> &Buf, const char *Macro,
                               const char *Command = "#define ") {
  Buf.insert(Buf.end(), Command, Command+strlen(Command));
  if (const char *Equal = strchr(Macro, '=')) {
    // Turn the = into ' '.
    Buf.insert(Buf.end(), Macro, Equal);
    Buf.push_back(' ');
    Buf.insert(Buf.end(), Equal+1, Equal+strlen(Equal));
  } else {
    // Push "macroname 1".
    Buf.insert(Buf.end(), Macro, Macro+strlen(Macro));
    Buf.push_back(' ');
    Buf.push_back('1');
  }
  Buf.push_back('\n');
}

int
main(int argc, char **argv)
{
  llvm::cl::ParseCommandLineOptions(argc, argv, "Automatic Script Wrapper\n");
  
  if (inputFilenames.empty()) {
    std::cerr << "error: no input files specified\n";
    exit(EX_USAGE);
  }
  // TODO: Support multiple files
  for (llvm::cl::list<std::string>::iterator i = inputFilenames.begin();
       i != inputFilenames.end() ; i ++)
  {    
    PrepCtxt prepCtxt;
    // Add header search directories
    clang::InitHeaderSearch init(prepCtxt.headers);
    init.AddDefaultSystemIncludePaths(prepCtxt.opts);
    init.Realize();
    
    const clang::FileEntry* file = prepCtxt.fm.getFile(*i);
    clang::ASTConsumer *consumer = new PythonConsumer(*i, modName);

    if (file == 0) {
      std::cerr << "error: unknown file '" << *i << "'\n";
      return 0;
    }
    prepCtxt.sm.createMainFileID(file, clang::SourceLocation());
    
    ParseAST(prepCtxt.prep, consumer);  // calls pp.EnterMainSourceFile() for us
    
    delete consumer;
  }
  
  return 0;
}
