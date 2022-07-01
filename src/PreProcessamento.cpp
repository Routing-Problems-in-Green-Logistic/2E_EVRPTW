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

using namespace N_PreProcessamento;
using namespace N_heap;

N_PreProcessamento::ShortestPathSatCli::ShortestPathSatCli(Instance &instancia)
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

    std::vector<PreDecessorNo> preDecessor(instancia.numNos, PreDecessorNo());
    std::vector<bool> excluidos(instancia.numNos, false);

    // Exclui deposito e satelites
    excluidos[instancia.getDepotIndex()] = true;
    std::fill((excluidos.begin()+instancia.getFirstSatIndex()), (excluidos.begin()+instancia.getEndSatIndex()+1), true);

    // Para cada estacao e cliente (possicao) guarda o indice da min heap
    std::vector<int> vetIndiceMinHeap(1 + instancia.getNSats() + instancia.getN_RechargingS()+instancia.numClients, -1);

    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {
        excluidos[sat] = false;

        if(sat != instancia.getFirstSatIndex())
        {
            std::fill(preDecessor.begin(), preDecessor.end(), PreDecessorNo());
            std::fill(vetIndiceMinHeap.begin(), vetIndiceMinHeap.end(), -1);
            //std::fill(excluidos.begin(), excluidos.end(), false);
        }

        // Inicialmente possui estacao de recarga e clientes
        std::vector<DijkstraNo> minHeap(instancia.getN_RechargingS()+instancia.numClients);

        // Inicializa o campo clienteId do vet minHeap

        int posMinHeap = 0;

        for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        {
            minHeap[posMinHeap].clienteId = i;
            vetIndiceMinHeap[i] = posMinHeap;

            posMinHeap += 1;
        }

        for(int i=instancia.getFirstRechargingSIndex(); i <= instancia.getEndRechargingSIndex(); ++i)
        {
            minHeap[posMinHeap].clienteId = i;
            vetIndiceMinHeap[i] = posMinHeap;

            posMinHeap += 1;
        }

        NS_Auxiliary::printVectorCout(minHeap, int64_t(posMinHeap));

        // Encontra o menor caminho viavel de sat ate todos os clientes passando pelas estacoes de recarga
        dijkstra(instancia, sat, instancia.getFirstEvIndex(), 0, minHeap, preDecessor, excluidos, false, vetIndiceMinHeap);

        // Recupera a rota
        for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        {
            if(preDecessor[i].preDecessor != -1)
            {
                auto caminho = shortestPathSatCli.getShortestPath(i);
                if(preDecessor[i].dist < caminho.distIda)
                {
                    // Escreve a rota INVERTIDA
                    std::vector<int> rota;
                    rota.reserve(10);

                    int indice = i;

                    while(preDecessor[indice].preDecessor != -1)
                    {
                        rota.push_back(indice);
                        indice = preDecessor[indice].preDecessor;

                        if(indice == -1)
                            break;
                    }

                    if(indice != -1)
                        rota.push_back(indice);

                    caminho.caminhoIda = std::move(rota);
                    caminho.distIda    = preDecessor[i].dist;

                }
            }
        }



        excluidos[sat] = true;

    }

    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        auto caminho = shortestPathSatCli.getShortestPath(i);

        if(caminho.distIda < DOUBLE_INF)
        {
            std::reverse(caminho.caminhoIda.begin(), caminho.caminhoIda.end());
            string rotaStr = NS_Auxiliary::printVector(caminho.caminhoIda, caminho.caminhoIda.size());
            cout<<"Rota cliente "<<i<<": "<<rotaStr<<"\n";
        }
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
 */

void N_PreProcessamento::dijkstra(Instance &instancia, const int clienteSorce, const int veicId, double bateria, std::vector<DijkstraNo> &minHeap, std::vector<PreDecessorNo> &preDecessor,
                                  const std::vector<bool> &excluidos, const bool clienteCliente, std::vector<int> &vetIndiceMinHeap)
{

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
        id = minHeap.size()-1;
        minHeap[id].dist = 0.0;
        minHeap[id].bateria = bateria;
    }

    NS_Auxiliary::printVectorCout(minHeap, minHeap.size());

    if(id != 0)
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

    }

    int tamMinHeap = minHeap.size();

    // Exclui os clientes da minHeap que estao no vetor excluidos
    for(int i=0; i < instancia.numNos; ++i)
    {
        if(excluidos[i])
        {
            if(vetIndiceMinHeap[i] != -1)
            {
                cout<<"EXCLUIDO: "<<i<<"\n";
                id = vetIndiceMinHeap[i];

                minHeap[id].dist = DOUBLE_INF;
                shifitDownMinHeap(minHeap, tamMinHeap, id, vetIndiceMinHeap);

                if(id != (tamMinHeap-1))
                {
                    cout<<"RESULTADO ("<<id<<") shifitDownMinHeap EH DIFERENTE DE "<<tamMinHeap-1<<"\n";
                    throw "ERRO";
                }

                //tamMinHeap -= 1;
            }


        }
    }

    // Extrai o topo da heap
    DijkstraNo dijkNoTopo = minHeap[0];

    while(dijkNoTopo.dist < DOUBLE_INF && vetIndiceMinHeap[dijkNoTopo.clienteId] != -1)
    {

        cout<<"No: "<<minHeap[0].clienteId<<" FECHADO\n";
        cout<<"BAT: "<<minHeap[0].bateria<<"; DIST: "<<minHeap[0].dist<<"\n\n";

        minHeap[0].dist = DOUBLE_INF;
        vetIndiceMinHeap[dijkNoTopo.clienteId] = -1;
        shifitDownMinHeap(minHeap, tamMinHeap, 0, vetIndiceMinHeap);

        cout<<"APOS HEAP DOWN 0:\n";
        NS_Auxiliary::printVectorCout(minHeap, minHeap.size());


        if(dijkNoTopo.dist >= DOUBLE_INF)
        {
            return;
        }

        // Fecha o no
        vetIndiceMinHeap[dijkNoTopo.clienteId] = -1;

        // Verrifica se o no eh cliente
        if(instancia.isClient(dijkNoTopo.clienteId) && !clienteCliente)
            continue;

        const bool noTopoCliente = instancia.isClient(dijkNoTopo.clienteId);

        // Atualiza os nos que sao alcancaveis por dijkNoTopo
        for(int i=0; i < instancia.numNos; ++i)
        {
            if(!excluidos[i] && vetIndiceMinHeap[i] != -1)
            {
                if(!(!clienteCliente && noTopoCliente && instancia.isClient(i)))
                {

                    id = vetIndiceMinHeap[i];

                    // Verifica viabilidade de bateria e distancia
                    double dist = instancia.getDistance(dijkNoTopo.clienteId, i) + dijkNoTopo.dist;
                    double bat  = dijkNoTopo.bateria - dist;

                    cout<<"NO: "<<i<<" DIST: "<<dist<<"; BAT: "<<bat<<"\n";

                    if(bat >= -TOLERANCIA_BATERIA && dist < minHeap[id].dist)
                    {
                        cout<<"ATUALIZACAO NO: "<<i<<"\n";

                        // Atualiza minHeap
                        minHeap[id].dist = dist;
                        if(instancia.isRechargingStation(i))
                            bat = instancia.vectVeiculo[veicId].capacidadeBateria;

                        minHeap[id].bateria = bat;

                        // Atualiza pre decessor
                        preDecessor[i].preDecessor = dijkNoTopo.clienteId;
                        preDecessor[i].bateria     = bat;
                        preDecessor[i].dist        = dist;

                        shifitUpMinHeap(minHeap, id, vetIndiceMinHeap);

                    }
                }
            }
        }

        cout<<"FIM ITERACAO\n\n";
        NS_Auxiliary::printVectorCout(minHeap, minHeap.size());
        dijkNoTopo = minHeap[0];
    }

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

    while(filhoDir < pos)
    {
        if(minHeap[pos] > minHeap[filhoDir])
        {
            int min = filhoDir;

            if(filhoEsq < pos)
            {
                if(minHeap[pos] > minHeap[filhoEsq] && minHeap[filhoEsq] < minHeap[filhoDir])
                {
                    min = filhoEsq;
                }
            }

            DijkstraNo temp = minHeap[pos];
            minHeap[pos] = minHeap[min];
            minHeap[min] = temp;
            vetIndice[minHeap[pos].clienteId] = pos;

            pos = filhoDir;
        }
        else if(filhoEsq < pos)
        {
            if(minHeap[pos] > minHeap[filhoEsq])
            {

                DijkstraNo temp = minHeap[pos];
                minHeap[pos] = minHeap[filhoEsq];
                minHeap[filhoEsq] = temp;
                vetIndice[minHeap[pos].clienteId] = pos;

                pos = filhoEsq;
            }
        }


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

    vetIndice[minHeap[pos].clienteId] = pos;
    return pos;

}
