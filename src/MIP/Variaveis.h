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

#define MATRIX_SANITY_CHECK TRUE

namespace VariaveisNs
{
    class MatrixGRBVar
    {
    private:
        GRBVar *vetVar = nullptr;
        int numLin, numCol;
        char typeVar;

    public:

        MatrixGRBVar(GRBModel &model, int numLin, int numCol, const string &&nome, char type);
        MatrixGRBVar()=default;
        MatrixGRBVar(const MatrixGRBVar&)=delete;
        ~MatrixGRBVar(){delete []vetVar;}
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
        void setUB(double ub);
        void setLB(double lb);
        void setUB_LB(double ub, double lb);
        void printVars();
    };

    class VectorGRBVar
    {
    private:
        GRBVar *vetVar = nullptr;
        int num;
        char typeVar;

    public:

        VectorGRBVar(GRBModel &model, int num_, const string &&nome, char type);
        VectorGRBVar()=default;
        VectorGRBVar(const MatrixGRBVar&)=delete;
        ~VectorGRBVar(){delete []vetVar;}
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
        void setUB(double ub);
        void setLB(double lb);
        void setUB_LB(double ub, double lb);
        void printVars();
    };

    class Variaveis
    {
    public:

        int numVertices;                    //numero de satelites mais deposito

        MatrixGRBVar matrix_x;
        MatrixGRBVar matrixDem;
        VectorGRBVar vetY, vetZ, vetT;

        explicit Variaveis(const Instancia &instancia, GRBModel &modelo);
        ~Variaveis();
    };

}


#endif //INC_2E_EVRP_VARIAVEIS_H