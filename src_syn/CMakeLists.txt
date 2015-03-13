cmake_minimum_required(VERSION 2.8) 
project(BigMaC)

set(CMAKE_CXX_FLAGS "-std=c++11 -g")

add_subdirectory(evaluation)
add_subdirectory(rules)
add_subdirectory(routing)

find_package(Boost 1.55.0 REQUIRED COMPONENTS system thread filesystem iostreams log log_setup)
if(Boost_FOUND)
	message(STATUS "Boost found")
	message(STATUS "BOOST_LIBRARIES: " ${Boost_LIBRARIES})
endif()

add_definitions(-DBOOST_LOG_DYN_LINK)

set(simulation test_eva.cpp)
add_executable(test_eva ${simulation})


target_link_libraries(
    test_eva
    evaluation
    ${Boost_LIBRARIES}
)

message(STATUS "c++ flag : "${CMAKE_CXX_FLAGS}) 
message(STATUS "this is source dir "${project_source_dir})
message(STATUS "this is binary dir "${project_binary_dir})
