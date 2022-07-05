//
// Created by igor on 27/06/22.
//

/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    27/06/22
 *  Arquivo: PreProcessamento.cpp
 * ****************************************
 * ****************************************/

#include "PreProcessamento.h"
#include "Heap.h"
#include "ViabilizadorRotaEv.h"

using namespace N_PreProcessamento;
using namespace N_heap;
using namespace NameViabRotaEv;

ShortestPathSatCli::ShortestPathSatCli(Instance &instancia)
{
    fistCliente = instancia.getFirstClientIndex();
    numClientes = instancia.getNClients();
    numEstacoes = instancia.getN_RechargingS();

    vetShortestPath.reserve(numClientes);

    for(int i=fistCliente; i < (fistCliente+numClientes); ++i)
    {
        vetShortestPath.emplace_back(-1, i);
    }
}

ShortestPathNo &ShortestPathSatCli::getShortestPath(int cliente)
{

    cliente -= fistCliente;
    return vetShortestPath[cliente];

}


void N_PreProcessamento::dijkstraSatCli(Instance &instancia, ShortestPathSatCli &shortestPathSatCli)
{

    std::vector<PreDecessorNo> preDecessorIda(instancia.numNos, PreDecessorNo());
    std::vector<PreDecessorNo> preDecessorVolta(instancia.numNos, PreDecessorNo());

    std::vector<bool> excluidos(instancia.numNos, false);

    // Exclui deposito e satelites
    excluidos[instancia.getDepotIndex()] = true;
    std::fill((excluidos.begin()+instancia.getFirstSatIndex()), (excluidos.begin()+instancia.getEndSatIndex()+1), true);

    // Para cada estacao e cliente (possicao) guarda o indice da min heap
    std::vector<int> vetIndiceMinHeap(1 + instancia.getNSats() + instancia.getN_RechargingS()+instancia.numClients, -1);

    std::vector<int8_t> vetFechados(instancia.numNos);

    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {
        excluidos[sat] = false;

        if(sat != instancia.getFirstSatIndex())
        {
            std::fill(preDecessorIda.begin(), preDecessorIda.end(), PreDecessorNo());
            std::fill(preDecessorVolta.begin(), preDecessorVolta.end(), PreDecessorNo());

            std::fill(vetIndiceMinHeap.begin(), vetIndiceMinHeap.end(), -1);
            //std::fill(excluidos.begin(), excluidos.end(), false);
        }

        // Inicialmente possui estacao de recarga e clientes
        std::vector<DijkstraNo> minHeap(instancia.getN_RechargingS()+instancia.numClients);

        //NS_Auxiliary::printVectorCout(minHeap, int64_t(posMinHeap));

        // Encontra o menor caminho viavel de sat ate todos os clientes passando pelas estacoes de recarga
        dijkstra(instancia, sat, instancia.getFirstEvIndex(), 0, minHeap, preDecessorIda, excluidos, false, vetIndiceMinHeap, vetFechados);

        std::fill(excluidos.begin()+instancia.getFirstClientIndex(), excluidos.begin()+instancia.getEndClientIndex()+1, true);

        // Percorre os clientes para encontrar a menor rota viavel de cliente ate satelite, utilizando a bateria
        for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        {
            if(preDecessorIda[i].preDecessor != -1)
            {
                excluidos[i] = false;
                preDecessorVolta[sat].preDecessor = -1;

                cout<<"Dijkstra volta cliente: "<<i<<"\n\n";
                dijkstra(instancia, i, instancia.getFirstEvIndex(), preDecessorIda[i].bateria, minHeap, preDecessorVolta, excluidos, false, vetIndiceMinHeap, vetFechados);

                if(preDecessorVolta[sat].preDecessor != -1)
                {
                    ShortestPathNo &caminho = shortestPathSatCli.getShortestPath(i);
                    double dist = preDecessorIda[i].dist + preDecessorVolta[sat].dist;

                    if(dist < caminho.distIdaVolta)
                    {
                        caminho.satId = sat;

                        auto recuperaRota = [&](bool ida)
                        {
                            std::vector<int> rota;
                            rota.reserve(10);

                            int indice;

                            if(ida)
                                indice = i;
                            else
                                indice = sat;

                            vector<PreDecessorNo> *vetPreDecessor;
                            if(ida)
                                vetPreDecessor = &preDecessorIda;
                            else
                                vetPreDecessor = &preDecessorVolta;

                            while((*vetPreDecessor)[indice].preDecessor != -1)
                            {
                                rota.push_back(indice);
                                indice = (*vetPreDecessor)[indice].preDecessor;

                                if(indice == -1)
                                    break;
                            }

                            if(indice != -1)
                                rota.push_back(indice);

                            if(ida)
                            {
                                caminho.caminhoIda = std::move(rota);
                                caminho.distIda = preDecessorIda[i].dist;
                            }
                            else
                            {
                                caminho.caminhoVolta = std::move(rota);
                                caminho.distVolta = preDecessorVolta[sat].dist;
                                caminho.distIdaVolta = caminho.distIda+caminho.distVolta;
                            }
                        };

                        recuperaRota(true);
                        recuperaRota(false);


                    }


                }

                excluidos[i] = true;
            }
        }

        excluidos[sat] = true;

    }

    cout<<"\n\nROTAS:\n\n";

    shortestPathSatCli.vetEvRoute = vector<EvRoute>(instancia.getNClients(), EvRoute(instancia.getFirstSatIndex(), instancia.getFirstEvIndex(), instancia.getEvRouteSizeMax(), instancia));

    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        ShortestPathNo &caminho = shortestPathSatCli.getShortestPath(i);
        EvRoute &evRoute = shortestPathSatCli.getEvRoute(i);

        cout<<i<<" : "<<caminho.distIdaVolta<<"\n\n";

        if(caminho.distIda < DOUBLE_INF)
        {
            std::reverse(caminho.caminhoIda.begin(), caminho.caminhoIda.end());
            string rotaStr = NS_Auxiliary::printVectorStr(caminho.caminhoIda, caminho.caminhoIda.size());
            //string rotaVolta = NS_Auxiliary::printVectorStr(caminho.caminhoVolta, caminho.caminhoVolta.size());

            cout<<"\tRota Ida cliente "<<i<<"Dist: "<<caminho.distIda<<":\n\t"<<rotaStr<<"\n\n";

        }


        if(caminho.distVolta < DOUBLE_INF)
        {
            std::reverse(caminho.caminhoVolta.begin(), caminho.caminhoVolta.end());
//            string rotaStr = NS_Auxiliary::printVectorStr(caminho.caminhoIda, caminho.caminhoIda.size());
            string rotaVolta = NS_Auxiliary::printVectorStr(caminho.caminhoVolta, caminho.caminhoVolta.size());

            cout<<"\tRota Volta cliente "<<i<<"Dist: "<<caminho.distVolta<<"\n\t"<<rotaVolta<<"\n\n";
        }

        if(caminho.distIdaVolta < DOUBLE_INF)
        {
            evRoute.satelite = caminho.satId;
            evRoute.demanda  = instancia.vectCliente[i].demanda;
            evRoute.distancia = caminho.distIdaVolta;

            for(int t=0; t < caminho.caminhoIda.size(); ++t)
            {
                evRoute.route[t].cliente = caminho.caminhoIda[t];
            }

            evRoute.routeSize = caminho.caminhoIda.size();

            for(int t=1; t < caminho.caminhoVolta.size(); ++t)
            {
                evRoute.route[(caminho.caminhoIda.size()+t-1)].cliente = caminho.caminhoVolta[t];
                evRoute.routeSize += 1;
            }

            cout<<"\tRota Completa: ";
            int sat = evRoute[0].cliente;

            bool r = testaRota(evRoute, evRoute.routeSize, instancia, true, instancia.vetTempoSaida[sat], 0, nullptr);
            cout<<"Resultado: "<<r<<"\n\t";
            NS_Auxiliary::printVectorCout(evRoute.route, evRoute.routeSize);



        }

        cout<<"\n";
    }

}
/**
 *
 * @param instancias
 * @param clienteSorce      pode ser sat, estacao ou cliente
 * @param bateria           Quantidade de bateria (eh desconsiderado se clienteSorce eh estacao)
 * @param minHeap
 * @param preDecessor       Guarda o caminho. Possui numNos
 * @param excluidos         vetor com tamanho 1(dep) + num sat + num est + num clientes :: numNos
 * @param clienteCliente    Indica se eh considerado (i,j), sendo i e j clientes
 * @param vetIndiceMinHeap  Guarda o indice de cada cliente na minHeap. Possui numNos
 * @param vetFechados
 */

void N_PreProcessamento::dijkstra(Instance &instancia, const int clienteSorce, const int veicId, double bateria, std::vector<DijkstraNo> &minHeap, std::vector<PreDecessorNo> &preDecessor,
                                  const std::vector<bool> &excluidos, const bool clienteCliente, std::vector<int> &vetIndiceMinHeap, std::vector<int8_t> &vetFechados)
{

    std::fill(vetFechados.begin(), vetFechados.end(), -1);

    cout<<"Cliente sorce: "<<clienteSorce<<"\n";

    const bool sorceSat = instancia.isSatelite(clienteSorce);

    if(instancia.isRechargingStation(clienteSorce) || instancia.isSatelite(clienteSorce))
    {
        bateria = instancia.vectVeiculo[veicId].capacidadeBateria;
        cout<<"veicId: "<<veicId<<"\n";
    }

    cout<<"BATERIA: "<<bateria<<"\n";

    // Verifica se clienteSorce esta em minHeap

    int id = vetIndiceMinHeap[clienteSorce];
    if(id == -1)
    {
        minHeap.emplace_back(clienteSorce);
        minHeap[0] = DijkstraNo(clienteSorce, 0.0, bateria);
        id = 0;
    }

    NS_Auxiliary::printVectorCout(minHeap, 1);

/*    if(id != 0)
    {
        // move para cima clienteSorce
        id = shifitUpMinHeap(minHeap, id, vetIndiceMinHeap);

        NS_Auxiliary::printVectorCout(minHeap, minHeap.size());

        // clienteSorce deve ser o elemento de maior prioridade
        if(id != 0)
        {
            cout<<"ERRO, existe mais do que um elemento com prioridade 0 em  minHeap!!\n";
            throw "ERRO";
        }

    }*/

    int tamMinHeap = 1;

/*    // Exclui os clientes da minHeap que estao no vetor excluidos
    for(int i=0; i < instancia.numNos; ++i)
    {
        if(excluidos[i])
        {
            if(vetIndiceMinHeap[i] != -1)
            {
                cout<<"EXCLUIDO: "<<i<<"\n";
                id = vetIndiceMinHeap[i];

                minHeap[id].dist = -DOUBLE_INF;
                shifitUpMinHeap(minHeap, id, vetIndiceMinHeap);
                removeTopo(minHeap, &tamMinHeap, vetIndiceMinHeap);

                //tamMinHeap -= 1;
            }


        }
    }*/

    // Extrai o topo da heap
    DijkstraNo dijkNoTopo = minHeap[0];

    while(tamMinHeap > 0)
    {

        cout<<"\n\n**************************************************************\n\n";

        dijkNoTopo = minHeap[0];

        cout<<"No: "<<minHeap[0].clienteId<<" FECHADO  "<<dijkNoTopo.clienteId<<"\n";
        cout<<"BAT: "<<minHeap[0].bateria<<"; DIST: "<<minHeap[0].dist<<"\n\n";

        removeTopo(minHeap, &tamMinHeap, vetIndiceMinHeap);

        cout<<"APOS REMOVER TOPO:\n";
        NS_Auxiliary::printVectorCout(minHeap, tamMinHeap);
        NS_Auxiliary::printVectorCout(vetIndiceMinHeap, vetIndiceMinHeap.size());


        if(dijkNoTopo.dist >= DOUBLE_INF)
        {   cout<<"return\n";
            return;
        }

        // Fecha o no
        vetIndiceMinHeap[dijkNoTopo.clienteId] = -1;
        vetFechados[dijkNoTopo.clienteId] = 1;

        // Verrifica se o no eh cliente
        if(instancia.isClient(dijkNoTopo.clienteId) && !clienteCliente && dijkNoTopo.clienteId != clienteSorce)
        {   cout<<"no "<<dijkNoTopo.clienteId<<" eh cliente\n";
            continue;
        }

        const bool noTopoCliente = instancia.isClient(dijkNoTopo.clienteId);

        cout<<"\n\n";

        // Atualiza os nos que sao alcancaveis por dijkNoTopo
        for(int i=0; i < instancia.numNos; ++i)
        {
            cout<<"i: "<<i<<": "<<!excluidos[i]<<" : "<<(vetFechados[i]!=1)<<"\n";
            if((!excluidos[i]) && vetFechados[i] != 1)
            {

                /* casos:
                 *
                 *      1° !noTopoCliente -> ANY
                 *      2° noTopoCliente  -> clienteCliente
                 */

                cout<<"1° if\n";

                if(!noTopoCliente || (noTopoCliente && clienteCliente && instancia.isClient(i)) || (noTopoCliente && !instancia.isClient(i)))
                {

                    id = vetIndiceMinHeap[i];

                    // Verifica viabilidade de bateria e distancia
                    double dist = instancia.getDistance(dijkNoTopo.clienteId, i) + dijkNoTopo.dist;
                    double bat  = dijkNoTopo.bateria - instancia.getDistance(dijkNoTopo.clienteId, i);

                    cout<<"\tNO: "<<i<<" DIST: "<<dist<<"; BAT: "<<bat<<"\n";

                    bool cond = (id == -1);

                    if(!cond)
                        cond = (dist < minHeap[id].dist);

                    if(bat >= -TOLERANCIA_BATERIA && cond)
                    {
                        cout<<"\tATUALIZACAO NO: "<<i<<"\n\n";


                        if(instancia.isRechargingStation(i))
                            bat = instancia.vectVeiculo[veicId].capacidadeBateria;

                        // Atualiza minHeap

                        if(id == -1)
                        {
                            minHeap[tamMinHeap] = DijkstraNo(i, dist, bat);
                            id = tamMinHeap;
                            tamMinHeap += 1;
                            vetIndiceMinHeap[i] = id;


                        }
                        else
                        {
                            minHeap[id].dist = dist;
                            minHeap[id].bateria = bat;
                        }

                        // Atualiza pre decessor
                        preDecessor[i].preDecessor = dijkNoTopo.clienteId;
                        preDecessor[i].bateria     = bat;
                        preDecessor[i].dist        = dist;

                        shifitUpMinHeap(minHeap, id, vetIndiceMinHeap);

                    }
                }
            }
        }

        cout<<"\n\nFIM ITERACAO\n\n";
        NS_Auxiliary::printVectorCout(minHeap, tamMinHeap);
        NS_Auxiliary::printVectorCout(vetIndiceMinHeap, vetIndiceMinHeap.size());

        dijkNoTopo = minHeap[0];

        if(!(dijkNoTopo.dist < DOUBLE_INF))
            break;


    }
}

EvRoute& ShortestPathSatCli::getEvRoute(int cliente)
{
    return vetEvRoute[cliente-fistCliente];
}

int N_PreProcessamento::getPaiMinHeap(int pos)
{

    if((pos%2) == 0)
        return (pos-2)/2;
    else
        return (pos-1)/2;
}

int N_PreProcessamento::shifitUpMinHeap(std::vector<DijkstraNo> &minHeap, int pos, std::vector<int> &vetIndice)
{

    int pai = getPaiMinHeap(pos);
    while(minHeap[pos] < minHeap[pai])
    {
        // swap pos com pai
        DijkstraNo temp = minHeap[pos];
        minHeap[pos] = minHeap[pai];
        minHeap[pai] = temp;
        vetIndice[minHeap[pos].clienteId] = pos;

        pos = pai;
        if(pos == 0)
            break;

        pai = getPaiMinHeap(pos);

    }

    vetIndice[minHeap[pos].clienteId] = pos;
    return pos;

}


int N_PreProcessamento::shifitDownMinHeap(std::vector<DijkstraNo> &minHeap, int tam, int pos, std::vector<int> &vetIndice)
{
    int filhoDir = 2*pos+1;
    int filhoEsq = 2*pos+2;

    while(filhoDir < tam)
    {
        if(minHeap[pos] > minHeap[filhoDir])
        {
            int min = filhoDir;

            if(filhoEsq < tam)
            {
                if(minHeap[pos] > minHeap[filhoEsq] && minHeap[filhoEsq] < minHeap[filhoDir])
                {
                    min = filhoEsq;
                }
            }

            DijkstraNo temp = minHeap[pos];
            minHeap[pos] = minHeap[min];
            minHeap[min] = temp;

            if(vetIndice[minHeap[pos].clienteId] != -1)
                vetIndice[minHeap[pos].clienteId] = pos;

            pos = filhoDir;
        }
        else if(filhoEsq < tam)
        {
            if(minHeap[pos] > minHeap[filhoEsq])
            {

                DijkstraNo temp = minHeap[pos];
                minHeap[pos] = minHeap[filhoEsq];
                minHeap[filhoEsq] = temp;

                if(vetIndice[minHeap[pos].clienteId] != -1)
                    vetIndice[minHeap[pos].clienteId] = pos;

                pos = filhoEsq;
            }
            else
                break;
        }
        else
            break;


        filhoDir = 2*pos+1;
        filhoEsq = 2*pos+2;

        if(!(filhoDir < tam))
            break;
    }

    if((!(minHeap[filhoDir] < minHeap[pos])) && (filhoEsq < tam))
    {
        if (minHeap[pos] > minHeap[filhoEsq])
        {

            DijkstraNo temp = minHeap[pos];
            minHeap[pos] = minHeap[filhoEsq];
            minHeap[filhoEsq] = temp;

            pos = filhoEsq;
        }
    }

    if(vetIndice[minHeap[pos].clienteId] != -1)
        vetIndice[minHeap[pos].clienteId] = pos;

    return pos;

}

void N_PreProcessamento::removeTopo(std::vector<DijkstraNo> &minHeap, int *tam, std::vector<int> &vetIndice)
{
    if(*tam == 0)
        return;
    else if(*tam == 1)
    {
        vetIndice[minHeap[0].clienteId] = -1;
        *tam -= 1;
        return;
    }

    int ultimo = *tam - 1;
    vetIndice[minHeap[0].clienteId] = -1;
    minHeap[0] = minHeap[ultimo];
    vetIndice[minHeap[0].clienteId] = 0;
    *tam -= 1;

    shifitDownMinHeap(minHeap, *tam, 0, vetIndice);

}
