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
#include "Construtivo2.h"
#include "Vnd.h"
#include "mersenne-twister.h"
#include <unordered_set>
#include "VetorHash.h"

using namespace NameS_IG;
using namespace NameS_Grasp;
using namespace NS_Construtivo;
using namespace NS_Construtivo2;
using namespace NS_vnd;
using namespace NS_VetorHash;

// ./run ../../instancias/2e-vrp-tw/Customer_100/C101_21x.txt --numItTotal 1000 --mt IG --seed 1673390763

Solucao* NameS_IG::iteratedGreedy(Instancia &instancia, ParametrosGrasp &parametros, NameS_Grasp::Estatisticas &estat,
                              const ublas::matrix<int> &matClienteSat, BoostC::vector<NS_vnd::MvValor> &vetMvValor,
                              BoostC::vector<NS_vnd::MvValor> &vetMvValor1Nivel, NS_parametros::ParametrosSaida &parametrosSaida)
{

    std::unordered_set<VetorHash, VetorHash::HashFunc> hashSolSetCorrente;    // Solucao Corrente
    int numSolCorrente = 0;

    std::unordered_set<VetorHash, VetorHash::HashFunc> hashSolSetConst;       // Apos construtivo
    std::unordered_set<VetorHash, VetorHash::HashFunc> hashSolSetVnd;         // Apos vnd
    int numSolConstVia = 0;

    // Gera uma sol inicial com grasp
    NS_parametros::ParametrosSaida parametrosSaidaGrasp = parametrosSaida;
    ParametrosGrasp parametrosGrasp = parametros;
    //parametrosGrasp.numIteGrasp = 300;

    Solucao *solG = grasp(instancia, parametrosGrasp, estat, true,
                          matClienteSat, vetMvValor, vetMvValor1Nivel, parametrosSaidaGrasp);

    if(!solG->viavel)
        return nullptr;

    Solucao solBest(instancia);
    solBest.copia(*solG);
    delete solG;
    solG = nullptr;

cout<<"GRASP: "<<solBest.distancia<<"\n\n";

    Solucao solC(instancia);
    solC.copia(solBest);
    int numEvN_Vazias = 0;

    for(int sat=1; sat <= instancia.numSats; ++sat)
    {
        for(int ev=0; ev < instancia.numEv; ++ev)
        {
            EvRoute &evRoute = solBest.satelites[sat].vetEvRoute[ev];
            if(evRoute.routeSize > 2)
                numEvN_Vazias += 1;
        }
    }


    //Parametros
    const float alfa  = 0.8; //0.8
    const float beta  = alfa;

    const int numEvRm = min(int(0.1*numEvN_Vazias+1), 100);
    const int numItSemMelhoraResetSolC = 10;
    int ultimaA = 0;
    int numSolG = 1;

    //cout<<"NUM EV: "<<numEvRm<<"\n\n";

    auto funcDestroi0 = [&](Solucao &sol, const int numRotas)
    {
        for(int i=0; i < numRotas; ++i)
        {
            bool escolheRota = false;

            // Percorre os sat
            const int satIdPrim = rand_u32() % instancia.numSats;
            int satId = satIdPrim;

            do
            {
                Satelite &sat = sol.satelites[satId];

                // Percorre os EVs
                const int evPrim = rand_u32() % instancia.numEv;
                int ev = evPrim;

                do
                {
                    EvRoute &evRoute = sat.vetEvRoute[ev];
                    if(evRoute.routeSize <= 2)
                    {
                        ev = (ev+1) % instancia.numEv;
                        continue;
                    }

                    //cout<<"\t\t";

                    escolheRota = true;
                    for(int j=1; j < (evRoute.routeSize-1); ++j)
                    {
                        if(instancia.isClient(evRoute[j].cliente))
                        {
                            sol.vetClientesAtend[evRoute[j].cliente] = int8_t(0);
                            //cout<<evRoute[j].cliente<<" ";
                        }
                    }

                    //cout<<"\n\n";

                    // Remove distancia do ev do sat e da sol
                    sat.distancia -= evRoute.distancia;
                    sol.distancia -= evRoute.distancia;

                    // Remove carga do ev do sat e da sol
                    sat.demanda -= evRoute.demanda;
                    int temp = evRoute.idRota;

                    //string strRota;
                    //evRoute.print(strRota, instancia, true);
                    //cout<<"ROTA ANTES: "<<strRota<<"\n";

                    sat.vetEvRoute[ev] = EvRoute(satId, evRoute.idRota, evRoute.routeSizeMax, instancia);
                    sat.vetEvRoute[ev].idRota = temp;

                    //strRota = "";
                    //evRoute.print(strRota, instancia, true);
                    //cout<<"ROTA DEPOIS: "<<strRota<<"\n\n";

                    sol.numEv -= 1;
                    sol.rotaEvAtualizada(satId, ev);
                    break;

                }while(ev != evPrim);

                if(escolheRota)
                    break;

                satId = (satId+1) % instancia.numSats;
            }while(satId != satIdPrim);
        }

        sol.reseta1Nivel(instancia);
        sol.resetaIndiceEv(instancia);

    };

    for(int i=0; i < parametros.numIteGrasp; ++i)
    {
        if((i-ultimaA) == numItSemMelhoraResetSolC)
        {
            solC = Solucao(instancia);
            solC.copia(solBest);
            ultimaA = i;
        }

        hashSolSetCorrente.insert(VetorHash(solC, instancia));
        numSolCorrente += 1;

//cout<<"SOL "<<i<<": "<<solC.distancia<<"\n";

        funcDestroi0(solC, numEvRm);

//cout<<"***********************************\n\n";


        construtivo(solC, instancia, alfa, beta, matClienteSat, true, true, false);
        //construtivo(solC, instancia, alfa, beta, matClienteSat, true, true);

        if(!solC.viavel)
        {
            solC = Solucao(instancia);
            solC.copia(solBest);
//cout<<"\tSOL INVIAVEL\n";

        }
        else
        {
            hashSolSetConst.insert(VetorHash(solC, instancia));
            numSolConstVia += 1;

            numSolG += 1;
            rvnd(solC, instancia, beta, vetMvValor, vetMvValor1Nivel);
            hashSolSetVnd.insert(VetorHash(solC, instancia));

//cout<<"\tSOL VIAVEL "<<i<<": "<<solC.distancia<<"\n";
        }

        if(NS_Auxiliary::menor(solC.distancia, solBest.distancia))
        {
            solBest = Solucao(instancia);
            solBest.copia(solC);
            solBest.ultimaA = i;
cout<<"ATUALIZACAO "<<i<<": "<<solBest.distancia<<"\n\n";

        }
    }

    solC = Solucao(instancia);
    solC.copia(solBest);

    for(int i=0; i < 10; ++i)
    {
        solC.reseta1Nivel(instancia);
        construtivoPrimeiroNivel(solC, instancia, beta, true);
        if(NS_Auxiliary::menor(solC.distancia, solBest.distancia))
        {
            solBest = Solucao(instancia);
            solBest.copia(solC);

cout<<"UPDATE 1 NIVEL: "<<solBest.distancia<<"\n\n";

        }
    }

    double fatorSolCorr = (double(hashSolSetCorrente.size())/numSolCorrente) * 100;
    double fatorSolConst = (double(hashSolSetConst.size())/numSolConstVia) * 100;
    double fatorSolVnd = (double(hashSolSetVnd.size())/numSolConstVia) * 100;

cout<<"Sol Corrente: "<<fatorSolCorr<<"\n";
cout<<"Sol Construtivo: "<<fatorSolConst<<"\n";
cout<<"Sol VND: "<<fatorSolVnd<<"\n\n";

    auto funcAddParaSaida = [&](string &&strParm, double val)
    {

        parametrosSaida.mapNoSaida[strParm] = NS_parametros::NoSaida(strParm);
        parametrosSaida.mapNoSaida[strParm].addSaida(SAIDA_EXEC_VAL);
        parametrosSaida.mapNoSaida[strParm](val);
    };

    funcAddParaSaida("numSol", numSolG);
    funcAddParaSaida("fatorSolCorr", fatorSolCorr);
    funcAddParaSaida("fatorSolConst", fatorSolConst);
    funcAddParaSaida("fatorSolVnd", fatorSolVnd);

    Solucao *solPtr = new Solucao(instancia);
    solPtr->copia(solBest);
    return solPtr;
}
