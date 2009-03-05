# Todo, replace with CMake script file


LIBS=-lLLVMSupport -lLLVMCore -lLLVMSystem -lLLVMBitWriter -lLLVMBitReader -lclangBasic -lclangLex -lclangParse -lclangAST -lclangDriver -lclangSema
FILES=autowrap.cc scriptlang.cc
LIBDIR=/Users/holm/Projects/orbit/oo/build/autowrap/llvm/Debug/lib
INCDIR=-I/Users/holm/Projects/orbit/oo/build/autowrap/llvm/include
INCDIR+=-I/Users/holm/Projects/orbit/oo/build/autowrap/llvm/tools/clang/include
DEFS=-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS


autowrap:
	g++ -g -fno-rtti -c scriptlang.cc ${DEFS} ${INCDIR} -L${LIBDIR} -o scriptlang.o
	g++ -g -fno-rtti -c autowrap.cc ${DEFS} ${INCDIR} -L${LIBDIR} -o autowrap.o
	g++ -g -fno-rtti scriptlang.o autowrap.o -nortti -L${LIBDIR} ${LIBS} -o autowrap
	
clean:
	rm -f autowrap autowrap.o scriptlang.o