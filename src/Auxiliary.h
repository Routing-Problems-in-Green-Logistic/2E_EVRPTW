//
// Created by igor on 19/11/2021.
//

#ifndef INC_2E_EVRP_AUXILIARY_H
#define INC_2E_EVRP_AUXILIARY_H

#include <vector>
#include <iostream>
#include <cfloat>
#include <limits>

using namespace std;

#define TOLERANCIA_BATERIA      1e-4
#define TOLERANCIA_DISTANCIA    1e-4
#define TOLERANCIA_DIST_SOLUCAO 1e-2
#define TOLERANCIA_TEMPO        1e-4
#define TOLERANCIA_JANELA_TEMPO 1e-4
#define TOLERANCIA_DEMANDA      1E-5
#define INCREM_DIST             1E-2
#define TOLERANCIA_DIF_ROTAS    1E-2

#define FLOAT_MAX               FLT_MAX
#define FLOAT_MIN               FLT_MIN

#define DOUBLE_MAX              DBL_MAX
#define DOUBLE_MIN              DBL_MIN

#define NUM_MAX_EST_POR_ARC     3

#define TRUE  1
#define FALSE 0

#define TEMPO_FUNC_VIABILIZA_ROTA_EV TRUE

#include <string.h>

#define __PRETTYFILE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define PRINT_DEBUG(inicio, texto) cout<<inicio<<"DEBUG: "<<texto<<"  FILE: "<<__PRETTYFILE__<<"  FUNC: "<<__PRETTY_FUNCTION__<<"  LINHA: "<<__LINE__<<"\n";

extern const double DOUBLE_INF;

namespace NS_Auxiliary
{

    template<typename T>

    /*
     *
     * Realiza um shift de 'quant' em um vector de tipo T
     * Considera que a ultima posicao, vector[sizeVector-1] ira para vector[sizeVector-1+quant]
     * Realiza o shift ate pos(inclusive)
     *
     */

    void shiftVectorDir(std::vector<T> &vector, const int pos, const int quant, const int sizeVector)
    {

        for(int i=sizeVector-1; i >= pos; --i)
        {
            vector[i + quant] = vector[i];
        }

    }

    template<typename T>
    void shiftVectorClienteDir(std::vector<T> &vector, const int pos, const int quant, const int sizeVector)
    {

        for(int i=sizeVector-1; i >= pos; --i)
        {
            vector[i + quant].cliente = vector[i].cliente;
        }
    }

    template<typename T>
    void shiftVectorClienteEsq(std::vector<T> &vector, const int pos, const int sizeVector)
    {

        for(int i=pos; (i+1) < sizeVector; ++i)
            vector[i].cliente = vector[i+1].cliente;

    }

    template<typename T>
    void copiaVector(std::vector<T> &vector, std::vector<T> &vectorDest, const int n)
    {
        if(n > vector.size() || n > vectorDest.size())
            throw "ERRO";

        for(int i=0; i < n; ++i)
            vectorDest[i] = vector[i];
    }

    template<typename T>
    int64_t buscaBinaria(const std::vector<T> &vector, T &ele, const int64_t tam)
    {
        if(tam <= 0)
            return -1;

        int64_t inicio = 0;
        int64_t fim = tam-1;

        int64_t meio = (fim-inicio)/2 + inicio;

        while(vector[meio] != ele && fim != inicio)
        {
            if(ele > vector[meio])
                inicio = meio+1;
            else
                fim = meio-1;

            meio = (fim-inicio)/2+inicio;

        }

        if(inicio == fim)
        {
            if(vector[inicio] == ele)
                return inicio;
        }

        if(vector[meio] == ele)
            return meio;

        return -1;

    }


    template<typename T>
    string printVector(const std::vector<T> &vector, const int64_t tam)
    {
        string str;
        for(int i=0; i < tam; ++i)
            str += to_string(static_cast<T>(vector[i])) + " ";

        return str;
    }


    template<typename T>
    void printVectorCout(const std::vector<T> &vector, const int64_t tam)
    {

        for(int i=0; i < tam; ++i)
            cout<<vector[i]<<"  ";

        cout<<"\n";
    }


    template<typename T>
    string printVectorStr(const std::vector<T> &vector, const int64_t tam)
    {
        string str;

        for(int i=0; i < tam; ++i)
            str += to_string(vector[i])+" ";

        return std::move(str);
    }


    inline bool menor(double x, double y)
    {
        if((x-y) < -TOLERANCIA_DIF_ROTAS)
            return true;

        else
            return false;
    }
}

#endif //INC_2E_EVRP_AUXILIARY_H
