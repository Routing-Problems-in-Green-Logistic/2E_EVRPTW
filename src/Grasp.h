/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    02/06/22
 *  Arquivo: Grasp.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_GRASP_H
#define INC_2E_EVRP_GRASP_H

#include "Instancia.h"
#include "Solucao.h"
#include "Vnd.h"
#include "Parametros.h"

namespace NameS_Grasp
{

    struct Estatisticas
    {
        int numIte                  = 0;
        int numSol                  = 0;
        double distAcum             = 0.0;
        int ultimaAtualizacaoBest   = 0;
        string erro;

        double media(){return distAcum/double(numSol);}
    };

    struct EstDist
    {
        int estacao;
        double distancia;

        bool operator < (const EstDist &outro) const{return distancia < outro.distancia;}
    };

    // Armaze o numero de vezes que o cliente i NAO esta na solucao
    class QuantCliente
    {
    public:

        int cliente = -1;
        int quant   =  0;
        int prob    =  0;       // Probabilidade do cliente nao estar na solucao

        QuantCliente()=default;
        QuantCliente(int cliente_, int quant_){cliente=cliente_; quant=quant_;}
        void add1Quant(){quant += 1;}
        void calculaProb(int total)
        {
            //cout<<"quant: "<<quant<<"\n";
            prob = 100-int(100.0*(quant/float(total)));

        }

        bool operator < (const QuantCliente &outro) const
        {
            if(prob != outro.prob)
                return prob < outro.prob;
            else
                return quant > outro.quant;
        }


        friend std::ostream& operator <<(std::ostream &os, const QuantCliente &quantCliente)
        {

            os<<"(Cli: "<<quantCliente.cliente<<", Quant: "<<quantCliente.quant<<", Prob: "<<quantCliente.prob<<")";
            return os;
        }

    };

/*    class Parametros
    {

   public:

        int numIteGrasp                     = 0;         // Numero de itracoes para o grasp
        int iteracoesCalProb                = 50;        // Numero de iteracoes para calcular a prob de inserir uma rota: sat ... EST ... CLIENTE ... EST ... sat
        Vector<float> vetAlfa;
        int numAtualProbReativo             = 50;        // Numero de iteracoes para atualizar a probabilidade do reativo
        int numMaxClie                      = 0;         // Numero maximo de clientes(que NAO conseguem ser inseridos com heur) adicionados a solucao
        float paramFuncSetSatParaCliente    = 0.1;       // Percentual de sat a ser considerados na funcao GreedyAlgNS::setSatParaCliente
        Parametros()=default;
        Parametros(int numIteGrasp_, int iteracoesCalProb_, const Vector<float> &vetAlfa_, int numAtualProbReativo_, int numMaxClie_, float paramFuncSetSatParaCliente_)
        {
            numIteGrasp                 = numIteGrasp_;
            iteracoesCalProb            = iteracoesCalProb_;
            vetAlfa                     = vetAlfa_;
            numAtualProbReativo         = numAtualProbReativo_;
            numMaxClie                  = numMaxClie_;
            paramFuncSetSatParaCliente  = paramFuncSetSatParaCliente_;
        }
    };*/

    Solucao *grasp(Instancia &instance, ParametrosGrasp &parametros, Estatisticas &estat, const bool retPrimeiraSol,
                   const Matrix<int> &matClienteSat, Vector<NS_vnd::MvValor> &vetMvValor,
                   Vector<NS_vnd::MvValor> &vetMvValor1Nivel, NS_parametros::ParametrosSaida &parametrosSaida);


    double getDistMaisPenalidade(Solucao &sol, Instancia &instancia);
    double getPenalidade2Nivel(Solucao &sol, Instancia &instancia, float f);
    double getPenalidade1Nivel(Solucao &sol, Instancia &instance, float f);

    void inicializaSol(Solucao &sol, Instancia &instance);

    // Adiciona uma rota da forma: sat ... EST ... CLIENTE ... EST ... sat
    void addRotaCliente(Solucao &sol, Instancia &instancia, const EvRoute &evRoute, const int cliente);




    std::string converteDouble(double num, int numCasas);
    std::string converteFloat(float num, int numCasas);


}

#endif //INC_2E_EVRP_GRASP_H
