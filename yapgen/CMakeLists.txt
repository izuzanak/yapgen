
cmake_minimum_required(VERSION 3.0.2)

project(yapgen)

find_package(Lua 5.1.5 REQUIRED)

set(PROC_DIR "${CMAKE_CURRENT_BINARY_DIR}/proc_files")
file(MAKE_DIRECTORY ${PROC_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

# - process target source file -
function(process_source TARGET)
  execute_process(COMMAND process --deps "${SOURCE_DIR}/${TARGET}.cc" ${COMP_HEADER_DIRS}
      OUTPUT_VARIABLE DEPS
      ERROR_QUIET)
  add_custom_command(OUTPUT "${PROC_DIR}/p.${TARGET}.cc"
      DEPENDS ${DEPS}
      COMMAND process "--output" "${PROC_DIR}/p.${TARGET}.cc" "${SOURCE_DIR}/${TARGET}.cc" ${COMP_HEADER_DIRS})
endfunction()

# - process component -
function(process_component)
  set(PROC_DIR "${PROC_DIR}/${COMP_NAME}")
  file(MAKE_DIRECTORY ${PROC_DIR})

  set(PROC_SOURCE_FILES)
  foreach(SOURCE_DIR ${COMP_SOURCE_DIRS})
    set(SOURCE_FILES)
    aux_source_directory(${SOURCE_DIR} SOURCE_FILES)

    foreach(SOURCE_FILE ${SOURCE_FILES})
      get_filename_component(TARGET ${SOURCE_FILE} NAME_WE)
      process_source(${TARGET})

      list(APPEND PROC_SOURCE_FILES "${PROC_DIR}/p.${TARGET}.cc")
    endforeach()
  endforeach()

  add_executable(${COMP_NAME} ${PROC_SOURCE_FILES})
  target_include_directories(${COMP_NAME} PUBLIC ${COMP_INCLUDE_DIRS})
  target_compile_definitions(${COMP_NAME} PUBLIC ${COMP_COMP_DEFS})
  target_link_libraries(${COMP_NAME} ${COMP_LINK_LIBS})
endfunction()

# - process yapgen -
set(COMP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
get_filename_component(COMP_NAME ${COMP_DIR} NAME)

set(COMP_HEADER_DIRS "${COMP_DIR}/header_files")
set(COMP_SOURCE_DIRS "${COMP_DIR}/source_files")
set(COMP_INCLUDE_DIRS ${LUA_INCLUDE_DIR})
set(COMP_COMP_DEFS)
set(COMP_LINK_LIBS ${LUA_LIBRARIES})

process_component()

# -----

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_definitions(-O2 -DNDEBUG -Wall -Wfatal-errors -Wno-class-memaccess -DLINUX)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  add_definitions("/DWINDOWS /D_CRT_SECURE_NO_WARNINGS /wd4244")
endif()

