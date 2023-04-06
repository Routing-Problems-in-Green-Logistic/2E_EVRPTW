//
// Created by igor on 19/11/2021.
//

#include <boost/format.hpp>
#include "Auxiliary.h"

const double DOUBLE_INF = std::numeric_limits<double>::infinity();

void NS_Auxiliary::criaDiretorio(const string& caminho)
{

    if(mkdir(caminho.c_str(), 0777) == -1)
    {
        if(errno != EEXIST)
        {
            cout << "ERRO, NAO FOI POSSIVEL CRIAR O DIRETORIO: " << caminho << "\nErro: " << strerror(errno) << "\n\n";
            throw "ERRO";
        }
    }
}

string NS_Auxiliary::float_to_string(float num, int numCasas)
{
    const string fmt = "%."+std::to_string(numCasas)+"f";
    return string(str(boost::format(fmt) % (num)));
}

string NS_Auxiliary::float_to_string(double num, int numCasas)
{
    const string fmt = "%."+std::to_string(numCasas)+"f";
    return string(str(boost::format(fmt) % (num)));
}
