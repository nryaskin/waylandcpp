set(QUERY_SRCS main.cpp)
add_executable(query_display ${QUERY_SRCS})
target_link_libraries(query_display PRIVATE waylandcpp)
target_compile_features(query_display PUBLIC cxx_std_20)
