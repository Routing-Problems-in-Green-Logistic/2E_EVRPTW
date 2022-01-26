//
// Created by igor on 23/01/2022.
//

#include "ViabilizadorRotaEv.h"
#include "Auxiliary.h"

using namespace NameViabRotaEv;
using namespace NS_Auxiliary;

/**
 *
 * @param evRoute
 * @param pos
 * @param instance
 * @param best
 * @param insercaoEstacao
 * @return
 */

bool NameViabRotaEv::viabilizaRotaEv(vector<int> &route, int tamRoute, const Instance &instance, const bool best, InsercaoEstacao &insercaoEstacao)
{
    //PRINT_DEBUG("", "Rota: ");
    //for(int i=0; i < tamRoute; ++i)
    //    cout<<route[i]<<" ";

    //cout<<"\n\n";

    std::vector<PosicaoEstacao> vectorPosicaoEstacao(instance.getN_RechargingS());
    int proxVectorPosEst = 0;

    // Encontra estacoes que sao utilizadas
    for(int i=0; i < tamRoute; ++i)
    {
        if(instance.isRechargingStation(route[i]))
        {
            vectorPosicaoEstacao[proxVectorPosEst].pos = i;
            vectorPosicaoEstacao[proxVectorPosEst].rechargingStationId = route[i];
            ++proxVectorPosEst;
        }
    }

    if(proxVectorPosEst == instance.getN_RechargingS())
        return false;

    if(proxVectorPosEst != 0)
        sort(vectorPosicaoEstacao.begin(), vectorPosicaoEstacao.begin()+proxVectorPosEst);



    // Encontra estacoes que nao sao utilizadas
    std::vector<int> estacoesNaoUtilizadas(instance.getN_RechargingS());
    int proxEstN = 0;
    PosicaoEstacao posicaoEstacao;

    //cout<<"proxVectorPosEst: "<<proxVectorPosEst<<"\n";

    for(int i=instance.getFirstRechargingSIndex(); i <= instance.getEndRechargingSIndex(); ++i)
    {
        posicaoEstacao.rechargingStationId = i;
        if(buscaBinaria(vectorPosicaoEstacao, posicaoEstacao, proxVectorPosEst) == -1)
        {
            estacoesNaoUtilizadas[proxEstN] = i;
            proxEstN += 1;
        }
    }

    if(proxEstN == 0)
        return false;

    insercaoEstacao.distanciaRota = FLOAT_MAX;
    insercaoEstacao.estacao = -1;
    insercaoEstacao.pos = -1;

    for(int est = 0; est < proxEstN; ++est)
    {

        shiftVectorDir(route, 1, 1, tamRoute);
        tamRoute += 1;

        for(int i = 0; (i+2) < (tamRoute); ++i)
        {
            route[i + 1] = estacoesNaoUtilizadas[est];
            float dist = testaRota(route, tamRoute, instance, nullptr);

            if(dist > 0.0 && dist < insercaoEstacao.distanciaRota)
            {
                insercaoEstacao.distanciaRota = dist;
                insercaoEstacao.pos = i;
                insercaoEstacao.estacao = estacoesNaoUtilizadas[est];

                if(!best)
                    return true;
            }

            route[i+1] = route[i+2];

        }

        route[tamRoute-2] = route[tamRoute-1];
        tamRoute -= 1;

    }


    if(insercaoEstacao.distanciaRota < FLOAT_MAX)
    {
        return true;
    }
    else
        return false;

}

float NameViabRotaEv::testaRota(const vector<int> &evRoute, const int tamRoute, const Instance &instance, vector<float> *vetRemainingBattery)
{

    float bateriaRestante = instance.getEvBattery();
    float distanciaRota = 0.0;

/*    PRINT_DEBUG("\t", "TESTA ROTA:");
    cout<<"\tRota: "<<printVector(evRoute, tamRoute)<<";  ";*/

    for(int i=0; i < (tamRoute-1); ++i)
    {
        float dist = instance.getDistance(evRoute[i], evRoute[i+1]);
        bateriaRestante -= dist;
        distanciaRota += dist;

        if(bateriaRestante < -BATTERY_TOLENCE)
        {
            //cout<<"FALHA\n";
            return -1.0;
        }

        if(instance.isRechargingStation(evRoute[i+1]))
            bateriaRestante = instance.getEvBattery();

        if(vetRemainingBattery)
            (*vetRemainingBattery)[i+1] = bateriaRestante;

    }

    //cout<<"OK\n\tDISTANCIA: "<<distanciaRota<<"\n";
    return distanciaRota;

}
