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
#include <utility>
#include <map>
#include "../Solucao.h"

using namespace std;

#define SAIDA_TIPO_INT   'i'
#define SAIDA_TIPO_FLOAT 'f'

#define SAIDA_EXEC_MIN   0
#define SAIDA_EXEC_MAX   1
#define SAIDA_EXEC_AVG   2
#define SAIDA_EXEC_STD   3
#define SAIDA_EXEC_VAL   4

#define SAIDA_ULTIMO     SAIDA_EXEC_VAL

namespace NS_parametros
{

    struct Parametros
    {

        int64_t semente        = 0;                 // --seed  semente
        string caminhoPasta    = "resultados";      // --pasta caminhoPasta/
        int numExecucoesTotais = 0;                 // --execTotal num
        int execucaoAtual      = -1;                // --execAtual 0 ; 0, ..., num-1
        string resultadoCSV    = "resultados.csv";  // --resulCSV
        string instancia;
        string nomeInstancia;
        string data;

    };

    struct NoSaida
    {
        NoSaida()
        {

        }

        string nome;
        char tipo = SAIDA_TIPO_FLOAT;
        vector<int> saidaExec;
        list<float> listVal;

        void operator ()(float val)
        {
            listVal.push_back(val);
        }


        void operator ()(double val)
        {
            listVal.push_back(val);
        }

        void addSaida(int saida)
        {
            saidaExec.push_back(saida);
        }

        NoSaida(const NoSaida &outro)
        {
            nome = outro.nome;
            tipo = outro.tipo;
            saidaExec = vector<int>(outro.saidaExec);
            listVal = list<float>(outro.listVal);
        }

        NoSaida(string nome_, char tipo_=SAIDA_TIPO_FLOAT)
        {
            nome = std::move(nome_);
            tipo = tipo_;
        }

        NoSaida(string nome_, char tipo_, std::vector<int> &saidaExec_)
        {
            nome = std::move(nome_);
            tipo = tipo_;
            saidaExec = saidaExec_;
        }

        void getCabecalho(string &saidaStr)
        {
            for(int &saida:saidaExec)
            {
                switch(saida)
                {
                    case SAIDA_EXEC_MIN:
                        saidaStr += nome + "_min; ";
                        break;

                    case SAIDA_EXEC_MAX:
                        saidaStr += nome + "_max; ";
                        break;

                    case SAIDA_EXEC_AVG:
                        saidaStr += nome + "_avg; ";
                        break;

                    case SAIDA_EXEC_STD:
                        saidaStr += nome + "_std; ";
                        break;

                    case SAIDA_EXEC_VAL:
                        if(saidaExec.size() > 1)
                        {
                            cout<<"OPCAO(SAIDA_EXEC_VAL) SO PODE SER UTILIZADA SOZINHA!\n";
                            throw "ERRO";
                        }

                        saidaStr += nome + "; ";
                        break;
                }
            }

            saidaStr.resize(saidaStr.size()-2);
        }

        void getVal(string &saidaStr)
        {

            vector<float> vectorSaida(listVal.size());
            std::copy(listVal.begin(), listVal.end(), vectorSaida.begin());
            float min = FLOAT_MAX;
            float max = FLOAT_MIN;
            float avg = 0.0;

            for(const float i:vectorSaida)
            {
                if(i < min)
                    min = i;

                if(i > max)
                    max = i;

                avg += i;
            }

            avg = avg/float(vectorSaida.size());
            float std = 0.0;

            for(int &saida:saidaExec)
            {

                switch(saida)
                {
                    case SAIDA_EXEC_MIN:
                        if(tipo == SAIDA_TIPO_FLOAT)
                            saidaStr += to_string(min) + "; ";
                        else
                            saidaStr += to_string(int(min)) + "; ";

                        break;

                    case SAIDA_EXEC_MAX:
                        if(tipo == SAIDA_TIPO_FLOAT)
                            saidaStr += to_string(max) + "; ";
                        else
                            saidaStr += to_string(int(max)) + "; ";
                        break;

                    case SAIDA_EXEC_AVG:
                        saidaStr += to_string(avg) + "; ";
                        break;

                    case SAIDA_EXEC_STD:

                        for(const float i:vectorSaida)
                            std += pow(i-avg, float(2));

                        std = sqrt(std);
                        saidaStr += to_string(std) + "; ";
                        break;

                    case SAIDA_EXEC_VAL:

                        if(listVal.size() > 1)
                        {

                            cout<<"OPCAO(SAIDA_EXEC_VAL) SO PODE SER UTILIZADA SOZINHA!\n";
                            throw "ERRO";
                        }


                        if(tipo == SAIDA_TIPO_FLOAT)
                            saidaStr += to_string(min) + "; ";
                        else
                            saidaStr += to_string(int(min)) + "; ";

                }
            }

            saidaStr.resize(saidaStr.size()-2);

        }



    };

    struct ParametrosSaida
    {
    public:

        std::map<std::string, NoSaida> mapNoSaida;
        double tempo;

        template<class T>
        void setTempo(T start, T end)
        {

            std::chrono::duration<double> tempoAux = end - start;
            tempo = tempoAux.count();
        }
    };

    void escreveSolCompleta(Parametros &paramEntrada, Solucao &sol, Instance &instancia);
    void escreveSolParaPrint(Parametros &paramEntrada, Solucao &sol, Instance &instancia);
    void escreveResultadosAcumulados(Parametros &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol);
    void consolidaResultados(Parametros &paramEntrada);
    void saida(Parametros &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol, Instance &instancia);
    string getNomeInstancia(string str);
    void caregaParametros(Parametros &paramEntrada, int argc, char* argv[]);

}

#endif //PARAMETROSENTRADA_H
