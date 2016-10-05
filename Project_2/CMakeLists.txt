cmake_minimum_required(VERSION 3.6)
project(raycast)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

set(SOURCE_FILES
        JSONParser.h
    Makefile
    objects.json
        PixTool.h
    test.ppm
    tracer
        RayTracer.c
        RayTracer.h
    VectorMath.h)

add_executable(raycast ${SOURCE_FILES})