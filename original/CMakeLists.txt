CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(mathtex)

SET(CMAKE_C_FLAGS_RELEASE "-DLATEX=\"/usr/bin/latex\" -DDVIPNG=\"/usr/bin/dvipng\"")
ADD_EXECUTABLE(mathex.cgi mathtex.c)


