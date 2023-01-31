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

    rvnd(*solG, instancia, 0.4, vetMvValor, vetMvValor1Nivel);

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

    const int numEvRm = min(int(0.1*numEvN_Vazias+1), 5);
    const int numItSemMelhoraResetSolC = 400;
    int ultimaA = 0;
    int numSolG = 1;
    int numFuncDestroi = 0;

    //cout<<"NUM EV: "<<numEvRm<<"\n\n";


    // Escolhe aleatoriamente numRotas nao vazias e as removem da solucao
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

    // Remove aleatoriamente um sat, o sat pode estar vazio(nao faz nada)
    auto funcDestroi1 = [&](Solucao &sol) -> bool
    {
        if(instancia.numSats == 1)
            return false;

        int sat = 1 + (rand_u32()%instancia.numSats);
        Satelite &satelite = sol.satelites[sat];
        if(satelite.vazio())
            return false;

        const int numEvSat = satelite.numEv();
        sol.numEv -= numEvSat;

        double dist = satelite.excluiSatelite(instancia, sol.vetClientesAtend);
        sol.distancia -= dist;
        sol.vetMatSatEvMv[sat] =  ublas::matrix<int>(instancia.numEv, NUM_MV, 0);

        sol.reseta1Nivel(instancia);
        sol.resetaIndiceEv(instancia);

        return true;
    };

    for(int i=0; i < parametros.numIteGrasp; ++i)
    {
        if(i%200 == 0)
            cout<<"ITERACAO: "<<i<<"\n";

        if((i-ultimaA) == numItSemMelhoraResetSolC)
        {
            solG = nullptr;
            while(solG == nullptr)
            {
                solG = grasp(instancia, parametrosGrasp, estat, true, matClienteSat, vetMvValor, vetMvValor1Nivel, parametrosSaidaGrasp);
            }

            if(!solG->viavel)
            {
                cout<<"SOL INVIAVEL!\n";
                throw "ERRO";
            }

            rvnd(*solG, instancia, 0.4, vetMvValor, vetMvValor1Nivel);

            solC = Solucao(instancia);
            solC.copia(*solG);
            ultimaA = i;
            numFuncDestroi = 0;
            delete solG;
        }
        else if((i-ultimaA) % 100 == 0 && i!=ultimaA)
        {
            solC = Solucao(instancia);
            solC.copia(solBest);
        }

        hashSolSetCorrente.insert(VetorHash(solC, instancia));
        numSolCorrente += 1;

//cout<<"SOL "<<i<<": "<<solC.distancia<<"\n";

        if(numFuncDestroi == 0)
        {
            funcDestroi0(solC, numEvRm);
            numFuncDestroi = 1;
        }
        else
        {
            if(!funcDestroi1(solC))
                funcDestroi0(solC, numEvRm);

            numFuncDestroi = 0;
        }

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
            string strErro;
            if(!solC.checkSolution(strErro, instancia))
            {
                PRINT_DEBUG("", "");
                cout<<"ERRO NO IG APOS O CONSTRUTIVO\nERRO:\n\n";
                cout<<strErro<<"\n\n";
                throw "ERRO";
            }

            hashSolSetConst.insert(VetorHash(solC, instancia));
            numSolConstVia += 1;

            numSolG += 1;
            rvnd(solC, instancia, beta, vetMvValor, vetMvValor1Nivel);
            hashSolSetVnd.insert(VetorHash(solC, instancia));

//cout<<"\tSOL VIAVEL "<<i<<": "<<solC.distancia<<"\n";
        }

        if(NS_Auxiliary::menor(solC.distancia, solBest.distancia))
        {
            string erro;
            if(!solC.checkSolution(erro, instancia))
            {
                PRINT_DEBUG("", "");
                cout<<"ERRO, IG\n";
                cout<<erro<<"\n\n";
                throw "ERRO";
            }

            solBest = Solucao(instancia);
            solBest.copia(solC);
            solBest.ultimaA = i;
            numFuncDestroi = 0;
cout<<"ATUALIZACAO "<<i<<": "<<solBest.distancia<<"\n\n";

        }
    }

    double fatorSolCorr = (double(hashSolSetCorrente.size())/numSolCorrente) * 100;
    double fatorSolConst = (double(hashSolSetConst.size())/numSolConstVia) * 100;
    double fatorSolVnd = (double(hashSolSetVnd.size())/numSolConstVia) * 100;

//cout<<"Sol Corrente: "<<fatorSolCorr<<"\n";
//cout<<"Sol Construtivo: "<<fatorSolConst<<"\n";
//cout<<"Sol VND: "<<fatorSolVnd<<"\n\n";

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

    string erro;
    if(!solBest.checkSolution(erro, instancia))
    {

        PRINT_DEBUG("", "");
        cout<<"ERRO, IG\n";
        cout<<erro<<"\n\n";
        throw "ERRO";
    }

    cout<<"IG: "<<solBest.distancia<<"\n\n";

    Solucao *solPtr = new Solucao(instancia);
    solPtr->copia(solBest);
    return solPtr;
}
