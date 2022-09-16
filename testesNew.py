import os
import subprocess
import sys
import pandas as pd
import math
import time

if(len(sys.argv) != 2):
    print("NUMERO DE ARGUMENTOS ESTA ERRADO.\npython3 teste.py caminho")
    exit(-1)

numExecucoes = 10
caminhoDir = str(sys.argv[1])

parametros = " --pasta '" + caminhoDir +"' --resulCSV 'resultados.csv' --execTotal "+str(numExecucoes)+ " --execAtual "



#tamanhoInst =  ['5', '10', '15', '100_0', '100_1', '100_2']
tamanhoInst =  ['5']
diretorioIni = 'instancias/2e-vrp-tw/'
instancias = []

for i in tamanhoInst:
    caminho = diretorioIni + 'Customer_' + str(i) + '/'
    files = os.listdir(caminho)
    files.sort()

    for f in files:
        instancias.append(caminho+f)
        
        

for instancia in instancias:
    
    for i in range(numExecucoes):

      strExecutavel = caminhoDir+'//run ' + str(instancia) + parametros + str(i)
      os.system(strExecutavel)
      #print(strExecutavel)
      
    print("\n\n")


