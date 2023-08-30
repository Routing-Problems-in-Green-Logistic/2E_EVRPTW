/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    27/06/22
 *  Arquivo: PreProcessamento.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_PREPROCESSAMENTO_H
#define INC_2E_EVRP_PREPROCESSAMENTO_H

#include <iostream>


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

        Vector<int> caminhoIda;
        Vector<int> caminhoVolta;

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
            caminhoIda = Vector<int>(outro.caminhoIda);
            caminhoVolta = Vector<int>(outro.caminhoVolta);
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

        Vector<ShortestPathNo> vetShortestPath;
        Vector<EvRoute> vetEvRoute;


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

    void dijkstraSatCli(Instancia &instancia);
    void dijkstra(Instancia &instancia, const int clienteSorce, const int veicId, double bateria,
                  Vector<DijkstraNo> &minHeap, Vector<PreDecessorNo> &preDecessor,
                  const VectorBool &excluidos, const bool clienteCliente, Vector<int> &vetIndiceMinHeap,
                  Vector<int8_t> &vetFechados, const double tempoSaida, const double minBtCli);

    void calculaMinBtCliente(Instancia &instance, Vector<Matrix<double>> &matMinBat);

    int getPaiMinHeap(int pos);
    int shifitUpMinHeap(Vector<DijkstraNo> &minHeap, int pos, Vector<int> &vetIndice);
    int shifitDownMinHeap(Vector<DijkstraNo> &minHeap, int tam, int pos, Vector<int> &vetIndice);
    void removeTopo(Vector<DijkstraNo> &minHeap, int *tam, Vector<int> &vetIndice);

}


class ShortestPathSatCli
{

private:

    int fistCliente = -1;
    int numClientes = -1;
    int numEstacoes = -1;

public:

    Vector<N_PreProcessamento::ShortestPathNo> vetShortestPath;
    Vector<EvRoute> vetEvRoute;


    explicit ShortestPathSatCli(Instancia &instancia);
    ShortestPathSatCli()=default;

    void start(Instancia &instance);

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
