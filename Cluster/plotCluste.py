import pandas as pd
import matplotlib.pyplot as plt

satColors = ['#2ff3e0', '#f8d210', '#fa26a0', '#f51720', '#e1ad01', '#909090', '#ff7538', '#00a500', '#004953', '#aa6c39']

fig = plt.figure(dpi=800)
ax = fig.gca()
plt.grid(visible=True, alpha=0.3)

dados = pd.read_csv("cluster.csv", names=['i', 'x', 'y', 'cluster', 'tipo'])
print(dados)

print('\n')
for i in range(len(dados.index)):
    
    x = int(dados.loc[i, 'x'])
    y = int(dados.loc[i, 'y'])
    no = int(dados.loc[i,'i'])
    tipo = str(dados.loc[i,'tipo'])
    tipo = tipo.replace(" ", "")   
        
    color = satColors[dados.loc[i, 'cluster']]
    print(no, ': ', tipo, ' ', tipo)
    
    #if(tipo != 'X'):
    #    plt.annotate(no, (x,y), color='r', zorder=10)
    
    plt.scatter(x, y, c=color, marker=tipo, s=65, zorder= 5)
    
plt.savefig('fig_' + ''+ '.png')    
    
    

    
