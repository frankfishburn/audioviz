cmake_minimum_required (VERSION 3.0)
project (audioviz)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
include_directories("${PROJECT_BINARY_DIR}")

# Find OpenGL
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL include directory: ${OPENGL_INCLUDE_DIR}")
message(STATUS "OpenGL libraries: ${OPENGL_LIBRARIES}")

# Find FFTW
find_path(FFTW_INCLUDE_DIR fftw3.h)
find_library(FFTW_LIBS REQUIRED NAMES fftw3)
find_library(FFTWF_LIBS REQUIRED NAMES fftw3f)
message(STATUS "FFTW include directory: ${FFTW_INCLUDE_DIR}")
message(STATUS "FFTW float library: ${FFTWF_LIBS}")
message(STATUS "FFTW double library: ${FFTW_LIBS}")

find_path(SPECTROGRAM_INCLUDE_DIR spectrogram.h PATH_SUFFIXES include)
find_library(SPECTROGRAM_LIB NAMES spectrogram PATH_SUFFIXES build/src)
include_directories(${SPECTROGRAM_INCLUDE_DIR})

add_subdirectory(src)
