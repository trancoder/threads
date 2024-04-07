cmake_minimum_required(VERSION 3.0)
project(BoostAsioExample)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/boost_1_75_0)
# Find Boost
find_package(Boost REQUIRED)

# Include Boost headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/boost_1_75_0)

# Add your application files
add_executable(receiver receiver.cpp)

find_package(Threads REQUIRED)
target_link_libraries(receiver ${CMAKE_THREAD_LIBS_INIT} Boost::boost rt)
