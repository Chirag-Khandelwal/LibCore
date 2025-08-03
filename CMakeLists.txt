cmake_minimum_required(VERSION 3.31)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

option(BUILD_TESTING "Build the test suite" OFF)

if(NOT DEFINED PROJECT_NAME)
	set(NOT_SUBPROJECT ON)
else()
	set(NOT_SUBPROJECT OFF)
endif()

project(LibCore
	DESCRIPTION "Useful suite of boilerplate code"
	LANGUAGES CXX)

set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "1")
set(PROJECT_VERSION_PATCH "0")

# LibCore Library

file(GLOB_RECURSE INCS RELATIVE "${PROJECT_SOURCE_DIR}" "include/*.hpp")
file(GLOB_RECURSE SRCS RELATIVE "${PROJECT_SOURCE_DIR}" "src/*.cpp")
add_library(LibCore STATIC ${SRCS} ${INCS})
# Alias for LibCore
add_library(LibCore::LibCore ALIAS LibCore)
target_compile_features(LibCore PUBLIC cxx_std_20)
target_include_directories(LibCore PUBLIC "${PROJECT_SOURCE_DIR}/include")
# For MSVC. It requires externed variables to be specified as __declspec(dllexport/dllimport) depending on
# if the the DLL is being generated, or is being used.
target_compile_definitions(LibCore PUBLIC EXPORT_FOR_DLL=true)
target_compile_definitions(LibCore PUBLIC $<$<CONFIG:Debug,RelWithDebInfo>:BUILD_DEBUG=true>)
set_target_properties(LibCore
	PROPERTIES
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	POSITION_INDEPENDENT_CODE TRUE
	PREFIX "" # 'lib' as prefix is not needed on linux since the name is LibCore anyway
	LINK_FLAGS "${EXTRA_LD_FLAGS}"
	MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>"
	INSTALL_RPATH_USE_LINK_PATH TRUE
)
# Install C++ Includes
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/include/core"
	FILES_MATCHING PATTERN "*.hpp"
)

# Testing

if(BUILD_TESTING AND NOT_SUBPROJECT)
	include(FetchContent)
	FetchContent_Declare(
		Catch2
		GIT_SHALLOW	TRUE
		GIT_REPOSITORY	https://github.com/catchorg/Catch2.git
		GIT_TAG		v3.9.0
	)
	FetchContent_MakeAvailable(Catch2)
	include(CTest)
	include(Catch)
	list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
	file(GLOB_RECURSE testfiles RELATIVE "${PROJECT_SOURCE_DIR}" "test/*.cpp")
	foreach(testfile ${testfiles})
		string(SUBSTRING ${testfile} 4 -1 m)
		get_filename_component(testname ${m} NAME_WE)
		add_executable(${testname} "${testfile}")
		target_compile_features(${testname} PUBLIC cxx_std_20)
		target_link_libraries(${testname} LibCore::LibCore Catch2::Catch2WithMain)
		set_target_properties(${testname}
			PROPERTIES
			CXX_STANDARD_REQUIRED ON
			CXX_EXTENSIONS OFF
			LINK_FLAGS "${EXTRA_LD_FLAGS}"
		)
		catch_discover_tests(${testname})
	endforeach()
endif()