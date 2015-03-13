cmake_minimum_required(VERSION 2.8) 
project(BigMaC)

set(evaluation BigMaC.cpp SimuTest.cpp)
add_library(evaluation ${evaluation})

target_link_libraries(evaluation topo_routing rule_traffic)
