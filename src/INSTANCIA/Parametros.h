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
#include <chrono>
#include "../Solucao.h"
#include "../Vnd.h"

using namespace std;

#define SAIDA_TIPO_INT   'i'
#define SAIDA_TIPO_FLOAT 'f'

#define SAIDA_EXEC_MIN   0
#define SAIDA_EXEC_MAX   1
#define SAIDA_EXEC_AVG   2
#define SAIDA_EXEC_STD   3
#define SAIDA_EXEC_VAL   4
#define SAIDA_EXEC_SEM   5

#define METODO_GRASP 0
#define METODO_ACO   1
#define METODO_IG    2

#define SAIDA_ULTIMO     SAIDA_EXEC_SEM

namespace NS_parametros
{

    struct Parametros
    {
        int64_t semente        = 0;                 // --seed  semente
        string caminhoPasta    = "resultados";      // --pasta caminhoPasta/
        int numExecucoesTotais = 0;                 // --execTotal num
        int execucaoAtual      = -1;                // --execAtual 0 ; 0, ..., num-1
        int numItTotal         = 1000;              // --numItTotal
        string resultadoCSV    = "resultados.csv";  // --resulCSV
        string instancia;
        string nomeInstancia;
        string data;
        string paramIg;
        int metodo = METODO_GRASP;
        string commit;

        string getParametros();

    };

    struct NoSaida
    {
        NoSaida()
        {

        }

        string nome;
        char tipo = SAIDA_TIPO_FLOAT;
        BoostC::vector<int> saidaExec;
        list<double> listVal;
        uint64_t semente=0;

        void operator ()(float val)
        {
            listVal.push_back(val);
        }


        void operator ()(double val)
        {
            listVal.push_back(val);
        }


        void operator ()(int val)
        {
            listVal.push_back(val);
        }

        void addSemente(uint64_t sem_)
        {
            semente = sem_;
        }

        void addSaida(int saida)
        {
            saidaExec.push_back(saida);
        }

        NoSaida(const NoSaida &outro)
        {
            nome = outro.nome;
            tipo = outro.tipo;
            saidaExec = BoostC::vector<int>(outro.saidaExec);
            listVal = list<double>(outro.listVal);
        }

        NoSaida(string nome_, char tipo_=SAIDA_TIPO_FLOAT)
        {
            nome = std::move(nome_);
            tipo = tipo_;
        }

        NoSaida(string nome_, char tipo_, BoostC::vector<int> &saidaExec_)
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
                        saidaStr += nome + "_min \t ";
                        break;

                    case SAIDA_EXEC_MAX:
                        saidaStr += nome + "_max \t ";
                        break;

                    case SAIDA_EXEC_AVG:
                        saidaStr += nome + "_avg \t ";
                        break;

                    case SAIDA_EXEC_STD:
                        saidaStr += nome + "_std \t ";
                        break;

                    case SAIDA_EXEC_VAL:
                        if(saidaExec.size() > 1)
                        {
                            cout<<"OPCAO(SAIDA_EXEC_VAL) SO PODE SER UTILIZADA SOZINHA!\n";
                            throw "ERRO";
                        }

                        saidaStr += nome + " \t ";
                        break;

                    case SAIDA_EXEC_SEM:

                        if(saidaExec.size() > 1)
                        {
                            cout<<"OPCAO(SAIDA_EXEC_SEM) SO PODE SER UTILIZADA SOZINHA!\n";
                            throw "ERRO";
                        }

                        saidaStr += "sem \t ";
                        break;
                }
            }

            //saidaStr.resize(saidaStr.size()-2);
        }

        void getVal(string &saidaStr)
        {

            BoostC::vector<double> vectorSaida(listVal.size());
            std::copy(listVal.begin(), listVal.end(), vectorSaida.begin());
            double min = DOUBLE_MAX;
            double max = DOUBLE_MIN;
            double avg = 0.0;

            for(const double i:vectorSaida)
            {
                if(i < min)
                    min = i;

                if(i > max)
                    max = i;

                avg += i;
            }

            avg = avg/double(vectorSaida.size());
            double std = 0.0;

            for(int &saida:saidaExec)
            {

                switch(saida)
                {
                    case SAIDA_EXEC_MIN:
                        if(tipo == SAIDA_TIPO_FLOAT)
                            saidaStr += to_string(min) + " \t ";
                        else
                            saidaStr += to_string(int(min)) + " \t ";

                        break;

                    case SAIDA_EXEC_MAX:
                        if(tipo == SAIDA_TIPO_FLOAT)
                            saidaStr += to_string(max) + " \t ";
                        else
                            saidaStr += to_string(int(max)) + " \t ";
                        break;

                    case SAIDA_EXEC_AVG:
                        saidaStr += to_string(avg) + " \t ";
                        break;

                    case SAIDA_EXEC_STD:

                        for(const double i:vectorSaida)
                            std += pow(i-avg, double(2));

                        std = sqrt(std/vectorSaida.size());
                        saidaStr += to_string(std) + " \t ";
                        break;

                    case SAIDA_EXEC_VAL:

                        if(listVal.size() > 1)
                        {

                            cout<<"OPCAO(SAIDA_EXEC_VAL) SO PODE SER UTILIZADA SOZINHA!\n";
                            throw "ERRO";
                        }

                        if(tipo == SAIDA_TIPO_FLOAT)
                            saidaStr += to_string(min) + " \t ";
                        else
                            saidaStr += to_string(int(min)) + " \t ";

                        break;

                    case SAIDA_EXEC_SEM:

                        if(saidaExec.size() > 1)
                        {
                            cout<<"OPCAO(SAIDA_EXEC_SEM) SO PODE SER UTILIZADA SOZINHA!\n";
                            throw "ERRO";
                        }

                        saidaStr += to_string(semente)+" \t ";
                        break;
                }
            }
            //saidaStr.resize(saidaStr.size()-2);
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

        void getCabecalho(string &saida)
        {
            for(auto &it:mapNoSaida)
            {
                it.second.getCabecalho(saida);
            }
        }

        void getVal(string &saida)
        {
            for(auto &it:mapNoSaida)
            {
                it.second.getVal(saida);
            }
        }
    };

    void escreveSolCompleta(Parametros &paramEntrada, Solucao &sol, Instancia &instancia);
    void escreveSolParaPrint(Parametros &paramEntrada, Solucao &sol, Instancia &instancia);
    void escreveResultadosAcumulados(Parametros &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol);
    void consolidaResultados(Parametros &paramEntrada, ParametrosSaida &paramSaida);
    void saida(Parametros &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol, Instancia &instancia);
    string getNomeInstancia(string str);
    void caregaParametros(Parametros &paramEntrada, int argc, char* argv[]);
    ParametrosSaida getParametros();

    template <typename T>
    void setParametrosSaida(ParametrosSaida &parametrosSaida, const Parametros &parametros, Solucao &best, T start, T end, BoostC::vector<NS_vnd::MvValor> &vetMvValor, BoostC::vector<NS_vnd::MvValor> &vetMvValor1Nivel)
    {

        parametrosSaida.setTempo(start, end);
        if(parametrosSaida.tempo <= 2.0)
            sleep(1);

        parametrosSaida.mapNoSaida["dist"](best.distancia);
        parametrosSaida.mapNoSaida["t(s)"](parametrosSaida.tempo);
        parametrosSaida.mapNoSaida["ultimaA"](best.ultimaA);
        parametrosSaida.mapNoSaida["sem"].addSemente(parametros.semente);
        parametrosSaida.mapNoSaida["numSatVazios"](best.numSatVazios());
        parametrosSaida.mapNoSaida["numEV"](best.getNumEvNaoVazios());


/*        for(int k=0; k < NUM_MV_LS; ++k)
        {
            string nome  = "mv_"+ to_string(k);
            string nome1 = nome+"_num";
            string nome2 = nome + "_percent";
            string nome3 = nome1+"total";

            parametrosSaida.mapNoSaida[nome](vetMvValor[k].getMedia());
            parametrosSaida.mapNoSaida[nome1](vetMvValor[k].quant);
            parametrosSaida.mapNoSaida[nome2](vetMvValor[k].getMediaPercent());
            parametrosSaida.mapNoSaida[nome3](vetMvValor[k].quantChamadas);

        }

        parametrosSaida.mapNoSaida["mv_4_1Nivel"](vetMvValor1Nivel[0].getMedia());
        parametrosSaida.mapNoSaida["mv_4_1Nivel_num"](vetMvValor1Nivel[0].quant);

        parametrosSaida.mapNoSaida["mv_6_1Nivel"](vetMvValor1Nivel[1].getMedia());
        parametrosSaida.mapNoSaida["mv_6_1Nivel_num"](vetMvValor1Nivel[1].quant);
        */


        //  numSatVazios numEV
    }
}

#endif //PARAMETROSENTRADA_H
