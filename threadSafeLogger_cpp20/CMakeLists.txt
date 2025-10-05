# CMakeList.txt : CMake project for threadSafeLogger_cpp20, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.31)

project ("threadSafeLogger_cpp20" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# output binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add source to this project's executable.
#add_executable (threadSafeLogger_cpp20 
	#"src/main.cpp"
    #"src/threadSafeLogger_cpp20.cpp"
#)

file(GLOB SOURCES "src/*.cpp")
#file(GLOB CONFIGURE_DEPENDS SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(threadSafeLogger_cpp20 ${SOURCES})

target_include_directories(threadSafeLogger_cpp20 PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    #"E:/lib/OpenSSL/include"
    #"E:/lib/nlohman/include"
)

#target_link_libraries(threadSafeLogger_cpp20 PRIVATE 
    #ws2_32
    #"E:/lib/OpenSSL/lib/VC/x64/MD/libssl.lib"
    #"E:/lib/OpenSSL/lib/VC/x64/MD/libcrypto.lib"
#)

# use the mold linker if available
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fuse-ld=mold" HAS_MOLD)

if(HAS_MOLD)
    message(STATUS "Using mold linker")
    add_link_options(-fuse-ld=mold)
endif()
