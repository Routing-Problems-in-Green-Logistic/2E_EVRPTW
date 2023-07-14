/* ****************************************
 * ****************************************
 *  Nome: ***********************
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
#include <sys/stat.h>
#include <fstream>
#include "Construtivo3.h"
#include "Construtivo4.h"
#include "Parametros.h"
#include "Modelo.h"

using namespace NameS_IG;
using namespace NameS_Grasp;
using namespace NS_Construtivo;
using namespace NS_Construtivo2;
using namespace NS_vnd;
using namespace NS_VetorHash;
using namespace NS_parametros;
using namespace ModeloNs;


typedef std::unordered_set<NS_VetorHash::VetorHash, NS_VetorHash::VetorHash::HashFunc> SetVetorHash;

#define PRINT_IG        FALSE
#define WRITE_SOL_PRINT FALSE


enum Estrategia{Est100, Est15};

std::string NameS_IG::strDescricaoIg = "";
        //"Teste do construtivo(Construtivo2) com torneio para numero de clientes == 100\nALFA: 0.6";

Solucao* NameS_IG::iteratedGreedy(Instancia &instancia, ParametrosGrasp &parametrosGrasp, NameS_Grasp::Estatisticas &estat,
                                  const ublas::matrix<int> &matClienteSat, BoostC::vector<NS_vnd::MvValor> &vetMvValor,
                                  BoostC::vector<NS_vnd::MvValor> &vetMvValor1Nivel, NS_parametros::ParametrosSaida &parametrosSaida,
                                  NS_parametros::Parametros &parametros)
{


    //ParametrosIG parametrosIg(parametros.paramIgFile);
    ParametrosIG parametrosIg = parametros.paramIg;

    // Zera variaveis globais
    VarAuxiliaresIgNs::num_sumQuantCand     = 0;
    VarAuxiliaresIgNs::num_sumQuantCliRm    = 0;
    VarAuxiliaresIgNs::sumQuantCand         = 0;
    VarAuxiliaresIgNs::sumQuantCliRm        = 0;

    // Cria o diretorio plotInter
    const string dirPlotInter = parametros.caminhoPasta+"/plotInter";

#if WRITE_SOL_PRINT
    NS_Auxiliary::criaDiretorio(parametros.caminhoPasta);
    NS_Auxiliary::criaDiretorio(dirPlotInter);
#endif

    auto writeSol = [](const string &dirPlotInter, const int it, const string &solAntes, const string &solDepois, const string solVnd)
    {
        const string fileAntes = dirPlotInter + "/"+ to_string(it)+"_sol_antes.txt";
        const string fileDepois = dirPlotInter + "/"+ to_string(it)+"_sol_depois.txt";
        const string fileVnd    = dirPlotInter + "/"+ to_string(it)+"_sol_vnd.txt";



        std::ofstream outfile;
        outfile.open(fileAntes, std::ios_base::out);

        if(outfile.is_open())
        {
            outfile<<solAntes<<"\n";
            outfile.close();
        }
        else
        {
            cout<<"NAO FOI POSSIVEL ABRIR O ARQUIVO: "<<fileAntes<<"\n";
            throw "ERRO";
        }
        outfile.close();


        outfile.open(fileDepois, std::ios_base::out);

        if(outfile.is_open())
        {
            outfile<<solDepois<<"\n";
            outfile.close();
        }
        else
        {
            cout<<"NAO FOI POSSIVEL ABRIR O ARQUIVO: "<<fileDepois<<"\n";
            throw "ERRO";
        }

        outfile.close();


        outfile.open(fileVnd, std::ios_base::out);

        if(outfile.is_open())
        {
            outfile<<solVnd<<"\n";
            outfile.close();
        }
        else
        {
            cout<<"NAO FOI POSSIVEL ABRIR O ARQUIVO: "<<fileVnd<<"\n";
            throw "ERRO";
        }

        outfile.close();

    };

    SetVetorHash hashSolSetCorrente;    // Solucao Corrente
    int numSolCorrente = 0;

    SetVetorHash hashSolSetConst;       // Apos construtivo
    SetVetorHash hashSolSetVnd;         // Apos vnd

    SetVetorHash hashRotaEv;

    auto funcAddRotasHash = [](const Instancia &instancia, SetVetorHash &hash, const Solucao &solucao)
    {
        for(int sat=1; sat < instancia.numSats; ++sat)
        {
            const Satelite &satelite = solucao.satelites[sat];

            if(satelite.demanda <= 0.0)
                continue;

            for(int ev=0; ev < instancia.numEv; ++ev)
            {
                const EvRoute &evRoute = satelite.vetEvRoute[ev];
                if(evRoute.routeSize <= 2)
                    continue;

                hash.insert(VetorHash(evRoute));
            }
        }
    };

    int numSolConstVia = 0;

    // Gera uma sol inicial com grasp
    NS_parametros::ParametrosSaida parametrosSaidaGrasp = parametrosSaida;

    Solucao *solG = grasp(instancia, parametrosGrasp, estat, true, matClienteSat, vetMvValor, vetMvValor1Nivel, parametrosSaidaGrasp);

    if(!solG->viavel)
        return nullptr;

    rvnd(*solG, instancia, 0.4, vetMvValor, vetMvValor1Nivel);

    Solucao solBest(instancia);
    solBest.copia(*solG);
    ValBestNs::distBest = solBest.distancia;

    delete solG;
    solG = nullptr;

#if PRINT_IG
cout<<"GRASP: "<<solBest.distancia<<"\n\n";
#endif

    Solucao solC(instancia);
    solC.copia(solBest);
    int temp = 0;

    for(int sat=1; sat <= instancia.numSats; ++sat)
    {
        for(int ev=0; ev < instancia.numEv; ++ev)
        {
            EvRoute &evRoute = solBest.satelites[sat].vetEvRoute[ev];
            if(evRoute.routeSize > 2)
                temp += 1;
        }
    }

    const int numEvN_Vazias  = temp;
    const int estrategia     = (instancia.numClients > 15) ? Int8(Est100):Int8(Est15);
    const double difMax      = 0.1;
    const float alfaSeg      = parametrosIg.alfaSeg;         // Segundo Nivel
    const float betaPrim     = parametrosIg.betaPrim;        // Primeiro  Nivel
    const int numEvRmMin     = min(int(parametrosIg.taxaRm * numEvN_Vazias + 1), numEvN_Vazias);
    int numEvRmCorrente      = numEvRmMin;
    const int numClientesRm  = max(int(NS_Auxiliary::upperVal(parametrosIg.taxaRm*instancia.numClients)), 1);

    int ultimaA             = 0;
    int ultimaABest         = 0;
    int numSolG             = 1;
    int numFuncDestroi      = 0;
    const int numChamadasDestroi0 = int(NS_Auxiliary::upperVal(numEvN_Vazias/float(numEvRmCorrente)))*parametrosIg.fatorNumChamadas;
    //cout<<"numChamadasDestroi: "<<numChamadasDestroi0<<"\n";

    //auto funcAtualNumChamDest0 = [&](){numChamadasDestroi0 = int(NS_Auxiliary::upperVal(numEvN_Vazias/float(numEvRmCorrente)));};

    const bool torneio = parametrosIg.torneio;

    int tempTipoConst = CONSTRUTIVO1;
    if(estrategia == Est15)
        tempTipoConst = parametrosIg.tipoConstrutivo15;

    const int tipoConst = tempTipoConst;


    //const Int8 estrategia = Int8(0);


#if PRINT_IG

    cout<<"ParametrosIG: \n"<<parametrosIg.printParam()<<"\n\n";

    cout<<"destroi0 num chamadas: "<<numChamadasDestroi0<<"\n";
    cout<<"num rotas removidas min: "<<numEvRmMin<<"\n";
    cout<<"num rotas nao vazias: "<<numEvN_Vazias<<"\n";
    cout<<"num clientes removidos: "<<numClientesRm<<"\n";
    cout<<"Estrategia: "<<int(estrategia)<<"\n";
    cout << "ALFA: " << alfaSeg << "\nBETA: " << betaPrim << "\n";
    cout<<"TORNEIO: "<<torneio<<"\n\n";
#endif


    // Quarda o numero de inviabilidades
    BoostC::vector<int> vetInviabilidate(SIZE_ENUM_INV, 0);

    // Escolhe aleatoriamente numRotas nao vazias e as removem da solucao
    auto funcDestroi0 = [&](Solucao &sol, const int numRotas)
    {
        bool rmPeloMenosUmCli = false;

        for(int i=0; i < numRotas; ++i)
        {
            bool escolheRota = false;

            // Percorre os sat
            const int satIdPrim = 1 + (rand_u32() % instancia.numSats);
            int satId = satIdPrim;

            do
            {
                if(satId == 0)
                {
                    satId = 1;
                    if(satIdPrim == satId)
                        break;
                    else
                        continue;
                }

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
                            VarAuxiliaresIgNs::sumQuantCliRm += 1;
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
                    sat.vetEvRoute[ev] = EvRoute(satId, evRoute.idRota, evRoute.routeSizeMax, instancia);
                    sat.vetEvRoute[ev].idRota = temp;
                    sol.numEv -= 1;


#if UTILIZA_MAT_MV
                    sol.rotaEvAtualizada(satId, ev);
#endif

                    break;

                }while(ev != evPrim);

                if(escolheRota)
                {   rmPeloMenosUmCli = true;
                    break;
                }

                satId = (satId+1) % (instancia.numSats+1);
            }while(satId != satIdPrim);
        }

        if(rmPeloMenosUmCli)
            VarAuxiliaresIgNs::num_sumQuantCliRm += 1;

        sol.reseta1Nivel(instancia);
        sol.resetaIndiceEv(instancia);
        //cout<<"********************\n\n";

    };

    // Remove aleatoriamente um sat, o sat pode estar vazio(nao faz nada)
    auto funcDestroi1 = [&](Solucao &sol, int numSat) -> bool
    {
        if(instancia.numSats == 1)
            return false;

        bool rm = false;

        for(int i=0; i < numSat; ++i)
        {

            int sat = 1 + (rand_u32() % instancia.numSats);
            Satelite *satelite = &sol.satelites[sat];
            const int satIni = sat;

            do
            {
                if(!satelite->vazio())
                    break;
                sat = (sat+1) % instancia.numSats;
                if(sat == 0)
                    sat = 1;
                satelite = &sol.satelites[sat];
            }
            while(satIni != sat);

            if(satelite->vazio())
                continue;


            const int numEvSat = satelite->numEv();
            sol.numEv -= numEvSat;

            double dist = satelite->excluiSatelite(instancia, sol.vetClientesAtend);
            sol.distancia -= dist;
            rm = true;
        }

#if UTILIZA_MAT_MV
        //sol.vetMatSatEvMv[sat] =  ublas::matrix<int>(instancia.numEv, NUM_MV, 0);
        sol.todasRotasEvAtualizadas();
#endif
        sol.reseta1Nivel(instancia);
        sol.resetaIndiceEv(instancia);

        return true;
    };

    // Remove numClientes clientes da solução
    auto funcDestroi2 = [&](Solucao &sol, const int numClientes)
    {

        VarAuxiliaresIgNs::sumQuantCliRm += numClientes;
        VarAuxiliaresIgNs::num_sumQuantCliRm += 1;

/*
std::cout<<"FUNCAO funcDestroi2\n\n";

string solStr;
sol.print(solStr, instancia);
std::cout<<"SOLUCAO ANTES: \n"<<solStr<<"\n";
*/

        std::list<RotaInfo> listRotaInfo;

        //cout<<"CLIENTES RM: ";

        int rmClientes = 0;

        string strSol;
        sol.print(strSol, instancia);

        while(rmClientes != numClientes)
        {
            const int numClieIt = rmClientes;

            // Percorre os sat
            int sat = 1 + (rand_u32()%instancia.numSats);
            const int satIni = sat;
            do
            {
                Satelite &satelite = sol.satelites[sat];

                // Percorre as rotas
                int ev = rand_u32()%instancia.numEv;
                const int evIni = ev;
                do
                {
                    EvRoute &evRoute = satelite.vetEvRoute[ev];
                    if(evRoute.routeSize <= 2)
                    {
                        ev = (ev+1)%instancia.numEv;
                        continue;
                    }

                    // Verifica se existe mais de um cliente na rota
                    int numClientesEv = 0;
                    for(int i=1; i < (evRoute.routeSize-1); ++i)
                    {
                        if(instancia.isClient(evRoute[i].cliente))
                            numClientesEv += 1;
                    }

                    if(numClientesEv <= 1)
                    {
                        ev = (ev+1)%instancia.numEv;
                        continue;
                    }

                    // Encontra pos para retirar o cliente
                    int pos = rand_u32()%evRoute.routeSize;
                    const int posIni = pos;

                    while(!instancia.isClient(evRoute[pos].cliente))
                    {
                        pos = (pos+1)%evRoute.routeSize;
                        if(pos == posIni)
                        {
                            PRINT_DEBUG("", "");
                            cout<<"ERRO, Percorreu todo ev e nao encontrou um cliente!!\nROTA: ";
                            string strRota;
                            evRoute.print(strRota, instancia, true);
                            cout<<strRota<<"\n\n";
                            throw "ERRO";
                        }
                    }

                    const int clienteRm = evRoute[pos].cliente;
                    //cout<<clienteRm<<" ";
                    const double demClienteRm = instancia.getDemand(clienteRm);
                    sol.vetClientesAtend[clienteRm] = Int8(0);

                    NS_Auxiliary::shiftVectorClienteEsq(evRoute.route, pos, evRoute.routeSize);
                    evRoute.routeSize -= 1;
                    evRoute.demanda -= demClienteRm;
                    satelite.demanda -= demClienteRm;

                    listRotaInfo.emplace_back(sat, ev);
                    rmClientes += 1;

                    break;

                }while(ev != evIni);

                if(numClieIt != rmClientes)
                    break;

                sat = 1 + ((sat+1)%instancia.numSats);

            }
            while(sat != satIni);

            // Verifica se a iteracao atual removeu um cliente
            if(numClieIt == rmClientes)
                break;
        }

        //cout<<"\n\n";

        // Corrigir as rotas de listRotaInfo
        for(auto itRotaInfo:listRotaInfo)
        {
            Satelite &satelite = sol.satelites[itRotaInfo.satId];
            EvRoute &evRoute   = satelite.vetEvRoute[itRotaInfo.evRouteId];
            //string evRotaStr;
            //evRoute.print(evRotaStr, instancia, true);

            sol.distancia -= evRoute.distancia;
            satelite.distancia -= evRoute.distancia;


            // Remove RS repetida
            bool fimEv = false;
            int i = 1;
            //while((i+1) < evRoute.routeSize)
            {
                for(; (i+1) < evRoute.routeSize; ++i)
                {
                    const int clienteI = evRoute[i].cliente;
                    const int clienteII = evRoute[i+1].cliente;

                    if(clienteI == clienteII)
                    {
                        NS_Auxiliary::shiftVectorClienteEsq(evRoute.route, i+1, evRoute.routeSize);
                        evRoute.routeSize -= 1;
                    }
                }
            }
            string strDebug;
            const double custo = NS_viabRotaEv::testaRota(evRoute, evRoute.routeSize, instancia, true,
                                                    evRoute.route[0].tempoSaida, 0, &strDebug, &vetInviabilidate);

            if(custo <= 0.0)
            {
                PRINT_DEBUG("", "");
                std::cout<<"\n\nROTA EV ID: "<<itRotaInfo.evRouteId<<"; SAT: "<<itRotaInfo.satId<<"\n";
                string rotaStr;
                evRoute.print(rotaStr, instancia, true);
                cout<<"ROTA: "<<rotaStr<<"\n\n";
                cout<<"Sol: \n"<<strSol<<"\n";
                cout<<"debug: "<<strDebug<<"\n\n";
                cout<<"Tempo saida sat: "<<instancia.vetTempoSaida[itRotaInfo.satId]<<"\n\n";

                throw "ERRO";
            }

            evRoute.distancia = custo;
            satelite.distancia += custo;
            solC.distancia += custo;

            evRoute.atualizaParametrosRota(instancia);
            sol.rotaEvAtualizada(itRotaInfo.satId, itRotaInfo.evRouteId);
        }

        sol.reseta1Nivel(instancia);
        sol.resetaIndiceEv(instancia);

        /*
        solStr = "";
        sol.print(solStr, instancia);
        std::cout<<"SOLUCAO DEPOIS: \n"<<solStr<<"\n";
        */

    };


    // Remove uma rota de caminhao(primeiro nivel)
    auto funcDestroi3 = [&](Solucao &sol, int numRouteRm) -> bool
    {
        for(int i=0; i < numRouteRm; ++i)
        {
            int route = rand_u32() % instancia.numTruck;
            const int routeIni = route;

            while(sol.primeiroNivel[route].routeSize <= 2)
            {
                route = (route + 1) % instancia.numTruck;
                if(route == routeIni)
                {
/*                    PRINT_DEBUG("", "");
                    cout << "Nao foi possivel achar uma rota!\n";
                    string strRota;
                    sol.print(strRota, instancia);
                    cout << "Solucao:\n" << strRota << "\n\n";
                    throw "ERRO";*/
                    return true;
                }
            }

            sol.distancia -= sol.primeiroNivel[route].totalDistence;
            sol.primeiroNivel[route].resetaRoute();
        }

        //sol.reseta1Nivel(instancia);

        return true;
    };

    BoostC::vector<DadosIg> vetDadosIg;
    vetDadosIg.reserve(parametros.numItTotal);

    bool escreveSol = false;
    const int segFuncDest = 1;
    int numSatRm = 1;

    Solucao solC_copia(instancia);


    for(int i=0; i < parametros.numItTotal; ++i)
    {

#if PRINT_IG
if(i%200 == 0)
{
    cout << "ITERACAO: " << i << "\n";
    //cout<<"num rotas removidas corrente: "<<numEvRmCorrente<<"\n";
    //cout<<"destroi0 num chamadas: "<<numChamadasDestroi0<<"\n\n";
}
#endif

/*        if(instancia.numSats > 1 && numSatRm == 1 && (i-ultimaA) == 500)
        {
            numSatRm = 2;
            cout<<"numSatRm: "<<numSatRm<<"\n\a\a\a";
        }*/

        const double dif = (solC.distancia-solBest.distancia)/solBest.distancia;

        //if((i-ultimaA) % numItSemMelhoraResetSolC == 0 && i!=ultimaA)
        if(dif > parametrosIg.difBest)
        {
            //solC = Solucao(instancia);
            solC.copia(solBest);
        }

        string strSolC;
        if(escreveSol)
            solC.printPlot(strSolC, instancia);

        hashSolSetCorrente.insert(VetorHash(solC, instancia));
        numSolCorrente += 1;
        DadosIg dadosIg;

        dadosIg.it = i;
        dadosIg.solCorrente = solC.distancia;

        //Solucao solC_copia(instancia);
        solC_copia.copia(solC);

        NameS_IG::atualizaTempoSaidaInstancia(solC, instancia);
        bool construtivoFull = true;

        if(estrategia == Int8(0))
        {

            if(numFuncDestroi < numChamadasDestroi0)
            {
                funcDestroi0(solC, numEvRmCorrente);
                numFuncDestroi += 1;
            }
            else
            {

                //solC = Solucao(instancia);
                solC.copia(solBest);

                if(!funcDestroi1(solC, numSatRm))
                    funcDestroi0(solC, numEvRmCorrente);

                numFuncDestroi = 0;

            }
        }
        else
        {
            funcDestroi2(solC, numClientesRm);
        }

        if(construtivoFull)
        {
            if(tipoConst == CONSTRUTIVO1)
                NS_Construtivo3::construtivo(solC, instancia, alfaSeg, betaPrim, matClienteSat, true, false, false,
                                             &vetInviabilidate, torneio);

            else
                NS_Construtivo4::construtivo(solC, instancia, alfaSeg, betaPrim, matClienteSat, true, false, false,
                                             &vetInviabilidate, torneio);

        }

        if(!solC.viavel)
        {
            dadosIg.solConst = -1.0;
            dadosIg.solVnd   = -1.0;

            //solC = Solucao(instancia);
            solC.copia(solC_copia);

        }
        else
        {
            string strSolConst;
            string strErro;
            if(!solC.checkSolution(strErro, instancia))
            {
                PRINT_DEBUG("", "");
                cout<<"ERRO NO IG APOS O CONSTRUTIVO\nERRO:\n\n";
                cout<<strErro<<"\n\n";
                cout<<"Sol: \n";
                string strSol;
                solC.print(strSol, instancia);
                cout<<strSol<<"\n";
                throw "ERRO";
            }

            hashSolSetConst.insert(VetorHash(solC, instancia));
            numSolConstVia += 1;

            dadosIg.solConst = solC.distancia;

            numSolG += 1;
            //lucao solTemp(instancia);
            //solTemp.copia(solC);
            string strSolAntes;
            //solC.print(strSolAntes, instancia);

            rvnd(solC, instancia, betaPrim, vetMvValor, vetMvValor1Nivel);

            string erro;
            if(!solC.checkSolution(erro, instancia))
            {
                PRINT_DEBUG("", "");
                cout<<"ERRO APOS NOVO MV, IG\n";
                cout<<erro<<"\n\n";
                erro = "";
                solC.print(erro, instancia);
                cout<<erro;
                cout<<"######################################################\n\n";
                cout<<"Sol antes rvnd: "<<strSolAntes;

                throw "ERRO";
            }

            hashSolSetVnd.insert(VetorHash(solC, instancia));
            dadosIg.solVnd   = solC.distancia;
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

            ValBestNs::distBest = solC.distancia;

            solBest = Solucao(instancia);
            solBest.copia(solC);
            solBest.ultimaA = i;
            numFuncDestroi = 0;
            ultimaA = i;

            ultimaABest = i;
            numEvRmCorrente = numEvRmMin;
#if PRINT_IG
cout<<"ATUALIZACAO "<<i<<": "<<solBest.distancia<<"\n\n";
#endif


        }

        dadosIg.solBest = solBest.distancia;
        vetDadosIg.push_back(dadosIg);

    } // END for ig

    cout<<"FIM IG\n";

    // Inicio MIP model
    modelo(instancia, hashSolSetVnd, solBest);
    // Fim MIP model


    //cout<<"\tDist: "<<solBest.distancia<<"\n\n";

    double fatorSolCorr = (double(hashSolSetCorrente.size())/numSolCorrente) * 100;
    if(numSolCorrente == 0)
        fatorSolCorr = 0.0;

    double fatorSolConst = (double(hashSolSetConst.size())/numSolConstVia) * 100;
    if(numSolConstVia == 0)
        fatorSolConst = 0.0;

    double fatorSolVnd = (double(hashSolSetVnd.size())/numSolConstVia) * 100;
    if(numSolConstVia == 0)
        fatorSolVnd = 0.0;

    auto funcAddParaSaidaDouble = [&](string &&strParm, double val)
    {
        parametrosSaida.mapNoSaida[strParm] = NS_parametros::NoSaida(strParm);
        parametrosSaida.mapNoSaida[strParm].addSaida(SAIDA_EXEC_VAL);
        parametrosSaida.mapNoSaida[strParm](val);
    };


    const double mediaQuantCand = double(VarAuxiliaresIgNs::sumQuantCand)/VarAuxiliaresIgNs::num_sumQuantCand;
    const double mediaCliRm     = double(VarAuxiliaresIgNs::sumQuantCliRm)/VarAuxiliaresIgNs::num_sumQuantCliRm;

    funcAddParaSaidaDouble("numSol", numSolG);
    funcAddParaSaidaDouble("fatorSolCorr", fatorSolCorr);
    funcAddParaSaidaDouble("fatorSolConst", fatorSolConst);
    funcAddParaSaidaDouble("fatorSolVnd", fatorSolVnd);
    funcAddParaSaidaDouble("mediaQuantCand", mediaQuantCand);
    funcAddParaSaidaDouble("mediaCliRm", mediaCliRm);


    auto funcAddParaSaidaInt = [&](string &&strParm, int val)
    {
        parametrosSaida.mapNoSaida[strParm] = NS_parametros::NoSaida(strParm, SAIDA_TIPO_INT);
        parametrosSaida.mapNoSaida[strParm].addSaida(SAIDA_EXEC_VAL);
        parametrosSaida.mapNoSaida[strParm](val);
    };

    funcAddParaSaidaInt("invCarga", vetInviabilidate[NS_Auxiliary::Inv_Carga]);
    funcAddParaSaidaInt("invTw", vetInviabilidate[NS_Auxiliary::Inv_tw]);
    funcAddParaSaidaInt("invBat", vetInviabilidate[NS_Auxiliary::Inv_bat]);
    funcAddParaSaidaInt("inv1Nivel", vetInviabilidate[NS_Auxiliary::Inv_1_Nivel]);
    funcAddParaSaidaInt("inv1NivelUnico", vetInviabilidate[NS_Auxiliary::Inv_1_Nivel_unico]);
    funcAddParaSaidaInt("invNaoRs", vetInviabilidate[NS_Auxiliary::Inv_nao_ev_rs]);

    string erro;
    if(!solBest.checkSolution(erro, instancia))
    {

        PRINT_DEBUG("", "");
        cout<<"ERRO, IG\n";
        cout<<erro<<"\n\n";
        throw "ERRO";
    }

    printVetDadosIg(vetDadosIg, parametros);

    Solucao *solPtr = new Solucao(instancia);
    solPtr->copia(solBest);
    return solPtr;
}

void NameS_IG::printVetDadosIg(BoostC::vector<DadosIg> &vetDadosIg, NS_parametros::Parametros &parametros)
{
    if(vetDadosIg[0].solConst < 0.0)
    {
        vetDadosIg[0].solConst = vetDadosIg[0].solCorrente;
        vetDadosIg[0].solVnd   = vetDadosIg[0].solCorrente;
    }

    for(int i=1; i < vetDadosIg.size(); ++i)
    {
        if(vetDadosIg[i].solConst < 0.0)
            vetDadosIg[i].solConst = vetDadosIg[i-1].solConst;

        if(vetDadosIg[i].solVnd < 0.0)
            vetDadosIg[i].solVnd = vetDadosIg[i-1].solVnd;
    }

    string strVet = "it,corr,const,vnd,best\n";
    const int ultimaIt = parametros.numItTotal-1;

    for(auto it:vetDadosIg)
    {
        strVet += to_string(it.it) + ", " + to_string(it.solCorrente) + ", " + to_string(it.solConst) + ", " + to_string(it.solVnd) +
                + ", " + to_string(it.solBest) + "\n";
    }


    if(mkdir(parametros.caminhoPasta.c_str(), 0777) == -1)
    {
        if(errno != EEXIST)
        {
            cout << "ERRO, NAO FOI POSSIVEL CRIAR O DIRETORIO: " << parametros.caminhoPasta << "\nErro: " << strerror(errno) << "\n\n";
            throw "ERRO";
        }
    }

    const string caminhoInst = parametros.caminhoPasta + "/dadosIg/";

    if(mkdir(caminhoInst.c_str(), 0777) == -1)
    {
        if(errno != EEXIST)
        {
            cout << "ERRO, NAO FOI POSSIVEL CRIAR O DIRETORIO: " << caminhoInst << "\nErro: " << strerror(errno) << "\n\n";
            throw "ERRO";
        }
    }


    const string caminho = caminhoInst + parametros.nomeInstancia + "/";

    if(mkdir(caminho.c_str(), 0777) == -1)
    {
        if(errno != EEXIST)
        {
            cout << "ERRO, NAO FOI POSSIVEL CRIAR O DIRETORIO: " << caminho << "\nErro: " << strerror(errno) << "\n\n";
            throw "ERRO";
        }
    }
    //cout<<"DIRETORIO: "<<caminho<<" criado\n";

    string strSaida = caminho+parametros.nomeInstancia+"_EXE_"+ to_string(parametros.execucaoAtual)+".csv";

    std::ofstream outfile;
    outfile.open(strSaida, std::ios_base::out);

    if(outfile.is_open())
    {
        outfile<<strVet<<"\n";
        outfile.close();
    }
    else
    {
        cout<<"NAO FOI POSSIVEL ABRIR O ARQUIVO: "<<strSaida<<"\n";
    }

}

void NameS_IG::atualizaTempoSaidaInstancia(Solucao &solucao, Instancia &instancia)
{

    BoostC::vector<double> &vetTempoSaida = instancia.vetTempoSaida;
    for(int sat=1; sat <= instancia.numSats; ++sat)
    {
        vetTempoSaida[sat] = max(solucao.satTempoChegMax[sat], vetTempoSaida[sat]);
    }

}

/*NameS_IG::ParametrosIG::ParametrosIG(const std::string& fileStr)
{

    std::ifstream file(fileStr);
    if(!file.is_open())
    {
        std::cout<<"Nao foi possuivel abrir arquivo: "<<fileStr<<"\n";
        PRINT_DEBUG("","");
        throw "ERRO";
    }


    std::map<std::string, std::string> entradaParam;

    while(file.peek() != EOF)
    {
        std::string param, val;
        file>>param>>val;
        entradaParam[param] = val;
    }

    alfaSeg100 = std::stof(entradaParam["alfaSeg100"]);
    alfaSeg15  = std::stof(entradaParam["alfaSeg15"]);

    betaPrim100 = std::stof(entradaParam["betaPrim100"]);
    betaPrim15  = std::stof(entradaParam["betaPrim15"]);

    torneio100  = (std::stoi(entradaParam["torneio100"]) == 1);
    torneio15   = (std::stoi(entradaParam["torneio15"]) == 1);

    tipoConstrutivo15 = static_cast<TipoConstrutivo>(std::stoi(entradaParam["tipoConstrutivo15"]));

    file.close();
}

std::string ParametrosIG::printParam()
{
   std::string saida;

   saida += "alfaSeg100 \t\t " + std::to_string(alfaSeg100);
   saida += "\nalfaSeg15 \t\t " + std::to_string(alfaSeg15);
   saida += "\nbetaPrim100 \t\t " + std::to_string(betaPrim100);
   saida += "\nbetaPrim15 \t\t " + std::to_string(betaPrim15);

   saida += "\ntorneio100 \t\t " + std::to_string(torneio100);
   saida += "\ntorneio15 \t\t " + std::to_string(torneio15);

   saida += "\ntipoConstrutivo15 \t " + std::to_string(tipoConstrutivo15);

   return saida;
}*/
