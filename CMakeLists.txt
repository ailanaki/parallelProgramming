cmake_minimum_required(VERSION 3.23)
project (parallelProgramming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-o")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-ltbb")

find_package(TBB 2021.5.0 REQUIRED)
add_executable(parallelProgramming lab1/lab1.cpp main.cpp lab2/lab2.cpp)
target_link_libraries(parallelProgramming PRIVATE TBB::tbb TBB::tbbmalloc TBB::tbbmalloc_proxy)

