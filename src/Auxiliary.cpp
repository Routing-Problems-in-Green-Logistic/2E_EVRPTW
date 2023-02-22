//
// Created by igor on 19/11/2021.
//

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
