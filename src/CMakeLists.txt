set(SOURCES main.cpp Instance.cpp Satelite.cpp Satelite.h)
add_executable(run ${SOURCES} Solution.cpp algorithm.cpp localsearches.cpp localSearch.cpp vns.cpp)
