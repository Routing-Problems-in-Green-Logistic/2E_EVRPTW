/* ****************************************
 * ****************************************
 *  Nome: ***********************
 *  Data:    18/09/22
 *  Arquivo: k_means.cpp
 * ****************************************
 * ****************************************/

#include "k_means.h"
#include "../mersenne-twister.h"

using namespace N_k_means;

void N_k_means::converteClientes(Instancia &instancia, Vector<Ponto> &vetPonto)
{
    vetPonto = Vector<Ponto>(instancia.numNos);
    vetPonto[0].set(-1.0, -1.0);

    for(int i=1; i < instancia.numNos; ++i)
    {
        vetPonto[i] = Ponto(instancia.vectCliente[i]);
        //cout<<i<<": "<<vetPonto[i]<<"\n";
    }
}

Matrix<int> N_k_means::k_means(Instancia &instancia, Vector<int> &vetSatAtendCliente, Vector<int> &satUtilizado, bool seed)
{
    static Vector<Ponto> vetPonto;
    static Vector<double> vetRaioSat;

    static bool staticStart = false;

    if(instancia.numSats > 0)
    {
        std::fill((vetSatAtendCliente.begin()+instancia.getFirstClientIndex()),
                  (vetSatAtendCliente.begin()+instancia.getEndClientIndex()+1), instancia.getFirstSatIndex());

        satUtilizado[instancia.getFirstSatIndex()] = 1;
        Matrix<int> matSaida(instancia.numNos, instancia.numNos, 1);
        return matSaida;
    }
    else
    {
        std::fill(vetSatAtendCliente.begin(), vetSatAtendCliente.end(), -1);
        std::fill(vetSatAtendCliente.begin()+1, vetSatAtendCliente.end(), -1);
    }

    if(!staticStart)
    {
        converteClientes(instancia, vetPonto);
        if(seed)
            vetRaioSat = calculaRaioSatSeedK_means(instancia);

        staticStart = true;
    }

    const int numSats = instancia.numSats;
    Vector<Ponto> centroide(numSats, Ponto());
    Vector<Ponto> centroideAnt(numSats, Ponto());

    // Clusters [0, numSats)
    Vector<int> clienteCluster(instancia.numNos, -1);

    if(!seed)
    {
        // Inicializa os centroides com os sats
        for(int sat = instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
        {
            centroide[sat - instancia.getFirstSatIndex()] = vetPonto[sat];
            clienteCluster[sat] = sat - instancia.getFirstSatIndex();
        }
    }
    else
    {
        // Inicializa os centroides em um raio dos sats
        for(int sat = instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
        {
            Ponto ponto = vetPonto[sat];
            const Ponto centro(ponto);

            if(vetRaioSat[sat] == 0.0)
            {
                centroide[sat-instancia.getFirstSatIndex()] = ponto;
                clienteCluster[sat] = sat - instancia.getFirstSatIndex();
                continue;
            }

            double temp = rand_u32() % int(floor(vetRaioSat[sat]));
            const double raio =temp+double((rand_u32()%100)/100.0);

            int angulo = rand_u32()%360; // [0,359]
            if(angulo < 90)
            {
                //cout<<"< 90\n";
                const double complementar = angulo*0.0174533;

                ponto.x += abs(cos(complementar)*raio);
                ponto.y += abs(sin(complementar)*raio);

            }
            else if((angulo>=90) && (angulo<180))
            {
                //cout<<">= 90; < 180\n";
                const double complementar = (180-angulo)*0.0174533;

                ponto.x += -abs(cos(complementar)*raio);
                ponto.y += abs(sin(complementar)*raio);
            }
            else if((angulo>=180) && (angulo<270))
            {
                //cout<<">= 180; < 270\n";
                const double complementar = (angulo-180)*0.0174533;

                ponto.x += -abs(cos(complementar)*raio);
                ponto.y += -abs(sin(complementar)*raio);
            }
            else // angulo >= 270 && angulo < 360
            {
                //cout<<">= 270; < 360\n";
                const double complementar = (360-angulo)*0.0174533;

                ponto.x += abs(cos(complementar)*raio);
                ponto.y += -abs(sin(complementar)*raio);
            }

            double dist = ponto.distancia(centro);

            if(abs(dist-raio) > 10E-2)
            {
                cout<<"ERRO DIST(CENTRO, PONTO)("<<dist<<") != RAIO("<<raio<<")\n\n";
            }

            //cout<<"SAT("<<sat<<") PONTO("<<vetPonto[sat]<<"); RAIO("<<raio<<"); CENTROIDE: "<<ponto<<"; DIST(PONTO, SAT): "<<dist<<"\n";
            centroide[sat-instancia.getFirstSatIndex()] = ponto;
            clienteCluster[sat] = sat - instancia.getFirstSatIndex();
        }

        //cout<<"\n\n";

    }

    Vector<DistCluster> vetDistCluster(numSats);

    auto encontraClusterParaCli = [&](const int cli, Vector<DistCluster> &vetDistCluster)
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

    auto calculaCentroide = [&](Vector<int> &clienteCluster, Vector<Ponto> &centroide)
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
            const int cluster = clienteCluster[rs];
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

    for(int i = 0; i < 400; ++i)
    {
        //cout<<"i: "<<i<<"\n\n";
        //cout<<"centroide: "<<&centroide[0]<<"; ant: "<<&centroideAnt[0]<<"\n\n";

        // Aloca o cluster(centroide) mais prox para cada cliente
        for(int cli=instancia.getFirstClientIndex(); cli <= instancia.getEndClientIndex(); ++cli)
        {
            encontraClusterParaCli(cli, vetDistCluster);
            clienteCluster[cli] = vetDistCluster[0].cluster;
        }

        // Aloca o cluster(centroide) mais prox para cada RS
        for(int rs=instancia.getFirstRS_index(); rs <= instancia.getEndRS_index(); ++rs)
        {
            encontraClusterParaCli(rs, vetDistCluster);
            clienteCluster[rs] = vetDistCluster[0].cluster;
        }

        // Guarda o centroide da it anterior
        if(i != 0)
        {
            for(int k=0; k < numSats; ++k)
                centroideAnt[k] = centroide[k];
        }

        calculaCentroide(clienteCluster, centroide);

        // Calcula a dist entre centroide da iteracao anterior e o novo
        if(i != 0)
        {
            double dist = 0.0;
            for(int k = 0; k < instancia.numSats; ++k)
                dist += centroide[k].distancia(centroideAnt[k]);

            dist /= instancia.numSats;
            if(dist <= 1E-3)
            {
                //cout<<"NUM IT: "<<i+1<<"\n\n";
                break;
            }
        }
    }

    for(int cli=instancia.getFirstClientIndex(); cli <= instancia.getEndClientIndex(); ++cli)
        encontraClusterParaCli(cli, vetDistCluster);

    for(int rs=instancia.getFirstRS_index(); rs <= instancia.getEndRS_index(); ++rs)
        encontraClusterParaCli(rs, vetDistCluster);

    // Seta os vetores vetSatAtendCliente e satUtilizado utilizados no construtivo
    for(int cli=instancia.getFirstClientIndex(); cli <= instancia.getEndClientIndex(); ++cli)
    {
        const int sat = clienteCluster[cli]+instancia.getFirstSatIndex();
        vetSatAtendCliente[cli] = sat;
        satUtilizado[sat] = 1;
    }

    Matrix<int> matSaida(instancia.numNos, numSats, 0);      // Para uma posicao: mat(clienteI, cluster_0) = 0,1: indica se o clienteI pode ser atendido pelo sat cluster_0 + sat_0_id

    // Preenche matSaida

    // ********************************************************************************************
    Vector<double> mediaIntraCluster(instancia.numNos, 0.0);
    Vector<double> mediaInterCluster(instancia.numNos, DOUBLE_INF);
    Vector<int>    clusterMinInterCluster(instancia.numNos, -1);


    Matrix<int> matCluster(instancia.numNos, numSats, 0);      //cluster: [0, numSat)
    Vector<int> vetNumElemPorCluster(instancia.numSats, 0);        //cluster: [0, numSat)

    // Prenche matCluster com os elementos de cada cluster
    for(int i=1; i < instancia.numNos; ++i)
    {
        int cluster = clienteCluster[i];
        int prox = vetNumElemPorCluster[cluster];
        matCluster(prox, cluster) = i;
        vetNumElemPorCluster[cluster] += 1;
    }

    for(int i=1; i < instancia.numNos; ++i)
    {
        if(!(i >= instancia.getFirstSatIndex() && i <= instancia.getEndSatIndex()))
        {
            int cluster = clienteCluster[i];

            for(int j=0; j < vetNumElemPorCluster[cluster]; ++j)
            {
                int cliente = matCluster(j, cluster);
                mediaIntraCluster[i] += instancia.getDistance(i, cliente);
            }

            mediaIntraCluster[i] /= vetNumElemPorCluster[cluster]-1;

            for(int clus=0; clus < numSats; ++clus)
            {
                if(clus != cluster)
                {
                    double media = 0.0;
                    for(int j=0; j < vetNumElemPorCluster[clus]; ++j)
                        media += instancia.getDistance(i, matCluster(j, clus));

                    media /= vetNumElemPorCluster[clus];
                    if(media < mediaInterCluster[i])
                    {
                        mediaInterCluster[i] = media;
                        clusterMinInterCluster[i] = clus;
                    }
                }
            }
        }
    }

/*    Vector<double> vetSilhouette(instancia.numNos, 0.0);

    for(int i=1; i < instancia.numNos; ++i)
    {
        if(!(i >= instancia.getFirstSatIndex() && i <= instancia.getEndSatIndex()))
        {
            const double b = mediaInterCluster[i];
            const double a = mediaIntraCluster[i];

            vetSilhouette[i] = (b-a)/max(a,b);
            cout<<"Silhouette("<<i<<"): "<<vetSilhouette[i]<<"; CLUSTER MAIS PROX: "<<clusterMinInterCluster[i]<<"\n";
        }
    }
    cout<<"\n\n";*/

    // Preenche matSaida
    for(int i=1; i < instancia.numNos; ++i)
    {
        if(!(i >= instancia.getFirstSatIndex() && i <= instancia.getEndSatIndex()))
        {
            int cluster = clienteCluster[i];
            matSaida(i, cluster) = 1;

            cluster = clusterMinInterCluster[i];
            matSaida(i, cluster) = 1;
        }
    }


/*    for(int c=0; c < instancia.numSats; ++c)
    {
        Vector<DistCluster> vetCluste(instancia.numSats-1, DistCluster());
        int prox = 0;

        for(int c1=0; c1 < instancia.numSats; ++c1)
        {
            if(c1 != c)
            {
                vetCluste[prox].cluster = c1;
                vetCluste[prox].dist = instancia.getDistance((c+instancia.getFirstSatIndex()), (c1+instancia.getFirstSatIndex()));
                prox += 1;
            }
        }


        std::sort(vetCluste.begin(), vetCluste.end());


        for(int i=instancia.getEndSatIndex()+1; i < instancia.numNos; ++i)
        {
            //cout<<"clienteCluste["<<i<<"]("<<clienteCluster[i]<<" == "<<c<<"\n";
            if(clienteCluster[i] == c)
            {
                for(int t=0; t < 2; ++t)
                    matSaida(i, vetCluste[t].cluster) = 1;

                //cout<<"OK\n";
            }
        }
    }*/

    Matrix<int> mat;
    converteMatClusterMatSat(matSaida, mat, instancia);

    return mat;


/*    cout<<"\t";
    for(int i=0; i < instancia.numSats; ++i)
        cout<<i<<"\t";

    cout<<"\n";

    for(int i=instancia.getEndSatIndex()+1; i < instancia.numNos; ++i)
    {
        cout<<i<<"; "<<clienteCluster[i]<<":\t";

        for(int c=0; c < instancia.numSats; ++c)
            cout<<matSaida(i, c)<<"\t";
        cout<<"\n";
    }*/


    // ********************************************************************************************



/*    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        cout << i << " " << clienteCluster[i]<<"; "<<vetSatAtendCliente[i]<<"\n";
    }

    cout<<"\n\n";

    cout<<"\n\nCENTROIDE:\n";

    for(int sat=0; sat < numSats; ++sat)
        cout<<sat<<": "<<centroide[sat]<<"\n";

    cout<<"\n\nPONTOS: \n\n";


    cout<<"i, x, y, cluster, tipo\n";

    const int clusterPrint = -1;

    for(int i=instancia.getFirstSatIndex(); i <= instancia.getEndSatIndex(); ++i)
    {
        if((clusterPrint >= 0 && clusterPrint == clienteCluster[i]) || clusterPrint < 0)
            cout<<i<<", "<<vetPonto[i]<<", "<<clienteCluster[i]<<", D"<<"\n";
    }


    for(int i=instancia.getFirstClientIndex(); i <= instancia.getEndClientIndex(); ++i)
    {
        if((clusterPrint >= 0 && clusterPrint == clienteCluster[i]) || clusterPrint < 0)
            cout<<i<<", "<<vetPonto[i]<<", "<<clienteCluster[i]<<", o"<<"\n";
    }


    for(int i=instancia.getFirstRS_index(); i <= instancia.getEndRS_index(); ++i)
    {
        if((clusterPrint >= 0 && clusterPrint == clienteCluster[i]) || clusterPrint < 0)
            cout<<i<<", "<<vetPonto[i]<<", "<<clienteCluster[i]<<", ^"<<"\n";
    }

    for(int i=0; i < instancia.numSats; ++i)
        cout<<(i+instancia.numNos)<<", "<<centroide[i].x<<", "<<centroide[i].y<<", "<<i<<", X\n";


    cout<<"\n\n";

    for(int i=0; i < instancia.numNos; ++i)
    {
        if(!(i >= instancia.getFirstSatIndex() && i <= instancia.getEndSatIndex()))
        {
            if((clusterPrint >= 0 && clusterPrint == clienteCluster[i]) || clusterPrint < 0)
            {
                int j = clusterMinInterCluster[i];
                Ponto ponto = centroide[j];

                cout<<vetPonto[i]<<", "<<ponto<<"\n";
            }
        }
    }*/


}

void N_k_means::printVetPonto(const Vector<Ponto> &vetPonto)
{
    for(const Ponto &ponto:vetPonto)
        cout<<ponto<<"\n";

    cout<<"\n";
}

/*
 * Excentricidade de um vertice:
 *
 *      Sendo um grafo G(V, E), V conjunto de vertices e E conjunto de arstas, tal que (i,j), i!=j, i,j \in V
 *   Cada aresta de E esta associada uma distância, d(i,j) como sendo d a distacia euclidiana  entre i e j.
 *
 *      A  excentricidade e(i), i \in V eh igual a: max: d(i,j) para todo j \in V
 *
 * Centro do grafo
 *
 *
 */


Vector<double>N_k_means::calculaRaioSatSeedK_means(Instancia &instancia)
{
    // Calcula a excentricidade de cada vertice
    Vector<double> vetExcentricidadeNos(instancia.numNos, DOUBLE_MIN);
    int centro = -1;
    double eCentro = DOUBLE_MAX;

    for(int i=1; i < instancia.numNos; ++i)
    {
        for(int j=1; j < instancia.numNos; ++j)
            vetExcentricidadeNos[i] = max(vetExcentricidadeNos[i], instancia.getDistance(i, j));

        if(vetExcentricidadeNos[i] < eCentro)
        {
            centro = i;
            eCentro = vetExcentricidadeNos[i];
        }
        //cout<<"e("<<i<<") = "<<vetExcentricidadeNos[i]<<"\n";
    }
    //cout<<"Centro do grafo: "<<centro<<"\n\n";

    Vector<double> raioSat(instancia.numSats+1, 0.0);

    for(int sat=instancia.getFirstSatIndex(); sat <= instancia.getEndSatIndex(); ++sat)
    {
        raioSat[sat] = instancia.getDistance(sat, centro);
        //cout<<"Raio("<<sat<<") = "<<raioSat[sat]<<"\n";
    }

    return std::move(raioSat);
}

void N_k_means::converteMatClusterMatSat(const Matrix<int> &matEntrada, Matrix<int> &matSaida, Instancia &instancia)
{
    matSaida = Matrix<int>(instancia.numNos, instancia.numNos, 0);
    const int fistSat = instancia.getFirstSatIndex();

    for(int i=instancia.getEndSatIndex()+1; i < instancia.numNos; ++i)
    {
        for(int c=0; c < instancia.numSats; ++c)
            matSaida(i, (c+fistSat)) = matEntrada(i,c);
    }

}