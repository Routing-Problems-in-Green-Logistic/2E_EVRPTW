

#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_FLAGS_RELEASE "-g3")
set(CMAKE_CXX_FLAGS_RELEASE "-ggdb")
set(CMAKE_BUILD_TYPE Debug)


set(CMAKE_CXX_FLAGS_RELEASE "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-Wnull-dereference")
set(CMAKE_CXX_FLAGS_RELEASE "-fdelete-null-pointer-checks")
set(CMAKE_CXX_FLAGS_RELEASE "-Wuninitialized")
set(CMAKE_CXX_FLAGS_RELEASE "-Winit-self")
set(CMAKE_CXX_FLAGS_RELEASE "-Wmisleading-indentation")
set(CMAKE_CXX_FLAGS_RELEASE "-Wunknown-pragmas")
set(CMAKE_CXX_FLAGS_RELEASE "-Wfloat-equal")
set(CMAKE_CXX_FLAGS_RELEASE "-Wconversion")



target_sources(run PUBLIC Aco.h Aco.cpp)