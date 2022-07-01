/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    27/06/22
 *  Arquivo: PreProcessamento.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_PREPROCESSAMENTO_H
#define INC_2E_EVRP_PREPROCESSAMENTO_H

#include <iostream>
#include <vector>
#include "Instance.h"
#include "Auxiliary.h"

namespace N_PreProcessamento
{

    class ShortestPathNo
    {

    public:

        int satId       = -1;
        int clienteId   = -1;

        std::vector<int> caminhoIda;
        std::vector<int> caminhoVolta;

        double distIda   = DOUBLE_INF;
        double distVolta = DOUBLE_INF;

        ShortestPathNo()=default;
        ShortestPathNo(int sat, int cli){satId=sat; clienteId=cli;}
    };

    class ShortestPathSatCli
    {

    private:

        int fistCliente = -1;
        int numClientes = -1;
        int numEstacoes = -1;

    public:

        std::vector<ShortestPathNo> vetShortestPath;

        explicit ShortestPathSatCli(Instance &instancia);
        ShortestPathNo& getShortestPath(int cliente);

    };

    class DijkstraNo
    {
    public:

        int clienteId   = -1;
        double dist     = DOUBLE_INF;
        double bateria  = 0.0;

        DijkstraNo()=default;
        explicit DijkstraNo(int cliente){clienteId=cliente;}
        DijkstraNo(int c, double d, double b){clienteId=c; dist=d; bateria=b;}

        bool operator < (const DijkstraNo &outro) const
        {
            return dist < outro.dist;
        }


        bool operator > (const DijkstraNo &outro) const
        {
            return dist > outro.dist;
        }

        friend ostream& operator<<(ostream& os, const DijkstraNo &dijkstraNo)
        {
            os<<dijkstraNo.clienteId<<":"<<dijkstraNo.dist;
            return os;
        }

    };


    class PreDecessorNo
    {
    public:

        int preDecessor = -1;
        double bateria  = 0.0;
        double dist     = 0.0;

        PreDecessorNo()=default;
        PreDecessorNo(int pre, double b, double d){preDecessor=pre; bateria=b; dist=d;}
    };

    void dijkstraSatCli(Instance &instancia, ShortestPathSatCli &shortestPathSatCli);
    void dijkstra(Instance &instancia, int clienteSorce, int veicId, double bateria,
                  std::vector<DijkstraNo> &minHeap, std::vector<PreDecessorNo> &preDecessor,
                  const std::vector<bool> &excluidos, bool clienteCliente,
                  std::vector<int> &vetIndiceMinHeap, std::vector<int8_t> &vetFechados);


    int getPaiMinHeap(int pos);
    int shifitUpMinHeap(std::vector<DijkstraNo> &minHeap, int pos, std::vector<int> &vetIndice);
    int shifitDownMinHeap(std::vector<DijkstraNo> &minHeap, int tam, int pos, std::vector<int> &vetIndice);
    void removeTopo(std::vector<DijkstraNo> &minHeap, int *tam, std::vector<int> &vetIndice);

}

#endif //INC_2E_EVRP_PREPROCESSAMENTO_H
