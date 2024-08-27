add_library(waylandcpp-common INTERFACE)
target_include_directories(waylandcpp-common INTERFACE include)

add_subdirectory(generated)

add_library(waylandcpp)
target_sources(waylandcpp
               PRIVATE
                socket.cpp
                object_builder.cpp
                buffer_parser.cpp
                buffer.cpp)
target_compile_features(waylandcpp PUBLIC cxx_std_20)
target_include_directories(waylandcpp PUBLIC ${waylandcpp_SOURCE_DIR}/include)
target_link_libraries(waylandcpp PUBLIC waylandcpp-common waylandcpp-generated)
