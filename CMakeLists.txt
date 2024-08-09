cmake_minimum_required(VERSION 3.0)
project(ftp C)

set(CMAKE_C_STANDARD 11)

add_executable(ftp_server server.c)
