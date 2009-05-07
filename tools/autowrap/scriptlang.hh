
#ifndef SCRIPTLANG_HH_NILH71HL
#define SCRIPTLANG_HH_NILH71HL

#include <map>
#include <string>
#include <clang/AST/AstConsumer.h>
#include <clang/AST/TranslationUnit.h>

class PythonWrapperContext;

class WrappedTypeInfo {
protected:
  PythonWrapperContext *ctxt;
  std::string name;
public:
  WrappedTypeInfo(PythonWrapperContext *ctxt, const std::string &name) : ctxt(ctxt), name(name) {}
  const std::string &getName() {return name;}
  std::string getWrappedPtrName() { return std::string(name).append("_ptr");};
  
  virtual void genPythonFuncWrappers() = 0;
  virtual void genPythonTypeStruct() = 0;
  virtual void genPythonTypeObject() = 0;
  
};

class WrappedRecordInfo : public WrappedTypeInfo {
protected:
  clang::RecordDecl::field_iterator field_start; //!< Start of fields if this is a record
  clang::RecordDecl::field_iterator field_end;
  
  std::vector<clang::FunctionDecl*> methods; //!< All methods that take a pointer to a
                                             //! record of this type as first argument
public:
  WrappedRecordInfo(PythonWrapperContext *ctxt,
                    const std::string &name,
                    clang::RecordDecl::field_iterator start,
                    clang::RecordDecl::field_iterator end)
    : WrappedTypeInfo(ctxt, name), field_start(start), field_end(end) {}
  
  virtual void genPythonFuncWrappers();
  virtual void genPythonTypeStruct();
  virtual void genPythonTypeObject();
};

class PythonWrapperContext {
  std::string headerName; //!< The parsed header
  std::string moduleName; //!< The parsed header


  std::map<clang::Type*, WrappedTypeInfo*> types;
  std::vector<clang::FunctionDecl*> methods;
  std::vector<clang::EnumDecl*> enums;
public:
  typedef std::vector<WrappedTypeInfo*>::iterator type_iter;
  typedef std::vector<clang::FunctionDecl*>::iterator method_iter;
  typedef std::vector<clang::EnumDecl*>::iterator enum_iter;

  PythonWrapperContext(std::string &hdrFile, std::string &modName)
    : headerName(hdrFile), moduleName(modName) {}
  
  const std::string& getModuleName() {return moduleName;};
  void genHeader();
  void genTypes();
  void genMethods();
  void genEnums();
  void genInit();
  
  void addType(WrappedTypeInfo*ti) {types.push_back(ti);}
  
  type_iter
  type_begin() {return types.begin();}
  
  type_iter
  type_end() {return types.end();}
  
  method_iter
  method_begin() {return methods.begin();}
  
  method_iter
  method_end() {return methods.end();}
  
  enum_iter
  enum_begin() {return enums.begin();}
  
  enum_iter
  enum_end() {return enums.end();}
  
};

class PythonConsumer : public clang::ASTConsumer {
  std::string fileName; //!< Main file that we are parsing
  std::string moduleName; //!< Module name of python wrapper
  std::string outputFileName; //!< Main file that we are writing to
  
public:
  PythonConsumer(const std::string &name, const std::string &mod)
    : fileName(name), moduleName(mod) {}
  void HandleTranslationUnit(clang::TranslationUnit& TU);
};

#endif /* end of include guard: SCRIPTLANG_HH_NILH71HL */
