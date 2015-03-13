cmake_minimum_required(VERSION 2.8) 
project(BigMaC)

set(topo_routing GraphGen.cpp)
add_library(topo_routing ${topo_routing})
