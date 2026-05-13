# CMakeList.txt : CMake project for tariffserver_2026_rb_queue, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.26)

project ("tariffserver_2026_rb_queue" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckCXXCompilerFlag)
if(WIN32)
    set(LLD_TEST_FLAG "-fuse-ld=lld-link")
else()
    set(LLD_TEST_FLAG "-fuse-ld=lld")
endif()
set(CMAKE_REQUIRED_FLAGS ${LLD_TEST_FLAG})
check_cxx_compiler_flag(${LLD_TEST_FLAG} HAS_LLD)

unset(CMAKE_REQUIRED_FLAGS)
if(HAS_LLD)
    message(STATUS "LLD available. Using LLD linker.")
    if(WIN32)
        add_link_options("/time" "/threads:12")
        if(NOT CMAKE_CXX_LINKER_ID MATCHES "LLD")
            add_link_options("-fuse-ld=lld-link")
        endif()
    else()
        add_link_options("-fuse-ld=lld")
        add_link_options("-Wl,--stats" "-Wl,--threads=12")
    endif()
endif()


find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "CCACHE available. Using ccache")
    set(CMAKE_C_COMPILER_LAUNCHER   ${CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()


# output binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


file(GLOB SOURCES "src/*.cpp")
#file(GLOB CONFIGURE_DEPENDS SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(tariffserver_2026_rb_queue ${SOURCES})


find_package(PkgConfig REQUIRED)
pkg_check_modules(MARIADB REQUIRED mariadb)


target_include_directories(tariffserver_2026_rb_queue PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${MARIADB_INCLUDE_DIRS}
)

target_link_directories(tariffserver_2026_rb_queue PRIVATE
    ${MARIADB_LIBRARY_DIRS}
)

target_link_libraries(tariffserver_2026_rb_queue PRIVATE
    ${MARIADB_LIBRARIES}
)
