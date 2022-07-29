import os
import subprocess
import sys
import pandas as pd
import math
import time
'''
date '+%d_%m_%Y_%k_%M_%S'

'''

#data = strftime("%d_%m_%Y_%H_%M", gmtime())

numExecucoes = 1

'''if len(sys.argv) < 5:
    print 'Numero de  argumentos errado'
    print 'python prog.py saida.csv heuristica_i heuristica_j instancia1.dat instancia1. dat ...'
    
    exit(-1)'''

#ramdom = open("ramdomNumbers.txt","w")

p = subprocess.Popen(["date", "+%d/%m/%Y %k:%M"], stdout=subprocess.PIPE)
saida = p.communicate()
saida = saida[0]
data = saida.decode("utf-8")
print(data)

'''
nome ='mkdir ' + 'resultados_'  + data

os.system(nome)

nomeParciais = 'resultados_' + data + '/resultadosParciais'
parciais = 'mkdir ' + nomeParciais

os.system(parciais)

nomeCompletos ='resultados_' + data + '/resultadosCompletos'

completos = 'mkdir ' + nomeCompletos
os.system(completos)

nomeGrupos ='resultados_' + data + '/resultadosPorGrupos'

completos = 'mkdir ' + nomeGrupos

os.system(completos)



#csvSaida = pd.read_csv(sys.argv[1])
'''

#instancias = sys.argv[2:]


'''
inst1 = instancias[0]
nome = inst1.split('/')

grupos =  {}.fromkeys([10,15,20,25,50,75,100], False)
'''

'''
Tabela com:
    emissao tempoCpu gap% emissaoDarci tempoCpuDarci
    
    Meu : Poluicao (kg), tempo cpu (min), ultima atualizacao, numero de solucoes inviaveis, tempo total de viagem (min), distancia total (km).
    Literatura: Poluicao (kg), tempo cpu (min),  tempo total de viagem (min), distancia total (km).
'''
# instancia, best, gap,  mediaPoluicao, mediaTempo, mediaUltimaAt, medSolucInv, mediaTempViagem, mediaDis, co2L, tempoTotalL, disTotalL, tempoL

lista = [(' ', 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)]

tamanhoInst = [5, 10, 15, 100]
diretorioIni = 'instancias/2e-vrp-tw/'
instancias = []

for i in tamanhoInst:
    caminho = diretorioIni + 'Customer_' + str(i) + '/'
    files = os.listdir(caminho)
    files.sort()

    for f in files:
        instancias.append(caminho+f)

for instancia in instancias:

    '''
    nome = instancia.split('/')
    instanciaNome = nome[len(nome)-1]
    instanciaNome = instanciaNome.split('.')
    instanciaNome = instanciaNome[0]
    nome = nome[len(nome)-1]
    nome = nome.split('.')
    nome = nome[0]

    
    aux = nome
    
    nome = ''
    
    for i in range(3):
    
        if aux[3+i] >= '0' and aux[3+i] <= '9':
            nome += aux[3+i]
        else:
            break
            
    idGrupo = int(nome)        
    
    if grupos[idGrupo] is False:
        print '\n*****************************************************************************************************************************************************************************************************************'
        print '\nGRUPO ' + str(idGrupo) + '\n'
    
    grupos[idGrupo] = True
    '''
    
    #print(instancia)
    
    for i in range(numExecucoes):

      time.sleep(1)
      strExecutavel = 'src/cmake-build-debug//run ' + str(instancia) # + ' > /dev/null'
      os.system(strExecutavel)
       	#ramdom.write(str(semente))
       	#ramdom.write('\n')

    '''
    nomeArq = nomeParciais + '/' + instanciaNome + '.txt'
    
    arquivo = open(nomeArq,"r")
    
    mediaTempo = 0.0
    mediaPol = 0.0
    veiculosDis = 0.0
    veiculosUsados = 0.0
    solucoesInviaveis = 0.0
    ultimaAtualizacao = 0.0
    vetPoluicaoMv = []
    vetTempoMv = []
    
    for i in range(8):
        vetPoluicaoMv.append(float(0.0))
            
    for i in range(8):
        vetTempoMv.append(float(0.0))  
    
    best = float("inf")
    bestCpu = float("inf")
    
    # instanci, best, gap,  mediaPoluicao, mediaTempo, mediaUltimaAt, medSolucInv, mediaTempViagem, mediaDis, co2L, tempoTotalL, disTotalL, tempoL
    
    num = 0.0
    
    for i in range(numExecucoes):
        
        linha = arquivo.readline()
        resultado = linha.split(" ")
        
        if float(resultado[0]) <= 0.0:
            continue
        
        mediaPol += float(resultado[0])
        mediaTempo += float(resultado[1])
        veiculosDis += float(resultado[2])
        veiculosUsados += float(resultado[3])

        solucoesInviaveis += float(resultado[4])
        ultimaAtualizacao += float(resultado[5])
        num += 1.0
        
        for i in range(8):
            vetPoluicaoMv[i] += float(resultado[6+i])
            
        for i in range(9):
            vetTempoMv[i] += float(resultado[14+i])         
            
        
        
        if (float(resultado[0]) < best) and float(resultado[0] > 0):
            best = float(resultado[0])
            bestCpu = float(resultado[1])
        
    arquivo.close()
    
 
    
    if num > 0.0:
    
        mediaTempo = mediaTempo/float(num)
        mediaPol = mediaPol/float(num)
        veiculosDis = veiculosDis/float(num)
        veiculosUsados = veiculosUsados/float(num)
        mediaSolucoes = solucoesInviaveis/float(num)
        ultimaAtualizacao = ultimaAtualizacao/float(num)
        
        for i in range(8):
            vetPoluicaoMv[i] = vetPoluicaoMv[i]/float(num)
            
        for i in range(8):
            vetTempoMv[i] = vetTempoMv[i]/float(num)
    else:
        mediaTempo = mediaPol = mediaVeiUsados = veiculosUsados = mediaTempo = best = bestCpu = mediaSolucoes = ultimaAtualizacao = 0.0
    
    arquivoGrupo = nomeGrupos + '/resutados_grupo_' + str(idGrupo) + '_.txt'
    
    arquivo = open(arquivoGrupo,"a+")
    
    escrita = nome + ' ' + str(mediaPol) +  ' '+ str(veiculosDis) + ' ' + str(veiculosUsados) + ' ' + str(mediaTempo) + ' ' + str(mediaSolucoes) + ' ' + str(ultimaAtualizacao) + ' '
    
    if num > 0.0:
    
        for i in range(8):
            escrita += str(vetPoluicaoMv[i]) + ' '
        
        for i in range(9):
            escrita += str(vetTempoMv[i]) + ' '
            
    else:
        
        for i in range(8):
            escrita += str(0.0) + ' '
        
        for i in range(8):
            escrita += str(0.0) + ' '
                
        
    escrita += '\n'        
        
    
    arquivo.write(escrita)
    arquivo.close()
    
    arquivoGrupo = nomeGrupos + '/resutados_grupo_' + str(idGrupo) + '_best.txt'
    
    arquivo = open(arquivoGrupo,"a+")
    
    escrita = nome + ' ' + str(best) + ' ' + str(bestCpu) + '\n'
    
    arquivo.write(escrita)
    arquivo.close()
    
#round

    
# instancia, best, gap,  mediaPoluicao, mediaTempo, mediaUltimaAt, medSolucInv, mediaTempViagem, mediaDis, co2L, tempoTotalL, disTotalL, tempoL    

##resultado = pd.DataFrame(columns=['grupo', 'data', 'poluicaoL', 'tempoL', 'veiculosDisL', 'veiculosUsadosL', 'minVeiculosL', 'poluicaoBestL', 'poluicao','gapAvg',  'tempoAvg', 'veiculosDis', 'veiculosUsados', 'minVeiculos', 'solucoesInviaveis', 'poluicaoBest', 'tempoBest', 'gapBest' ])    

resultadosLiteratura = pd.read_csv("resultadosLiteraturaPorGrupo.csv")

resultados = [(0, data, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)]

for i in grupos:
    
    if grupos[i] is True:
        
        arquivoGrupo = nomeGrupos + '/resutados_grupo_' + str(i) + '_.txt'
        arquivo = open(arquivoGrupo,"r")
    
        poluicao = 0.0
        veiculosDis = 0
        veiculosUsados = 0.0
        minVeiculos = float("inf")
        tempo = 0.0
        solucoesInviaveis = 0.0
        ultimaAtualizacao = 0.0
        vetPoluicaoMv = []
        vetTempoMv = []
    
        for k in range(8):
            vetPoluicaoMv.append(float(0.0))
            
        for k in range(8):
            vetTempoMv.append(float(0.0))  
        
        tam = 0.0
        
        for aux in arquivo:
            
            linha = aux.split(' ')
            
            if float(linha[1]) <= 0.0:
                continue
            
            poluicao += float(linha[1])
            
            veiculosUsados += float(linha[3])
            
            if float(linha[3]) < minVeiculos:
                minVeiculos = float(linha[3])
                
            tempo += float(linha[4])
            solucoesInviaveis += float(linha[5])
            ultimaAtualizacao += float(linha[6])
            
            for k in range(8):
                vetPoluicaoMv[k] += float(linha[7+k])
                
            for k in range(8):
                vetTempoMv[k] += float(linha[15+k])
            
            tam += 1.0
            
        arquivo.close()
        

       
        if tam > 0.0:
            poluicao /= tam
            veiculosUsados /=tam
            tempo /= tam
            solucoesInviaveis /= tam
            ultimaAtualizacao /= tam
            
            for k in range(8):
                vetPoluicaoMv[k] = vetPoluicaoMv[k]/float(tam)
            
            for k in range(8):
                
                vetTempoMv[k] = vetTempoMv[k]/float(tam)
                
        
        arquivoGrupo = nomeGrupos + '/resutados_grupo_' + str(i) + '_best.txt'
        arquivo = open(arquivoGrupo,"r")
        
        bestPoluicao = 0.0
        bestCpu = 0.0
        tam = 0.0
        
        for aux in arquivo:            
            linha = aux.split(' ')
            
            if float(linha[1]) <=  0.0:
                continue
            
            bestPoluicao += float(linha[1])
            bestCpu += float(linha[2])
            tam += 1.0
            
        arquivo.close()
        
        if tam > 0.0:
        
            bestPoluicao /= tam
            bestCpu /= tam
        
        
        resultadosGrupo = resultadosLiteratura[resultadosLiteratura.Grupo==int(i)]
        
        
        resultadosGrupo = resultadosGrupo.iloc[0]
        
        gapAvg = ((poluicao - resultadosGrupo[1])/resultadosGrupo[1]) * 100.0
        gapBest = ((bestPoluicao - resultadosGrupo[6])/resultadosGrupo[6]) * 100.0
        
        resultados.append((int(i), data, resultadosGrupo[1], resultadosGrupo[2], resultadosGrupo[3], 
            resultadosGrupo[4], resultadosGrupo[5], resultadosGrupo[6], resultadosGrupo[7], resultadosGrupo[8],
            round(poluicao,1), round(gapAvg,1), round(tempo,3), math.ceil(float(i)/5.0), round(veiculosUsados,1), round(minVeiculos,1), round(solucoesInviaveis,1), round(bestPoluicao,1) ,round(bestCpu,3), round(gapBest,1), round(ultimaAtualizacao, 1) ,str(20), str(int(tam)),
            round(vetPoluicaoMv[0], 1), round(vetPoluicaoMv[1], 1),round(vetPoluicaoMv[2], 1),round(vetPoluicaoMv[3], 1),round(vetPoluicaoMv[4], 1),round(vetPoluicaoMv[5], 1),round(vetPoluicaoMv[6], 1),round(vetPoluicaoMv[7], 1), 
            round(vetTempoMv[0], 4), round(vetTempoMv[1], 4),round(vetTempoMv[2], 4),round(vetTempoMv[3], 4),round(vetTempoMv[4], 4),round(vetTempoMv[5], 4),round(vetTempoMv[6], 4),round(vetGapMv[7], 2)))

resultadosDF = pd.DataFrame(resultados, columns=['grupo', 'data', 'poluicaoL', 'tempoL', 'veiculosDisL', 'veiculosUsadosL', 'minVeiculosL', 'poluicaoBestL', 'instanciasL', 'viaveisL',
                                'poluicao','gapAvg',  'tempoAvg', 'veiculosDis', 'veiculosUsados','minVeiculos' ,'solucoesInviaveis','poluicaoBest' ,'tempoBest', 'gapBest', 'ultimaAtualizacao','instancia', 'viavel', 
                                'poluicao_0', 'poluicao_1','poluicao_2','poluicao_3','poluicao_4','poluicao_5','poluicao_6','poluicao_7',
                                'tempo_0', 'tempo_1','tempo_2','tempo_3','tempo_4','tempo_5','tempo_6','tempo_7'])
#ramdom.close()
if not os.path.isfile(sys.argv[1]):

   resultadosDF.to_csv(sys.argv[1], header='column_names', index=False)

else: # else it exists so append without mentioning the header

   resultadosDF.to_csv(sys.argv[1], mode='a', header=False, index=False)
    
'''
