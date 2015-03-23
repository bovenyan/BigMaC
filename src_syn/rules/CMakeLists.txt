cmake_minimum_required(VERSION 2.8) 
project(BigMaC)

set(rule_traffic synTraceGen.cpp BucketTree.cpp)
add_library(rule_traffic ${rule_traffic})
