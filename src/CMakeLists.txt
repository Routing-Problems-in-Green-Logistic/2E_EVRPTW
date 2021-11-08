find_package(Boost 1.77 REQUIRED)
include_directories(${BOOST_INCLUDE_DIRS})
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCES main.cpp Instance.cpp Solution.cpp algorithm.cpp localsearches.cpp localSearch.cpp vns.cpp mersenne-twister.cpp Constants.cpp Constants.h)
add_executable(run ${SOURCES})

target_link_libraries(run ${Boost_LIBRARIES})
