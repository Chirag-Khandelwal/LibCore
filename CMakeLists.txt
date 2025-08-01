cmake_minimum_required(VERSION 3.31)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

project(LibCore
	VERSION 1.0.0
	DESCRIPTION "Useful suite of boilerplate code"
	LANGUAGES CXX)

# libcore Library
file(GLOB_RECURSE INCS RELATIVE "${PROJECT_SOURCE_DIR}" "include/*.hpp")
file(GLOB_RECURSE SRCS RELATIVE "${PROJECT_SOURCE_DIR}" "src/*.cpp")
add_library(libcore STATIC ${SRCS} ${INCS})
# Alias for libcore
add_library(libcore::libcore ALIAS libcore)
target_compile_features(libcore PUBLIC cxx_std_20)
# For MSVC. It requires externed variables to be specified as __declspec(dllexport/dllimport) depending on
# if the the DLL is being generated, or is being used.
target_compile_definitions(libcore PRIVATE EXPORT_FOR_DLL=true)
target_include_directories(libcore PUBLIC ${PROJECT_SOURCE_DIR}/include)
set_target_properties(libcore
	PROPERTIES
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	POSITION_INDEPENDENT_CODE TRUE
	PREFIX "" # 'lib' as prefix is not needed on linux since the name is libcore anyway
	LINK_FLAGS "${EXTRA_LD_FLAGS}"
	MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>"
	INSTALL_RPATH_USE_LINK_PATH TRUE
)

# Install C++ Includes
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/include/core"
	FILES_MATCHING PATTERN "*.hpp"
)
