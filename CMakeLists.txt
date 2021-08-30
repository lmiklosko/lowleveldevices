cmake_minimum_required (VERSION 3.10)

project( Ligntning VERSION 0.0.2
	DESCRIPTION "Raspberry Pi low level devices driver for userspace"
	LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build development application by default when building in debug mode
option(BUILD_DEV_APP "Build development application" $<IF:$<CONFIG:Debug>,ON,OFF>)

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
set( lld_HEADERS
		${PROJECT_SOURCE_DIR}/include/bcm_host.hpp
		${PROJECT_SOURCE_DIR}/include/clock.hpp
		${PROJECT_SOURCE_DIR}/include/exceptions.hpp
		${PROJECT_SOURCE_DIR}/include/filesystem.hpp
		${PROJECT_SOURCE_DIR}/include/ilowleveldevices.hpp
		${PROJECT_SOURCE_DIR}/include/lowleveldevices.hpp

		${PROJECT_SOURCE_DIR}/include/devices/gpio.hpp
		${PROJECT_SOURCE_DIR}/include/devices/pwm.hpp
		${PROJECT_SOURCE_DIR}/include/devices/spi.hpp

		${PROJECT_SOURCE_DIR}/include/providers/gpio/igpio.hpp
		${PROJECT_SOURCE_DIR}/include/providers/pwm/ipwm.hpp
		${PROJECT_SOURCE_DIR}/include/providers/i2c/ii2c.hpp
		${PROJECT_SOURCE_DIR}/include/providers/spi/ispi.hpp

		${PROJECT_SOURCE_DIR}/include/providers/pwm/dmapwmprovider.hpp
		${PROJECT_SOURCE_DIR}/include/providers/gpio/dmagpioprovider.hpp
)

# Create library and include appropriate directories
add_library( lld SHARED ${lld_SOURCES} ${lld_HEADERS})
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

install( TARGETS lld DESTINATION lib)
install( FILES ${lld_HEADERS} TYPE INCLUDE)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_CONTACT "Lukas MIKLOSKO l.miklosko@gmail.com")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lukas MIKLOSKO")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
include(CPack)