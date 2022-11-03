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

const float fator = 0.1;

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
                             std::vector<NS_vnd::MvValor> &vetMvValor, std::vector<NS_vnd::MvValor> &vetMvValor1Nivel,
                             NS_parametros::ParametrosSaida &parametrosSaida)
{
    vetMvValor1Nivel = std::vector<MvValor>(2);

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

    construtivo(gul, instance, 0.0, 0.0, matClienteSat);
    const double gulCusto = getDistMaisPenalidade(gul, instance);
    double custoBest = gulCusto;

    //Vetores para o reativo
    std::vector<double> vetorProbabilidade(tamAlfa);
    std::fill(vetorProbabilidade.begin(), vetorProbabilidade.begin()+tamAlfa, 100.0/float(tamAlfa));

    std::vector<int>    vetorFrequencia(tamAlfa);
    std::fill(vetorFrequencia.begin(), vetorFrequencia.begin()+tamAlfa, 1);

    std::vector<double> solucaoAcumulada(tamAlfa);
    std::fill(solucaoAcumulada.begin(), solucaoAcumulada.begin()+tamAlfa, gulCusto);

    std::vector<double> vetorMedia(tamAlfa);
    std::fill(vetorMedia.begin(), vetorMedia.begin()+tamAlfa, 0.0);

    std::vector<double> proporcao(tamAlfa);
    std::fill(proporcao.begin(), proporcao.begin()+tamAlfa, 0.0);


    auto atualizaProb = [&]()
    {
        double somaProporcoes = 0.0;

        //Calcular média
        for(int i = 0; i < tamAlfa; ++i)
            vetorMedia[i] = solucaoAcumulada[i]/double(vetorFrequencia[i]);

        //Calcula proporção.
        for(int i = 0; i < tamAlfa; ++i)
        {
            proporcao[i] = custoBest/vetorMedia[i];
            somaProporcoes += proporcao[i];
        }

        //Calcula probabilidade
        for(int i = 0; i< tamAlfa; ++i)
            vetorProbabilidade[i] = 100.0*(proporcao[i]/somaProporcoes);

        //cout<<"vet prob: \n";
        //string vet = NS_Auxiliary::printVector(vetorProbabilidade, int64_t(vetorProbabilidade.size()));

        //cout<<vet<<"\n";

        double sum = 0.0;
        for(int i=0; i < tamAlfa; ++i)
            sum += vetorProbabilidade[i];

        //cout<<"sum: "<<sum<<"\n\n";

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
    std::vector<QuantCliente> vetQuantCliente(instance.getNClients());
    std::vector<QuantCliente> vetQuantProb0;

    for(int i=instance.getFirstClientIndex(); i <= instance.getEndClientIndex(); ++i)
        vetQuantCliente[convIndClienteVet(i)].cliente = i;


    double somaProb = 0.0;
    int posAlfa = 0;
    int valAleatorio = 0;
    float alfa = 0;

    int addRotaClienteProbIgual = 0;
    int clienteAdd = -1;

    vetMvValor = std::vector<MvValor>(NUM_MV_LS);
    for(int i=0; i < NUM_MV_LS; ++i)
        vetMvValor[i].mv = i;

    std::unordered_set<VetorHash, VetorHash::HashFunc> hashRotaSet;
    std::unordered_set<VetorHash, VetorHash::HashFunc> hashSolSet;

    int numSolGeradas   = 0;
    int numRotasGeradas = 0;

    std::unordered_set<VetorHash, VetorHash::HashFunc> hashRotasIncluidas;
    std::unordered_set<VetorHash, VetorHash::HashFunc> hashRotasExcluidas;

    bool iniRS = false;

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
                //cout<<"t: "<<t<<"\n";
                //cout<<"conv(t): "<<convClienteIndVet(t)<<"\n";

                const int sat = vetSatAtendCliente[convClienteIndVet(t)];

                if(sat < 0)
                    continue;

                //cout<<"vetSatAtendCliente: "<<sat<<"\n";

                const EvRoute &evRouteAux = instance.shortestPath[sat].getEvRoute(convClienteIndVet(t));

//cout<<"evRouteAux.routeSize="<<evRouteAux.routeSize<<"\n";

                vetQuantCliente.at(t).calculaProb(i);
                if(vetQuantCliente.at(t).prob == 0 && evRouteAux.routeSize > 2)
                    addRotaClienteProbIgual += 1;

                else if(vetQuantCliente.at(t).prob >= 90 && evRouteAux.routeSize > 2)
                    vetQuantCliente[t].prob = 90;

                else if(evRouteAux.routeSize <= 2)
                    vetQuantCliente[t].prob = 100;
            }

            std::sort(vetQuantCliente.begin(), vetQuantCliente.end());

            if(estat.numSol == 0)
            {
                parametros.numMaxClie += 1;
                if(parametros.numMaxClie > instance.getN_Evs())
                    parametros.numMaxClie -= 1;

            }

        }

        Solucao solTemp(instance);


        // Insere  rotas da forama: sat cliente sat
        if(i >= parametros.iteracoesCalProb && parametros.iteracoesCalProb > 0 && instance.shortestPath &&
                (i < parametros.numIteGrasp/2 || (i > parametros.numIteGrasp/2 && instance.nivel2Viavel)) && !iniRS)
        {
            int clientesAdd = 0;

            bool add   = false;
            bool igual = false;

            int t = rand_u32()%instance.getNClients();
            const int inicio = t;


            if(vetQuantCliente[0].prob != 100)
            {

                std::vector<int> vetSatRotaInicializada(1+(instance.numSats*instance.numEv), 0);

                do
                {
                    if(vetQuantCliente[t].prob != 100)
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
        else if((!instance.nivel2Viavel && i >= parametros.numIteGrasp/2) || iniRS)
        {

            iniRS = true;

            std::vector<int> vetRS_uti(instance.getEndSatIndex()+1, 0);
            std::fill(vetRS_uti.begin(), vetRS_uti.begin()+instance.getFirstRS_index(), -1);

            ublas::matrix<int> matrixSatEv(instance.numSats+1, instance.numEv, 0);

            int numRs = 0;


            while(numRs < parametros.numMaxClie)
            {
                bool inserRS = false;

                if(numRs == instance.numEv)
                    break;

                // Encontra um estacao de recarga nao utilizada

                int rs = instance.getFirstRS_index()+(rand_u32()%instance.numRechargingS);

                while(vetRS_uti[rs] == 1)
                    rs = instance.getFirstRS_index()+((rs+1)%instance.numRechargingS);

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
                            evRouteAux[3].cliente = sat;

                            evRouteAux.routeSize = 4;

                            double dist = NameViabRotaEv::testaRota(evRouteAux, evRouteAux.routeSize, instance, false,
                                                                    evRouteAux[0].tempoSaida, 0, nullptr);
                            if(dist > 0.0)
                            {
                                evRouteAux.distancia = dist;
                                sol.satelites[sat].vetEvRoute[ev].copia(evRouteAux, true, &instance);
                                sol.satelites[sat].distancia += dist;
                                sol.distancia += dist;
                                matrixSatEv(sat,ev) = 1;
                                numRs += 1;
                                inserRS = true;

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

        valAleatorio = rand_u32() % 100;

        for(int j=0;somaProb < valAleatorio; ++j)
        {

            if(j >= tamAlfa)
            {
                break;
            }

            somaProb+= (vetorProbabilidade[j]);
            posAlfa = j;
        }


        alfa = parametros.vetAlfa[posAlfa];
        vetorFrequencia[posAlfa] += 1;

        solTemp.copia(sol);
        construtivo(sol, instance, alfa, alfa, matClienteSat);

        // Add 1 se o cliente t nao foi atendido
        if(!sol.viavel && parametros.iteracoesCalProb > 0 && instance.shortestPath)
        {
            int quantCliInv = 0;
            for(int t=instance.getFirstClientIndex(); t <= instance.getEndClientIndex(); ++t)
            {
                if(sol.vetClientesAtend[t] == 0)
                {

                    quantCliInv += 1;

                    for(int sat=instance.getFirstSatIndex(); sat <= instance.getEndSatIndex(); ++sat)
                    {
                        if(matClienteSat(t,sat) == 1)
                        {   // ERRO vetSatAtendCliente NAO Guarda mais o sat que atende o cliente t
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
                cout<<"SOLUCAO INICIAL:\n\n";
                solTemp.print(instance);


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

                    solBest->copia(sol);
                    solBest->ultimaA = i;

                    if(retPrimeiraSol)
                        return solBest;

                    custoBest = solBest->distancia;
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


/*            for(int s=1; s <= instance.numSats; ++s)
            {
                for(int ev=0; ev < instance.numEv; ++ev)
                {
                    VetorHash vetorHash(sol.satelites[s].vetEvRoute[ev]);

                    if(hashRotasIncluidas.find(vetorHash) != hashRotasIncluidas.end())
                    {
                        hashRotasExcluidas.insert(vetorHash);

                        string saida;
                        sol.satelites[s].vetEvRoute[ev].print(saida, instance, true);
                        cout<<"EXCLUINDO ROTA: "<<saida<<"\n";
                    }
                }
            }*/

            {
                //cout<<"ATUAL\n";
                solBest->copia(sol);

                //cout<<"seg: "<<sol.segundoNivelViav<<"\n";
                //cout<<"seg: "<<solBest->segundoNivelViav<<"\n\n";

                double aux = sol.distancia + getPenalidade(sol, instance, fator);
                if(aux < custoBest)
                    custoBest = aux;
            }
        }

        if(solBest->viavel && i >= 500 && (i-solBest->ultimaA) >= 400)
        {
            //cout<<"break i: "<<i<<"\n";
            //cout<<"UltimaA: "<<solBest->ultimaA<<"\n";
            break;
        }

        if(!sol.viavel)
            solucaoAcumulada[posAlfa] += sol.distancia + getPenalidade(sol, instance, fator);
        else
            solucaoAcumulada[posAlfa] += sol.distancia;

        if(i>0 && (i%parametros.numAtualProbReativo)==0)
            atualizaProb();


        //if(iniRS) cout<<"\n*********************************************\n";

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

double NameS_Grasp::getPenalidade(Solucao &sol, Instancia &instancia, float f)
{
    static double penalidade = 1.2*instancia.penalizacaoDistEv;


    if(sol.viavel)
        return 0.0;

    int num = 0;
    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        if(sol.vetClientesAtend[i] == 0)
            num += 1;
    }

    return f*num*penalidade;
}

double NameS_Grasp::getDistMaisPenalidade(Solucao &sol, Instancia &instancia)
{
    static double penalidade = 1.2*instancia.penalizacaoDistEv;// + instancia.penalizacaoDistComb;

    if(sol.viavel)
        return sol.distancia;

    int num = 0;
    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        if(sol.vetClientesAtend[i] == 0)
            num += 1;
    }

    return sol.distancia + num*penalidade;
}

void NameS_Grasp::inicializaSol(Solucao &sol, Instancia &instance)
{

    std::vector<EstDist> vetEstDist(instance.getN_RechargingS());
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
    sol.vetClientesAtend[cliente] = 1;
    sol.solInicializada = true;

}