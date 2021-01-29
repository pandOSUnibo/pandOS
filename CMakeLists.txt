cmake_minimum_required(VERSION 3.10)

project(pandos VERSION 0.1)

add_executable(${PROJECT_NAME} testers/p1test.c)

add_subdirectory(phase1)


target_include_directories(${PROJECT_NAME} PUBLIC h/)
target_link_libraries(${PROJECT_NAME} PUBLIC phase1)
