include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(debug)

add_executable(emulator main.cpp Processor.cpp Memory.cpp Instruction.cpp)
target_link_libraries(emulator debugger unwind readline)
