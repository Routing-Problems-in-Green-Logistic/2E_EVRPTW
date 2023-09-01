/* ****************************************
 * ****************************************
 *  Nome: ***********************
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
#include <csignal>
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


enum TipoConstrutivo{CONSTRUTIVO1, CONSTRUTIVO2};

namespace NS_parametros
{
    class ParametrosIG
    {
    public:

        float alfaSeg     = 0.2;                          // Valor do parametro alfa do segundo nivel EV
        float betaPrim    = 0.15;                         // Valor do parametro beta do primeiro nivel ~EV
        double difBest    = 0.1;
        double fatorNumCh = 2;
        bool  torneio     = true;                         // Utilizacao de torneio em inst de 100 clientes
        TipoConstrutivo tipoConstrutivo15 = CONSTRUTIVO1;   // Tipo de construtivo para inst <= 15

        double taxaRm    = 0.1;
        std::string fileSaida;                           // Arquivo para salvar o valor da solucao no Irace!

        ParametrosIG()=default;
        ParametrosIG(const std::string& fileStr);
        std::string printParam();
    };

    struct ParametrosMip
    {
        /*
         * presolve(--mip_presolve)   -1 0 1 2
         * cuts(--mip_cuts)           -1 0 1 2 3
         * mipGap(--mip_gap)          0.0 0.01 0.03 0.05 0.1 0.15
         * restT(--mip_restTempo)     0 1
         *
         */

        int presolve   = -1;
        int cuts       = -1;
        double mipGap  = 0.0;
        int restTempo  = 0;
        int outputFlag = 1;

        void print()
        {
            cout<<"presolve: \t"    <<presolve<<"\n";
            cout<<"cuts: \t\t"        <<cuts<<"\n";
            cout<<"mipGap: \t"      <<mipGap<<"\n";
            cout<<"restTempo: \t"   <<restTempo<<"\n";
            cout<<"outputFlag: \t"  <<outputFlag<<"\n\n";
        }
    };

    struct Parametros
    {
        int64_t semente        = 0;                 // --seed  semente
        string caminhoPasta    = "resultados";      // --pasta caminhoPasta/
        int numExecucoesTotais = 0;                 // --execTotal num
        int execucaoAtual      = -1;                // --execAtual 0 ; 0, ..., num-1
        int numItTotal         = 1000;              // --numItTotal
        string resultadoCSV    = "resultados.csv";  // --resulCSV
        string fileNum         = "fileNum";          // --fileNum
        string fileSeed        = "";                // --fileSeed
        string fileSol         = "";                // --fileSol
        string fileSolPrint    = "";                // --fileSolPrint
        bool mip               = true;              // --mip
        string instancia;
        string nomeInstancia;
        string data;
        string paramIgFile;
        int metodo = METODO_GRASP;
        string commit;
        ParametrosIG paramIg;
        ParametrosMip parametrosMip;

        string getParametros();

    };

    struct NoSaida
    {
        NoSaida()
        {

        }

        string nome;
        char tipo = SAIDA_TIPO_FLOAT;
        Vector<int> saidaExec;
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
            saidaExec = Vector<int>(outro.saidaExec);
            listVal = list<double>(outro.listVal);
        }

        NoSaida(string nome_, char tipo_=SAIDA_TIPO_FLOAT)
        {
            nome = std::move(nome_);
            tipo = tipo_;
        }

        NoSaida(string nome_, char tipo_, Vector<int> &saidaExec_)
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
        void getVal(string &saidaStr);

    };

    struct ParametrosSaida
    {
    public:

        std::map<std::string, NoSaida> mapNoSaida;
        double tempo;

        template<class T>
        void setTempo(T start, T end)
        {
            //std::chrono::duration<double> tempoAux = end - start;
            tempo = double(end - start) / double(CLOCKS_PER_SEC);
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

    void escreveSolCompleta(const Parametros &paramEntrada, Solucao &sol, Instancia &instancia, const string &file);
    void escreveSolParaPrint(const Parametros &paramEntrada, Solucao &sol, Instancia &instancia, const string &file);
    void escreveResultadosAcumulados(Parametros &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol);
    void consolidaResultados(Parametros &paramEntrada, ParametrosSaida &paramSaida);
    //void saida(Parametros &paramEntrada, ParametrosSaida &paramSaida, Solucao &sol, Instancia &instancia);
    void saidaNew(Solucao &sol, Instancia &instancia, double tempoCpu, const Parametros &parametros, string &cabecalho, string &valores);
    string getNomeInstancia(string str);
    void caregaParametros(Parametros &paramEntrada, int argc, char* argv[]);
    ParametrosSaida getParametros();

    template <typename T>
    void setParametrosSaida(ParametrosSaida &parametrosSaida, const Parametros &parametros, Solucao &best, T start, T end, Vector<NS_vnd::MvValor> &vetMvValor, Vector<NS_vnd::MvValor> &vetMvValor1Nivel)
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
