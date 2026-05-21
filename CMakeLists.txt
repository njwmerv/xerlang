cmake_minimum_required(VERSION 3.22)
project(Xerlang VERSION 0.1.0 LANGUAGES CXX)

add_library(XerlangCore
  # SOURCEs
  
  visitors/typecheck.cpp
  visitors/printer.cpp
  visitors/visitor_helper.cpp
  
  parser/parser.cpp
  parser/symbol_table.cpp
  parser/ast.cpp
  
  scanner/scanner.cpp
  
  # HEADERs
  
  visitors/typecheck.h
  visitors/printer.h
  visitors/visitor_helper.h
  
  parser/parser.h
  parser/parser_constants.h
  parser/symbol_table.h
  parser/ast.h
  
  scanner/scanner.h
  
  util/types.h
)

target_compile_features(XerlangCore PUBLIC cxx_std_23)

target_include_directories(XerlangCore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

add_executable(Xerlang main.cpp)

target_link_libraries(Xerlang PRIVATE XerlangCore)