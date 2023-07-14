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
#include "../ViabilizadorRotaEv.h"

using namespace ModeloNs;
using namespace VariaveisNs;
using namespace NS_viabRotaEv;
using namespace NS_VetorHash;

void ModeloNs::modelo(const Instancia &instancia, const SetVetorHash &hashSolSet, const Solucao &solucao)
{
    BoostC::vector<RotaEvMip> vetHashSol(hashSolSet.size(), RotaEvMip(instancia.evRouteSizeMax, instancia));
    auto itHashSol = hashSolSet.begin();

    for(int i=0; i < hashSolSet.size(); ++i)
    {

        cout<<"hash: "<<(*itHashSol).valHash<<"\n";
        vetHashSol[i].inicializa(instancia, (*itHashSol));
        ++itHashSol;


        if(i==1)
            break;
    }

    ERRO();

    try
    {

        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);
        setParametrosModelo(model);

        Variaveis variaveis(instancia, model);
        model.update();

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

RotaEvMip::RotaEvMip(const Instancia &instancia, const NS_VetorHash::VetorHash &vetorHash):evRoute(vetorHash.vet[0], -1, instancia.evRouteSizeMax, instancia)
{
    inicializado = true;

    // Copia rota
    evRoute[0].cliente = vetorHash.vet[0];
    evRoute[0].tempoSaida = instancia.getDistance(0, evRoute[0].cliente);
    sat = evRoute[0].cliente;

    for(int i=1; i < instancia.evRouteSizeMax; ++i)
    {
        const int cliente = vetorHash.vet[i];
        evRoute[i].cliente = cliente;
        evRoute.demanda += instancia.getDemand(cliente);

        if(evRoute[i].cliente == evRoute[0].cliente)
        {
            evRoute.routeSize = i+1;
            break;
        }
    }

    evRoute.distancia = testaRota(evRoute, evRoute.routeSize, instancia, true, evRoute[0].tempoSaida, 0, nullptr, nullptr);
    if(evRoute.distancia <= 0.0)
    {
        PRINT_DEBUG("", "");
        cout<<"ERRO, rota ev eh inviavel\n";
        ERRO();
    }

    const int idMenorFolga  = evRoute[0].posMenorFolga;
    const int clienteMenorF = evRoute[idMenorFolga].cliente;

    const double inc = instancia.getFimJanelaTempoCliente(clienteMenorF) - evRoute[idMenorFolga].tempoCheg;
    if(inc < -1E-3)
    {
        PRINT_DEBUG("", "");
        cout<<"Possivel erro: inc("<<inc<<") < 0\n";
        ERRO();
    }

    tempoSaidaMax = evRoute[0].tempoSaida+inc;

    //string strRota;
    //evRoute.print(strRota, instancia, true);
    //cout<<"Rota: "<<strRota<<"\n";
    //cout<<"tempoSaidaMax: "<<tempoSaidaMax<<"\n";


    //ERRO();
}

RotaEvMip::RotaEvMip(int evRouteSizeMax, const Instancia &instancia): evRoute(1, instancia.getFirstEvIndex(), evRouteSizeMax, instancia){}

void RotaEvMip::inicializa(const Instancia &instancia, const VetorHash &vetorHash)
{
    if(inicializado)
    {
        PRINT_DEBUG("", "");
        cout<<"RotaEvMip ja foi inicializado!";
        ERRO();
    }

    inicializado = true;

    // Copia rota
    evRoute[0].cliente    = vetorHash.vet[0];
    evRoute.satelite      = vetorHash.vet[0];
    evRoute[0].tempoSaida = instancia.getDistance(0, evRoute[0].cliente);
    sat = evRoute[0].cliente;

    for(int i=1; i < instancia.evRouteSizeMax; ++i)
    {
        const int cliente = vetorHash.vet[i];
        evRoute[i].cliente = cliente;
        evRoute.demanda += instancia.getDemand(cliente);

        if(evRoute[i].cliente == evRoute[0].cliente)
        {
            evRoute.routeSize = i+1;
            break;
        }
    }

    evRoute.distancia = testaRota(evRoute, evRoute.routeSize, instancia, true, evRoute[0].tempoSaida, 0, nullptr, nullptr);
    if(evRoute.distancia <= 0.0)
    {
        PRINT_DEBUG("", "");
        cout<<"ERRO, rota ev eh inviavel\n";
        ERRO();
    }

    const int idMenorFolga  = evRoute[0].posMenorFolga;
    const int clienteMenorF = evRoute[idMenorFolga].cliente;

    const double inc = instancia.getFimJanelaTempoCliente(clienteMenorF) - evRoute[idMenorFolga].tempoCheg;
    if(inc < -1E-3)
    {
        PRINT_DEBUG("", "");
        cout<<"Possivel erro: inc("<<inc<<") < 0\n";
        ERRO();
    }

    tempoSaidaMax = evRoute[0].tempoSaida+inc;


    string strRota;
    evRoute.print(strRota, instancia, true);
    cout<<"Rota: "<<strRota<<"\n";
    cout<<"tempoSaidaMax: "<<tempoSaidaMax<<"\n**************************\n\n";
}
