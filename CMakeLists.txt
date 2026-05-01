cmake_minimum_required(VERSION 3.22)
project(Xerlang VERSION 0.1.0 LANGUAGES CXX)

add_library(XerlangCore
  # SOURCEs
  parser/parser.cpp
  scanner/scanner.cpp
  # HEADERs
  parser/parser.h
  parser/parser_constants.h
  scanner/scanner.h
  util/types.h
)

target_compile_features(XerlangCore PUBLIC cxx_std_23)

target_include_directories(XerlangCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_executable(Xerlang main.cpp)

target_link_libraries(Xerlang PRIVATE XerlangCore)