set(CMAKE_CXX_STANDARD 20)
cmake_minimum_required(VERSION 3.20)
project(2E_EVRP)

set(CMAKE_CXX_FLAGS "-O3 -g3")
#set(CMAKE_CXX_FLAGS "-march=native")

#set(CMAKE_CXX_FLAGS "-g3 -ggdb")
#set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D __FILENAME__='\"$(subst  ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
#CXX_FLAGS+=-D__FILENAME__='\"$(subst $(SOURCE_PREFIX)/,,$(abspath $<))\"'"

set(SOURCES newMain.cpp
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
            IG.cpp)



add_executable(run ${SOURCES})# INSTANCIA/ParametrosEntrada.cpp INSTANCIA/ParametrosEntrada.h)


#target_include_directories(run PUBLIC "ACO")
#add_subdirectory("ACO")

add_subdirectory("INSTANCIA")
target_include_directories(run PUBLIC "INSTANCIA")
link_directories("INSTANCIA")


target_sources(run PUBLIC ${SOURCES})

#target_link_libraries(use_boost ${Boost_LIBRARIES})
target_link_libraries(run LINK_PUBLIC ${Boost_LIBRARIES})
