cmake_minimum_required(VERSION 3.25)

project(waylandcpp
        VERSION 0.1
        DESCRIPTION "waylandcpp implementation without libwayland bindings"
        LANGUAGES CXX)

option(WAYLANDCPP_APPS "Build sample applications")

add_subdirectory(src)
if (WAYLANDCPP_APPS)
    add_subdirectory(apps)
endif()
