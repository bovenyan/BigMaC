cmake_minimum_required(VERSION 2.8) 
project(CAB_SDN)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "-std=c++11 -g")

find_package(Boost 1.54.0 REQUIRED COMPONENTS thread log log_setup program_options system)
if(Boost_FOUND)
	message(STATUS "Boost found")
	message(STATUS "BOOST_LIBRARIES: " ${Boost_LIBRARIES})
endif()

add_definitions(-DBOOST_LOG_DYN_LINK)

add_subdirectory(mdCoding)
# add_subdirectory(placement)
# add_subdirectory(ruleGen)

add_executable(bigmac run.cpp)

if(UNIX)
	target_link_libraries(
		bigmac
        mdcode
        #place
		${Boost_LIBRARIES}
		pthread
		)
elseif(WIN32)
elseif(APPLE)
endif(UNIX)

message(STATUS "c++ flag : " ${CMAKE_CXX_FLAGS}) 
message(STATUS "this is source dir " ${project_source_dir})
message(STATUS "this is binary dir " ${project_binary_dir})
