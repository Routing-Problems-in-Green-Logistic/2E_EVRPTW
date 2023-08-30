/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    13/07/23
 *  Arquivo: Variaveis.h
 * ****************************************
 * ****************************************/

#ifndef INC_2E_EVRP_VARIAVEIS_H
#define INC_2E_EVRP_VARIAVEIS_H

#include "gurobi_c++.h"
#include "../Auxiliary.h"
#include "Instancia.h"
#include "../VetorHash.h"

#define MATRIX_SANITY_CHECK TRUE

namespace VariaveisNs
{
    class MatrixGRBVar
    {
    private:
        bool inicializado = false;
        GRBVar *vetVar = nullptr;
        double *vetDoubleAttr_X = nullptr;
        int numLin, numCol;
        char typeVar;

    public:

        MatrixGRBVar(GRBModel &model, int numLin, int numCol, const string &&nome, char type);
        MatrixGRBVar()=default;
        MatrixGRBVar(const MatrixGRBVar&)=delete;
        void operator = (MatrixGRBVar &)=delete;
        void inicializa(GRBModel &model, int numLin, int numCol, const string &&nome, char type);

        ~MatrixGRBVar();
        inline __attribute__((always_inline)) GRBVar& operator ()(const int indexI, const int indexJ)
        {

#if MATRIX_SANITY_CHECK
            if(indexI >= numLin || indexI < 0)
            {
                std::cout<<"Erro indice i: "<<indexI<<" esta errado para matrix de tam "<<numLin<<" x "<<numCol<<"\n";
                throw std::out_of_range("");
            }

            if(indexJ >= numCol)
            {
                std::cout<<"Erro indice j: "<<indexJ<<" esta errado para matrix de tam "<<numLin<<" x "<<numCol<<"\n";
                throw std::out_of_range("");
            }
#endif

            return vetVar[indexI*numCol+indexJ];
        }

        inline __attribute__((always_inline)) double getX_value(const int indexI, const int indexJ)
        {
            if(vetDoubleAttr_X == nullptr)
            {
                cout<<"ERRO, vetDoubleAttr eh igual a nullptr\n";
                ERRO();
            }


#if MATRIX_SANITY_CHECK
            if(indexI >= numLin || indexI < 0)
            {
                std::cout<<"Erro indice i: "<<indexI<<" esta errado para matrix de tam "<<numLin<<" x "<<numCol<<"\n";
                throw std::out_of_range("");
            }

            if(indexJ >= numCol)
            {
                std::cout<<"Erro indice j: "<<indexJ<<" esta errado para matrix de tam "<<numLin<<" x "<<numCol<<"\n";
                throw std::out_of_range("");
            }
#endif

            return vetDoubleAttr_X[indexI * numCol + indexJ];
        }

        void setUB(double ub);
        void setLB(double lb);
        void setUB_LB(double ub, double lb);
        void printVars();

        void setVetDoubleAttr_X(GRBModel &model, bool X_n);
        void setAttr_Start0();

    };

    class VectorGRBVar
    {
    private:
        bool inicializado = false;
        GRBVar *vetVar = nullptr;
        double *vetDoubleAttr_X = nullptr;
        int num;
        char typeVar;

    public:

        inline __attribute__((always_inline)) int getNum(){return num;}
        VectorGRBVar(GRBModel &model, int num_, const string &&nome, char type);
        void inicializa(GRBModel &model, int num_, const string &&nome, char type);

        VectorGRBVar()=default;
        VectorGRBVar(const MatrixGRBVar&)=delete;
        void operator = (const VectorGRBVar &)=delete;
        ~VectorGRBVar(){delete []vetVar; vetVar= nullptr;}
        inline __attribute__((always_inline)) GRBVar& operator ()(const int indexI)
        {

#if MATRIX_SANITY_CHECK
            if(indexI >= num || indexI < 0)
            {
                std::cout<<"Erro indice i: "<<indexI<<" esta errado para vector de tam "<<num<<"\n";
                throw std::out_of_range("");
            }
#endif

            return vetVar[indexI];
        }

        inline __attribute__((always_inline)) double getX_value(int indexI)
        {

            if(vetDoubleAttr_X == nullptr)
            {
                cout<<"ERRO, vetDoubleAttr eh igual a nullptr\n";
                ERRO();
            }

#if MATRIX_SANITY_CHECK
            if(indexI >= num || indexI < 0)
            {
                std::cout<<"Erro indice i: "<<indexI<<" esta errado para vector de tam "<<num<<"\n";
                throw std::out_of_range("");
            }
#endif

            return vetDoubleAttr_X[indexI];
        }

        void setUB(double ub);
        void setLB(double lb);
        void setUB_LB(double ub, double lb);
        void setUB_LB(double ub, double lb, int i);
        void printVars();

        void setVetDoubleAttr_X(GRBModel &model, bool Xn);
        void setAttr_Start0();
    };


    class RotaEvMip
    {
    private:
        bool inicializado = false;

    public:

        EvRoute evRoute;
        double tempoSaidaMax;
        Vector<Int8> vetAtend;      // indica se um cliente eh atendido
        int sat;

        RotaEvMip(const Instancia &instancia, const NS_VetorHash::VetorHash &vetorHash);
        RotaEvMip()= default;
        RotaEvMip(const Instancia &instancia, const EvRoute &evRoute_);//: evRoute(evRoute_){}
        explicit RotaEvMip(int evRouteSizeMax, const Instancia &instancia);
        void inicializa(const Instancia &instancia, const NS_VetorHash::VetorHash &vetorHash);
    };

    class Variaveis
    {
    public:

        int numVertices;                    //numero de satelites mais deposito

        MatrixGRBVar matrix_x;
        MatrixGRBVar matrixDem;
        VectorGRBVar vetY, vetZ, vetT;

        Variaveis(const Instancia &instancia, GRBModel &modelo, const Vector<RotaEvMip> &vetRotasEv);
        ~Variaveis()=default;
        void setVetDoubleAttr_X(GRBModel &model, bool Xn);
        void setAttr_Start0();
    };

}


#endif //INC_2E_EVRP_VARIAVEIS_H
