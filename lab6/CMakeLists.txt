cmake_minimum_required(VERSION 3.20)
project(main LANGUAGES CXX)
add_executable(client ./src/client.cpp)
target_compile_options(client PUBLIC -fsanitize=address -g)
target_link_options(client PUBLIC -fsanitize=address -g)
add_executable(server ./src/server.cpp)
target_compile_options(server PUBLIC -fsanitize=address -g)
target_link_options(server PUBLIC -fsanitize=address -g)
target_link_libraries(client zmq)
target_link_libraries(server zmq)
