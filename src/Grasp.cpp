/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    02/06/22
 *  Arquivo: Grasp.cpp
 * ****************************************
 * ****************************************/

#include "Grasp.h"
#include "greedyAlgorithm.h"
#include "Auxiliary.h"
#include "mersenne-twister.h"

#define NUM_EST_INI 3

using namespace GreedyAlgNS;
using namespace NameS_Grasp;

/*
Solucao* NameS_Grasp::grasp(Instance &instance, const int numIte, const float alfa, const float beta, Estatisticas &estat)
{
    Solucao *solBest = new Solucao(instance);
    solBest->distancia = DOUBLE_MAX;

    estat.numSol = 0.0;
    estat.numIte = numIte;
    estat.distAcum = 0.0;
    estat.erro = "";
    bool gabi = false;

    for(int i=0; i < numIte; ++i)
    {
        Solucao sol(instance);


        if(estat.numSol == 0 && i == 110)
        {
            gabi = true;

        }

        if(gabi)


        construtivo(sol, instance, alfa, beta);

        if(sol.viavel)
        {

            if(!sol.checkSolution(estat.erro, instance))
            {
                cout<<"\n\nSOLUCAO:\n\n";
                sol.print(instance);

                cout << estat.erro<< "\n****************************************************************************************\n\n";
                delete solBest;
                return nullptr;
            }
            else
            {

                estat.numSol += 1;
                estat.distAcum += sol.distancia;

                if(sol.distancia < solBest->distancia)
                {
                    solBest->copia(sol);
                }
            }
        }

    }

    return solBest;

}


void NameS_Grasp::inicializaSol(Solucao &sol, Instance &instance)
{

    std::vector<EstDist> vetEstDist(instance.getN_RechargingS());
    EvRoute evRoute(-1, instance.getFirstEvIndex(), instance.getEvRouteSizeMax(), instance);

    for(int i=instance.getFirstSatIndex(); i <= instance.getEndSatIndex(); ++i)
    {
        int vetTam = 0;
        evRoute.satelite = i;

        for(int r=instance.getFirstRechargingSIndex(); r <= instance.getEndRechargingSIndex(); ++r)
        {
            vetEstDist[vetTam].estacao = r;
            vetEstDist[vetTam].distancia = instance.getDistance(i, r);
            ++vetTam;
        }

        std::sort(vetEstDist.begin(), vetEstDist.begin()+vetTam);

        const int numEst = min(NUM_EST_INI, instance.getN_RechargingS());
        const int pos = (rand_u32()%numEst);
        int p = pos;
        evRoute[0].cliente = evRoute[2].cliente = i;

        double taxaConsumoDist = instance.vectVeiculo[instance.getFirstEvIndex()].taxaConsumoDist;

        do
        {
            evRoute[1].cliente = vetEstDist[p].estacao;


            if(vetEstDist[p].distancia * instance.vectVeiculo[instance.getFirstEvIndex()].taxaConsumoDist < instance.vectVeiculo[instance.getFirstEvIndex()].capacidadeBateria)
            {

            }
            p += 1;
        }while(p != pos);
    }


}
*/