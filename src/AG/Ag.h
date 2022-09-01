/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    01/09/22
 *  Arquivo: ag.h
 * ****************************************
 * ****************************************/

#ifndef PLOTSOLUTION_PRIMEIRO_NIVEL_PY_AG_H
#define PLOTSOLUTION_PRIMEIRO_NIVEL_PY_AG_H

#include <vector>
#include "Instance.h"
#include "../Solucao.h"

namespace NS_Ag
{
    struct RandomNo
    {
        int cliente = -1;
        float chave = 0.0;

        bool operator < (const RandomNo &outro) const
        {
            return chave < outro.chave;
        }
    };

    struct RandomKey
    {

        std::vector<RandomNo> vetRandom;
        std::vector<RandomNo> vetDecod;

        explicit RandomKey(Instance &instancia);
        void geraRandoKey();
        void ordenaVetDecod();
        void printVetDecod(string &str);

    };

    void decodificaSol(Instance &instancia, RandomKey &randKey, Satelite &sat, const vector<int> &vetSatAtendCliente, const double tempoSaida);

}

#endif //PLOTSOLUTION_PRIMEIRO_NIVEL_PY_AG_H
