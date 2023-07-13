set(CMAKE_CXX_STANDARD 20)
cmake_minimum_required(VERSION 3.2)
project(2E_EVRP)

enable_language(CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_FLAGS "-O3")
#set(CMAKE_CXX_FLAGS "-march=native")

#set(CMAKE_CXX_FLAGS "-g3 -ggdb")
#set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D __FILENAME__='\"$(subst  ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
#CXX_FLAGS+=-D__FILENAME__='\"$(subst $(SOURCE_PREFIX)/,,$(abspath $<))\"'"


find_package(GUROBI REQUIRED)
include_directories(${GUROBI_INCLUDE_DIRS})
link_directories($ENV{GUROBI_HOME}/lib)


include_directories(INSTANCIA)
include_directories(ACO)
include_directories(K_MEANS)
include_directories(CONSTRUTIVO)
include_directories(BUSCA_LOCAL)
include_directories(MIP)

set(SOURCES
        INSTANCIA/Parametros.cpp
        INSTANCIA/Instance.cpp
        CONSTRUTIVO/Construtivo.cpp
        CONSTRUTIVO/Construtivo2.cpp
        CONSTRUTIVO/Construtivo3.cpp
        CONSTRUTIVO/Construtivo4.cpp
        BUSCA_LOCAL/LocalSearch.cpp
        BUSCA_LOCAL/LocalSearch2.cpp
        MIP/Modelo.cpp
        ACO/Aco.cpp
        K_MEANS/k_means.cpp
        newMain.cpp
        Satelite.cpp
        Satelite.h
        Auxiliary.cpp
        Auxiliary.h
        Route.cpp
        Vnd.cpp
        Vnd.h
        ViabilizadorRotaEv.cpp
        ViabilizadorRotaEv.h
        Teste.cpp
        Teste.h
        Solucao.cpp
        EvRoute.cpp
        mersenne-twister.h
        mersenne-twister.cpp
        Constants.h
        Constants.cpp
        Grasp.cpp
        Grasp.h
        PreProcessamento.cpp
        PreProcessamento.h
        Heap.h
        common.h
        VetorHash.h
        IG.h
        IG.cpp MIP/Variaveis.h MIP/Variaveis.cpp)



add_executable(run ${SOURCES})# INSTANCIA/ParametrosEntrada.cpp INSTANCIA/ParametrosEntrada.h)

target_link_libraries(run LINK_PUBLIC ${Boost_LIBRARIES})

target_link_libraries(run gurobi_c++)
target_link_libraries(run gurobi)

