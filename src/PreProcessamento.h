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
#pragma once
//#include "Instance.h"
#include "common.h"
#include "Auxiliary.h"
#include "EvRoute.h"

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
        double batIda    = 0.0;

        double distVolta = DOUBLE_INF;
        double batVolta  = 0.0;

        double distIdaVolta = DOUBLE_INF;

        ShortestPathNo()=default;
        ShortestPathNo(int sat, int cli){satId=sat; clienteId=cli;}
        ShortestPathNo(const ShortestPathNo &outro)
        {
            satId = outro.satId;
            clienteId = outro.clienteId;
            caminhoIda = std::vector<int>(outro.caminhoIda);
            caminhoVolta = std::vector<int>(outro.caminhoVolta);
            distIda = outro.distIda;
            distVolta = outro.distVolta;
            distIdaVolta = outro.distIdaVolta;
        }

        ~ShortestPathNo()=default;
    };

/*    class ShortestPathSatCli
    {

    private:

        int fistCliente = -1;
        int numClientes = -1;
        int numEstacoes = -1;

    public:

        std::vector<ShortestPathNo> vetShortestPath;
        std::vector<EvRoute> vetEvRoute;


        explicit ShortestPathSatCli(Instance &instancia);
        ShortestPathSatCli()=default;

        ShortestPathNo& getShortestPath(int cliente);
        EvRoute& getEvRoute(int cliente);

    };*/

    class DijkstraNo
    {
    public:

        int clienteId       = -1;
        double dist         = DOUBLE_INF;
        double bateria      = 0.0;
        double tempoChegada = 0.0;
        double tempoSaida   = 0.0;

        DijkstraNo()=default;
        ~DijkstraNo()=default;

        explicit DijkstraNo(int cliente){clienteId=cliente;}

        DijkstraNo(const int cliente_, const double dist_, const double bat_, const double tempoC, const double tempoS)
        {
            clienteId       = cliente_;
            dist            = dist_;
            bateria         = bat_;
            tempoChegada    = tempoC;
            tempoSaida      = tempoS;
        }

        void set(const int cliente_, const double dist_, const double bat_, const double tempoC, const double tempoS)
        {

            clienteId       = cliente_;
            dist            = dist_;
            bateria         = bat_;
            tempoChegada    = tempoC;
            tempoSaida      = tempoS;
        }

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

        int preDecessor     = -1;
        double bateria      = 0.0;
        double dist         = 0.0;
        double tempoSaida   = 0.0;

        PreDecessorNo()=default;
        PreDecessorNo(int pre, double b, double d){preDecessor=pre; bateria=b; dist=d;}
    };

    void dijkstraSatCli(Instance &instancia);
    void dijkstra(Instance &instancia, const int clienteSorce, const int veicId, double bateria,
                  std::vector<DijkstraNo> &minHeap, std::vector<PreDecessorNo> &preDecessor,
                  const std::vector<bool> &excluidos, const bool clienteCliente, std::vector<int> &vetIndiceMinHeap,
                  std::vector<int8_t> &vetFechados, const double tempoSaida, const double minBtCli);

    void calculaMinBtCliente(Instance &instance, std::vector<ublas::matrix<double>> &matMinBat);

    int getPaiMinHeap(int pos);
    int shifitUpMinHeap(std::vector<DijkstraNo> &minHeap, int pos, std::vector<int> &vetIndice);
    int shifitDownMinHeap(std::vector<DijkstraNo> &minHeap, int tam, int pos, std::vector<int> &vetIndice);
    void removeTopo(std::vector<DijkstraNo> &minHeap, int *tam, std::vector<int> &vetIndice);

}


class ShortestPathSatCli
{

private:

    int fistCliente = -1;
    int numClientes = -1;
    int numEstacoes = -1;

public:

    std::vector<N_PreProcessamento::ShortestPathNo> vetShortestPath;
    std::vector<EvRoute> vetEvRoute;


    explicit ShortestPathSatCli(Instance &instancia);
    ShortestPathSatCli()=default;

    void start(Instance &instance);

    N_PreProcessamento::ShortestPathNo& getShortestPath(int cliente);
    EvRoute& getEvRoute(int cliente);

};

#endif //INC_2E_EVRP_PREPROCESSAMENTO_H

/*
 * 5
 * 7
 * 8
 *
 */
