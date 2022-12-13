

target_sources(run PUBLIC Instancia.h Instance.cpp Parametros.h Parametros.cpp)

target_include_directories(run PUBLIC "../ACO" )
add_subdirectory("../ACO" ACO)


target_include_directories(run PUBLIC "../AG" )
add_subdirectory("../AG" AG)


target_include_directories(run PUBLIC "../K_MEANS" )
add_subdirectory("../K_MEANS" K_MEANS)

target_include_directories(run PUBLIC "../HASH" )
add_subdirectory("../HASH" HASH)

target_include_directories(run PUBLIC "../BUSCA_LOCAL" )
add_subdirectory("../BUSCA_LOCAL" BUSCA_LOCAL)

target_include_directories(run PUBLIC "../CONSTRUTIVO" )
add_subdirectory("../CONSTRUTIVO" CONSTRUTIVO)
