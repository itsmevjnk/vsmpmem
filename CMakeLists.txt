cmake_minimum_required(VERSION 3.7)

# set up MinGW
if(UNIX)
set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX i686-w64-mingw32) # use i686 for 32-bit binaries
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})
add_link_options(-static -static-libgcc -static-libstdc++) # static link everything
endif(UNIX)

project(
	vsmpmem
	DESCRIPTION "Smart 8-bit parallel RAM/ROM simulation model for Labcenter Proteus"
	LANGUAGES CXX
)

add_library(
	vsmpmem SHARED
	vsmpmem.cpp ActiveModel.cpp DsimModel.cpp 
	ActiveModel.h DsimModel.h
	sdk/vdm.hpp sdk/vdm11.hpp sdk/vdm51.hpp sdk/vdmpic.hpp sdk/vsm.hpp
)
set_target_properties(vsmpmem PROPERTIES PREFIX "")