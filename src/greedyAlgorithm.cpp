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
    const double tempoSaidaInicialSat = GreedyAlgNS::calculaTempoSaidaInicialSat(instance);


    EvRoute evRouteAux(-1, -1, instance.getEvRouteSizeMax(), instance);

    while(!visitAllClientes(visitedClients, instance))
    {

        std::list<Insertion> listaCandidatos;

        //for(auto& client : clientsByTime)
        for(int clientId = FistIdClient; clientId <= LastIdClient; ++clientId)
        {


            if(!visitedClients[clientId])
            {
                //PRINT_DEBUG("", "clienteId: "<<clientId);

                for(int satId = instance.getFirstSatIndex(); satId <= instance.getEndSatIndex(); satId++)
                {

                    //PRINT_DEBUG("", "SAT ID: "<<satId);

                    Satelite *sat = sol.getSatelite(satId);

                    bool routeEmpty = false;
//                    for(int routeId = 0; routeId < sol.getSatelite(satId)->getNRoutes(); routeId++)
                    for(int routeId = 0; routeId < sat->getNRoutes(); routeId++)
                    {
                        //PRINT_DEBUG("", "ROUTE ID: "<<routeId);

                        EvRoute &route = sat->getRoute(routeId);
                        Insertion insertion(routeId);
                        insertion.satId = satId;
                        bool resultado = canInsert(route, clientId, instance, insertion, satId, tempoSaidaInicialSat, evRouteAux);

                        if(resultado)
                            listaCandidatos.push_back(insertion);

                        //cout<<"\n******************************************************************************************************";
                        //cout<<"\n******************************************************************************************************\n\n";
                    }
                }
            }
        }

        if(listaCandidatos.empty())
        {
            PRINT_DEBUG("\t","LISTA DE CANDIDATOS VAZIA");
            //cout<<"LISTA DE CANDIDADOS VAZIA\n\n";
            sol.viavel = false;
            return false;
        }


        int randIndex = rand_u32()%(int(alpha * listaCandidatos.size() + 1));

        auto topItem = std::next(listaCandidatos.begin(), randIndex);

        visitedClients[topItem->clientId] = 1;

        Satelite *satelite = sol.getSatelite(topItem->satId);
        satelite->demanda += topItem->demand;
        EvRoute &evRoute = satelite->getRoute(topItem->routeId);
        insert(evRoute, *topItem, instance, tempoSaidaInicialSat, sol);
    }

    PRINT_DEBUG("", "vetTempoSaidaEvRoute size: "<<sol.satelites[0].vetTempoSaidaEvRoute.size());

    return true;
}



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

    PRINT_DEBUG("\t", "vetTempoSaidaEvRoute size: "<<sol.satelites[1].vetTempoSaidaEvRoute.size());

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
        std::list<Candidato> listaCandidatos;

        // Percorre os satellites
        for(int i=1; i < NumSatMaisDep; ++i)
        {
            Satelite &satelite = sol.satelites[i];

            PRINT_DEBUG("\t", "i: "<<i<<"\t\tsat id: "<<satelite.sateliteId);
            PRINT_DEBUG("\t", "vetTempoSaidaEvRoute size: "<<satelite.vetTempoSaidaEvRoute.size());

            // Verifica se a demanda não atendida eh positiva
            if(demandaNaoAtendidaSat[i] > 0.0)
            {
                PRINT_DEBUG("\t", "DEMANDA NAO ATENDIDA SAT "<<i<<": "<<demandaNaoAtendidaSat[i]);
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

                        PRINT_DEBUG("\t", "ROUTE ID: "<<rotaId<<", demanda: "<<demandaAtendida);

                        Candidato candidato(rotaId, i, demandaAtendida, DOUBLE_MAX);

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
                                PRINT_DEBUG("\t\t", "INCREMENTO: "<<incrementoDist);
                                // Calcula o tempo de chegada e verifica a janela de tempo
                                const double tempoChegCand = clienteP.tempoChegada + Inst.getDistance(clienteP.satellite, i);

                                bool satViavel = true;

                                if(verificaViabilidadeSatelite(tempoChegCand, satelite, Inst, false))
                                {
                                    double tempoChegTemp = tempoChegCand + Inst.getDistance(i, clientePP.satellite);

                                    // Verificar viabilidade dos outros satelites
                                    for(int t=p+1; (t+1) < route.routeSize; ++t)
                                    {
                                        tempoChegTemp += Inst.getDistance(route.rota[t].satellite, route.rota[t+1].satellite);
                                        Satelite &sateliteTemp = sol.satelites[route.rota[t+1].satellite];

                                        if(!verificaViabilidadeSatelite(tempoChegTemp, sateliteTemp, Inst, false))
                                        {
                                            PRINT_DEBUG("\t", "verificaViabilidadeSat = false");
                                            satViavel = false;
                                            break;
                                        }

                                    }

                                }
                                else
                                {
                                    PRINT_DEBUG("\n\t", "VIABILIDADE SAT: FALSE");
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
            PRINT_DEBUG("", "");

            listaCandidatos.sort();

            // Escolhe o candidado da lista restrita
            int tam = int(beta * listaCandidatos.size()) + 1;
            int escolhido = rand_u32() % tam;
            auto it = listaCandidatos.begin();

            std::advance(it, escolhido);
            Candidato &candidato = *it;

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

                    cout<<"ERRO! FUNCAO 'verificaViabilidadeSatelite' DEVERIA RETORNAR TRUE. TEMPO SAIDA SATELITE: "<<tempoSaida<<"\n\tSATELITE:\n\n"<<satStr<<"\n\n";
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
            PRINT_DEBUG("", "PRIMEIRO NIVEL FALHOU");
            sol.viavel = false;
            break;
        }
    }

}


// Com o tempo de chegada ao satelite, eh verificado se as rotas dos EV's podem sair apos o tempo de chegada do veic a combustao

bool GreedyAlgNS::verificaViabilidadeSatelite(const double tempoChegada, Satelite &satelite, const Instance &instance, const bool modificaSatelite)
{

    bool viavel = true;

    PRINT_DEBUG("\t\t\t", "TEMPO CHEGADA VEIC COMB: "<<tempoChegada);


    // Verifica se os tempos de saida das rotas dos EV's eh maior que o tempo de chegada do veic a combustao
    for(int evId = 0; evId < instance.getN_Evs(); ++evId)
    {
        TempoSaidaEvRoute &tempoSaidaEv = satelite.vetTempoSaidaEvRoute[evId];
        const double tempoEv = tempoSaidaEv.evRoute->route[0].tempoSaida;

        PRINT_DEBUG("\t\t\t", "TEMPO SAIDA EV: "<<tempoEv<<"; TEMPO CHEGADA VEIC COMB: "<<tempoChegada);

        if(tempoSaidaEv.evRoute->routeSize > 2)
        {

/*            if(tempoEv >= tempoChegada)
            {
                viavel = true;
                break;
            } else*/
            if(tempoEv < tempoChegada)
            {
                // Verifica se eh possivel realizar um shift na rota

                const int indice = tempoSaidaEv.evRoute->route[0].posMenorFolga;
                //cout << "indice: " << indice << "\n";
                const int cliente = tempoSaidaEv.evRoute->route[indice].cliente;
                const double twFim = instance.vectCliente[cliente].fimJanelaTempo;

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
        return viavel;
    else
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
                            throw "ERRO";
                        }

                    }

                }

            }
        }


        PRINT_DEBUG("", "ERRO SATELITE: "<<satelite.sateliteId<<", DEVERIA TER PELO MENOS UMA ROTA(ORDENADAS DE FORMA CRESENTE COM TEMPO DE SAIDA) COM TEMPO DE SAIDA MAIOR OU IGUAL A "<<tempoChegada<<"\n\n");
        throw "ERRO";

    }

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

        PRINT_DEBUG("\n\n", "Segundo nivel");
        //sol.print(Inst);


    }

}



bool GreedyAlgNS::canInsert(EvRoute &evRoute, int node, const Instance &instance, Insertion &insertion, const int satelite, const double tempoSaidaSat, EvRoute &evRouteAux)
{
    double demand = instance.getDemand(node);
    double bestInsertionCost = DOUBLE_MAX;
    bool viavel = false;


//    EvRoute evRouteAux(satelite, evRoute.idRota, evRoute.routeSizeMax, instance);

    evRouteAux.satelite = satelite;
    evRouteAux.idRota = evRoute.idRota;
    evRouteAux.routeSizeMax = evRoute.routeSizeMax;


    InsercaoEstacao insercaoEstacao;


    if((evRoute.getDemand() + demand) > instance.getEvCap(evRoute.idRota))
    {
        //PRINT_DEBUG("\t\t", "DEMANDA INVIAVEL\n");
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

        //int prevNode = evRoute.route.at(pos);
        //int nextNode = evRoute.route.at(pos + 1);
        evRouteAux.route[pos+1].cliente = node;

/*        PRINT_DEBUG("", "");
        cout<<"\n\nNOVA ROTA: ";
        evRouteAux.print(instance, true);
        cout<<"\n";*/


        double distanceAux = instance.getDistance(evRouteAux[pos].cliente, node) + instance.getDistance(node, evRouteAux[pos+2].cliente) -
                             instance.getDistance(evRouteAux[pos].cliente, evRouteAux[pos+2].cliente);

        double tempoChegada = evRouteAux[pos].tempoSaida + instance.getDistance(evRouteAux[pos].cliente, node);

        //cout<<"tempo de chegada no: "<<node<<".: "<<tempoChegada<<"  fim janela de tempo: "<<instNode.fimJanelaTempo<<"\n";

        // Verifica a janela de tempo
        if((tempoChegada <= instNode.fimJanelaTempo) || (abs(tempoChegada-instNode.fimJanelaTempo) <= TOLERANCIA_JANELA_TEMPO))
        {
            if(tempoChegada < instNode.inicioJanelaTempo)
                tempoChegada = instNode.inicioJanelaTempo;

            tempoChegada += instNode.tempoServico;

            // Calcula tempo de chegada em CLIENTE[pos+2]
            tempoChegada += instance.getDistance(node, evRouteAux[pos + 2].cliente);


            const ClienteInst &instPosProx = instance.vectCliente[evRouteAux[pos+2].cliente];
            double tempo = tempoChegada;

            //cout<<"tempo de chegada no pos+1: "<<evRouteAux[pos+2].cliente<<".: "<<tempoChegada<<"  fim janela de tempo: "<<instPosProx.fimJanelaTempo<<"\n";

            // Verifica janela de tempo cliente(pos+2)
            if((tempoChegada <= instPosProx.fimJanelaTempo) || (abs(tempoChegada-instPosProx.fimJanelaTempo) <= TOLERANCIA_JANELA_TEMPO))
            {
                if(tempoChegada < instPosProx.inicioJanelaTempo)
                    tempoChegada = instPosProx.inicioJanelaTempo;

                bool janelaTempoAtend = true;

                if(!instance.isSatelite(evRouteAux[pos+2].cliente))
                {
                    // Verificar se eh possivel atender o cliente que possui a menor diferenca entre o final da janela de tempo e a sua janela de tempo

                    const int indice = evRouteAux[pos+2].posMenorFolga;

                    const int posProxEstacao = evRouteAux[pos+2].posProxEstacao;

                    if(indice >= 0)
                    {
                        const double incremento = tempoChegada - evRouteAux[pos+2].tempoCheg;
                        if(incremento < 0.0)
                        {
                            throw string("INCREMENTO NEGATIVO\n FILE: "+ string(__FILE__)+"  LINHA: "+to_string(__LINE__)+"\n");
                        }

                        double tempoChegadaMenor = evRoute[indice].tempoCheg + incremento;
                        const ClienteInst &clienteInstMenor = instance.vectCliente[evRoute[indice].cliente];

                        janelaTempoAtend = ((tempoChegadaMenor <= clienteInstMenor.fimJanelaTempo) ||
                                                (abs(tempoChegadaMenor - clienteInstMenor.fimJanelaTempo) <= TOLERANCIA_JANELA_TEMPO));


                    }
                }

                if(janelaTempoAtend)
                {

                    double batteryConsumptionAux = 0.0;

                    // Ate nextNode(pos+1)
                    double remaingBattery = evRouteAux[pos].bateriaRestante - instance.getEvTaxaConsumo(evRoute.idRota)*(instance.getDistance(evRouteAux[pos].cliente, node) +
                            instance.getDistance(node, evRouteAux[pos+2].cliente));

                    bool batteryOk = true;


                    if(remaingBattery < -TOLERANCIA_BATERIA)
                    {
                        batteryOk = false;
                    }

                    if(batteryOk)
                    {


                        tempo = tempoChegada;

                        if(instance.isRechargingStation(evRouteAux[pos+2].cliente))
                        {
                            // Acresenta tempo de recarga
                            tempo += instance.tempoRecarga(evRouteAux[pos+2].cliente, remaingBattery);
                            remaingBattery = instance.getEvBattery(evRoute.idRota);

                        }



                        for(int i = pos+2; (i+1) < (evRoute.routeSize+1); ++i)
                        {
                            const int clienteI  = evRouteAux[i].cliente;
                            const int clienteII = evRouteAux[i + 1].cliente;

                            remaingBattery -= instance.getDistance(clienteI, clienteII);
                            tempo += instance.getDistance(clienteI, clienteII);

                            if(remaingBattery < -TOLERANCIA_BATERIA)
                            {
                                batteryOk = false;
                                //PRINT_DEBUG("\t\t", "pos: " << pos << ";  remaingBattery: " << remaingBattery);
                                break;
                            }

                            if(!TESTE_JANELA_TEMPO(tempo, clienteII, instance))
                            {
                                janelaTempoAtend = false;
                                break;
                            }

                            if(tempo < instance.getInicioJanelaTempoCliente(clienteII))
                                tempo = instance.getInicioJanelaTempoCliente(clienteII);



                            if(instance.isRechargingStation(clienteII))
                            {
                                tempo += instance.tempoRecarga(evRoute.idRota, remaingBattery);
                                remaingBattery = instance.getEvBattery(evRoute.idRota);
                            }
                            else
                            {
                                tempo += instance.vectCliente[clienteII].tempoServico;
                            }


                            //cout<<"\tpos: "<<pos<<"; "<<evRoute[pos]<<": "<<remaingBattery<<";\n";

                            //cout<<evRoute[i+1]<<": "<<remaingBattery<<"\n";
                            //cout<<"pos: "<<i+1<<"; "<<evRoute[i+1]<<": "<<remaingBattery<<";\n";
                        }
                    }

                    if(batteryOk && janelaTempoAtend)
                    {
                        //cout << "BATERIA OK\n";

                        double insertionCost = distanceAux;
                        if(insertionCost < bestInsertionCost)
                        {
                            bestInsertionCost = insertionCost;
                            insertion = Insertion(pos, node, insertionCost, demand, batteryConsumptionAux, insertion.routeId, insertion.satId, -1, -1, {});
                            viavel = true;
                           // cout<<"\t\tATUALIZOU BEST_INSERTION\n";

                        }
                    } else
                    {

                        //cout<<"VIABILIZA ROTA: "<<evRouteAux.getRota(instance, true) << "\n";

                        if(viabilizaRotaEv(evRouteAux, instance, true, insercaoEstacao))
                        {
                            double insertionCost = insercaoEstacao.distanciaRota - distanciaRota;
                            if(insertionCost < bestInsertionCost)
                            {
                                bestInsertionCost = insertionCost;
                                insertion = Insertion(pos, node, insertionCost, demand, batteryConsumptionAux, insertion.routeId, insertion.satId, -1, -1, insercaoEstacao);
                                viavel = true;
                                //cout<<"\t\tVIABILIZA ROTA ATUALIZOU BEST_INSERTION\n";
                            }
                        }

                    }
                }

            }

        }

        evRouteAux[pos+1] = evRouteAux[pos+2];

        //PRINT_DEBUG("", "");

    }

    //PRINT_DEBUG("", "");

/*    cout<<"\tVIAVEL: "<<viavel<<"\n";
    if(viavel)
        cout<<"\tpos: "<<insertion.pos<<"\n\tpos bateria: "<<insertion.insercaoEstacao.pos<<"\n";*/

    //cout<<"\n\n";
    return viavel;

}

double GreedyAlgNS::calculaTempoSaidaInicialSat(const Instance &instance)
{
    double dist = DOUBLE_MAX;

    for(int i=instance.getFirstSatIndex(); i <= instance.getEndSatIndex(); ++i)
    {
        if(instance.getDistance(instance.getDepotIndex(), i) < dist)
            dist = instance.getDistance(instance.getDepotIndex(), i);
    }

    return dist;

}

bool GreedyAlgNS::insert(EvRoute &evRoute, Insertion &insertion, const Instance &instance, const double tempoSaidaSat, Solution &sol)
{
    //cout<<"**********INSERT***********\n";

    const int pos = insertion.pos;
    const int node = insertion.clientId;

    //PRINT_DEBUG("", "INSERT NO: "<<node<<"; POS: "<<pos);

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

    // atualiza estruturas auxiliares:


    // ignoring battery distance for now;
    //this->rechargingStations;
    int k = pos;


    //cout<<"ROTA ANTIGA: "<<printVector(evRoute.route, evRoute.routeSize)<<"\n";



    shiftVectorDir(evRoute.route, pos+1, 1, evRoute.routeSize);
    evRoute.route[pos+1].cliente = node;
    evRoute.routeSize += 1;

    //cout<<"ROTA CLIENTE: "<<printVector(evRoute.route, evRoute.routeSize)<<"\n";

    if(insertion.insercaoEstacao.pos >= 0)
    {
        shiftVectorDir(evRoute.route, insertion.insercaoEstacao.pos+1, 1, evRoute.routeSize);
        evRoute.route[insertion.insercaoEstacao.pos+1].cliente = insertion.insercaoEstacao.estacao;
        evRoute.routeSize += 1;
    }

    //cout<<"INSERTE\n";

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
        cout<<" EH INVALIDA\nFILE: "<<__FILE__<<"\nLINHA: "<<__LINE__<<"\n\n";
        throw "ERRO";
    }


    //evRoute.print(Inst);
//    evRoute.vetRemainingBattery

    return true;
}

bool GreedyAlgNS::insereEstacao(int rotaId, int satId) { return false; };
