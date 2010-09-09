CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mathtex)

SET(mathtex_SRCS
    src/mathex_global.h
    src/mathex_string.h
    src/mathex_util.h
    src/mathex_util.cpp
    src/mathex.h
    src/mathex.cpp
    src/mathex_main.h
    src/mathex_main.cpp
)

SET(CMAKE_C_FLAGS_RELEASE "-DLATEX=\"/usr/bin/latex\" -DDVIPNG=\"/usr/bin/dvipng\"")
ADD_EXECUTABLE(mathex.cgi ${mathex_SRCS})