set(LLVM_BASE "" CACHE PATH "Path to llvm lib dir")
set(CLANG_BASE "" CACHE PATH "Path to clang lib dir")

# LLVM libs
find_library(LLVM_SUPPORT_LIB LLVMSupport PATHS ${LLVM_BASE})
find_library(LLVM_CORE_LIB LLVMCore PATHS ${LLVM_BASE})
find_library(LLVM_SYSTEM_LIB LLVMSystem PATHS ${LLVM_BASE})
find_library(LLVM_BIT_WRITER_LIB LLVMBitWriter PATHS ${LLVM_BASE})
find_library(LLVM_BIT_READER_LIB LLVMBitReader PATHS ${LLVM_BASE})

# Clang libs
find_library(CLANG_BASIC_LIB clangBasic PATHS ${CLANG_BASE})
find_library(CLANG_LEX_LIB clangLex PATHS ${CLANG_BASE})
find_library(CLANG_PARSE_LIB clangParse PATHS ${CLANG_BASE})
find_library(CLANG_AST_LIB clangAST PATHS ${CLANG_BASE})
find_library(CLANG_DRIVER_LIB clangDriver PATHS ${CLANG_BASE})
find_library(CLANG_SEMA_LIB clangSema PATHS ${CLANG_BASE})

set(LLVM_LIBS ${LLVM_SUPPORT_LIB} ${LLVM_CORE_LIB} ${LLVM_SYSTEM_LIB}
              ${LLVM_BIT_WRITER_LIB} ${LLVM_BIT_READER_LIB})
set(CLANG_LIBS ${CLANG_BASIC_LIB} ${CLANG_LEX_LIB} ${CLANG_PARSE_LIB}
               ${CLANG_AST_LIB} ${CLANG_DRIVER_LIB} ${CLANG_SEMA_LIB})

find_path(LLVM_INC "llvm/Type.h")
find_path(CLANG_INC "clang/Basic/TargetInfo.h")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fno-rtti")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti")
set(CMAKE_CXX_FLAGS_ANALYSE "${CMAKE_CXX_FLAGS_ANALYSE} -g -fno-rtti")

add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D__STDC_CONSTANT_MACROS)
include_directories(${LLVM_INC} ${CLANG_INC})

set(AUTOWRAP_SRC autowrap.cc scriptlang.cc)
add_executable(autowrap ${AUTOWRAP_SRC})
target_link_libraries(autowrap ${LLVM_LIBS} ${CLANG_LIBS})
