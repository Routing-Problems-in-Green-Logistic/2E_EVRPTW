set(CMAKE_CXX_STANDARD 20)
cmake_minimum_required(VERSION 3.1)
project(2E_EVRP)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_RELEASE "-Wall")
#set(CMAKE_CXX_FLAGS_RELEASE "-pg")

#debug de memnoria:
#cmake .. -DCMAKE_BUILD_TYPE=asan

set(CMAKE_CXX_FLAGS_TSAN  "-fsanitize=thread -g -O1" CACHE STRING "Flags used by the C++ compiler during ThreadSanitizer builds." FORCE)
set(CMAKE_CXX_FLAGS_ASAN "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1" CACHE STRING "Flags used by the C++ compiler during AddressSanitizer builds." FORCE)
set(CMAKE_CXX_FLAGS_LSAN "-fsanitize=leak -fno-omit-frame-pointer -g -O1" CACHE STRING "Flags used by the C++ compiler during LeakSanitizer builds." FORCE)
set(CMAKE_CXX_FLAGS_MSAN "-fsanitize=memory -fno-optimize-sibling-calls -fsanitize-memory-track-origins=2 -fno-omit-frame-pointer -g -O2" CACHE STRING "Flags used by the C++ compiler during MemorySanitizer builds." FORCE)
set(CMAKE_CXX_FLAGS_UBSAN "-fsanitize=undefined" CACHE STRING "Flags used by the C++ compiler during UndefinedBehaviourSanitizer builds." FORCE)

add_subdirectory(src)
