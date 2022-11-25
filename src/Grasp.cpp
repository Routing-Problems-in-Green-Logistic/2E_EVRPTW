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
#include "LocalSearch.h"
#include "PreProcessamento.h"
#include "Vnd.h"
#include <fstream>
#include <unordered_set>
#include "k_means.h"
#include "VetorHash.h"

#define NUM_EST_INI 3

using namespace GreedyAlgNS;
using namespace NameS_Grasp;
using namespace NS_LocalSearch;
using namespace NS_vnd;
using namespace NS_VetorHash;

const float fator       = 1.4;//0.1344;
const float fator1Nivel = 1.1;//0;01

const bool ListaRestTam = true;

/**
 *
 * @param instance
 * @param parametros
 * @param estat
 * @param retPrimeiraSol
 * @param matClienteSat         Para uma posicao: matClienteSat(clienteI, sat_0) = 0,1: indica se o clienteI pode ser atendido pelo sat_0
 * @return
 */
Solucao * NameS_Grasp::grasp(Instancia &instance, ParametrosGrasp &parametros, Estatisticas &estat,
                             const bool retPrimeiraSol, const ublas::matrix<int> &matClienteSat,
                             BoostC::vector<NS_vnd::MvValor> &vetMvValor, BoostC::vector<NS_vnd::MvValor> &vetMvValor1Nivel,
                             NS_parametros::ParametrosSaida &parametrosSaida)
{
    vetMvValor1Nivel = BoostC::vector<MvValor>(2);

    vetMvValor1Nivel[0].mv = MV_EV_SHIFIT_INTER_ROTAS_INTER_SAT;
    vetMvValor1Nivel[1].mv = MV_EV_SWAP_INTER_ROTAS_INTER_SAT;

    Solucao *solBest = new Solucao(instance);
    solBest->distancia = DOUBLE_MIN;
    solBest->viavel = false;
    EvRoute evRoute(1, instance.getFirstEvIndex(), instance.getEvRouteSizeMax(), instance);


    vector<int> vetSatAtendCliente(instance.numNos, -1);
    vector<int> satUtilizado(instance.numSats+1, 0);


    estat.numSol = 0.0;
    estat.numIte = parametros.numIteGrasp;
    estat.distAcum = 0.0;
    estat.erro = "";

    const int tamAlfa = parametros.vetAlfa.size();

    // Solucao para inicializar reativo
    Solucao gul(instance);

    construtivo(gul, instance, 0.0, 0.0, matClienteSat, ListaRestTam);
    const double gulCusto2Nivel = getDistMaisPenalidade(gul, instance);
    double temp = gul.getDist1Nivel();
    if(!gul.viavel)
    {
        temp = 0.0;
        for(int sat=instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
        {
            temp += 2.0*instance.getDistance(0, sat);
        }

        temp *= 1.2;
    }

    const double gulCusto1Nivel = temp;

    double custoBest = gulCusto2Nivel+gulCusto1Nivel;
    double custoBest1Nivel = gulCusto1Nivel;

    //Vetores para o reativo
    BoostC::vector<double> vetorProbabilidade2Nivel(tamAlfa, 100.0/tamAlfa);
    BoostC::vector<double> vetorProbabilidade1Nivel(tamAlfa, 100.0/tamAlfa);

    BoostC::vector<int>    vetorFrequencia2Nivel(tamAlfa, 0);
    BoostC::vector<int>    vetorFrequencia1Nivel(tamAlfa, 0);

    BoostC::vector<double> solucaoAcumulada2Nivel(tamAlfa, 0.0);
    BoostC::vector<double> solucaoAcumulada1Nivel(tamAlfa, 0.0);

    BoostC::vector<double> vetorMedia2Nivel(tamAlfa, 0.0);
    BoostC::vector<double> vetorMedia1Nivel(tamAlfa, 0.0);

    BoostC::vector<double> proporcao2Nivel(tamAlfa, 0.0);
    BoostC::vector<double> proporcao1Nivel(tamAlfa, 0.0);

    auto atualizaProb = [&]()
    {

        double penalidade1Nivel = getPenalidade1Nivel(*solBest, instance, fator1Nivel);
        custoBest = solBest->distancia + getPenalidade2Nivel(*solBest, instance, fator) + penalidade1Nivel;
        custoBest1Nivel = solBest->getDist1Nivel() + penalidade1Nivel;

        double somaProporcoes2Nivel = 0.0;
        double somaProporcoes1Nivel = 0.0;

        //Calcular média
        for(int i = 0; i < tamAlfa; ++i)
        {
            vetorMedia2Nivel[i] = solucaoAcumulada2Nivel[i] / double(vetorFrequencia2Nivel[i]);
            vetorMedia1Nivel[i] = solucaoAcumulada1Nivel[i] / double(vetorFrequencia1Nivel[i]);
        }

        //Calcula proporção.
        for(int i = 0; i < tamAlfa; ++i)
        {
            proporcao2Nivel[i] = custoBest / vetorMedia2Nivel[i];
            proporcao1Nivel[i] = custoBest1Nivel / vetorMedia1Nivel[i];

            somaProporcoes2Nivel += proporcao2Nivel[i];
            somaProporcoes1Nivel += proporcao1Nivel[i];
        }

        //Calcula probabilidade
        for(int i = 0; i< tamAlfa; ++i)
        {
            vetorProbabilidade2Nivel[i] = 100.0 * (proporcao2Nivel[i] / somaProporcoes2Nivel);
            vetorProbabilidade1Nivel[i] = 100.0 * (proporcao1Nivel[i] / somaProporcoes1Nivel);
        }


       // cout<<solBest->viavel2Nivel(instance)<<": 2NIVEL\n";
        //cout<<solBest->viavel<<": sol\n";

        //cout<<"2NIVEL VIAVEL: "<<(getPenalidade2Nivel(*solBest, instance, fator))<<"\n";
        //cout<<"1NIVEL VIAVEL: "<<(getPenalidade1Nivel(*solBest, instance, fator1Nivel))<<"\n";

        //cout<<"BEST: "<<custoBest<<"; \n1NIVEL: "<<custoBest1Nivel<<"\n\n";

        //cout<<"SOL_BEST: "<<solBest->distancia<<"\n";

/*        for(int i = 0; i< tamAlfa; ++i)
        {
            cout<<parametros.vetAlfa[i]<<" \t ";
        }*/

        //cout<<"\n1º:\n";


/*        for(int i = 0; i< tamAlfa; ++i)
        {
            cout<<converteDouble(vetorProbabilidade1Nivel[i], 1)<<" \t ";
        }*/

        //cout<<"\n";
/*

        for(int i = 0; i < tamAlfa; ++i)
        {
            cout<<vetorMedia1Nivel[i]<<" \t ";
        }

        cout<<"\n";

        for(int i=0; i < tamAlfa; ++i)
        {
            cout<<vetorFrequencia1Nivel[i]<<" \t ";
        }
*/


       /*
        cout<<"\nPROB.; SOL ACUM.; MEDIA; FREQ.\n\n";

        cout<<"\n\n2º:\n";

        for(int i = 0; i< tamAlfa; ++i)
        {
            cout<<converteDouble(vetorProbabilidade2Nivel[i], 1)<<" \t ";
        }

        cout<<"\n";

        for(int i = 0; i< tamAlfa; ++i)
            cout<<converteDouble(solucaoAcumulada2Nivel[i],1)<<" \t ";


        cout<<"\n";

        for(int i = 0; i < tamAlfa; ++i)
        {
            cout<<vetorMedia2Nivel[i]<<" \t ";
        }
        cout<<"\n";


        for(int i=0; i < tamAlfa; ++i)
        {
            cout<<vetorFrequencia2Nivel[i]<<" \t ";
        }

        cout<<"\n\n";
        cout<<"************************************************************\n\n";
*/

    };

    auto convIndClienteVet = [&](int cliente)
    {
        return cliente-instance.getFirstClientIndex();
    };

    auto convClienteIndVet = [&](int i)
    {
        return i+instance.getFirstClientIndex();
    };

    // Guarda o numero de vezes que o cliente i NAO a parece na solucao
    BoostC::vector<QuantCliente> vetQuantCliente(instance.getNClients());
    BoostC::vector<QuantCliente> vetQuantProb0;

    for(int i=instance.getFirstClientIndex(); i <= instance.getEndClientIndex(); ++i)
        vetQuantCliente[convIndClienteVet(i)].cliente = i;


    double somaProb = 0.0;
    int posAlfa = 0;
    int valAleatorio = 0;
    float alfa = 0;

    int addRotaClienteProbIgual = 0;
    int clienteAdd = -1;

    vetMvValor = BoostC::vector<MvValor>(NUM_MV_LS);
    for(int i=0; i < NUM_MV_LS; ++i)
        vetMvValor[i].mv = i;

    std::unordered_set<VetorHash, VetorHash::HashFunc> hashRotaSet;
    std::unordered_set<VetorHash, VetorHash::HashFunc> hashSolSet;

    int numSolGeradas   = 0;
    int numRotasGeradas = 0;

    std::unordered_set<VetorHash, VetorHash::HashFunc> hashRotasIncluidas;
    std::unordered_set<VetorHash, VetorHash::HashFunc> hashRotasExcluidas;

    bool iniRS = false;

    // Guarda os ev que sao inicializados com RS RS
    ublas::matrix<int> matrixSatEv(instance.numSats+1, instance.numEv, 0);

    for(int i=0; i < parametros.numIteGrasp; ++i)
    {
        hashRotasIncluidas.clear();

        //if(i>0 && (i%100)==0)
        //    cout<<"ITERACAO: "<<i<<"\n";

        Solucao sol(instance);
        //setSatParaCliente(instance, vetSatAtendCliente, satUtilizado, parametros);
        //N_k_means::k_means(instance, vetSatAtendCliente, satUtilizado, true);


        if(i == parametros.iteracoesCalProb && instance.shortestPath) //&& (i%parametros.iteracoesCalProb)==0)
        {

            for(int t=0; t < instance.getNClients(); ++t)
            {

                for(int sat=instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
                {

                    const EvRoute &evRouteAux = instance.shortestPath[sat].getEvRoute(convClienteIndVet(t));

                    if(evRouteAux.routeSize > 2)
                    {
                        vetQuantCliente.at(t).calculaProb(i);

                        if(vetQuantCliente.at(t).prob >= 90)
                            vetQuantCliente[t].prob = 90;

                        //cout << (t+instance.getFirstClientIndex()) << ": " << vetQuantCliente.at(t).prob << "\n";
                        break;
                    }
                }
            }

            //cout<<"\n\n";

            std::sort(vetQuantCliente.begin(), vetQuantCliente.end());

            if(estat.numSol == 0)
            {
                parametros.numMaxClie += 1;
                if(parametros.numMaxClie > instance.getN_Evs())
                    parametros.numMaxClie -= 1;

            }

        }

        Solucao solTemp(instance);
        bool segundaEst = false;

        // Insere  rotas da forama: sat cliente sat
        if(i >= parametros.iteracoesCalProb && parametros.iteracoesCalProb > 0 && instance.shortestPath && (i%2)==0)
        {
            int clientesAdd = 0;

            bool add   = false;
            bool igual = false;

            int t = rand_u32()%instance.getNClients();
            const int inicio = t;


            if(vetQuantCliente[0].prob != 100)
            {

                BoostC::vector<int> vetSatRotaInicializada(1+(instance.numSats*instance.numEv), 0);

                do
                {
                    if(vetQuantCliente[t].prob != 100 && vetQuantCliente[t].prob >= parametros.probCorte)
                    {

                        int rand = rand_u32() % 100;
                        if(rand >= vetQuantCliente[t].prob)
                        {

                            int cliente = vetQuantCliente[t].cliente;

                            int sat = rand_u32()%instance.numSats;
                            const int satIni = sat;

                            do
                            {
                                int satId = sat+instance.getFirstSatIndex();
                                int evEscolhido = 0;

                                if(matClienteSat(t, satId) == 1)
                                {
                                    for(int ev = 0; ev < instance.numEv; ++ev)
                                    {
                                        if(vetSatRotaInicializada[1+(satId-1)*instance.numEv+ev] == 0)
                                        {
                                            evEscolhido = ev;
                                            break;
                                        }
                                    }

                                    if(vetSatRotaInicializada[1+(satId-1)*instance.numEv+evEscolhido] == 0)
                                    {
                                        EvRoute &evRouteSP = instance.shortestPath[satId].getEvRoute(cliente);
                                        auto shortestPath = instance.shortestPath[satId].getShortestPath(cliente);

                                        if(shortestPath.distIdaVolta < DOUBLE_INF)
                                        {
                                            //auto it = hashRotasExcluidas.find(VetorHash(evRouteSP));
                                            //if(it == hashRotasExcluidas.end())
                                            {

                                                addRotaCliente(sol, instance, evRouteSP, cliente);
                                                string saida;
                                                evRouteSP.print(saida, instance, true);
                                                //cout << "INICIANDO SOL COM: " << saida << "\n";

                                                hashRotasIncluidas.insert(VetorHash(evRouteSP));

                                                clientesAdd += 1;
                                                vetSatRotaInicializada[1+(satId-1)*instance.numEv+evEscolhido] = 1;
                                                break;
                                            }
                                        }
                                    }

                                }

                                sat += 1;
                                sat = sat%instance.numSats;

                            }while(sat != satIni);

                        }

                        if(clientesAdd >= parametros.numMaxClie)
                            break;
                    }

                    t = (t+1) % instance.getNClients();

                }while(t != inicio);

            }
        }
        // Insere rotas da forma: sat RS RS sat
        else if(i >= parametros.iteracoesCalProb && parametros.iteracoesCalProb > 0 && ((i%2)==1) || instance.shortestPath == nullptr)
        {

            //cout<<"SEGUNDA ESTRATEGIA, i: "<<i<<"\n";

            segundaEst = true;

            BoostC::vector<int> vetRS_uti(instance.getEndRS_index()+1, 0);
            std::fill(vetRS_uti.begin(), vetRS_uti.begin()+instance.getFirstRS_index(), -1);

            matrixSatEv = ublas::zero_matrix<int>(instance.numSats+1, instance.numEv);

            int numRs = 0;


            while(numRs < parametros.numMaxClie)
            {


                bool inserRS = false;

                if(numRs == instance.numEv)
                    break;

                // Encontra um estacao de recarga nao utilizada

                int rs = instance.getFirstRS_index()+(rand_u32()%instance.numRechargingS);
                const int rsFist = rs;

                do
                    rs = instance.getFirstRS_index()+((rs+1)%instance.numRechargingS);
                while(vetRS_uti[rs]==1 && rs != rsFist);

                // Encontra um sat

                const int satIni = instance.getFirstSatIndex() + (rand_u32()%instance.numSats);
                int sat = satIni;

                // Percorre os sat
                // Inicio while(sat)
                do
                {

                    // Percorre as rotas
                    for(int ev=0; ev < instance.numEv; ++ev)
                    {

                        if(matrixSatEv(sat,ev) == 0)
                        {

                            EvRoute evRouteAux(sol.satelites[sat].vetEvRoute[ev]);

                            evRouteAux[1].cliente = evRouteAux[2].cliente = rs;
                            evRouteAux[3].cliente = evRouteAux[0].cliente = sat;

                            evRouteAux.routeSize = 4;

                            double dist = NS_viabRotaEv::testaRota(evRouteAux, evRouteAux.routeSize, instance, true,
                                                                   instance.vetTempoSaida[sat], 0, nullptr);
                            if(dist > 0.0)
                            {
                                evRouteAux.distancia = dist;
                                evRouteAux.atualizaParametrosRota(instance);
                                sol.satelites[sat].vetEvRoute[ev].copia(evRouteAux, true, &instance);
                                sol.satelites[sat].distancia += dist;
                                sol.distancia += dist;
                                matrixSatEv(sat,ev) = 1;
                                numRs += 1;
                                inserRS = true;
                                vetRS_uti[rs] = 1;

                                //string strRota;
                                //evRouteAux.print(strRota, instance, false);

                                if(numRs >= parametros.numMaxClie || numRs >= instance.numEv)
                                    break;
                            }


                        }

                        if(numRs >= parametros.numMaxClie || numRs >= instance.numEv)
                            break;

                        sat = instance.getFirstSatIndex() + ((sat + 1) % instance.numSats);
                    }

                    if(numRs >= parametros.numMaxClie || numRs >= instance.numEv)
                        break;

                }
                while(sat != satIni);

                if(!inserRS)
                    break;

            } // END whili(numRs)

        }

        somaProb = 0.0;
        posAlfa = 0;

        valAleatorio = rand_u32()%100;
        int jIni = rand_u32()%tamAlfa;
        int j=jIni;

        do
        {
            somaProb+= (vetorProbabilidade2Nivel[j]);
            posAlfa = j;

            j = (j+1)%tamAlfa;

        }while((somaProb < valAleatorio) && (j!=jIni));

        alfa = parametros.vetAlfa[posAlfa];

        somaProb = 0.0;
        valAleatorio = rand_u32()%100;
        jIni = rand_u32()%tamAlfa;
        j=jIni;
        int posBeta = 0;

        do
        {
            somaProb+= (vetorProbabilidade1Nivel[j]);
            posBeta = j;

            j = (j+1)%tamAlfa;

        }while((somaProb < valAleatorio) && (j!=jIni));

        /*for(int j=0;somaProb < valAleatorio; ++j)
        {

            if(j >= tamAlfa)
            {
                break;
            }

            somaProb+= (vetorProbabilidade1Nivel[j]);
            posBeta = j;
        }*/

        float beta = parametros.vetAlfa[posBeta];

        solTemp.copia(sol);
        construtivo(sol, instance, alfa, beta, matClienteSat, ListaRestTam);

        // Remove rotas sat RS RS sat que nao foram utilizadas
        if(segundaEst)
        {
            for(int sat=instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
            {
                for(int ev=0; ev < instance.numEv; ++ev)
                {
                    if(matrixSatEv(sat, ev) == 1)
                    {
                        Satelite &satelite = sol.satelites[sat];
                        EvRoute &evRoute = satelite.vetEvRoute[ev];
                        if(evRoute.routeSize == 4)
                        {
                            if(instance.isRechargingStation(evRoute[1].cliente) && instance.isRechargingStation(evRoute[2].cliente))
                            {

                                satelite.distancia -= evRoute.distancia;
                                sol.distancia -=  evRoute.distancia;

                                evRoute = EvRoute(sat, evRoute.idRota, evRoute.routeSizeMax, instance);

                            }
                            else
                            {
                                //cout<<"ERRO, NAO DEVERIA ENTRAR AQUI\t\t ????\n\n";
                                //evRoute.print(instance, true);
                                //PRINT_DEBUG("", "");
                                //throw "ERRO";
                                sol.viavel = false;
                            }
                        }
                        else
                        {
                            removeRS_Repetido(evRoute, instance, true);
                            sol.distancia -= evRoute.distancia;
                            sol.satelites[sat].distancia -= evRoute.distancia;

                            NS_viabRotaEv::testaRota(evRoute, evRoute.routeSize, instance, true, evRoute[0].tempoSaida, 0, nullptr);


                            sol.distancia += evRoute.distancia;
                            sol.satelites[sat].distancia += evRoute.distancia;

                        }


                    }
                }
            }
        }

        // Add 1 se o cliente t nao foi atendido
        if(!sol.viavel && parametros.iteracoesCalProb > 0 && instance.shortestPath)
        {
            int quantCliInv = 0;
            for(int t=instance.getFirstClientIndex(); t <= instance.getEndClientIndex(); ++t)
            {
                if(sol.vetClientesAtend[t] == int8_t(0))
                {

                    quantCliInv += 1;

                    for(int sat=instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
                    {
                        if(matClienteSat(t,sat) == 1)
                        {
                            const EvRoute &evRouteAux = instance.shortestPath[sat].getEvRoute(t);

                            if(evRouteAux.routeSize > 2)
                            {
                                vetQuantCliente[convIndClienteVet(t)].add1Quant();
                                break;
                            }
                        }
                    }
                }
            }
        }

        if(sol.viavel)
        {
            //cout<<"SOL VIAVEL\n\n";
            string erro;
            bool mv = true;

            for(Satelite &satelite:sol.satelites)
            {
                for(EvRoute &evRoute:satelite.vetEvRoute)
                {
                    if(evRoute.routeSize > 2)
                    {
                        hashRotaSet.insert(VetorHash(evRoute));
                        numRotasGeradas += 1;
                    }
                }
            }


            hashSolSet.insert(VetorHash(sol, instance));
            numSolGeradas += 1;

            if(!sol.checkSolution(erro, instance))
            {

                cout << "\n\nSOLUCAO:\n\n";
                sol.print(instance);

                cout<<"############################################################################################\n\n";
                //cout<<"SOLUCAO INICIAL:\n\n";
                //solTemp.print(instance);


                cout << erro
                     << "\n****************************************************************************************\n\n";
                mv = false;
                delete solBest;
                throw "ERRO";
            } else
            {

                rvnd(sol, instance, alfa, vetMvValor, vetMvValor1Nivel);

                if(sol.distancia < solBest->distancia || !solBest->viavel)
                {
                    //cout<<"UPDATE: "<<sol.distancia<<"\n";

                    solBest->copia(sol);
                    solBest->ultimaA = i;

                    if(retPrimeiraSol)
                        return solBest;

                    custoBest = solBest->distancia;
                    double aux = solBest->getDist1Nivel();
                    if(aux < custoBest1Nivel)
                        custoBest1Nivel = aux;

                    estat.ultimaAtualizacaoBest = i;

                }


            }

            if(sol.viavel)
            {

                estat.numSol += 1;
                estat.distAcum += sol.distancia;
            }


        }
        else if(!solBest->viavel && !sol.viavel)
        {

            double penal1Nivel = getPenalidade1Nivel(sol, instance, fator1Nivel);

            //cout<<"ATUAL\n";
            //solBest->copia(sol);

            double aux = sol.distancia + getPenalidade2Nivel(sol, instance, fator)+penal1Nivel;
            if(aux < custoBest)
            {
                //cout<<"ATUAL. BEST("<<converteDouble(custoBest,1)<<") ATUALI.("<<converteDouble(aux,1)<<")\n\n";
                custoBest = aux;
                solBest->copia(sol);
                custoBest1Nivel = solBest->getDist1Nivel()+penal1Nivel;
            }


        }

        if(!sol.viavel)
        {
            double penal1Nivel = getPenalidade1Nivel(sol, instance, fator1Nivel);

            double aux = sol.distancia + getPenalidade2Nivel(sol, instance, fator)+penal1Nivel;

            //if(posAlfa == 0)
            //    cout<<aux<<"\n";

            solucaoAcumulada2Nivel[posAlfa] += aux;
            vetorFrequencia2Nivel[posAlfa] += 1;

            aux = sol.getDist1Nivel() + penal1Nivel;
            //cout<<aux<<"\n\n";
            solucaoAcumulada1Nivel[posBeta] += aux;
            vetorFrequencia1Nivel[posBeta] += 1;
        }
        else
        {
            solucaoAcumulada2Nivel[posAlfa] += sol.distancia;
            vetorFrequencia2Nivel[posAlfa] += 1;

            solucaoAcumulada1Nivel[posBeta] += sol.getDist1Nivel();
            vetorFrequencia1Nivel[posBeta] += 1;
        }

        if(i >= parametros.numIteGrasp/2 && solBest->viavel && (i-solBest->ultimaA)>= parametros.numItSemMelhora)
        {
            break;
        }

        if(i>0 && (i%parametros.numAtualProbReativo)==0)
        {
            atualizaProb();
        }

    }
    // END FOR GRASP


    solBest->ultimaA = estat.ultimaAtualizacaoBest;

    parametrosSaida.mapNoSaida["rotas"] = NS_parametros::NoSaida("rotas");
    parametrosSaida.mapNoSaida["sol"] = NS_parametros::NoSaida("sol");
    parametrosSaida.mapNoSaida["numSol"] = NS_parametros::NoSaida("numSol");

    parametrosSaida.mapNoSaida["rotas"].addSaida(SAIDA_EXEC_VAL);
    parametrosSaida.mapNoSaida["sol"].addSaida(SAIDA_EXEC_VAL);
    parametrosSaida.mapNoSaida["numSol"].addSaida(SAIDA_EXEC_VAL);

    if(solBest->viavel)
    {
        estat.erro = "";

        if(!solBest->checkSolution(estat.erro, instance))
        {
            cout<<"\n\nSOLUCAO:\n\n";
            //solBest->print(instance);

            cout << estat.erro<< "\n****************************************************************************************\n\n";
            delete solBest;
            return nullptr;
        }

        //cout<<"ROTAS GERADAS: "<<numRotasGeradas<<"\nROTAS UNICAS GERADAS: "<<hashRotaSet.size()<<":  "<<(double(hashRotaSet.size())/numRotasGeradas)<<"\n\n";
        //cout<<"SOL GERADAS: "<<numSolGeradas<<"\nSOL UNICAS GERADAS: "<<hashSolSet.size()<<":  "<<(double(hashSolSet.size())/numSolGeradas)<<"\n";

        parametrosSaida.mapNoSaida["rotas"](double(hashRotaSet.size())/numRotasGeradas);
        parametrosSaida.mapNoSaida["sol"](double(hashSolSet.size())/numSolGeradas);
        parametrosSaida.mapNoSaida["numSol"](double(numSolGeradas));

    }

    return solBest;

}


std::string NameS_Grasp::converteDouble(double num, int numCasas)
{
    std::string format =  "%."+ std::to_string(numCasas)+"f";
    return str(boost::format(format.c_str())%num);
}


std::string NameS_Grasp::converteFloat(float num, int numCasas)
{
    std::string format =  "%."+ std::to_string(numCasas)+"f";
    return str(boost::format(format.c_str())%num);
}

double NameS_Grasp::getPenalidade2Nivel(Solucao &sol, Instancia &instancia, float f)
{
    static double penalidade = instancia.penalizacaoDistEv;

    if(sol.viavel)
        return 0.0;

    int num = 0;
    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        if(sol.vetClientesAtend[i] == int8_t(0))
            num += 1;
    }


    if(num == 0)
        return 0.0;
    else
    {
        //cout<<"2º NIVEL INVIAVEL\n";
        return f * num * penalidade;
    }
}


double NameS_Grasp::getPenalidade1Nivel(Solucao &sol, Instancia &instance, float f)
{
    double cargaVeic = 0.0;
    double cargaSat  = 0.0;

    static bool calcPenal = true;
    static double penalidade = 0.0;

    int num = 0;

    if(sol.viavel)
        return 0.0;

    //if(!calcPenal)
    {

        for(int i=instance.getFirstClientIndex(); i <= instance.getEndClientIndex(); ++i)
        {
            if(sol.vetClientesAtend[i] == int8_t(0))
                num += 1;
        }

        if(calcPenal)
        {

            penalidade = 0.0;
            for(int sat=instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
            {
                penalidade += 2.0*instance.getDistance(0, sat);
            }
            calcPenal = false;
        }

        if(num > 0)
        {
            //cout<<"2º NIVEL INVIAVEL\n";
            return penalidade*f;
        }
        else
        {
            double dist1Nivel = 0.0;

            for(int sat=instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
            {
                cargaSat += sol.satelites[sat].demanda;
            }

            for(Route &route:sol.primeiroNivel)
            {
                cargaVeic += route.totalDemand;
                dist1Nivel += route.totalDistence;
            }

            /*
             * cargaVeic ---------------------- dist1Nivel
             * (cargaSat-cargaVeic) ----------- ?
             *
             * ? = ((cargaSat-cargaVeic) * dist1Nivel)/cargaVeic
             *
             */

            //cout<<"CARGA SAT: "<<cargaSat<<"; CARGA VEIC: "<<cargaVeic<<"\n\n";


            double dist = ((cargaSat-cargaVeic) * dist1Nivel)/cargaVeic;
            return dist*f;

        }
    }

}

double NameS_Grasp::getDistMaisPenalidade(Solucao &sol, Instancia &instancia)
{
    static double penalidade = 1.2*instancia.penalizacaoDistEv;// + instancia.penalizacaoDistComb;

    if(sol.viavel)
        return sol.distancia;

    int num = 0;
    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        if(sol.vetClientesAtend[i] == int8_t(0))
            num += 1;
    }

    return sol.distancia + num*penalidade;
}

void NameS_Grasp::inicializaSol(Solucao &sol, Instancia &instance)
{

    BoostC::vector<EstDist> vetEstDist(instance.getN_RechargingS());
    EvRoute evRoute(-1, instance.getFirstEvIndex(), instance.getEvRouteSizeMax(), instance);

    for(int i=instance.getFirstSatIndex(); i <= instance.getEndSatIndex(); ++i)
    {
        int vetTam = 0;
        evRoute.satelite = i;

        for(int r= instance.getFirstRS_index(); r <= instance.getEndRS_index(); ++r)
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
        }
        while(p != pos);
    }


}

void NameS_Grasp::addRotaCliente(Solucao &sol, Instancia &instancia, const EvRoute &evRoute, const int cliente)
{

    if(evRoute.routeSize <= 2)
        return;

    string str;
    //evRoute.print(str, instancia, true);

    int sat = evRoute.satelite;

    int next = 0;

    for(int i=0; i < sol.satelites[sat].tamVetEvRoute; ++i)
    {
        next = i;
        if(sol.satelites[sat].vetEvRoute[i].routeSize <= 2)
            break;
    }

    if(next >= sol.satelites[sat].tamVetEvRoute)
        return;

    if(sol.satelites[sat].vetEvRoute[next].routeSize > 2)
        return;

    sol.numEv += 1;

    int aux = sol.satelites[sat].vetEvRoute[next].idRota;
    sol.satelites[sat].vetEvRoute[next].copia(evRoute, false, nullptr);
    sol.satelites[sat].vetEvRoute[next].idRota = aux;


    // Atualiza distacia e demanda
    double dist = sol.satelites[sat].vetEvRoute[next].distancia;
    //cout<<"DIST: "<<dist<<"\n";
    sol.satelites[sat].distancia += dist;
    sol.distancia += dist;
    sol.satelites[sat].demanda   += instancia.vectCliente[cliente].demanda;

    sol.satelites[sat].tamVetEvRoute += 1;
    sol.vetClientesAtend[cliente] = int8_t(1);
    sol.solInicializada = true;

}