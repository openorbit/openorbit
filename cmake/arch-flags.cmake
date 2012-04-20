# Default flags
set(CMAKE_C_FLAGS_DEBUG "-g -std=gnu99")
set(CMAKE_C_FLAGS_RELEASE "-std=gnu99 -O2")
set(CMAKE_C_FLAGS_ANALYSE "-g -std=gnu99")

set(ARCH_FLAGS_DEBUG "")
set(ARCH_FLAGS_RELEASE "")
set(ARCH_FLAGS_ANALYSE "")

if (ENABLE_VECTORISE)
    include(CheckCCompilerFlag)
    check_c_compiler_flag(-faltivec HAVE_ALTIVEC)
    check_c_compiler_flag(-msse3 HAVE_SSE3)

    if (HAVE_ALTIVEC)
        set(ARCH_FLAGS_RELEASE "${ARCH_FLAGS_RELEASE} -faltivec")
        set(ARCH_FLAGS_DEBUG "${ARCH_FLAGS_DEBUG} -faltivec")
        set(ARCH_FLAGS_ANALYSE "${ARCH_FLAGS_DEBUG} -faltivec")
    endif (HAVE_ALTIVEC)

    if (HAVE_SSE3)
        set(ARCH_FLAGS_RELEASE "${ARCH_FLAGS_RELEASE} -msse3")
        set(ARCH_FLAGS_DEBUG "${ARCH_FLAGS_DEBUG} -msse3")
        set(ARCH_FLAGS_ANALYSE "${ARCH_FLAGS_ANALYSE} -msse3")
    endif (HAVE_SSE3)
	
endif (ENABLE_VECTORISE)


set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${ARCH_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ARCH_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_ANALYSE "${CMAKE_C_FLAGS_ANALYSE} ${ARCH_FLAGS_ANALYSE}")
