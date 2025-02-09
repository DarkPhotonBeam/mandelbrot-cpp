cmake_minimum_required(VERSION 3.10)

project(Mandelbrot)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -flto -DNDEBUG")


include_directories(/usr/local/include)

find_package(OpenMP REQUIRED)


find_package(PkgConfig REQUIRED)
pkg_check_modules(gmpxx REQUIRED IMPORTED_TARGET gmpxx)
pkg_check_modules(mpfr REQUIRED IMPORTED_TARGET mpfr)

find_package(SDL3 CONFIG REQUIRED)

add_executable(Mandelbrot src/main.cpp
        src/MPC.cpp
        src/MPC.h
        src/Mandelbrot.cpp
        src/Mandelbrot.h
        src/MandelbrotDouble.cpp
        src/MandelbrotDouble.h
)
target_link_libraries(Mandelbrot PRIVATE PkgConfig::gmpxx PkgConfig::mpfr /usr/local/lib/libmpc.a SDL3::SDL3 OpenMP::OpenMP_CXX)
