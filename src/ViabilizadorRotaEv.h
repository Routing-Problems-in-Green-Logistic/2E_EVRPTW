#include "EvRoute.h"
#include "Auxiliary.h"
#include <cfloat>

#ifndef INC_2E_EVRP_VIABILIZADORROTAEV_H
#define INC_2E_EVRP_VIABILIZADORROTAEV_H


namespace NameViabRotaEv
{

#if TEMPO_FUNC_VIABILIZA_ROTA_EV
    extern double global_tempo;
#endif

    struct InsercaoEstacao
    {
        int estacao = -1;
        int pos = -1;
        double distanciaRota = DOUBLE_MAX;

        friend bool operator < (const InsercaoEstacao &aux0, const InsercaoEstacao &aux1)
        {
            return aux0.distanciaRota < aux1.distanciaRota;
        }
    };

    bool viabilizaRotaEv(EvRoute &evRoute, const Instance &instance, const bool best,
                         NameViabRotaEv::InsercaoEstacao &insercaoEstacao, const double custoInserMax,
                         const bool construtivo, const double tempoSaidaSat);
    double testaRota(EvRoute &evRoute, const int tamRoute, const Instance &instance, const bool escrita, const double tempoSaidaSat);
    double testaRotaTempo(EvRoute &evRoute, const int tamRoute, const Instance &instance, const bool escrita, const double tempoSaidaSat);
};


#endif //INC_2E_EVRP_VIABILIZADORROTAEV_H
