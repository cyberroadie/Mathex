PROJECT(mathtex)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

SET(mathtex_SRCS
    src/mathex_global.h
    src/mathex_string.h
    src/mathex_util.h
    src/mathex_util.cpp
    src/mathex.h
    src/mathex_main.h
    src/mathex_main.cpp
    src/mathex.cpp
)

SET(CMAKE_CXX_FLAGS_RELEASE "-DLATEX=\"/usr/bin/latex\" -DDVIPNG=\"/usr/bin/dvipng\"")

FILE(GLOB_RECURSE SRC RELATIVE src *.cpp)

ADD_EXECUTABLE(mathex.cgi
    ${mathtex_SRCS}
)