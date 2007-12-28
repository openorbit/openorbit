# Default flags
set(CMAKE_C_FLAGS_DEBUG "-g -std=c99")
set(CMAKE_C_FLAGS_RELEASE "-std=c99")


if (ENABLE_VECTORISE)
    include(CheckCCompilerFlag)
    check_c_compiler_flag(-faltivec ALTIVEC_AVAILABLE)
    check_c_compiler_flag(-msse3 SSE3_AVAILABLE)

    if (ALTIVEC_AVAILABLE)
        add_definitions(-DHAVE_ALTIVEC)
        set(CMAKE_C_FLAGS_RELEASE "-std=c99 -O2 -faltivec")
        set(CMAKE_C_FLAGS_DEBUG "-g -std=c99 -faltivec")
    endif (ALTIVEC_AVAILABLE)
    
    if (SSE3_AVAILABLE)
        add_definitions(-DHAVE_SSE3)
    endif (SSE3_AVAILABLE)
    
    add_definitions(-DENABLE_VECTORISE)
endif (ENABLE_VECTORISE)
