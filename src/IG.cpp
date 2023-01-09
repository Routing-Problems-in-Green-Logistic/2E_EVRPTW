//
// Created by igor on 09/01/23.
//

/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    09/01/23
 *  Arquivo: IG.cpp
 * ****************************************
 * ****************************************/

#include "IG.h"
#include "Construtivo.h"
#include "Vnd.h"

using namespace NameS_IG;
using namespace NameS_Grasp;
using namespace NS_Construtivo;
using namespace NS_vnd;

void NameS_IG::iteratedGreedy(Instancia &instance, ParametrosGrasp &parametros, NameS_Grasp::Estatisticas &estat,
                              const ublas::matrix<int> &matClienteSat, BoostC::vector<NS_vnd::MvValor> &vetMvValor,
                              BoostC::vector<NS_vnd::MvValor> &vetMvValor1Nivel, NS_parametros::ParametrosSaida &parametrosSaida)
{

    NS_parametros::ParametrosSaida parametrosSaidaGrasp = parametrosSaida;
    Solucao *solG = grasp(instance, parametros, estat, true,
                          matClienteSat, vetMvValor, vetMvValor1Nivel, parametrosSaidaGrasp);

    if(!solG->viavel)
        return;

    Solucao solBest(instance);
    solBest.copia(*solG);
    delete solG;
    solG = nullptr;

    Solucao solC(instance);
    solC.copia(solBest);

    auto funcDestroi0 = [&](Solucao &sol, const int numRotas, BoostC::vector<int> &vetClientesRm)
    {
        for(int i=0; i < numRotas; ++i)
        {

        }
    };

    BoostC::vector<int> vetClientesRm(instance.numClients+1, 0);

    float alfa = 0.1;
    float beta = 0.1;

    for(int i=0; i < parametros.numIteGrasp; ++i)
    {
        std::fill(vetClientesRm.begin(), vetClientesRm.end(), 0);
        funcDestroi0(solC, 1, vetClientesRm);
        construtivo(solC, instance, alfa, beta, matClienteSat, true);

        if(!solC.viavel)
        {
            solC = Solucao(instance);
            solC.copia(solBest);
        }
        else
        {
            rvnd(solC, instance, beta, vetMvValor, vetMvValor1Nivel);
        }

        if(NS_Auxiliary::menor(solC.distancia, solBest.distancia))
        {
            solBest.copia(solC);
            solBest.ultimaA = i;
        }

    }

}
