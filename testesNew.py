import os
import subprocess
import sys
import pandas as pd
import math
import time

if(len(sys.argv) < 2 and len(sys.argv) > 3):
    print("NUMERO DE ARGUMENTOS ESTA ERRADO.\npython3 teste.py caminho")
    exit(-1)
    
print("PID: ",os.getpid())
print("\n")

#tamanhoInst = ['5', '10', '15']
#tamanhoInst = ['100_0_0']
tamanhoInst = ['100_0_1']
numExecucoes = 10
caminhoDir = str(sys.argv[1])
metodo = "G"
parametros = " --pasta '" + caminhoDir+ "' --mt '"+str(metodo) +"' --resulCSV 'resultados.csv' --execTotal "+str(numExecucoes)+ " --execAtual "


diretorioIni = 'instancias/2e-vrp-tw/'
instanciasVet = []
numExecucoesVet = []

for i in tamanhoInst:
    caminho = diretorioIni + 'Customer_' + str(i) + '/'
    files = os.listdir(caminho)
    files.sort()

    for f in files:
        instanciasVet.append(caminho+f)
        numExecucoesVet.append(0)


instancias = pd.DataFrame()
instanciasCsv = caminhoDir+"/instancias.csv"

if(len(sys.argv) == 3):
    instancias = pd.read_csv(instanciasCsv)
    print(instancias)
    print("\n\n")
    #instancias = instancias.drop(columns=['0'])
    
else:
    temp = {'instancia' : instanciasVet, 'prox' : numExecucoesVet}
    instancias = pd.DataFrame(data=temp)
    instancias.to_csv(instanciasCsv, index=False)
    

sys.stdout.flush()


        
        
n = len(instancias.index)
for j in range(n):
    
    instancia = instancias.loc[j,'instancia']
    start = int(instancias.loc[j,'prox'])
    
    for i in range(start, numExecucoes, 1):

      strExecutavel = caminhoDir+'//run ' + str(instancia) + parametros + str(i)
      os.system(strExecutavel)
      #print(strExecutavel)
      instancias.loc[j,'prox'] = i+1
      instancias.to_csv(instanciasCsv, index=False)
      
    #print(instancias)
    print("\n\n")
    sys.stdout.flush()
