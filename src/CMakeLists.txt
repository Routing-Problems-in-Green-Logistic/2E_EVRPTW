find_package(Boost 1.77)
include_directories(${BOOST_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCES main.cpp Instance.cpp Solution.cpp algorithm.cpp localsearches.cpp localSearch.cpp vns.cpp mersenne-twister.cpp)
add_executable(run ${SOURCES})

target_link_libraries(use_boost ${Boost_LIBRARIES})
