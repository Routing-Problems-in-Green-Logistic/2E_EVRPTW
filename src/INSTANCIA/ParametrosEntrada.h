/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    21/08/22
 *  Arquivo: ParametrosEntrada.h
 * ****************************************
 * ****************************************/

#ifndef PARAMETROSENTRADA_H
#define PARAMETROSENTRADA_H

#include <cstdint>
#include <iostream>
#include "../Solucao.h"

using namespace std;

namespace NS_parametros
{
    struct ParametrosEntrada
    {

        uint32_t semente = 0;                    // --seed  semente
        string caminhoPasta;                     // --pasta caminhoPasta/
        int numExecucoesTotais = 0;              // --execTotal num
        int execucaoAual = -1;                   // --execAtual 0 ; 0, ..., num-1
        string resultadoCSV = "resultados.csv";  // --resulCSV
        string instancia;
        string nomeInstancia;
        string data;

    };

    struct ParametrosSaida
    {
        double tempo;
    };

    void escreveSolCompleta(ParametrosEntrada &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol);
    void escreveSolParaPrint(ParametrosEntrada &paramEntrada, Solucao &sol);
    void escreveResultadosAcumulados(ParametrosEntrada &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol);
    void consolidaResultados(ParametrosEntrada &paramEntrada);
    void saida(ParametrosEntrada &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol);
    string getNomeInstancia(string str);
    void caregaParametros(ParametrosEntrada &paramEntrada, int argc, char* argv[]);

}

#endif //PARAMETROSENTRADA_H
