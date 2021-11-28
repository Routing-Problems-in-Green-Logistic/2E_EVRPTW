set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D __FILENAME__='\"$(subst  ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
#CXX_FLAGS+=-D__FILENAME__='\"$(subst $(SOURCE_PREFIX)/,,$(abspath $<))\"'"

set(SOURCES main.cpp Instance.cpp Satelite.cpp Satelite.h TruckRoute.cpp TruckRoute.h Auxiliary.cpp Auxiliary.h LocalSearch.cpp LocalSearch.h Route.cpp)
add_executable(run ${SOURCES} Solution.cpp EvRoute.cpp Depot.cpp Depot.h greedyAlgorithm.cpp greedyAlgorithm.h)

#target_link_libraries(use_boost ${Boost_LIBRARIES})
target_link_libraries(run LINK_PUBLIC ${Boost_LIBRARIES})
