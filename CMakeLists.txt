cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)

project(linenoise-example)

add_subdirectory(src)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} example.c)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_link_libraries(${PROJECT_NAME} PRIVATE linenoise)
