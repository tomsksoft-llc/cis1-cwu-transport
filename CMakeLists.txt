cmake_minimum_required(VERSION 3.9)

project(cis1_cwu_transport)

set(${PROJECT_NAME}_SRC
    src/basic_ccwu_session.cpp
    src/ccwu_queue.cpp
    src/queued_ccwu_session.cpp
    src/ccwu_tcp_server.cpp
    src/ccwu_tcp_client.cpp)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SRC})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)

if(BUILD_TESTING)
    add_subdirectory(test_package)
endif(BUILD_TESTING)
