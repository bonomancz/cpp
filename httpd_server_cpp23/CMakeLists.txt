# CMakeList.txt : CMake project for httpd_server_cpp23, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.31)

project ("httpd_server_cpp23" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -O2 -g -pipe")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -O2 -march=native -pipe")

# output binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add source to this project's executable.
#add_executable (httpd_server_cpp23 
	#"src/main.cpp"
    #"src/httpd_server_cpp23.cpp"
#)

file(GLOB SOURCES "src/*.cpp")
#file(GLOB CONFIGURE_DEPENDS SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(httpd_server_cpp23 ${SOURCES})

target_include_directories(httpd_server_cpp23 PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    "E:/lib/OpenSSL/include"
    "E:/lib/nlohman/include"
)

target_link_libraries(httpd_server_cpp23 PRIVATE 
    ws2_32
    "E:/lib/OpenSSL/lib/VC/x64/MD/libssl.lib"
    "E:/lib/OpenSSL/lib/VC/x64/MD/libcrypto.lib"
)
