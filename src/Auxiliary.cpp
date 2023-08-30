//
// Created by ****** on 19/11/2021.
//


#include <fstream>
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
    static char buf[10000];
    std::snprintf(buf, 10000, "%.*f", numCasas, num);
    return string(buf);
}

string NS_Auxiliary::float_to_string(double num, int numCasas)
{
    static char buf[10000];
    std::snprintf(buf, 10000, "%.*f", numCasas, num);
    return string(buf);
}

void NS_Auxiliary::escreveStrEmArquivo(const std::string &string, const std::string &&nomeArq, ios::openmode openmode)
{
    std::fstream file;
    file.open(nomeArq, openmode);

    if(!file.is_open())
    {
        std::cerr << "Erro ao abrir arquivo: " << nomeArq << "\n";
        std::cerr << strerror(errno) << "\n\n";
        ERRO_();
    }

    file<<string;
    file.close();
}
