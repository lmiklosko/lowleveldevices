cmake_minimum_required (VERSION 3.10)

project( Ligntning VERSION 0.0.1
	DESCRIPTION "Raspberry Pi low level devices driver for userspace"
	LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build development application by default when building in debug mode,
# only enable gpio support by default
option(BUILD_DEV_APP "Build development application" $<IF:$<CONFIG:Debug>,ON,OFF>)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
	message(WARNING "Please note that all other systems than Linux have only limited support")
endif()

# DMA Polling accuracy for interrupt handling. The lower the value, the faster the sensing is.
# The value is specified in microseconds (defaults to 100us)
set(DMA_POLLING_ACCURACY 100 CACHE STRING "DMA Polling accuracy")

set( lld_SOURCES
	${PROJECT_SOURCE_DIR}/src/bcm.cpp
	${PROJECT_SOURCE_DIR}/src/gpio.cpp
	${PROJECT_SOURCE_DIR}/src/pwm.cpp
	${PROJECT_SOURCE_DIR}/src/clock.cpp
	${PROJECT_SOURCE_DIR}/src/lowleveldevices.cpp
	${PROJECT_SOURCE_DIR}/src/dmapwmprovider.cpp
	${PROJECT_SOURCE_DIR}/src/dmagpioprovider.cpp)

# Create library and include appropriate directories
add_library( lld SHARED ${lld_SOURCES} )
target_include_directories( lld
	PRIVATE
		${PROJECT_SOURCE_DIR}/include/providers
		${PROJECT_SOURCE_DIR}/include/providers/gpio
		${PROJECT_SOURCE_DIR}/include/providers/spi
		${PROJECT_SOURCE_DIR}/include/providers/i2c
		${PROJECT_SOURCE_DIR}/include/providers/pwm
		${PROJECT_SOURCE_DIR}/include/providers/serial

	PUBLIC
		${PROJECT_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/include/devices
)
target_link_libraries( lld PUBLIC $<$<PLATFORM_ID:Linux>:atomic> pthread)
target_compile_options( lld 
	PRIVATE 
		-Wall 
		-Werror 
		-Wextra 
		-Wno-error=parentheses 
		-Wno-error=unused-function
)
add_definitions(-fno-strict-aliasing)

# GCC requires stdc++fs for filesystem even with c++17 until version 9.0
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
		target_link_libraries(lld PUBLIC stdc++fs)
	endif()
endif()

target_compile_features( lld
	PUBLIC
	  cxx_constexpr
	  cxx_final
	  cxx_generic_lambdas
	  cxx_lambdas
	  cxx_noexcept)

# promote low level device library
set (lld_LIBS lld)

if (BUILD_DEV_APP)
	add_executable(devapp ${PROJECT_SOURCE_DIR}/src/devapp.cpp)
	target_link_libraries(devapp lld)
endif()

 install ( TARGETS lld
 	ARCHIVE
 	  DESTINATION lib
 	  COMPONENT Libraries
 	PUBLIC_HEADER
 	  DESTINATION include
 )