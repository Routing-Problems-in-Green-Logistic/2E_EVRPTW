/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    18/09/22
 *  Arquivo: k_means.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_K_MEANS_H
#define INC_2E_EVRP_K_MEANS_H

#include "Instancia.h"

namespace N_k_means
{

    struct Ponto
    {
        double x = 0.0;
        double y = 0.0;

        Ponto()=default;
        Ponto(double x_, double y_){x=x_; y=y_;}
        explicit Ponto(const ClienteInst &cli){x=cli.coordX; y=cli.coordY;}
        Ponto(const Ponto &outro){x=outro.x; y=outro.y;}
        void copia(const Ponto& outro){x=outro.x; y=outro.y;}
        void set(double x_, double y_){x=x_; y=y_;}

        [[nodiscard]] inline double distancia(const Ponto &outro) const
        {
            return sqrt(pow(x-outro.x,2.0)+pow(y-outro.y, 2.0));
        }

        friend ostream& operator<<(ostream& os, const Ponto &ponto)
        {
            os<<ponto.x<<", "<<ponto.y;
            return os;
        }
    };

    struct DistCluster
    {
        int cluster = -1;
        double dist = 0.0;

        DistCluster()=default;
        DistCluster(int clus, double dist_){cluster=clus; dist=dist_;}

        bool operator < (const DistCluster &outro) const
        {
            return dist < outro.dist;
        }
    };

    void converteClientes(Instancia &instancia, Vector<Ponto> &vetPonto);
    Matrix<int> k_means(Instancia &instancia, Vector<int> &vetSatAtendCliente, Vector<int> &satUtilizado, bool seed);
    void printVetPonto(const Vector<Ponto> &vetPonto);
    Vector<double> calculaRaioSatSeedK_means(Instancia &instance);
    void converteMatClusterMatSat(const Matrix<int> &matEntrada, Matrix<int> &matSaida, Instancia &instancia);

}

#endif //INC_2E_EVRP_K_MEANS_H
