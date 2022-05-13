#include "greedyAlgorithm.h"
#include "Auxiliary.h"
#include <set>
#include <cfloat>
#include "mersenne-twister.h"
#include "LocalSearch.h"
#include "ViabilizadorRotaEv.h"

using namespace GreedyAlgNS;
using namespace std;
using namespace NS_Auxiliary;
using namespace NS_LocalSearch;
using namespace NameViabRotaEv;
using namespace boost::numeric;

// Roteamento dos veiculos eletricos
bool GreedyAlgNS::secondEchelonGreedy(Solution& sol, const Instance& instance, const float alpha)
{

    std::vector<int> visitedClients(1 + instance.getNSats() + instance.getN_RechargingS() + instance.getNClients());

    for(int i=0; i < instance.getFirstClientIndex(); ++i)
        visitedClients[i] = -1;

    for(int i= instance.getFirstClientIndex(); i < visitedClients.size(); ++i)
        visitedClients[i] = 0;


    const int FistIdClient  = instance.getFirstClientIndex();
    const int LastIdClient  = FistIdClient + instance.getNClients()-1;
    const auto ItEnd        = visitedClients.begin() + instance.getNSats() + instance.getNClients();
    const std::vector<double> vetTempoSaida = GreedyAlgNS::calculaTempoSaidaInicialSat(instance);
    EvRoute evRouteAux(-1, -1, instance.getEvRouteSizeMax(), instance);

    std::list<CandidatoEV> listaCandidatos;
    std::list<int> clientesSemCandidato;
    static std::vector<ublas::matrix<CandidatoEV*>> matCandidato(1 + instance.getNSats());         //COLUNAS DA MATRIZ POSSUEM SOMENTE A QUANTIDADE DE CLIENTES!!
    static bool primeiraChamada = true;
    std::vector<CandidatoEV*> vetCandPtr(instance.getNClients(), nullptr);

    const int numLinhasMat      = instance.getN_Evs();
    const int numColMat         = instance.getNClients();
    const int idPrimeiroCliente = instance.getFirstClientIndex();
    const int idPrimeiroEv      = instance.getFirstEvIndex();

    auto transformaIdCliente  = [&](const int id){return (id - idPrimeiroCliente);};
    auto transformaIdEv       = [&](const int id){return (id-idPrimeiroEv);};


    if(primeiraChamada)
    {
        for(int satId = 1; satId < (1 + instance.getNSats()); ++satId)
        {
            matCandidato[satId].resize(numLinhasMat, numColMat, false);

            for(int i = 0; i < numLinhasMat; ++i)
            {
                for(int j = 0; j < numColMat; ++j)
                    matCandidato[satId](i, j) = nullptr;
            }
        }
        primeiraChamada = false;
    }

    // Cria um candidato para cada cliente e armazena em matCandidato
    for(int clientId = FistIdClient; clientId <= LastIdClient; ++clientId)
    {

        CandidatoEV candidatoEv;
        candidatoEv.clientId = clientId;

        for(int satId = instance.getFirstSatIndex(); satId <= instance.getEndSatIndex(); ++satId)
        {

            Satelite *sat = sol.getSatelite(satId);
            int routeId = 0;
            EvRoute &route = sat->getRoute(routeId);

            CandidatoEV candidatoEvAux(candidatoEv);
            candidatoEvAux.satId = satId;

            canInsert(route, clientId, instance, candidatoEv, satId, vetTempoSaida[satId], evRouteAux);


        }

        if(candidatoEv.pos != -1)
        {


            listaCandidatos.push_back(candidatoEv);
            CandidatoEV *candPtr = &listaCandidatos.back();


            matCandidato[candPtr->satId](transformaIdEv(candPtr->routeId), transformaIdCliente(candPtr->clientId)) = candPtr;
            vetCandPtr[transformaIdCliente(candPtr->clientId)] = candPtr;



        }
        else
            clientesSemCandidato.push_back(clientId);

    }



    auto printMatCand = [&]()
    {
        cout<<"\t";
        for(int i=idPrimeiroCliente; i <= instance.getEndClientIndex(); ++i)
            cout<<i<<"\t";

        cout<<"\n";

        for(int evId = instance.getFirstEvIndex(); evId <= instance.getEndEvIndex(); ++evId)
        {
            cout<<evId<<"\t";
            for(int i=idPrimeiroCliente; i <= instance.getEndClientIndex(); ++i)
            {
                CandidatoEV *candidatoEv = matCandidato[1](transformaIdEv(evId), transformaIdCliente(i));
                cout<<(candidatoEv != nullptr)<<"\t";
            }
            cout<<"\n";

        }


    };



    while(!visitAllClientes(visitedClients, instance) && !listaCandidatos.empty())
    {

        int randIndex = rand_u32()%(int(alpha * listaCandidatos.size() + 1));
        listaCandidatos.sort();

        auto topItem = std::next(listaCandidatos.begin(), randIndex);
        CandidatoEV *candEvPtr = &(*topItem);

        visitedClients[topItem->clientId] = 1;

        Satelite *satelite = sol.getSatelite(topItem->satId);
        satelite->demanda += topItem->demand;



        EvRoute &evRoute = satelite->getRoute(transformaIdEv(topItem->routeId));
        //cout<<"insereEstacao: "<<topItem->insercaoEstacao.pos<<"\n\n";
        bool resultadoTopItem = insert(evRoute, *topItem, instance, vetTempoSaida[topItem->satId], sol);

        if(!resultadoTopItem)
        {
            PRINT_DEBUG("", "ERRO, INSERT RETORNOU FALSE!!!\n\n");
            throw "ERRO";
        }

        matCandidato[topItem->satId](transformaIdEv(topItem->routeId), transformaIdCliente(topItem->clientId)) = nullptr;

        string strRota;
        evRoute.print(strRota, instance, false);

        // Corrigi a lista de candidatos
        // 1º Os candidatos que estao no mesmo satelite e na mesma rota precisao ser avaliados novamente
        // 2º Os clientes que nao possuem candidato so tem que ser avaliados na rota que houve mudanca
        // 3º Os candidatos que nao estao na mesma rota so precisao ser reavaliados na rota que houve mudanca



        // 1º Os candidatos que estao no mesmo satelite e na mesma rota precisao ser avaliados novamente em todos os sat e rotas

        Satelite *sat = sol.getSatelite(candEvPtr->satId);
        EvRoute &evRouteEsc = sat->vetEvRoute[transformaIdEv(candEvPtr->routeId)];

        if(listaCandidatos.size() > 1)
        {
            for(int clientId = instance.getFirstClientIndex(); clientId <= instance.getEndClientIndex(); ++clientId)
            {

                CandidatoEV *candidatoEvPtrAux = matCandidato[sat->sateliteId](transformaIdEv(evRouteEsc.idRota), transformaIdCliente(clientId));

                if((!visitedClients[clientId]) && candidatoEvPtrAux && clientId != topItem->clientId)
                {

                    matCandidato[sat->sateliteId](transformaIdEv(evRouteEsc.idRota), transformaIdCliente(clientId)) = nullptr;
                    CandidatoEV candidatoEv;
                    candidatoEv.clientId = clientId;

                    for(int satId = instance.getFirstSatIndex(); satId <= instance.getEndSatIndex(); satId++)
                    {

                        bool routeEmpty = false;


                        for(int routeId = instance.getFirstEvIndex(); routeId <= instance.getEndEvIndex(); routeId++)
                        {
                            Satelite *satAux = sol.getSatelite(satId);

                            EvRoute &route = satAux->getRoute(transformaIdEv(routeId));
                            canInsert(route, clientId, instance, candidatoEv, satId, vetTempoSaida[satId], evRouteAux);

                        }

                    }

                    if(candidatoEv.pos != -1)
                    {
                        // Encontrou uma rota viavel
                        *candidatoEvPtrAux = candidatoEv;
                        matCandidato[candidatoEv.satId](transformaIdEv(candidatoEv.routeId),
                                                        transformaIdCliente(candidatoEv.clientId)) = candidatoEvPtrAux;
                    } else
                    {
                        //cout<<"nao encontrou rota viavel para o cliente: "<<clientId<<"\n\n";

                        // Nao existe uma rota viavel para o cliente
                        listaCandidatos.remove(*candidatoEvPtrAux);
                        vetCandPtr[transformaIdCliente(clientId)] = nullptr;
                        candidatoEvPtrAux = nullptr;
                        clientesSemCandidato.push_back(clientId);

                    }

                }
                // 3º Os candidatos que nao estao na mesma rota so precisao ser reavaliados na rota que houve mudanca
                else if((!visitedClients[clientId]) && !candidatoEvPtrAux && clientId != topItem->clientId)
                {
                    candidatoEvPtrAux = vetCandPtr[transformaIdCliente(clientId)];


                    if(candidatoEvPtrAux)
                    {
                        CandidatoEV candCopia = *candidatoEvPtrAux;

                        //matCandidato[candidatoEvPtrAux->satId](candidatoEvPtrAux->routeId, transformaIdCliente(candidatoEvPtrAux->clientId)) = nullptr;

                        const int satId = topItem->satId;
                        const int routeId = topItem->routeId;

                        bool resultado = canInsert(sol.satelites[satId].getRoute(transformaIdEv(routeId)), clientId, instance,
                                                   *candidatoEvPtrAux, satId, vetTempoSaida[satId], evRouteAux);

                        if(resultado)
                        {
                            matCandidato[candCopia.satId](transformaIdEv(candCopia.routeId),
                                                                   transformaIdCliente(candCopia.clientId)) = nullptr;

                            matCandidato[satId](transformaIdEv(candidatoEvPtrAux->routeId),
                                                transformaIdCliente(candidatoEvPtrAux->clientId)) = candidatoEvPtrAux;
                        }

                    }
                }
            }
        }

        bool clientesAntendidos = visitAllClientes(visitedClients, instance);

        if(!clientesAntendidos)
        {
            // Tenta inserir os clientes sem candidato na nova rota

            auto itCliente = clientesSemCandidato.begin();
            int cliente = -1;

            while(itCliente != clientesSemCandidato.end())
            {
                cliente = *itCliente;
                CandidatoEV candidatoEv;

                const int satId = topItem->satId;
                const int routeId = topItem->routeId;

                bool resultado = canInsert(sol.satelites[satId].getRoute(transformaIdEv(routeId)), cliente, instance,
                                           candidatoEv, satId, vetTempoSaida[satId], evRouteAux);

                if(resultado)
                {
                    listaCandidatos.push_back(candidatoEv);
                    CandidatoEV *candPtr = &listaCandidatos.back();
                    matCandidato[satId](transformaIdEv(routeId), transformaIdCliente(cliente)) = candPtr;
                    vetCandPtr[transformaIdCliente(cliente)] = candPtr;
                    ++itCliente;
                    clientesSemCandidato.remove(cliente);
                }
                else
                    ++itCliente;

            }
        }

        // Remove candidato escolhido
        vetCandPtr[transformaIdCliente(topItem->clientId)] = nullptr;
        matCandidato[topItem->satId](transformaIdEv(topItem->routeId), transformaIdCliente(topItem->clientId)) = nullptr;
        listaCandidatos.erase(topItem);

        if(listaCandidatos.empty())
        {

            break;
        }

    }


    sol.viavel = visitAllClientes(visitedClients, instance);

    return sol.viavel;
}

// run ../instancias/2e-vrp-tw/Customer_5/C101_C5x.txt 1652454289

bool GreedyAlgNS::visitAllClientes(std::vector<int> &visitedClients, const Instance &instance)
{

    auto itClient = visitedClients.begin() + instance.getFirstClientIndex();
    int i=instance.getFirstClientIndex();

    for(; i < visitedClients.size(); ++i)
    {

        if(visitedClients[i] == 0)
            return false;
        else if(visitedClients[i] != 1)
        {
            PRINT_DEBUG("\t\t", "indice "<<i<<"eh invalido para vetor visitedClients");
            throw "ERRO";

        }

    }

    //for(i=0; i < visitedClients.size(); ++i)
     //   cout<<i<<": "<<visitedClients[i]<<"\n";

    return true;

}


void GreedyAlgNS::firstEchelonGreedy(Solution &sol, const Instance &Inst, const float beta)
{

    // Cria o vetor com a demanda de cada satellite



    std::vector<double> demandaNaoAtendidaSat;
    demandaNaoAtendidaSat.reserve(sol.getNSatelites()+1);
    int satId = 1;
    demandaNaoAtendidaSat.push_back(0.0);

    for(int sat=Inst.getFirstSatIndex(); sat <= Inst.getEndSatIndex(); ++sat)
    {
        demandaNaoAtendidaSat.push_back(sol.satelites[sat].demanda);
    }

    const int NumSatMaisDep = sol.getNSatelites()+1;

    // Enquanto existir um satellite com demanda > 0, continua
    while(existeDemandaNaoAtendida(demandaNaoAtendidaSat))
    {
        // Cria a lista de candidatos
        std::list<CandidatoVeicComb> listaCandidatos;

        // Percorre os satellites
        for(int i=1; i < NumSatMaisDep; ++i)
        {
            Satelite &satelite = sol.satelites[i];

            // Verifica se a demanda não atendida eh positiva
            if(demandaNaoAtendidaSat[i] > 0.0)
            {

                // Percorre todas as rotas
                for(int rotaId = 0; rotaId < sol.primeiroNivel.size(); ++rotaId)
                {
                    Route &route = sol.primeiroNivel[rotaId];

                    // Verifica se veiculo esta 100% da capacidade
                    if(route.totalDemand < Inst.getTruckCap(rotaId))
                    {
                        // Calcula a capacidade restante do veiculo
                        double capacidade = Inst.getTruckCap(rotaId) - route.totalDemand;
                        double demandaAtendida = capacidade;

                        if(demandaNaoAtendidaSat[i] < capacidade)
                            demandaAtendida = demandaNaoAtendidaSat[i];


                        CandidatoVeicComb candidato(rotaId, i, demandaAtendida, DOUBLE_MAX);

                        // Percorre todas as posicoes da rota
                        for(int p=0; (p+1) < route.routeSize; ++p)
                        {
                            double incrementoDist = 0.0;

                            // Realiza a insercao do satellite entre as posicoes p e p+1 da rota
                            const RouteNo &clienteP =  route.rota[p];
                            const RouteNo &clientePP = route.rota[p+1];

                            // Calcula o incremento da distancia (Sempre positivo, desigualdade triangular)
                            incrementoDist -= Inst.getDistance(clienteP.satellite, clientePP.satellite);
                            incrementoDist = incrementoDist+ Inst.getDistance(clienteP.satellite, i) + Inst.getDistance(i, clientePP.satellite);

                            if(incrementoDist < candidato.incrementoDistancia)
                            {

                                // Calcula o tempo de chegada e verifica a janela de tempo
                                const double tempoChegCand = clienteP.tempoChegada + Inst.getDistance(clienteP.satellite, i);

                                bool satViavel = true;

                                if(verificaViabilidadeSatelite(tempoChegCand, satelite, Inst, false))
                                {
                                    double tempoChegTemp = tempoChegCand + Inst.getDistance(i, clientePP.satellite);

                                    // Verificar viabilidade dos outros satelites
                                    for(int t=p+1; (t+1) < (route.routeSize); ++t)
                                    {
                                        Satelite &sateliteTemp = sol.satelites[route.rota[t].satellite];

                                        if(!verificaViabilidadeSatelite(tempoChegTemp, sateliteTemp, Inst, false))
                                        {
                                            satViavel = false;
                                            break;
                                        }

                                        tempoChegTemp += Inst.getDistance(route.rota[t].satellite, route.rota[t+1].satellite);

                                    }

                                }
                                else
                                {
                                    satViavel = false;
                                }

                                if(satViavel)
                                {

                                    candidato.incrementoDistancia = incrementoDist;
                                    candidato.pos = p;
                                    candidato.tempoSaida = tempoChegCand;
                                }
                            }
                        }

                        if(candidato.pos >= 0)
                            listaCandidatos.push_back(candidato);

                    }
                }
            }

        }


        if(!listaCandidatos.empty())
        {

            listaCandidatos.sort();

            // Escolhe o candidado da lista restrita
            int tam = int(beta * listaCandidatos.size()) + 1;
            int escolhido = rand_u32() % tam;
            auto it = listaCandidatos.begin();

            std::advance(it, escolhido);
            CandidatoVeicComb &candidato = *it;

            // Insere candidato na solucao
            Route &route = sol.primeiroNivel[candidato.rotaId];
            shiftVectorDir(route.rota, candidato.pos + 1, 1, route.routeSize);


            route.rota[candidato.pos+1].satellite = candidato.satelliteId;
            route.rota[candidato.pos+1].tempoChegada = candidato.tempoSaida;
            route.routeSize += 1;
            double tempoSaida = candidato.tempoSaida;

            for(int i=candidato.pos+1; (i+1) < route.routeSize; ++i)
            {
                const int satTemp = route.rota[i].satellite;

                if(!verificaViabilidadeSatelite(tempoSaida, sol.satelites[satTemp], Inst, true))
                {
                    string satStr;
                    sol.satelites[satTemp].print(satStr, Inst);


                    sol.print(Inst);

                    throw "ERRO!";
                }

                tempoSaida += Inst.getDistance(satTemp, route.rota[i+1].satellite);
            }

            // Atualiza demanda, vetor de demanda e distancia
            route.totalDemand += candidato.demand;
            sol.distancia += candidato.incrementoDistancia;
            route.satelliteDemand[candidato.satelliteId] = candidato.demand;
            route.totalDistence += candidato.incrementoDistancia;

            demandaNaoAtendidaSat[candidato.satelliteId] -= candidato.demand;

        }
        else
        {
            sol.viavel = false;
            break;
        }
    }


}

/*
 * Erro!
 * Verificar o shift da rota, possibilidade de atrasar a rota
 */

// Com o tempo de chegada ao satelite, eh verificado se as rotas dos EV's podem sair apos o tempo de chegada do veic a combustao
bool GreedyAlgNS::verificaViabilidadeSatelite(const double tempoChegada, Satelite &satelite, const Instance &instance, const bool modificaSatelite)
{

    bool viavel = true;

    if(satelite.sateliteId == 0)
        return true;


    // Verifica se os tempos de saida das rotas dos EV's eh maior que o tempo de chegada do veic a combustao
    for(int evId = 0; evId < instance.getN_Evs(); ++evId)
    {
        TempoSaidaEvRoute &tempoSaidaEv = satelite.vetTempoSaidaEvRoute[evId];
        const double tempoEv = tempoSaidaEv.evRoute->route[0].tempoSaida;


        if(tempoSaidaEv.evRoute->routeSize > 2)
        {

            // Verifica se o tempo de saida do ev eh inconpativel com o tempo de chegada do veic a comb
            if(tempoEv < tempoChegada)
            {
                string rotaStr;
                tempoSaidaEv.evRoute->print(rotaStr, instance, true);

                // Verifica se eh possivel realizar um shift na rota

                const int indice = tempoSaidaEv.evRoute->route[0].posMenorFolga;
                const int cliente = tempoSaidaEv.evRoute->route[indice].cliente;
                const double twFim = instance.vectCliente[cliente].fimJanelaTempo;

                // calcula a menor diferenca da rota
                double diferenca = twFim - tempoSaidaEv.evRoute->route[indice].tempoCheg;
                if(diferenca < 0.0)
                    diferenca = 0.0;



                if(!((tempoEv + diferenca) >= tempoChegada))
                {
                    viavel = false;
                    break;
                }

            }

        }
    }

    if(!modificaSatelite)
    {
        return viavel;
    }
    else if(viavel)
    {

        for(auto &tempoSaidaEv:satelite.vetTempoSaidaEvRoute)
        {

            const double tempoEv = tempoSaidaEv.evRoute->route[0].tempoSaida;

            if(tempoSaidaEv.evRoute->routeSize > 2)
            {
                if(tempoEv >= tempoChegada)
                {
                    return true;

                } else
                {
                    // Verifica se eh possivel realizar um shift na rota

                    const int indice = tempoSaidaEv.evRoute->route[0].posMenorFolga;
                    const int cliente = tempoSaidaEv.evRoute->route[indice].cliente;
                    const double twFim = instance.vectCliente[cliente].fimJanelaTempo;

                    double diferenca = twFim - tempoSaidaEv.evRoute->route[indice].tempoCheg;
                    if(diferenca < 0.0)
                        diferenca = 0.0;

                    if(!((tempoEv + diferenca) >= tempoChegada))
                    {
                        // Nao deve chegar aqui

                        return false;
                    } else
                    {
                        if(!tempoSaidaEv.evRoute->alteraTempoSaida(tempoChegada, instance))
                        {
                            PRINT_DEBUG("", "ERRO AO ALTERAR O TEMPO DE SAIDA DA ROTA EV DE ID: "
                                    << tempoSaidaEv.evRoute->idRota << " DO SATELITE: " << satelite.sateliteId
                                    << "\n\n");
                            cout<<"\n\n***********************************************************************VERIFICA VIABILIDADE SAT FIM**************************************************************************************\n\n";
                            throw "ERRO";
                        }

                    }

                }

            }
        }

        //PRINT_DEBUG("", "ERRO SATELITE: "<<satelite.sateliteId<<", DEVERIA TER PELO MENOS UMA ROTA(ORDENADAS DE FORMA CRESENTE COM TEMPO DE SAIDA) COM TEMPO DE SAIDA MAIOR OU IGUAL A "<<tempoChegada);
        //throw "ERRO";
        return true;

    }
    else
        return false;

}



bool GreedyAlgNS::existeDemandaNaoAtendida(std::vector<double> &demandaNaoAtendida)
{
    for(double dem:demandaNaoAtendida)
    {
        if(dem > 0.0)
            return true;
    }

    return false;
}

void GreedyAlgNS::greedy(Solution &sol, const Instance &Inst, const float alpha, const float beta)
{
    //if(secondEchelonGreedy(sol, Inst, alpha))
    if(secondEchelonGreedy(sol, Inst, alpha))
    {
        firstEchelonGreedy(sol, Inst, beta);


        //sol.print(Inst);

        if(sol.viavel)
            sol.atualizaVetSatTempoChegMax(Inst);


    }

}



bool GreedyAlgNS::canInsert(EvRoute &evRoute, int node, const Instance &instance, CandidatoEV &candidatoEv, const int satelite, const double tempoSaidaSat, EvRoute &evRouteAux)
{
    double demand = instance.getDemand(node);
    double bestIncremento = candidatoEv.incremento;
    bool viavel = false;

    //cout<<"func: canInsert\n";

//    EvRoute evRouteAux(satelite, evRoute.idRota, evRoute.routeSizeMax, instance);

    evRouteAux.satelite = satelite;
    evRouteAux.idRota = evRoute.idRota;
    evRouteAux.routeSizeMax = evRoute.routeSizeMax;


    InsercaoEstacao insercaoEstacao;


    if((evRoute.getDemand() + demand) > instance.getEvCap(evRoute.idRota))
    {

        return false;
    }


    //std::copy(evRoute.route.begin(), evRoute.route.begin()+evRoute.routeSize, evRouteAux.route.begin());

    copiaVector(evRoute.route, evRouteAux.route, evRoute.routeSize);
    shiftVectorDir(evRouteAux.route, 1, 1, evRoute.routeSize);
    evRouteAux.routeSize = evRoute.routeSize+1;

    evRouteAux[0].bateriaRestante = instance.getEvBattery(evRoute.idRota);
    evRouteAux[0].tempoSaida = tempoSaidaSat;

    //evRouteAux.print(instance, true);

    double distanciaRota  = 0.0;
    if(evRoute.routeSize > 2)
    {
        for(int i=0; (i+1) < evRoute.routeSize; ++i)
            distanciaRota += instance.getDistance(evRoute[i].cliente, evRoute[i + 1].cliente);
    }

/*    cout<<"\tnode: "<<node<<"\n";
    cout<<"ROTA: ";
    evRoute.print(instance, false);
    cout<<"\n";*/

    /* *****************************************************JANELA DE TEMPO******************************************************
     * **************************************************************************************************************************
     *
     * 0 1 2 3 0
     *
     *      Para verificar a janela de tempo ao inserir o cliente 4 eh necessario verificar se eh possivel chegar ao cliente 4,
     * verifica-se a viabilidade: <CLIENTE MENOR> acresentando a (diferença do tempo de chegada do proximo cliente).
     *
     *
     * **************************************************************************************************************************
     * **************************************************************************************************************************
     */

    const ClienteInst &instNode = instance.vectCliente[node];

    for(int pos = 0; pos < evRoute.routeSize-1; pos++)
    {

        evRouteAux.route[pos+1].cliente = node;
        double distanceAux = instance.getDistance(evRouteAux[pos].cliente, node) + instance.getDistance(node, evRouteAux[pos+2].cliente) -
                             instance.getDistance(evRouteAux[pos].cliente, evRouteAux[pos+2].cliente);


        if(distanceAux < bestIncremento)
        {

            double custo = testaRota(evRouteAux, evRouteAux.routeSize, instance, false, tempoSaidaSat);

            if(custo > 0.0)
            {

                bestIncremento = distanceAux;
                candidatoEv = CandidatoEV(pos, node, distanceAux, demand, 0.0, evRoute.idRota, evRoute.satelite, -1, -1, {});
                viavel = true;

            }
            else if(viabilizaRotaEv(evRouteAux, instance, true, insercaoEstacao))
            {

                double insertionCost = insercaoEstacao.distanciaRota - distanciaRota;
                if(insertionCost < bestIncremento)
                {
                    bestIncremento = insertionCost;
                    candidatoEv = CandidatoEV(pos, node, insertionCost, demand, 0.0, evRoute.idRota, evRoute.satelite, -1, -1, insercaoEstacao);
                    viavel = true;
                }
            }
        }

        evRouteAux[pos+1] = evRouteAux[pos+2];

    }
    //PRINT_DEBUG("", "<<viavel: "<<viavel);



    return viavel;

}

std::vector<double> GreedyAlgNS::calculaTempoSaidaInicialSat(const Instance &instance)
{
   vector<double> vetTempoSaida;
   vetTempoSaida.reserve(instance.getNSats()+1);
   vetTempoSaida.push_back(0.0);

   double dist;

    for(int i=instance.getFirstSatIndex(); i <= instance.getEndSatIndex(); ++i)
    {

        dist = instance.getDistance(instance.getDepotIndex(), i);
        vetTempoSaida.push_back(dist);
    }

    return std::move(vetTempoSaida);

}

bool GreedyAlgNS::insert(EvRoute &evRoute, CandidatoEV &insertion, const Instance &instance, const double tempoSaidaSat, Solution &sol)
{

    const int pos = insertion.pos;
    const int node = insertion.clientId;

    if(pos < 0 )
        return false;

    if(node < 0)
        return false;


    /*

        checar se consegue com a capacidade atual
        checar se consegue com a bateria atual (a bateria do veiculo antes da proxima recarga.
        inserir
        atualizar capacidade
        atualizar bateria
        atualizar custo

     */

    int k = pos;
    shiftVectorDir(evRoute.route, pos+1, 1, evRoute.routeSize);
    evRoute.route[pos+1].cliente = node;
    evRoute.routeSize += 1;

    if(insertion.insercaoEstacao.pos >= 0)
    {
        shiftVectorDir(evRoute.route, insertion.insercaoEstacao.pos+1, 1, evRoute.routeSize);
        evRoute.route[insertion.insercaoEstacao.pos+1].cliente = insertion.insercaoEstacao.estacao;
        evRoute.routeSize += 1;
    }


    if(evRoute.routeSize > 2)
    {
        sol.distancia += -evRoute.distancia;
        sol.satelites[evRoute.satelite].distancia += -evRoute.distancia;
    }


    evRoute.distancia = testaRota(evRoute, evRoute.routeSize, instance, true, tempoSaidaSat);
    evRoute.demanda += insertion.demand;
    sol.distancia += evRoute.distancia;


    sol.satelites[evRoute.satelite].distancia += evRoute.distancia;

    if(evRoute.distancia <= 0.0)
    {
        string rotaStr;
        evRoute.print(rotaStr, instance, false);

        PRINT_DEBUG("", "ERRO NA FUNCAO GreedyAlgNS::insert, BATERIA DA ROTA EH INVIAVEL: "<<rotaStr<<"\n\n");
        cout<<"FUNCAO TESTA ROTA RETORNOU DISTANCIA NEGATIVA, ROTA: ";
        evRoute.print(instance, false);

        evRoute.print(instance, true);
        cout<<" EH INVALIDA\nFILE: "<<__FILE__<<"\nLINHA: "<<__LINE__<<"\n\n";
        throw "ERRO";
    }


    return true;
}
