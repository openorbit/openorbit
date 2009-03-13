#include <iostream>

#include <clang/Basic/SourceManager.h>
#include <clang/AST/AstConsumer.h>
#include <clang/AST/TranslationUnit.h>

#include "scriptlang.hh"
using namespace llvm;
using namespace clang;

void
GenPointerTypeName(const std::string &baseType)
{
  std::cout << baseType << "_py_ptr";
}
void
GenStructTypeName(const std::string &baseType)
{
  std::cout << baseType << "_py_struct";
}

void
GenWrappedPtrType(const std::string &baseType)
{
  std::cout << "typedef struct ";
  GenPointerTypeName(baseType);
  std::cout << " {\n"
               "  PyObject_HEAD\n"
               "  struct " << baseType << " *obj;\n"
               "} ";
  GenPointerTypeName(baseType);
  std::cout << ";\n\n";
}
void
GenWrappedStructType(const std::string &baseType)
{
  std::cout << "typedef struct ";
  GenStructTypeName(baseType);
  std::cout << " {\n"
               "  PyObject_HEAD\n"
               "  struct " << baseType << " obj;\n"
               "} ";
 GenStructTypeName(baseType);
 std::cout << ";\n\n";
}
// PyObject* PyInt_FromLong(long ival)
//long PyInt_AS_LONG(PyObject *io)
//PyObject* PyBool_FromLong(long v)
//Py_RETURN_FALSE
//Py_RETURN_TRUE
//long PyLong_AsLong(PyObject *pylong)
//PY_LONG_LONG PyLong_AsLongLong(PyObject *pylong)
//void* PyLong_AsVoidPtr(PyObject *pylong)
//PyObject* PyLong_FromVoidPtr(void *p)
//PyObject* PyLong_FromSsize_t(Py_ssize_t v)
//PyObject* PyLong_FromSize_t(size_t v)
//PyObject* PyFloat_FromDouble(double v)
//double PyFloat_AsDouble(PyObject *pyfloat)
//Py_complex PyComplex_AsCComplex(PyObject *op)
//PyObject* PyComplex_FromDoubles(double real, double imag)
//PyObject* PyByteArray_FromStringAndSize(const char *string, Py_ssize_t len)
//char* PyByteArray_AsString(PyObject *bytearray)
//PyObject* PyString_FromString(const char *v)
//char* PyString_AsString(PyObject *string)
//PyObject* PyTuple_GetItem(PyObject *p, Py_ssize_t pos)
//PyObject* PyTuple_Pack(Py_ssize_t n, ...)
void
GenGetSetPair(const std::string &baseType, const std::string &field) { 
  std::cout << "static int\n"
            << baseType << "_set" << field << "(";
  GenPointerTypeName(baseType);          
  std::cout << "*self, PyObject *value, void *closure) {\n";
  std::cout << "  Py_INCREF(value);\n";
  std::cout << "  self->obj->" << field << " = " << "value;\n"; //TODO: Marshal type
  std::cout << "  return 0;\n";
  std::cout << "}\n\n";

  std::cout << "static PyObject*\n"
            << baseType << "_get" << field << "(";
  GenPointerTypeName(baseType);
  std::cout << "*self, void *closure) {\n";
  
  std::cout << "  PyObject *res = self->obj->" << field << ";\n"; // TODO: Marshal type
  
  std::cout << "  Py_INCREF(res);\n"
               "  return res;\n";

  std::cout << "}\n\n";
}

void
GenGetSetterArray(const std::string &baseType, RecordDecl::field_iterator start,
                  RecordDecl::field_iterator end)
{
  std::cout << "static PyGetSetDef " << baseType << "_getseters[] = {\n";
  while (start != end) {
    std::cout << "  {\"" << baseType << "\", \n"
       "    (getter)" << baseType << "_getfirst,\n"
       "    (setter)" << baseType << "_setfirst,\"\n"
       "    "<< baseType << " name\", NULL},\n";
    ++ start;
  }
  std::cout << "  {NULL}  /* Sentinel */\n";
  std::cout << "};\n\n";
}

  
void WrappedRecordInfo::genPythonFuncWrappers()
{}
void WrappedRecordInfo::genPythonTypeStruct()
{}
void WrappedRecordInfo::genPythonTypeObject()
{
  std::cout << "static PyTypeObject " << name << "{\n";
  
  std::cout << "  PyObject_HEAD_INIT(NULL)\n";
  std::cout << "  0,                         /*ob_size*/\n";
  std::cout << "  \"" << ctxt->getModuleName() << "." << name <<"\",             /*tp_name*/\n";
  std::cout << "  sizeof("<<name<<"),             /*tp_basicsize*/\n";
  std::cout << "  0,                         /*tp_itemsize*/\n";
  std::cout << "  (destructor)"<<name<<"_dealloc, /*tp_dealloc*/\n";
  std::cout << "  0,                         /*tp_print*/\n";
  std::cout << "  0,                         /*tp_getattr*/\n";
  std::cout << "  0,                         /*tp_setattr*/\n";
  std::cout << "  0,                         /*tp_compare*/\n";
  std::cout << "  0,                         /*tp_repr*/\n";
  std::cout << "  0,                         /*tp_as_number*/\n";
  std::cout << "  0,                         /*tp_as_sequence*/\n";
  std::cout << "  0,                         /*tp_as_mapping*/\n";
  std::cout << "  0,                         /*tp_hash */\n";
  std::cout << "  0,                         /*tp_call*/\n";
  std::cout << "  0,                         /*tp_str*/\n";
  std::cout << "  0,                         /*tp_getattro*/\n";
  std::cout << "  0,                         /*tp_setattro*/\n";
  std::cout << "  0,                         /*tp_as_buffer*/\n";
  std::cout << "  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/\n";
  std::cout << "  \""<<name<<" objects\",           /* tp_doc */\n";
  std::cout << "  0,		               /* tp_traverse */\n";
  std::cout << "  0,		               /* tp_clear */\n";
  std::cout << "  0,		               /* tp_richcompare */\n";
  std::cout << "  0,		               /* tp_weaklistoffset */\n";
  std::cout << "  0,		               /* tp_iter */\n";
  std::cout << "  0,		               /* tp_iternext */\n";
  std::cout << "  "<<name<<"_methods,             /* tp_methods */\n";
  std::cout << "  "<<name<<"_members,             /* tp_members */\n";
  std::cout << "  "<<name<<"_getseters,           /* tp_getset */\n";
  std::cout << "  0,                         /* tp_base */\n";
  std::cout << "  0,                         /* tp_dict */\n";
  std::cout << "  0,                         /* tp_descr_get */\n";
  std::cout << "  0,                         /* tp_descr_set */\n";
  std::cout << "  0,                         /* tp_dictoffset */\n";
  std::cout << "  (initproc)"<<name<<"_init,      /* tp_init */\n";
  std::cout << "  0,                         /* tp_alloc */\n";
  std::cout << "  "<<name<<"_new,                 /* tp_new */\n";
  std::cout << "};\n\n";
  
  
}

void
PythonWrapperContext::genHeader() 
{
  std::cout << "#include <Python.h>\n";
  std::cout << "#include \"" << headerName << "\"\n\n";
}

void
PythonWrapperContext::genTypes()
{
  for (type_iter i = type_begin(); i != type_end(); ++ i) {
    (*i)->genPythonTypeStruct();
    (*i)->genPythonFuncWrappers();
    (*i)->genPythonTypeObject();
  }
}

void PythonWrapperContext::genMethods()
{
  for (method_iter i = method_begin(); i != method_end(); ++ i) {
  }
}

void
PythonWrapperContext::genEnums()
{
  for (enum_iter i = enum_begin(); i != enum_end(); ++ i) {
  }
}

void
PythonWrapperContext::genInit()
{
  // TODO: Fix module name, we might whant several headers in the same module, what should
  //       we do in order avoid clashes?
  std::cout << "void\n"
               "init" << moduleName << "(void)\n"
               "{\n";
               
  std::cout << "}\n\n";
}

void
PythonConsumer::HandleTranslationUnit(TranslationUnit& TU) {
  SourceManager &sm = TU.getContext().getSourceManager();
  //const std::string &file = TU.getSourceFile();
  
  //std::cout << "file " << file << "\n";
  //std::cout << "#include <Python.h>\n";
  
  PythonWrapperContext wrapCtxt(fileName, moduleName);
  wrapCtxt.genHeader();
  
  for (TranslationUnit::iterator i = TU.begin(); i != TU.end(); i ++) {
    if (strcmp(sm.getBufferName(i->getLocation()), fileName.c_str())) {
      continue; // skip any treatment of other files, we only wrap the given file
    }
    
    if (RecordDecl *rec = dyn_cast<RecordDecl>(*i)) {
      
      GenWrappedPtrType(rec->getNameAsString());
      GenWrappedStructType(rec->getNameAsString());
      
      for (RecordDecl::field_iterator i = rec->field_begin(); i != rec->field_end() ; i ++) {
        GenGetSetPair(rec->getNameAsString(), i->getNameAsString());
      }
      
      GenGetSetterArray(rec->getNameAsString(), rec->field_begin(), rec->field_end());
      
      //GenPyTypeObj(std::string("openorbit"), rec->getNameAsString());

    } else if (TypedefDecl *tdef = dyn_cast<TypedefDecl>(*i)) {
      std::cout << "typedef found " << tdef->getNameAsString() << "\n";
    } else if (FunctionDecl *func = dyn_cast<FunctionDecl>(*i)) {
      // If the function has the first parameter being a pointer to a wrapped type
      // structure, then this function will be inserted into that types class
      std::cout << "func found " << func->getNameAsString() << "\n";
    } else {
      std::cout << "some other decl\n";
    }
  }
  
}

