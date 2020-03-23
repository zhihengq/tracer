cmake_minimum_required(VERSION 3.14)
project(FoveatedRaytracing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra -pedantic -Wno-sign-compare)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize= -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads)
find_package(glm REQUIRED)
find_package(SOIL REQUIRED)
find_package(GLEW REQUIRED)

add_executable(trace_test
	src/camera.cpp
	src/entity.cpp
	src/envmap.cpp
	src/make_scene.cpp
	src/sphere.cpp
	src/texture.cpp
	src/trace_test.cpp
	)
target_link_libraries(trace_test PRIVATE glm SOIL GLEW::GLEW Boost::headers Threads::Threads)

add_executable(trace_server
	src/camera.cpp
	src/entity.cpp
	src/envmap.cpp
	src/make_scene.cpp
	src/sphere.cpp
	src/texture.cpp
	src/trace_server.cpp
	)
target_link_libraries(trace_server PRIVATE glm SOIL GLEW::GLEW Boost::headers Boost::system Threads::Threads)
