PROJECT(mathtex)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)

SET(mathtex_SRCS
    src/mathex_global.h
    src/mathex_string.h
    src/mathex_md5.cpp
    src/mathex_time.cpp
    src/mathex_log.cpp
    src/mathex_util.h
    src/mathex_util.cpp
    src/mathex_latex.cpp
    src/mathex.h
    src/mathex_main.h
    src/mathex_main.cpp
    src/mathex.cpp
)

SET(resources_SRCS
    src/mathex_resources.h
    src/mathex_resources.cpp
)

ADD_LIBRARY(resourcesLib
    ${resources_SRCS}
)

SET(CMAKE_CXX_FLAGS_RELEASE "-DLATEX=\"/usr/bin/latex\" -DDVIPNG=\"/usr/bin/dvipng\"")

FILE(GLOB_RECURSE SRC RELATIVE src *.cpp)

ADD_EXECUTABLE(mathex.cgi
    ${mathtex_SRCS}
)

TARGET_LINK_LIBRARIES(mathex.cgi resourcesLib)