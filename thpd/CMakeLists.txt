cmake_minimum_required(VERSION 3.0)
project(thpd)
SET(CMAKE_CXX_STANDARD 11)
SET(THPD_SRC_PATH ${PROJECT_SOURCE_DIR}/src)
SET(THPD_INC_PATH ${PROJECT_SOURCE_DIR}/include)
SET(THPD_LIB_SRC_PATH ${THPD_SRC_PATH}/lib)
SET(THPD_LIB_INC_PATH ${THPD_INC_PATH}/lib)
SET(CMAKE_BUILD_TYPE Debug)
SET(THPD_LIB_SRC
	${THPD_LIB_SRC_PATH}/networking.cpp
	${THPD_LIB_INC_PATH}/networking.h
	${THPD_LIB_INC_PATH}/crc32.hpp
)

add_library(thpdlib STATIC ${THPD_LIB_SRC})
target_link_libraries(thpdlib INTERFACE pthread ssl crypto)
include_directories(${THPD_LIB_INC_PATH})
add_executable(thpd ${THPD_SRC_PATH}/main.cpp)
target_link_libraries(thpd thpdlib)

install (TARGETS thpd
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin)

