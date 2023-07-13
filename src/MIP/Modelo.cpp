/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    13/07/23
 *  Arquivo: Modelo.cpp
 * ****************************************
 * ****************************************/

#include "Modelo.h"
#include "gurobi_c++.h"
#include "Variaveis.h"

using namespace ModeloNs;
using namespace VariaveisNs;

void ModeloNs::modelo()
{
    try
    {

        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);
        setParametrosModelo(model);

        MatrixGRBVar matrix(model, 2, 2, "x", 'B');
        model.update();

        matrix.printVars();

        model.optimize();
    }
    catch(GRBException &grbException)
    {
        cout<<"ERRO CODE: "<<grbException.getErrorCode()<<"\nMessage: ";
        cout<<grbException.getMessage()<<"\n";
    }
}

void ModeloNs::setParametrosModelo(GRBModel &model)
{

    model.set(GRB_IntParam_Threads, 1);

}