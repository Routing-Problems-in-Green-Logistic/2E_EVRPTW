#include "EvRoute.h"
#include "Auxiliary.h"
#include <cfloat>

#ifndef INC_2E_EVRP_VIABILIZADORROTAEV_H
#define INC_2E_EVRP_VIABILIZADORROTAEV_H


namespace NameViabRotaEv
{

    struct InsercaoEstacao
    {
        int estacao = -1;
        int pos = -1;
        float distanciaRota = FLOAT_MAX;

        friend bool operator < (const InsercaoEstacao &aux0, const InsercaoEstacao &aux1)
        {
            return aux0.distanciaRota < aux1.distanciaRota;
        }
    };

    bool viabilizaRotaEv(vector<int> &route, int tamRout, const Instance &instance, const bool best, InsercaoEstacao &insercaoEstacao);
    float testaRota(const vector<int> &evRoute, const int tamRoute, const Instance &instance,
                    vector<float> *vetRemainingBattery);

};


#endif //INC_2E_EVRP_VIABILIZADORROTAEV_H
