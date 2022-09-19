/* ****************************************
 * ****************************************
 *  Nome:    Igor de Andrade Junqueira
 *  Data:    18/09/22
 *  Arquivo: k_means.cpp
 * ****************************************
 * ****************************************/

#include "k_means.h"

using namespace N_k_means;

void N_k_means::converteClientes(Instance &instancia, std::vector<Ponto> &vetPonto)
{
    vetPonto = std::vector<Ponto>(instancia.numNos);
    vetPonto[0].set(-1.0, -1.0);

    for(int i=1; i < instancia.numNos; ++i)
    {
        vetPonto[i] = Ponto(instancia.vectCliente[i]);
        //cout<<i<<": "<<vetPonto[i]<<"\n";
    }

}

void N_k_means::k_means(Instance &instancia)
{
    static std::vector<Ponto> vetPonto;
    static bool convCli = false;

    if(!convCli)
    {
        converteClientes(instancia, vetPonto);
        convCli = true;
    }

    const int numSats = instancia.numSats;
    std::vector<Ponto> centroide(numSats);
    std::vector<Ponto> centroideAnt(numSats);

    std::vector<int> clienteCluster(instancia.numNos, -1);

    // Inicializa os centroides com os sats
    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {
        centroide[sat-instancia.getFirstSatIndex()] = vetPonto[sat];
        clienteCluster[sat] = sat-instancia.getFirstSatIndex();
        //cout<<sat<<": "<<clienteCluster[sat]<<"\n";

    }

    std::vector<DistCluster> vetDistCluster(numSats);

    auto encontraClusterParaCli = [&](const int cli, std::vector<DistCluster> &vetDistCluster)
    {

        for(auto &it:vetDistCluster)
            it = DistCluster();

        Ponto &pontoCli = vetPonto[cli];

        for(int i=0; i < numSats; ++i)
        {
            vetDistCluster[i].cluster = i;
            vetDistCluster[i].dist = centroide[i].distancia(pontoCli);
        }

        std::sort(vetDistCluster.begin(), vetDistCluster.end());
    };

    auto calculaCentroide = [&](std::vector<int> &clienteCluster)
    {

        for(auto &it:centroide)
            it.set(0.0, 0.0);

        vector<int> quantPorCluster(numSats, 0);

        // Atualizacao do centroide pelos clientes
        for(int cli=instancia.getFirstClientIndex(); cli <= instancia.getEndClientIndex(); ++cli)
        {
            const int cluster = clienteCluster[cli];
            quantPorCluster[cluster] += 1;
            centroide[cluster].x += vetPonto[cli].x;
            centroide[cluster].y += vetPonto[cli].y;
        }

        // Atualizacao do centroide pelos satelites
        for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
        {
            const int cluster = (sat-instancia.getFirstSatIndex());
            quantPorCluster[cluster] += 1;
            centroide[cluster].x += vetPonto[sat].x;
            centroide[cluster].y += vetPonto[sat].y;
        }

        // Atualizacao do centroide pelos RS
        for(int rs=instancia.getFirstRS_index(); rs <= instancia.getEndRS_index(); ++rs)
        {
            const int cluster = (rs - instancia.getFirstSatIndex());
            quantPorCluster[cluster] += 1;
            centroide[cluster].x += vetPonto[rs].x;
            centroide[cluster].y += vetPonto[rs].y;
        }

        // Calculas medias
        for(int i=0; i < numSats; ++i)
        {
            centroide[i].x /= quantPorCluster[i];
            centroide[i].y /= quantPorCluster[i];
        }



    };

    for(int i = 0; i < 10; ++i)
    {
        cout<<"i: "<<i<<"\n\n";

        for(int cli=instancia.getFirstClientIndex(); cli <= instancia.getEndClientIndex(); ++cli)
        {
            encontraClusterParaCli(cli, vetDistCluster);

            //cout<<"\n";

            clienteCluster[cli] = vetDistCluster[0].cluster;
        }

        for(int rs=instancia.getFirstRS_index(); rs <= instancia.getEndRS_index(); ++rs)
        {
            encontraClusterParaCli(rs, vetDistCluster);
            clienteCluster[rs] = vetDistCluster[0].cluster;
        }

        if(i != 0)
            centroideAnt = std::vector<Ponto>(centroide);

        calculaCentroide(clienteCluster);

        // Calcula a dist ao quadrado

        if(i != 0)
        {
            double dist = 0.0;
            for(int k = 0; k < instancia.numSats; ++k)
                dist += centroide[k].distancia(centroideAnt[k]);
            dist /= instancia.numSats;
            cout<<"DIST MEDIA: "<<dist<<"\n\n";

            if(dist <= 1E-3)
                break;

        }

    }


    for(int cli=instancia.getFirstClientIndex(); cli <= instancia.getEndClientIndex(); ++cli)
        encontraClusterParaCli(cli, vetDistCluster);


    for(int rs=instancia.getFirstRS_index(); rs <= instancia.getEndRS_index(); ++rs)
        encontraClusterParaCli(rs, vetDistCluster);

    for(int cli=instancia.getFirstClientIndex(); cli <= instancia.getEndClientIndex(); ++cli)
    {
        cout<<cli<<": "<<clienteCluster[cli]<<"\n";
    }

    cout<<"\n\nCENTROIDE:\n";

    for(int sat=0; sat < numSats; ++sat)
        cout<<sat<<": "<<centroide[sat]<<"\n";

    cout<<"\n\nPONTOS: \n\n";


    cout<<"i, x, y, cluster, tipo\n";
    for(int i=instancia.getFirstSatIndex(); i <= instancia.getEndSatIndex(); ++i)
    {
        cout<<i<<", "<<vetPonto[i]<<", "<<clienteCluster[i]<<", D"<<"\n";
    }


    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        cout<<i<<", "<<vetPonto[i]<<", "<<clienteCluster[i]<<", o"<<"\n";
    }


    for(int i=instancia.getFirstRS_index(); i <= instancia.getEndRS_index(); ++i)
    {
        cout<<i<<", "<<vetPonto[i]<<", "<<clienteCluster[i]<<", ^"<<"\n";
    }

    for(int i=0; i < instancia.numSats; ++i)
        cout<<(i+instancia.numNos)<<", "<<centroide[i].x<<", "<<centroide[i].y<<", "<<i<<", X\n";

}