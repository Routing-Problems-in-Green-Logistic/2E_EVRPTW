//
// Created by ****** on 27/06/22.
//

/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    27/06/22
 *  Arquivo: PreProcessamento.cpp
 * ****************************************
 * ****************************************/

#include "PreProcessamento.h"
#include "Heap.h"
#include "ViabilizadorRotaEv.h"

using namespace N_PreProcessamento;
using namespace N_heap;
using namespace NS_viabRotaEv;

#define PRINT_DIJSTRA       FALSE
#define PRINT_DIJSTRA_CLIEN FALSE

ShortestPathSatCli::ShortestPathSatCli(Instancia &instancia)//:fistCliente(instancia.getFirstClientIndex())
{
    start(instancia);
}

void ShortestPathSatCli::start(Instancia &instancia)
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


void N_PreProcessamento::dijkstraSatCli(Instancia &instancia)
{
    instancia.shortestPath = new ShortestPathSatCli[instancia.getNSats()+1];

    for(int i=instancia.getFirstSatIndex(); i <= instancia.getEndSatIndex(); ++i)
    {
        instancia.shortestPath[i].start(instancia);
    }

    Vector<PreDecessorNo> preDecessorIda(instancia.numNos, PreDecessorNo());
    Vector<PreDecessorNo> preDecessorVolta(instancia.numNos, PreDecessorNo());

    VectorBool excluidos(instancia.numNos, Int8_false);

    // Exclui deposito e satelites

    excluidos[instancia.getDepotIndex()] = Int8_true;


    std::fill((excluidos.begin()+instancia.getFirstSatIndex()), (excluidos.begin()+instancia.getEndSatIndex()+1), Int8_true);

    // Para cada estacao e cliente (possicao) guarda o indice da min heap
    Vector<int> vetIndiceMinHeap(1 + instancia.getNSats() + instancia.getN_RechargingS()+instancia.numClients, -1);

    Vector<int8_t> vetFechados(instancia.numNos);

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
        Vector<DijkstraNo> minHeap(instancia.getN_RechargingS()+instancia.numClients+2);

        // Encontra o menor caminho viavel de sat ate todos os clientes passando pelas estacoes de recarga
        dijkstra(instancia, sat, instancia.getFirstEvIndex(), 0, minHeap, preDecessorIda, excluidos, false,
                 vetIndiceMinHeap, vetFechados, instancia.vetTempoSaida[sat], 0);

        std::fill(excluidos.begin()+instancia.getFirstClientIndex(), excluidos.begin()+instancia.getEndClientIndex()+1, true);

        // Percorre os clientes para encontrar a menor rota viavel de cliente ate satelite, utilizando a bateria
        for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        {
            if(preDecessorIda[i].preDecessor != -1)
            {
                excluidos[i] = false;
                preDecessorVolta[sat].preDecessor = -1;

                //cout<<"Dijkstra volta cliente: "<<i<<"\n\n";
                dijkstra(instancia, i, instancia.getFirstEvIndex(), preDecessorIda[i].bateria, minHeap,
                         preDecessorVolta, excluidos, false, vetIndiceMinHeap, vetFechados,
                         preDecessorIda[i].tempoSaida, 0);


                auto recuperaRota = [&](bool ida, ShortestPathNo &caminho)
                {
                    Vector<int> rota;
                    rota.reserve(10);

                    int indice;

                    if(ida)
                        indice = i;
                    else
                        indice = sat;

                    Vector<PreDecessorNo> *vetPreDecessor;
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

                        caminho.caminhoIda  = Vector<int>(rota);
                        caminho.distIda     = preDecessorIda[i].dist;
                        caminho.batIda      = preDecessorIda[i].bateria;
                    }
                    else
                    {
                        caminho.caminhoVolta = Vector<int>(rota);
                        caminho.distVolta    = preDecessorVolta[sat].dist;
                        caminho.distIdaVolta = caminho.distIda+caminho.distVolta;
                        caminho.batVolta     = preDecessorVolta[sat].bateria;
                    }
                };

                if(preDecessorVolta[sat].preDecessor != -1)
                {

                    ShortestPathNo &caminho = instancia.shortestPath[sat].getShortestPath(i);
                    double dist = preDecessorIda[i].dist + preDecessorVolta[sat].dist;

                    if(dist < caminho.distIdaVolta)
                    {
                        caminho.satId = sat;

                        recuperaRota(true, caminho);
                        recuperaRota(false, caminho);

                    }
                }
                else
                {
                    if(preDecessorIda[i].preDecessor != -1)
                    {
                        ShortestPathNo &caminho = instancia.shortestPath[sat].getShortestPath(i);

                        // Verifica se existe caminho de ida e volta
                        if(!(caminho.distIdaVolta < DOUBLE_INF))
                        {
                            caminho.satId = sat;
                            recuperaRota(true, caminho);
                        }
                    }
                }

                excluidos[i] = true;
            }
        }

        excluidos[sat] = true;
        //minHeap.erase(minHeap.begin(), minHeap.end());

    }

    //cout<<"\n\nROTAS:\n\n";

    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {
        instancia.shortestPath[sat].vetEvRoute = Vector<EvRoute>(instancia.getNClients(), EvRoute(instancia.getFirstSatIndex(), instancia.getFirstEvIndex(),
                                                                                                  instancia.getEvRouteSizeMax(), instancia));

        for(int i = instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
        {
            ShortestPathNo &caminho = instancia.shortestPath[sat].getShortestPath(i);
            EvRoute &evRoute = instancia.shortestPath[sat].getEvRoute(i);

            //cout<<i<<" : "<<caminho.distIdaVolta<<"\n\n";

            if(caminho.distIda < DOUBLE_INF)
            {
                std::reverse(caminho.caminhoIda.begin(), caminho.caminhoIda.end());

                string rotaStr = NS_Auxiliary::printVectorStr(caminho.caminhoIda, caminho.caminhoIda.size());

#if PRINT_DIJSTRA_CLIEN
                if(caminho.distIdaVolta >= DOUBLE_INF)
                    cout<<"\tRota Ida cliente "<<i<<" Dist: "<<caminho.distIda<<":\n\t"<<rotaStr<<"\n"<<"\tBT: "<<caminho.batIda<<"\n\n";
#endif

            }


            if(caminho.distVolta < DOUBLE_INF)
            {
                std::reverse(caminho.caminhoVolta.begin(), caminho.caminhoVolta.end());
            }

            if(caminho.distIdaVolta < DOUBLE_INF)
            {
                evRoute.satelite = caminho.satId;
                evRoute.demanda = instancia.vectCliente[i].demanda;
                evRoute.distancia = caminho.distIdaVolta;

                for(int t = 0; t < caminho.caminhoIda.size(); ++t)
                {
                    evRoute.route[t].cliente = caminho.caminhoIda[t];
                }

                evRoute.routeSize = caminho.caminhoIda.size();

                for(int t = 1; t < caminho.caminhoVolta.size(); ++t)
                {
                    evRoute.route[(caminho.caminhoIda.size() + t - 1)].cliente = caminho.caminhoVolta[t];
                    evRoute.routeSize += 1;
                }

#if PRINT_DIJSTRA_CLIEN
                cout<<"\tRota Completa: ";
                NS_Auxiliary::printVectorCout(evRoute.route, evRoute.routeSize);
#endif

                evRoute[0].tempoSaida = instancia.vetTempoSaida[sat];
                double r = testaRota(evRoute, evRoute.routeSize, instancia, true, instancia.vetTempoSaida[sat], 0,
                                     nullptr, nullptr);

                if(r <= 0.0)
                {
                    cout << "ERRO, DIST NEGATIVA\n";
                    string str;
                    evRoute.print(str, instancia, false);

                    cout << "ROTA: " << str << "\n";
                    PRINT_DEBUG("", "");
                    throw "ERRO";
                }
            }
        }
    }
}

/**
 *
 * @param instancia
 * @param clienteSorce      pode ser sat, estacao ou cliente
 * @param bateria           Quantidade de bateria (eh desconsiderado se clienteSorce eh estacao)
 * @param minHeap
 * @param preDecessor       Guarda o caminho. Possui numNos
 * @param excluidos         vetor com tamanho 1(dep) + num sat + num est + num clientes :: numNos
 * @param clienteCliente    Indica se eh considerado (i,j), sendo i e j clientes
 * @param vetIndiceMinHeap  Guarda o indice de cada cliente na minHeap. Possui numNos
 * @param vetFechados       Indica os nos que ja possuem caminho minimo
 * @param tempoSaida        Tempo de saida de clienteSorce
 * @param minBtCli          Bateria minima para chegada ao cliente
 */

void N_PreProcessamento::dijkstra(Instancia &instancia, const int clienteSorce, const int veicId, double bateria,
                                  Vector<DijkstraNo> &minHeap, Vector<PreDecessorNo> &preDecessor,
                                  const VectorBool &excluidos, const bool clienteCliente,
                                  Vector<int> &vetIndiceMinHeap, Vector<int8_t> &vetFechados,
                                  const double tempoSaida, const double minBtCli)
{

    #if PRINT_DIJSTRA
        cout<<"\n\n********************DIJKSTRA********************\n\n";
        cout<<"Cliente sorce: "<<clienteSorce<<"\n";
        cout<<"BATERIA: "<<bateria<<"\n";
    #endif

    std::fill(vetFechados.begin(), vetFechados.end(), -1);

    const bool sorceSat = instancia.isSatelite(clienteSorce);

    if(instancia.isRechargingStation(clienteSorce) || instancia.isSatelite(clienteSorce))
        bateria = instancia.vectVeiculo[veicId].capacidadeBateria;

    // Verifica se clienteSorce esta em minHeap

    int id = vetIndiceMinHeap[clienteSorce];
    if(id == -1)
    {
        minHeap.emplace_back(clienteSorce);
        minHeap[0].set(clienteSorce, 0.0, bateria, -1.0, tempoSaida);

        id = 0;
    }

    //NS_Auxiliary::printVectorCout(minHeap, 1);

    int tamMinHeap = 1;

    // Extrai o topo da heap
    DijkstraNo dijkNoTopo = minHeap[0];

    while(tamMinHeap > 0)
    {
        dijkNoTopo = minHeap[0];

        #if PRINT_DIJSTRA
            cout<<"\n\n**************************************************************\n\n";
            cout<<"No: "<<minHeap[0].clienteId<<" FECHADO  "<<dijkNoTopo.clienteId<<"\n";
            cout<<"BAT: "<<minHeap[0].bateria<<"; DIST: "<<minHeap[0].dist<<"\n";
            cout<<"TEMPO SAIDA: "<<minHeap[0].tempoSaida<<"\n";
            cout<<"TEMPO SAIDA TOPO: "<<dijkNoTopo.tempoSaida<<"\n\n";
        #endif

        removeTopo(minHeap, &tamMinHeap, vetIndiceMinHeap);

        if(dijkNoTopo.dist >= DOUBLE_INF)
            return;

        // Fecha o no
        vetIndiceMinHeap[dijkNoTopo.clienteId] = -1;
        vetFechados[dijkNoTopo.clienteId] = 1;

        // Verrifica se o no eh cliente
        if(instancia.isClient(dijkNoTopo.clienteId) && !clienteCliente && dijkNoTopo.clienteId != clienteSorce)
            continue;


        const bool noTopoCliente = instancia.isClient(dijkNoTopo.clienteId);

        // Atualiza os nos que sao alcancaveis por dijkNoTopo
        for(int i=0; i < instancia.numNos; ++i)
        {

            #if PRINT_DIJSTRA
                cout<<"clie: "<<i<<": "<<!excluidos[i]<<" : "<<(vetFechados[i]!=1)<<"\n";
            #endif

            if((excluidos[i]==Int8_false) && vetFechados[i] != 1)
            {

                /* casos:
                 *
                 *      1° !noTopoCliente -> ANY
                 *      2° noTopoCliente  -> clienteCliente
                 */


                if(!noTopoCliente || (noTopoCliente && clienteCliente && instancia.isClient(i)) || (noTopoCliente && !instancia.isClient(i)))
                {
                    id = vetIndiceMinHeap[i];

                    // Verifica viabilidade de bateria, distancia e janela de tempo
                    double dist     = instancia.getDistance(dijkNoTopo.clienteId, i) + dijkNoTopo.dist;
                    double tempoC   = dijkNoTopo.tempoSaida + instancia.getDistance(dijkNoTopo.clienteId, i);
                    double tempoS   = 0.0;
                    double bat      = dijkNoTopo.bateria - (instancia.getEvTaxaConsumo(veicId)*instancia.getDistance(dijkNoTopo.clienteId, i));
                    bool   cond     = (id == -1);

                    if(tempoC < instancia.vectCliente[i].inicioJanelaTempo)
                        tempoC = instancia.vectCliente[i].inicioJanelaTempo;

                    if(!cond)
                        cond = (dist < minHeap[id].dist);


                    bool batMin = true;

                    if(instancia.isClient(i))
                    {
                        batMin = bat >= minBtCli;
                    }


                    #if PRINT_DIJSTRA
                        cout<<"\tTEMPO_C: "<<tempoC<<"\n";
                        cout<<"\t("<<(bat >= -TOLERANCIA_BATERIA)<<"; "<<cond<<"; "<<(instancia.verificaJanelaTempo(tempoC, i))<<")\n";
                    #endif

                    if((bat >= -TOLERANCIA_BATERIA) && batMin && cond && instancia.verificaJanelaTempo(tempoC, i))
                    {
                        //cout<<"\t3° if\n";

                        // EXCLUIR

                            if(instancia.isRechargingStation(i))
                            {
                                tempoS = tempoC + (instancia.vectVeiculo[veicId].capacidadeBateria - bat) *
                                                  instancia.vectVeiculo[veicId].taxaRecarga;
                                bat = instancia.vectVeiculo[veicId].capacidadeBateria;
                            } else
                            {
                                tempoS += tempoC + instancia.vectCliente[i].tempoServico;
                            }

                        // EXCLUIR FIM


/*                        cout<<"\tNO: "<<i<<" DIST: "<<dist<<"; BAT: "<<bat<<"; TEMPO_C: "<<tempoC<<"\n";
                        cout<<"\tATUALIZACAO NO: "<<i<<"\n\n";*/


                        // Atualiza minHeap

                        if(id == -1)
                        {
                            minHeap[tamMinHeap].set(i, dist, bat, tempoC, tempoS);
                            id = tamMinHeap;
                            tamMinHeap += 1;
                            vetIndiceMinHeap[i] = id;

                        }
                        else
                        {
                            minHeap[id].dist            = dist;
                            minHeap[id].bateria         = bat;
                            minHeap[id].tempoChegada    = tempoC;
                            minHeap[id].tempoSaida      = tempoS;
                        }

                        // Atualiza pre decessor
                        preDecessor[i].preDecessor = dijkNoTopo.clienteId;
                        preDecessor[i].bateria     = bat;
                        preDecessor[i].dist        = dist;
                        preDecessor[i].tempoSaida  = tempoS;

                        shifitUpMinHeap(minHeap, id, vetIndiceMinHeap);

                    }
                }
            }
        }

        dijkNoTopo = minHeap[0];

        if(!(dijkNoTopo.dist < DOUBLE_INF))
            break;
    }
}

EvRoute& ShortestPathSatCli::getEvRoute(int cliente)
{
    //static const int fist = fistCliente;
    //cout<<cliente<<" "<<fist<<"\n";
    return vetEvRoute.at(cliente-fistCliente);
}

int N_PreProcessamento::getPaiMinHeap(int pos)
{
    if((pos%2) == 0)
        return (pos-2)/2;
    else
        return (pos-1)/2;
}

int N_PreProcessamento::shifitUpMinHeap(Vector<DijkstraNo> &minHeap, int pos, Vector<int> &vetIndice)
{

    int pai = getPaiMinHeap(pos);

    if(pai < 0)
    {
        return 0;
    }

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


int N_PreProcessamento::shifitDownMinHeap(Vector<DijkstraNo> &minHeap, int tam, int pos, Vector<int> &vetIndice)
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

        if(filhoEsq >= tam)
            break;
    }

    if(filhoEsq < tam && filhoDir < tam)
    {
        if((!(minHeap[filhoDir] < minHeap[pos])))
        {
            if(minHeap[pos] > minHeap[filhoEsq])
            {

                DijkstraNo temp = minHeap[pos];
                minHeap[pos] = minHeap[filhoEsq];
                minHeap[filhoEsq] = temp;

                pos = filhoEsq;
            }
        }
    }

    if(vetIndice[minHeap[pos].clienteId] != -1)
        vetIndice[minHeap[pos].clienteId] = pos;

    return pos;

}

void N_PreProcessamento::removeTopo(Vector<DijkstraNo> &minHeap, int *tam, Vector<int> &vetIndice)
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
/**
 * @param instance
 * @param matMinBat
 */
void N_PreProcessamento::calculaMinBtCliente(Instancia &instance, Vector<Matrix<double>> &matMinBat)
{

}
