cmake_minimum_required(VERSION 3.22)
project(Xerlang VERSION 0.1.0 LANGUAGES C CXX)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(XerlangCore
  # SOURCEs
  
  visitors/codegen.cpp
  visitors/typecheck.cpp
  visitors/printer.cpp
  visitors/visitor_helper.cpp
  
  parser/parser.cpp
  parser/symbol_table.cpp
  parser/ast.cpp
  
  scanner/scanner.cpp
  
  # HEADERs
  
  visitors/codegen.h
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

llvm_map_components_to_libnames(llvm_libs
  core
  native
  support
  analysis
  passes
)

target_link_libraries(XerlangCore PUBLIC ${llvm_libs})

add_executable(Xerlang main.cpp)

target_link_libraries(Xerlang PRIVATE XerlangCore ${llvm_libs})
