cmake_minimum_required (VERSION 3.10)

project (minimake C)

set(CMAKE_C_STANDARD 99)
set(C_MAKE_STANDARD_REQUIRED ON)

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

aux_source_directory (src SRC_LIST)

include_directories (include)

add_executable (minimake ${SRC_LIST})
