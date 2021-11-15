set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(SOURCES main.cpp Instance.cpp Satelite.cpp Satelite.h TruckRoute.cpp TruckRoute.h)
add_executable(run ${SOURCES} Solution.cpp EvRoute.cpp Depot.cpp Depot.h greedyAlgorithm.cpp greedyAlgorithm.h)

#target_link_libraries(use_boost ${Boost_LIBRARIES})
target_link_libraries(run LINK_PUBLIC ${Boost_LIBRARIES})
