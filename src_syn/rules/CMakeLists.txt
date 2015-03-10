cmake_minimum_required(VERSION 2.8) 
project(BigMaC_rule)

set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_CXX_FLAGS "-std=c++11 -g")

find_package(Boost 1.55.0 REQUIRED COMPONENTS system thread filesystem iostreams log log_setup)
if(Boost_FOUND)
	message(STATUS "Boost found")
	message(STATUS "BOOST_LIBRARIES: " ${Boost_LIBRARIES})
endif()

add_definitions(-DBOOST_LOG_DYN_LINK)

set(rule_traffic synTraceGen.cpp test_rule.cpp)
add_library(rule_traffic ${rule_traffic})
add_executable(test_rule test_rule.cpp)

if(UNIX)
	target_link_libraries(
		test_rule
		rule_traffic
		${Boost_LIBRARIES}
		pthread
		z
		pcap
		)
elseif(WIN32)
elseif(APPLE)
endif(UNIX)

message(STATUS "c++ flag : "${CMAKE_CXX_FLAGS}) 
message(STATUS "this is source dir "${project_source_dir})
message(STATUS "this is binary dir "${project_binary_dir})
