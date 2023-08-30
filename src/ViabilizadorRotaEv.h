#include "EvRoute.h"
#include "Auxiliary.h"
#include <cfloat>

#ifndef INC_2E_EVRP_VIABILIZADORROTAEV_H
#define INC_2E_EVRP_VIABILIZADORROTAEV_H


namespace NS_viabRotaEv
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

    bool viabilizaRotaEv(EvRoute &evRoute, Instancia &instance, const bool best,
                         NS_viabRotaEv::InsercaoEstacao &insercaoEstacao, double custoInserMax, const bool construtivo,
                         const double tempoSaidaSat, Vector<int> *vetInviabilidade);

    double testaRota(EvRoute &evRoute, const int tamRoute, const Instancia &instance, const bool escrita,
                     const double tempoSaidaSat, const int posIni, string *rotaBtDebug,
                     Vector<int> *vetInviabilidade);

    double testaRotaTempo(EvRoute &evRoute, const int tamRoute,
                          const Instancia &instance, const bool escrita,
                          const double tempoSaidaSat, const int posIni=0);
};


#endif //INC_2E_EVRP_VIABILIZADORROTAEV_H
