
set(WAYLAND_XML_PATH "/usr/share/wayland/wayland.xml" CACHE STRING "Path to wayland.xml file")
set(WAYLANDCPP_GENERATOR_PATH "" CACHE FILEPATH "Path to waylandcpp-generator")

find_program(
    WAYLANDCPP_GENERATOR
    "waylandcpp-generator"
    PATHS ${WAYLANDCPP_GENERATOR_PATH}
    DOC "waylandcpp-generator path"
    REQUIRED)

execute_process(COMMAND ${WAYLANDCPP_GENERATOR} --config ${WAYLAND_XML_PATH} --dry-run
                OUTPUT_VARIABLE WAYLANDCPP_HEADERS
                RESULT_VARIABLE WAYLANDCPP_GENERATOR_RESULT
                OUTPUT_STRIP_TRAILING_WHITESPACE)

if (NOT WAYLANDCPP_GENERATOR_RESULT EQUAL 0)
    message(FATAL_ERROR "waylandcpp-generator dry-run error" ${WAYLANDCPP_GENERATOR_RESULT})
endif()
string(REPLACE "\n" ";" WAYLANDCPP_HEADERS_LIST WAYLANDCPP_HEADERS)

add_custom_command(OUTPUT ${WAYLANDCPP_HEADERS_LIST}
                   COMMENT "Generating waylandcpp headers from ${WAYLAND_XML_PATH}"
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMAND ${WAYLANDCPP_GENERATOR} --config ${WAYLAND_XML_PATH} --output ${CMAKE_CURRENT_BINARY_DIR})

add_library(waylandcpp-generated INTERFACE "${WAYLANDCPP_HEADERS_LIST}")
