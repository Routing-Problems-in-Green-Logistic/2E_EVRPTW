/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    01/09/22
 *  Arquivo: ag.h
 * ****************************************
 * ****************************************/

#ifndef PLOTSOLUTION_PRIMEIRO_NIVEL_PY_AG_H
#define PLOTSOLUTION_PRIMEIRO_NIVEL_PY_AG_H



#include "Instancia.h"
#include "../Solucao.h"

namespace NS_Ag
{
    struct RandomNo
    {
        int cliente = -1;
        float chave = 0.0;                               // Chave com valor < 0, cliente ja esta na sol

        bool operator < (const RandomNo &outro) const
        {
            return chave < outro.chave;
        }
    };

    struct RandomKey
    {

        Vector<RandomNo> vetRandom;
        Vector<RandomNo> vetDecod;
        double val = 0.0;

        explicit RandomKey(Instancia &instancia);
        void geraRandoKey();
        void ordenaVetDecod();
        void printVetDecod(string &str);

    };

    void decodificaSol(Instancia &instancia, RandomKey &randKey, Satelite &sat, const Vector<int> &vetSatAtendCliente, const double tempoSaida);
    bool atendeTodosOsClientes(const Vector<int> &vetClienteAtend);
}

#endif //PLOTSOLUTION_PRIMEIRO_NIVEL_PY_AG_H
