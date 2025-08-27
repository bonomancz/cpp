# CMakeList.txt : CMake project for diskConsumptionFinder, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.20)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("diskConsumptionFinder")

# Add source to this project's executable.
#add_executable (diskConsumptionFinder 
	#"src/main.cpp"
    #"src/diskConsumptionFinder.cpp"
#)

file(GLOB SOURCES "src/*.cpp")
#file(GLOB CONFIGURE_DEPENDS SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(diskConsumptionFinder ${SOURCES})

target_include_directories(diskConsumptionFinder PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    #"E:/lib/OpenSSL/include"
    "E:/lib/nlohman/include"
)

#target_link_libraries(diskConsumptionFinder PRIVATE 
    #ws2_32
    #"E:/lib/OpenSSL/lib/VC/x64/MD/libssl.lib"
    #"E:/lib/OpenSSL/lib/VC/x64/MD/libcrypto.lib"
#)

set_property(TARGET diskConsumptionFinder PROPERTY CXX_STANDARD 23)

# TODO: Add tests and install targets if needed.
