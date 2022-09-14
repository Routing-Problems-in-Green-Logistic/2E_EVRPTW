set(CMAKE_CXX_STANDARD 20)
cmake_minimum_required(VERSION 3.20)
project(2E_EVRP)


#set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_RELEASE, "-march=native")

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


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D __FILENAME__='\"$(subst  ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
#CXX_FLAGS+=-D__FILENAME__='\"$(subst $(SOURCE_PREFIX)/,,$(abspath $<))\"'"

set(SOURCES main.cpp
            Satelite.cpp
            Satelite.h
        Auxiliary.cpp
            Auxiliary.h
            LocalSearch.cpp
            LocalSearch.h
            Route.cpp
            Vnd.cpp
            Vnd.h
            ViabilizadorRotaEv.cpp
            ViabilizadorRotaEv.h
            Teste.cpp
            Teste.h
        Solucao.cpp
        EvRoute.cpp
        greedyAlgorithm.cpp
        greedyAlgorithm.h
        mersenne-twister.h
        mersenne-twister.cpp
        Constants.h
        Constants.cpp
        Grasp.cpp
        Grasp.h
        PreProcessamento.cpp
        PreProcessamento.h
        Heap.h
        common.h)



add_executable(run INSTANCIA/ParametrosEntrada.cpp INSTANCIA/ParametrosEntrada.h)


#target_include_directories(run PUBLIC "ACO")
#add_subdirectory("ACO")


target_include_directories(run PUBLIC "INSTANCIA")
add_subdirectory("INSTANCIA")

target_sources(run PUBLIC ${SOURCES})

#target_link_libraries(use_boost ${Boost_LIBRARIES})
target_link_libraries(run LINK_PUBLIC ${Boost_LIBRARIES})
