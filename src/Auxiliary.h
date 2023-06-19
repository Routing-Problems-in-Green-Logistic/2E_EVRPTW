//
// Created by ****** on 19/11/2021.
//

#ifndef INC_2E_EVRP_AUXILIARY_H
#define INC_2E_EVRP_AUXILIARY_H

#include <boost/container/vector.hpp>
namespace BoostC = boost::container;
#include <iostream>
#include <cfloat>
#include <limits>
#include <cmath>
#include <sys/stat.h>

using namespace std;

#define TOLERANCIA_BATERIA          1e-4
#define TOLERANCIA_DISTANCIA        1e-4
#define TOLERANCIA_DIST_SOLUCAO     1e-2
#define TOLERANCIA_TEMPO            1e-4
#define TOLERANCIA_JANELA_TEMPO     1e-4
#define TOLERANCIA_DEMANDA          1E-5
#define INCREM_DIST                 1E-2
#define TOLERANCIA_DIF_ROTAS        1E-2

#define FLOAT_MAX                   FLT_MAX
#define FLOAT_MIN                   FLT_MIN

#define DOUBLE_MAX                  DBL_MAX
#define DOUBLE_MIN                  DBL_MIN

#define NUM_MAX_EST_POR_ARC         3



#define MV_EV_SHIFIT_INTRA_ROTA                          0
#define MV_EV_SWAP_INTRA_ROTA                            1
#define MV_EV_2OPT                                       2

#define MV_EV_SHIFIT_INTER_ROTAS_INTRA_SAT               3
#define MV_EV_SHIFIT_INTER_ROTAS_INTER_SAT               4

#define MV_EV_SWAP_INTER_ROTAS_INTRA_SAT                 5
#define MV_EV_SWAP_INTER_ROTAS_INTER_SAT                 6

#define MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTRA_SAT     7
#define MV_EV_SHIFIT_2CLIENTES_INTER_ROTAS_INTER_SATS    8


#define MV_EV_CROSS_INTRA_SAT                            9
#define MV_EV_CROSS_INTER_SATS                           10

#define MV_SHIFIT_EV_SATS                                11

#define NUM_MV                                           12
#define NUM_MV_LS                                        12

#define TRUE  1
#define FALSE 0

#define UTILIZA_MAT_MV TRUE

#define TEMPO_FUNC_VIABILIZA_ROTA_EV TRUE

#include <string.h>

#define __PRETTYFILE__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define PRINT_DEBUG(inicio, texto) cout<<inicio<<"DEBUG: "<<texto<<"  FILE: "<<__PRETTYFILE__<<"  FUNC: "<<__PRETTY_FUNCTION__<<"  LINHA: "<<__LINE__<<"\n";
#define ERRO() throw "ERRO";

extern const double DOUBLE_INF;

typedef int8_t Int8;

#define SIZE_ENUM_INV 6

namespace NS_Auxiliary
{

    enum EnumInviabilidade
    {
        Inv_Carga,
        Inv_tw,
        Inv_bat,
        Inv_1_Nivel,
        Inv_nao_ev_rs,
        Inv_1_Nivel_unico
    };

    template<typename T>

    /*
     *
     * Realiza um shift de 'quant' em um vector de tipo T
     * Considera que a ultima posicao, vector[sizeVector-1] ira para vector[sizeVector-1+quant]
     * Realiza o shift ate pos(inclusive)
     *
     */

    void shiftVectorDir(BoostC::vector<T> &vector, const int pos, const int quant, const int sizeVector)
    {

        if(((sizeVector-1)+quant) >= vector.size())
        {
            PRINT_DEBUG("", "");
            throw "ERRO";
        }

        for(int i=sizeVector-1; i >= pos; --i)
        {
            vector[i+quant] = vector[i];
        }

    }

    template<typename T>
    void shiftVectorClienteDir(BoostC::vector<T> &vector, const int pos, const int quant, const int sizeVector)
    {

        for(int i=sizeVector-1; i >= pos; --i)
        {
            vector[i + quant].cliente = vector[i].cliente;
        }
    }

    template<typename T>
    void shiftVectorClienteEsq(BoostC::vector<T> &vector, const int pos, const int sizeVector)
    {

        for(int i=pos; (i+1) < sizeVector; ++i)
            vector[i].cliente = vector[i+1].cliente;

    }

    template<typename T>
    void copiaVector(BoostC::vector<T> &vector, BoostC::vector<T> &vectorDest, const int n)
    {
        if(n > vector.size() || n > vectorDest.size())
            throw "ERRO";

        for(int i=0; i < n; ++i)
            vectorDest[i] = vector[i];
    }

    template<typename T>
    int64_t buscaBinaria(const BoostC::vector<T> &vector, T &ele, const int64_t tam)
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
    string printVector(const BoostC::vector<T> &vector, const int64_t tam)
    {
        string str;
        for(int i=0; i < tam; ++i)
            str += to_string(static_cast<T>(vector[i])) + " ";

        return str;
    }


    template<typename T>
    void printVectorCout(const BoostC::vector<T> &vector, const int64_t tam)
    {

        for(int i=0; i < tam; ++i)
            cout<<vector[i]<<"  ";

        cout<<"\n";
    }


    template<typename T>
    string printVectorStr(const BoostC::vector<T> &vector, const int64_t tam)
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

    inline float upperVal(float x)
    {
        return std::ceil(x);
    }


    inline double upperVal(double x)
    {
        return std::ceil(x);
    }

    inline float lowerVal(float x)
    {
        return std::floor(x);
    }


    inline double lowerVal(double x)
    {
        return std::floor(x);
    }

    void criaDiretorio(const string& caminho);
    string float_to_string(float num, int numCasas);
    string float_to_string(double num, int numCasas);
}


namespace VarAuxiliaresIgNs
{
    inline u_int64_t sumQuantCand       = 0;
    inline u_int64_t num_sumQuantCand   = 0;

    inline u_int64_t sumQuantCliRm      = 0;
    inline u_int64_t num_sumQuantCliRm  = 0;
}

#endif //INC_2E_EVRP_AUXILIARY_H
