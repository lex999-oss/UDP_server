cmake_minimum_required(VERSION 3.17)
project(tema6 C)

set(CMAKE_C_STANDARD 99)

add_executable(client client.c)
target_link_libraries(client -lwininet wsock32 ws2_32)


add_executable(server server.c)
target_link_libraries(server -lwininet wsock32 ws2_32)

