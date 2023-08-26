import os
import subprocess
import sys
import pandas as pd
import math
import time
# Color = Enum('Color', ['RED', 'GREEN', 'BLUE'], start=0)

'''
if(len(sys.argv) < 2 and len(sys.argv) > 3):
    print("NUMERO DE ARGUMENTOS ESTA ERRADO.\npython3 teste.py caminho")
    exit(-1)
'''

def dictToString(parametros):

	saida = ""

	for it in parametros.items():
		saida = saida + "--" + it[0] +  " "+  str(it[1]) + " "
	
	return saida


   
print("PID: ",os.getpid())
print("\n")

parametros = {"alphaSeg", "betaPrim", "difBest", "torneio", "taxaRm", "fatNumCh", "mip_presolve", "mip_cuts", "mip_restTempo", "mipGap"}

parametrosIni = {
	"alphaSeg": 0.45,
	"betaPrim": 0.8,
	"difBest": 0.04,
	"torneio": 1,
	"taxaRm": 0.25,
	"fatNumCh": 2,
	"mip_presolve": 1,
	"mip_cuts": 2,
	"mip_restTempo": 0,
	"mipGap": 0.0
}



parametrosDomi = {
    "alphaSeg":             [0.005, 0.01, 0.05, 0.1, 0.15, 0.25, 0.35, 0.65, 0.9],
	"betaPrim":             [0.15, 0.25, 0.4, 0.5, 0.65, 0.75, 0.85, 0.9],
	"difBest":              [0.005, 0.01, 0.015, 0.02, 0.03, 0.05, 0.07, 0.1, 0.15, 0.2, 0.3, 0.4, 0.6, 0.8],
	"torneio":              [1],
	"taxaRm":               [0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6],
	"fatNumCh":             [1, 1.5, 2.5, 3],
	"mip_presolve":         [-1, 2],
	"mip_cuts":             [-1, 1, 3],
	"mip_restTempo":        [0],
	"mipGap":               [0.01, 0.02, 0.03, 0.04, 0.05]
}

num = 0

config = []
config.append(dictToString(parametrosIni))



for paramNome in parametros:
	paramList = parametrosDomi[paramNome]

	for val in paramList:
		parametrosCp = parametrosIni.copy()
		parametrosCp.pop(paramNome)
		parametrosCp[paramNome] = val		
		config.append(dictToString(parametrosCp))
		
		


print("Num config: ", len(config))


configPd = pd.DataFrame(config, columns=["config"])


configParam = [""]
configValue = [0.0]

for paramNome in parametros:
	paramList = parametrosDomi[paramNome]

	for val in paramList:
		configParam.append(paramNome)
		configValue.append(float(val))

configPd["difParam"] = configParam
configPd["difValue"] = configValue

print(configPd)

configPd.to_csv("configSensi.csv")

exit()

#instanciasTeste = "instanciasTeste3.txt"
instanciasTeste = ""

#tamanhoInst = ['15']#, '100_0']
#tamanhoInst = ['5', '10', '15'] #, '100_0']#, '100_1', '100_2']
#tamanhoInst = ['15', '100_0']
#tamanhoInst = ['100_0_1']
#tamanhoInst = ['100_0', '100_1', '100_2']
tamanhoInst = ['100_2']
#tamanhoInst = ['100_2']
#tamanhoInst = ['100_1', '100_2']



numExecucoes = 10
caminhoDir = str(sys.argv[1])
#caminhoDir = ''
#IRACE: --alphaSeg 0.05 --betaPrim 0.8 --difBest 0.015 --numItIG 3000 --torneio 0 --taxaRm 0.15 --fatNumCh 2


#parametros = " --pasta '" + caminhoDir+ "' --resulCSV 'resultados.csv' --execTotal "+str(numExecucoes)+ " --alphaSeg 0.35 --betaPrim 0.15 --difBest 0.03 --numItIG 3000 --torneio 1 --taxaRm 0.2 --fatNumCh 3 --mip_presolve -1 --mip_cuts 1 --mip_restTempo 1 --mip_gap 0.00 --mip_outputFlag 0" + " --execAtual "

parametros = " --pasta '" + caminhoDir+ "' --resulCSV 'resultados.csv' --execTotal "+str(numExecucoes)+ " --alphaSeg 0.45 --betaPrim 0.8 --difBest 0.04 --numItIG 3000 --torneio 1 --taxaRm 0.25 --fatNumCh 2 --mip_presolve 1 --mip_cuts 2 --mip_restTempo 0 --mip_gap 0.0 --mip_outputFlag 0" + " --execAtual "

print("PARAMETROS: \n", parametros, "\n")










#parametros = " --pasta '" + caminhoDir+ "' --resulCSV 'resultados.csv' --execTotal "+str(numExecucoes)+ " --alphaSeg 0.35 --betaPrim 0.15 --difBest 0.03 --numItIG 3000 --torneio 1 --taxaRm 0.2 --fatNumCh 3 --mip_presolve -1 --mip_cuts 1 --mip_restTempo 1 --mip_gap 0.00 --mip_outputFlag 0" + " --execAtual "


diretorioIni = 'instancias/2e-vrp-tw/'
instanciasVet = []
numExecucoesVet = []



if not instanciasTeste:

	strInstancias = ""
	for i in tamanhoInst:
    		strInstancias += i + " "

	#print("INSTANCIAS: ", strInstancias, "\n")
	files = []
	for i in tamanhoInst:
		#print('Customer_' + str(i))
		caminho = diretorioIni + 'Customer_' + str(i) + '/'
		print(caminho)
		files = os.listdir(caminho)
		files.sort()

		for f in files:
			instanciasVet.append(caminho+f)
			numExecucoesVet.append(0)

else:
	print(instanciasTeste)
	inst = pd.read_csv(instanciasTeste)
	n = len(inst.index)
	strInst = "\t"
	for i in range(n):
		strTemp = inst.loc[i,'inst']
		if strTemp[0] == '#':
			continue
		
		strInst = strInst + inst.loc[i,'inst'] + "\n\t"
		caminho = diretorioIni + 'all/' + inst.loc[i,'inst'] + '.txt'
		instanciasVet.append(caminho)
		numExecucoesVet.append(0)
		
	print('Instancias:\n', strInst)


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
