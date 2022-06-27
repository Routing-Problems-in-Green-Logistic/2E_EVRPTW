//
// Created by igor on 27/06/22.
//

/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    27/06/22
 *  Arquivo: Heap.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_HEAP_H
#define INC_2E_EVRP_HEAP_H

#include <vector>

namespace N_heap
{

    int getPaiMinHeap(int pos)
    {
        // par
        if((pos%2) == 0)
            return (pos-2)/2;
        else
            return (pos-1)/2;
    }

    template<typename T>
    int shifitUpMinHeap(std::vector<T> &vet, int pos)
    {

        int pai = getPaiMinHeap(pos);
        while(vet[pos] < vet[pai])
        {
            // swap pos com pai
            T temp = vet[pos];
            vet[pos] = vet[pai];
            vet[pai] = temp;

            pos = pai;
            if(pos == 0)
                break;

            pai = getPaiMinHeap(pos);

        }

        return pos;

    }


    template<typename T>
    int shifitDownMinHeap(std::vector<T> &vet, int tam, int pos)
    {

        int filhoDir = 2*pos+1;
        int filhoEsq = 2*pos+2;

        while(filhoDir < pos)
        {
            if(vet[pos] > vet[filhoDir])
            {
                T temp = vet[pos];
                vet[pos] = vet[filhoDir];
                vet[filhoDir] = temp;

                pos = filhoDir;
            }
            else if(filhoEsq < pos)
            {
                if(vet[pos] > vet[filhoEsq])
                {

                    T temp = vet[pos];
                    vet[pos] = vet[filhoEsq];
                    vet[filhoEsq] = temp;

                    pos = filhoEsq;
                }
            }


            filhoDir = 2*pos+1;
            filhoEsq = 2*pos+2;
        }

        return pos;

    }
}

#endif //INC_2E_EVRP_HEAP_H
