cmake_minimum_required(VERSION 3.3)
project(SICXE-Assembler)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES sicxe_asm.cpp file_parser.cc file_parser.h file_parse_exception.h opcode_error_exception.h opcodetab.cc opcodetab.h symtab.cc symtab.h symtab_exception.h sicxe_asm.h)
add_executable(SICXE-Assembler sicxe_asm.cpp ${SOURCE_FILES})
