//
// Created by igor on 19/11/2021.
//

#ifndef INC_2E_EVRP_AUXILIARY_H
#define INC_2E_EVRP_AUXILIARY_H

#include <vector>

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
        for(int i=sizeVector-1; i <= pos; --i)
            vector[i+quant] = vector[i];
    }
}

#endif //INC_2E_EVRP_AUXILIARY_H
