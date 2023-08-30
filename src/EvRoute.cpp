#include <cfloat>

#include "EvRoute.h"
#include "Auxiliary.h"
#include "ViabilizadorRotaEv.h"
#include "Construtivo.h"

using namespace NS_Auxiliary;
using namespace std;
using namespace NS_viabRotaEv;
using namespace NS_Construtivo;



EvRoute::EvRoute(const int _satellite, const int _idRota, const int RouteSizeMax, const Instancia &instance):firstRechargingSIndex(
        instance.getFirstRS_index())
{
    satelite = _satellite;
    routeSize = 2;
    routeSizeMax = RouteSizeMax;
    distancia = 0.0;
    demanda = 0.0;
    idRota = _idRota;

    route.reserve(RouteSizeMax);

    if(idRota != -1)
    {
        route.emplace_back(satelite, instance.getEvBattery(idRota), -1.0, -1.0);
        route.emplace_back(satelite, instance.getEvBattery(idRota), -1.0, -1.0);
    }
    else
    {

        route.emplace_back(satelite, 0, -1.0, -1.0);
        route.emplace_back(satelite, 0, -1.0, -1.0);
    }


    for(int i=2; i < RouteSizeMax; ++i)
    {
        route.emplace_back();
    }


    vetRecarga.reserve(instance.getN_RechargingS());

    for(int i=0; i < instance.getN_RechargingS(); ++i)
    {
        vetRecarga.emplace_back((i+firstRechargingSIndex), 0);
    }

    numEstRecarga = instance.getN_RechargingS();
    numMaxUtilizacao = instance.numUtilEstacao;

}

EvRoute::EvRoute(const EvRoute &evRoute):firstRechargingSIndex(evRoute.firstRechargingSIndex)
{
    routeSizeMax = evRoute.routeSizeMax;
    route.reserve(routeSizeMax);

    if(idRota != -1)
    {
        route.emplace_back(satelite, evRoute.route[0].bateriaRestante, -1.0, -1.0);
        route.emplace_back(satelite, evRoute.route[0].bateriaRestante, -1.0, -1.0);
    }
    else
    {

        route.emplace_back(satelite, 0, -1.0, -1.0);
        route.emplace_back(satelite, 0, -1.0, -1.0);
    }


    for(int i=2; i < routeSizeMax; ++i)
    {
        route.emplace_back();
    }


    vetRecarga.reserve(evRoute.vetRecarga.size());

    for(int i=0; i < evRoute.vetRecarga.size(); ++i)
    {
        vetRecarga.emplace_back((i+firstRechargingSIndex), 0);
    }

    //numEstRecarga = instance.getN_RechargingS();
    numMaxUtilizacao = evRoute.numMaxUtilizacao;

    copia(evRoute, false, nullptr);
}

void EvRoute::copia(const EvRoute &evRoute, const bool calculaDemanda, Instancia *instancia)
{
    satelite = evRoute.satelite;
    routeSize = evRoute.routeSize;
    idRota = evRoute.idRota;

    if(!calculaDemanda)
        demanda = evRoute.demanda;
    else if(instancia)
        demanda = 0.0;
    else
    {
        PRINT_DEBUG("", "calculaDemanda=true e instancia=nullptr eh invalido");
        throw "ERRO";
    }

    numEstRecarga = evRoute.numEstRecarga;
    distancia = evRoute.distancia;
    firstRechargingSIndex = evRoute.firstRechargingSIndex;


    if(!calculaDemanda)
    {
        for(int i = 0; i < routeSize; ++i)
            route[i] = evRoute.route[i];
    }
    else
    {

        for(int i = 0; i < routeSize; ++i)
        {
            route[i] = evRoute.route[i];
            demanda += instancia->getDemand(evRoute.route[i].cliente);
        }
    }




    for(int i=0; i < numEstRecarga; ++i)
        vetRecarga[i] = evRoute.vetRecarga[i];

}

void EvRoute::copiaN(const EvRoute &evRoute, const int n, bool calculaDemanda, Instancia *instancia)
{

    satelite = evRoute.satelite;
    routeSize = n;
    idRota = evRoute.idRota;

    if(!calculaDemanda)
        demanda = evRoute.demanda;
    else if(instancia)
        demanda = 0.0;
    else
    {
        PRINT_DEBUG("", "calculaDemanda=true e instancia=nullptr eh invalido");
        throw "ERRO";
    }

    if(n < 2 || n > evRoute.routeSize)
    {

        PRINT_DEBUG("", "n("<<n<<") > routeSize("<<evRoute.routeSize<<")");
        throw "ERRO";
    }

    numEstRecarga = evRoute.numEstRecarga;
    distancia = 0.0;
    firstRechargingSIndex = evRoute.firstRechargingSIndex;


    if(!calculaDemanda)
    {
        for(int i = 0; i < n; ++i)
        {
            route[i] = evRoute.route[i];
        }
    }
    else
    {

        for(int i = 0; i < n; ++i)
        {
            route[i] = evRoute.route[i];
            demanda += instancia->getDemand(evRoute.route[i].cliente);
        }
    }

    for(int i=1; i < (n-1); ++i)
        distancia += instancia->getDistance(route[i-1].cliente, route[i].cliente);


    for(int i=0; i < numEstRecarga; ++i)
        vetRecarga[i] = evRoute.vetRecarga[i];
}

void EvRoute::copiaParametros(const EvRoute &evRoute)
{
    satelite = evRoute.satelite;

    satelite = evRoute.satelite;
    routeSize = evRoute.routeSize;
    idRota = evRoute.idRota;
    demanda = evRoute.demanda;
    numEstRecarga = evRoute.numEstRecarga;
    distancia = evRoute.distancia;
    firstRechargingSIndex = evRoute.firstRechargingSIndex;


    for(int i=0; i < numEstRecarga; ++i)
        vetRecarga[i] = evRoute.vetRecarga[i];

}


int EvRoute::size() const{
    return routeSize;
}


void EvRoute::print(const Instancia &instance, const bool somenteNo) const
{
    cout<<"Ev id: "<<idRota<<";  ";
    for(int i=0; i < routeSize; ++i)
    {
        if(!somenteNo)
        {
            if(instance.isRechargingStation(route[i].cliente))
                std::cout << "RS(" << route[i].cliente << ";[";
            else
                std::cout << "NO: " << route[i].cliente << "([";

            std::cout << route[i].tempoCheg << ";" << route[i].tempoSaida << "] BT: " << route[i].bateriaRestante
                      << "\t";
        }
        else
            cout<<route[i].cliente<<" ";
    }

    std::cout <<"\nDistancia: "<<distancia<<"; Demanda: "<<demanda<<"\n\n";

}


int EvRoute::getUtilizacaoRecarga(int id)
{
    const int temp = id - firstRechargingSIndex;
    if( temp < 0 || temp >= numEstRecarga)
        return -1;

    if(temp >= numEstRecarga)
    {
        PRINT_DEBUG("", "ERRO INDEX VET RECARGA!!!!");
        throw "ERRO";
    }

    return vetRecarga[temp].utilizado;
}

bool EvRoute::setUtilizacaoRecarga(const int id, const int utilizacao)
{
    if(utilizacao < 0 || utilizacao > numMaxUtilizacao)
        return false;
    else
    {
        if((id-firstRechargingSIndex) < 0 || (id-firstRechargingSIndex) >= numEstRecarga)
        {
            PRINT_DEBUG("", "ERRO INDEX VET RECARGA!!!!");
            throw "ERRO";
        }
        vetRecarga[id - firstRechargingSIndex].utilizado = utilizacao;
        return true;
    }
}

bool EvRoute::checkRoute(std::string &erro, const Instancia &instance) const
{

    if(routeSize < 2)
        return false;

    // Verifica se a rota eh vazia
    if(routeSize == 2)
    {
        if((route[0].cliente == satelite) && (route[1].cliente == satelite) && demanda == 0.0)
            return true;
        else
        {
            if(!((route[0].cliente == satelite) && (route[1].cliente == satelite)))
                erro += "ERRO SATELLITE " + std::to_string(satelite) + "\nINICIO E FIM DA ROTA EH DIFERENTE DE SATELLITE_ID.\nROTA: " +
                        std::to_string(route[0].cliente) + " " + std::to_string(route[1].cliente) + "\n\n";
            else
                erro += "ERRO SATELLITE " + std::to_string(satelite) + "\nDEMANDA (" + to_string(demanda) + ") DEVERIA SER ZERO, ROTA VAZIA\n\n";

            return false;
        }

    }

    // Verifica se o inicio e o final da rota sao iguais ao satellite
    if(!((route[0].cliente == satelite) && (route[routeSize - 1].cliente == satelite)))
    {
        erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nINICIO E FIM DA ROTA SAO DIFERENTES DE SATELLITE_ID.\nROTA: ";
        print(erro, instance, false);
        return false;
    }

    double battery = instance.getEvBattery(idRota);
    double distanceAux = 0.0;
    double demandaAux = 0.0;

    // Verifica se a bateria no  inicio da rota eh igual a quantidade de bateria
    if(route[0].bateriaRestante != battery)
    {
        erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nBATERIA RESTANTE INCORRETO. POSICAO: " + std::to_string(0) + "\nROTA: ";

        print(erro, instance, false);
        return false;
    }

    double tempo = route[0].tempoSaida;

    // Percorre todas as posicoes da rota
    for(int i=1; i < routeSize; ++i)
    {

        const int clienteI = route[i].cliente;

        double aux = instance.getDistance(route[i - 1].cliente, route[i].cliente);
        demandaAux += instance.getDemand(route[i].cliente);

        distanceAux += aux;
        battery -= aux;

        // Calcula o tempo de chegada
        tempo += aux;


        // Verifica se o EV chega antes do inicio da tw
        if(tempo < instance.vectCliente[clienteI].inicioJanelaTempo)
            tempo = instance.vectCliente[clienteI].inicioJanelaTempo;

        if(abs(tempo-route[i].tempoCheg) > TOLERANCIA_TEMPO)
        {
            erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nTEMPO DE CHEGADA AO CLIENTE " + to_string(route[i].cliente) +
                    " ESTA ERRADO. CALCULADO: " + to_string(tempo) + ", ARMAZENADO: " + to_string(route[i].tempoCheg) + "\nROTA:\n";
            print(erro, instance, false);
            return false;
        }

        // Verifica a janela de tempo
        if((tempo > instance.vectCliente[clienteI].fimJanelaTempo) && (abs(tempo - instance.vectCliente[clienteI].fimJanelaTempo) > TOLERANCIA_JANELA_TEMPO))
        {
            erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nJANELA DE TEMPO DO CLIENTE " + to_string(clienteI) +
                    " EH VIOLADA, FINAL DA TW: " + to_string(instance.vectCliente[clienteI].fimJanelaTempo) + "; TEMPO DE CHEGADA: " +
                    to_string(tempo);
        }


        if(instance.isRechargingStation(route[i].cliente))
        {

            // Adiciona o tempo para a recarga completa da bateria

            if(!instance.vectVeiculo[idRota].eletrico)
            {
                erro += "ERRO, ID ROTA: "+ to_string(idRota) + " NAO EH ELETRICO!\n";
                throw "ERRO";
            }

            double tempoRecarga = (instance.getEvBattery(idRota) - battery) * instance.vectVeiculo[idRota].taxaRecarga;
            tempo += tempoRecarga;

        }
        else
            tempo += instance.vectCliente[clienteI].tempoServico;



        // Verifica o tempo de saida
        if(abs(tempo - route[i].tempoSaida) > TOLERANCIA_TEMPO)
        {
            erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nTEMPO DE SAIDA DO CLIENTE " +
                    to_string(route[i].cliente) + " ESTA ERRADO. CALCULADO: " + to_string(tempo) +
                    ", ARMAZENADO: " + to_string(route[i].tempoSaida) + "\nROTA:\n";
            print(erro, instance, false);
            return false;
        }


        // Verifica a bateria
        if(battery < -TOLERANCIA_BATERIA)
        {
            erro += "ERRO EV_ROUTE, SATELLITE " + std::to_string(satelite) + "\nBATERIA!  " + " POSICAO: " +
                    std::to_string(i) + "\nROTA: ";

            print(erro, instance, false);

            erro += "BATERIA: "+ to_string(battery);

            return false;
        }

        if(instance.isRechargingStation(route[i].cliente))
            battery = instance.getEvBattery(idRota);

    }


    if(abs(demanda - demandaAux) > TOLERANCIA_DEMANDA)
    {
        string erroPrint;
        print(erroPrint, instance, false);

        erro += "ERRO, SATELLITE: " + to_string(satelite) + ", ROTA: " + erroPrint + "; DEMANDA ESTA ERRADA, DEMANDA ROTA: " +
                to_string(demanda) + ", DEMANDA CALCULADA: " + to_string(demandaAux) + "\n\n";

        for(int j = 0; j < routeSize; ++j)
        {
            cout << route[j] << ": " << instance.getDemand(route[j].cliente) << "\n";
        }
        cout<<"\n";

        return false;

    }

    if(abs(distanceAux-distancia) > TOLERANCIA_DISTANCIA)
    {

        string erroPrint;
        print(erroPrint, instance, false);

        erro += "ERRO, SATELLITE: " + to_string(satelite) + ", ROTA: " + erroPrint + "; DISTANCIA ESTA ERRADA, DISTANCIA ROTA: " +
                to_string(distancia) + ", DISTANCIA CALCULADA: " + to_string(distanceAux) + "\n\n";

        return false;
    }

    return true;
}

void EvRoute::print(std::string &strRota, const Instancia &instance, const bool somenteNo) const
{
    for(int i=0; i < routeSize; ++i)
    {
        if(!somenteNo)
        {
            if(instance.isRechargingStation(route[i].cliente))
            {
                //strRota += str(boost::format("RS(%.1f;[") % route[i].cliente);
                strRota += "RS(" + to_string(route[i].cliente) + ";[";
            }
            else
                strRota += to_string(route[i].cliente) + "([";

            strRota += float_to_string(route[i].tempoCheg,1) + ";" + float_to_string(route[i].tempoSaida,1) + "] BT: " +
                    float_to_string(route[i].bateriaRestante,1) + "\t";
            //strRota += str(boost::format("%.1f;%.1f] BT: %.1f)") % route[i].tempoCheg % route[i].tempoSaida % route[i].bateriaRestante) + "\t";
        }
        else
        {
            if(instance.isRechargingStation(route[i].cliente))
                strRota += "RS(" + to_string(route[i].cliente) + ") ";
            else
                strRota += to_string(route[i].cliente) + " ";
        }
    }

    strRota += "\n\tDistance: " + NS_Auxiliary::float_to_string(distancia, 2) + "; Demanda: " + to_string(int(demanda)) + "\n";

}

void EvRoute::atualizaParametrosRota(const Instancia &instance)
{

    if(routeSize > 2)
    {
        //print(instance, true);

        int posMenorFolga = routeSize-1;
        double menorDiferenca = instance.vectCliente[route[posMenorFolga].cliente].fimJanelaTempo - route[posMenorFolga].tempoCheg;
        int posProxEstacao = -1;

        for(int i=0; i < numEstRecarga; ++i)
        {
            vetRecarga[i].utilizado = 0;
        }

        for(int i=(routeSize-2); i >= 0; --i)
        {
            EvNo &evNo = route[i];

            evNo.posMenorFolga = posMenorFolga;
            evNo.posProxEstacao = posProxEstacao;

            if(instance.isRechargingStation(evNo.cliente))
            {
                evNo.estacaoBateriaRestante = route[i-1].bateriaRestante - instance.getDistance(route[i-1].cliente, evNo.cliente);
                posProxEstacao = i;

                vetRecarga[evNo.cliente-firstRechargingSIndex].utilizado += 1;

            }
            else
                evNo.estacaoBateriaRestante = -1.0;

            double diferenca = instance.vectCliente[evNo.cliente].fimJanelaTempo - evNo.tempoCheg;
            if(diferenca < menorDiferenca)
            {
                posMenorFolga = i;
                menorDiferenca = diferenca;
            }

        }

    }
}

bool EvRoute::alteraTempoSaida(const double novoTempoSaida, const Instancia &instance, const bool recalculaDist)
{


    if(routeSize <= 2)
        return false;

    route[0].tempoSaida = novoTempoSaida;
    route[0].bateriaRestante = instance.getEvBattery(idRota);

    if(recalculaDist)
        distancia = 0.0;

    for(int i=0; (i+1) < routeSize; ++i)
    {
        const double dist = instance.getDistance(route[i].cliente, route[i+1].cliente);
        //route[i+1].tempoCheg = route[i].tempoSaida + dist;
        route[i+1].bateriaRestante = route[i].bateriaRestante - dist;

        if(recalculaDist)
            distancia += dist;

        if(!(route[i+1].bateriaRestante >= -TOLERANCIA_BATERIA))
        {
            PRINT_DEBUG("", "BATERIA!");
            return false;
        }

        double tempo = route[i].tempoSaida + dist;

        if(!(TESTE_JANELA_TEMPO(tempo, route[i+1].cliente, instance)))
        {
            cout<<"tempo <= cliente.fimJanelaTempo: "<<(tempo <= instance.vectCliente[route[i+1].cliente].fimJanelaTempo)<<"\n";
            cout<<"abs(tempo - cliente.fimJanelaTempo): "<<(abs(tempo - instance.vectCliente[route[i + 1].cliente].fimJanelaTempo) <= 1e-4)<<"\n\n";

            cout<<"Tempo Saida: "<<novoTempoSaida<<"; i: "<<i<<"\n";
            cout<<"Chegada: "<<tempo<<"; TW FIM: "<<instance.vectCliente[route[i+1].cliente].fimJanelaTempo<<"\n";
            cout<<"No: "<<route[i+1].cliente<<"\n";

            PRINT_DEBUG("", "JANELA DE TEMPO");
            return false;
        }

        if(tempo < instance.vectCliente[route[i+1].cliente].inicioJanelaTempo)
            tempo = instance.vectCliente[route[i+1].cliente].inicioJanelaTempo;

        route[i+1].tempoCheg = tempo;

        if(instance.isRechargingStation(route[i+1].cliente))
        {
            const double incrementoBateria = instance.getEvBattery(idRota) - route[i+1].bateriaRestante;

            if(!instance.vectVeiculo[idRota].eletrico)
            {
                PRINT_DEBUG("", "ERRO EV_ROUTE ID: "<<idRota<<", NAO EH ELETRICO\n");
                cout<<"Veic nao eh eletrico\n";
                throw "ERRO";
            }

            route[i+1].tempoSaida = route[i+1].tempoCheg + incrementoBateria*instance.vectVeiculo[idRota].taxaRecarga;
            route[i+1].bateriaRestante = instance.getEvBattery(idRota);
        }
        else
        {
            route[i+1].tempoSaida = route[i+1].tempoCheg + instance.vectCliente[route[i+1].cliente].tempoServico;
            //cout<<route[i+1].cliente<<": cheg: "<<route[i+1].tempoCheg<<"; saida: "<<route[i+1].tempoSaida<<"; tempoServ: "<<instance.vectCliente[route[i+1].cliente].tempoServico<<"\n\n";
        }
    }

    atualizaParametrosRota(instance);
    return true;

}

EvNo::EvNo(const EvNo &outro)
{

    cliente = outro.cliente;
    bateriaRestante = outro.bateriaRestante;
    tempoCheg = outro.tempoCheg;
    tempoSaida = outro.tempoSaida;
}

EvNo::EvNo(int _cliente, double _bateeria, double _tempo_ini, double _tempo_f): cliente(_cliente), bateriaRestante(_bateeria), tempoCheg(_tempo_ini), tempoSaida(_tempo_f) {}

double EvRoute::getCurrentTime()
{
    return route[routeSize-1].tempoCheg;
}

std::string EvRoute::getRota(const Instancia &instance, const bool somenteNo)
{
    string str;

    for(int i=0; i < routeSize; ++i)
    {
        if(instance.isRechargingStation(route[i].cliente))
            str += "RS("+ to_string(route[i].cliente)+";[";
        else
            str +=  to_string(route[i].cliente)+"([";

        str += to_string(route[i].tempoCheg) + ";" + to_string(route[i].tempoSaida) + "] BT: " + to_string(route[i].bateriaRestante) + "\t";
    }

    str += "\nDistance: " + to_string(distancia) + "\n\n";

    return str;
}

void EvRoute::copiaCliente(const EvRoute &outro)
{


    idRota    = outro.idRota;
    satelite  = outro.satelite;
    routeSize = outro.routeSize;
    firstRechargingSIndex = outro.firstRechargingSIndex;

    for(int i=0; i < routeSize; ++i)
        route[i].cliente = outro.route[i].cliente;

}

void EvRoute::resetaEvRoute()
{
    routeSize = 2;
    route[0].cliente = satelite;
    route[0].tempoSaida = -1.0;
    route[0].tempoCheg  = -1.0;
    route[0].posMenorFolga = -1;
    route[0].posProxEstacao = -1;

    route[1].cliente = satelite;

    for(auto it:vetRecarga)
    {
        it.utilizado = 0;
        it.recargaId = -1;
    }

    distancia = 0.0;
    demanda   = 0.0;
}
