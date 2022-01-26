//
// Created by igor on 19/11/2021.
//

#ifndef INC_2E_EVRP_AUXILIARY_H
#define INC_2E_EVRP_AUXILIARY_H

#include <vector>
#include <iostream>
using namespace std;

#define BATTERY_TOLENCE  0.0
#define DISTANCE_TOLENCE 1e-2
#define DEMAND_TOLENCE 1E-3
#define FLOAT_MAX FLT_MAX

#define PRINT_DEBUG(inicio, texto) cout<<inicio<<"DEBUG: "<<texto<<"  FILE: "<<__FILE__<<"  LINHA: "<<__LINE__<<"\n";

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
            str += to_string(vector[i]) + " ";

        return str;
    }


}

#endif //INC_2E_EVRP_AUXILIARY_H
