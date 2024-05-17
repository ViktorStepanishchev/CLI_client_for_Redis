cmake_minimum_required(VERSION 3.28)
project(CLI_client_REDIS)

set(CMAKE_CXX_STANDARD 17)

add_executable(CLI_client_REDIS my_redis_cli.cpp
)

#cmake_minimum_required(VERSION 3.28)
#project(CLI_client_REDIS)
#
#set(CMAKE_CXX_STANDARD 17)
#
#find_package(OpenSSL REQUIRED)
#
#add_executable(CLI_client_REDIS my_redis_cli.cpp
#)
#include_directories(${OPENSSL_INCLUDE_DIR})
#target_link_libraries(CLI_client_REDIS ${OPENSSL_LIBRARIES})
