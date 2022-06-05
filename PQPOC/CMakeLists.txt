cmake_minimum_required(VERSION 3.10)

# set the project name
project(PQPOC)

# add the executable
add_executable( pqpoc DriverProgram.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(pqpoc PRIVATE Threads::Threads)