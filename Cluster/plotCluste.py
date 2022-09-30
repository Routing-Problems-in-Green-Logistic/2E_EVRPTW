import pandas as pd
import matplotlib.pyplot as plt
import math

def drawArrow(x1, x2, y1, y2, width, color, ls):
    dist = math.dist((x1, y1), (x2, y2))
    dx = 1*(x2 - x1)
    dy = 1*(y2 - y1)
    #if not dx or not dy:
       # return
    zero = 1E-7

    if not dx:
        dx = zero
    else:
        dx *= (1 - 0.5/abs(dx)) #assuming length != 0

    if not dy:
        dy = zero
    else:
        dy *= (1 - 0.5/abs(dy))

    plt.arrow(x1, y1, dx, dy, head_width=0.25, length_includes_head=True, edgecolor=None, color=color, zorder=1,  ls=ls, aa=True, alpha=0.8, overhang=0.5)

satColors = ['#2ff3e0', '#f8d210', '#fa26a0', '#f51720', '#e1ad01', '#909090', '#ff7538', '#00a500', '#004953', '#aa6c39']

fig = plt.figure(dpi=800)
ax = fig.gca()
plt.grid(visible=True, alpha=0.3)

dados = pd.read_csv("cluster.csv", names=['i', 'x', 'y', 'cluster', 'tipo'])
arcos = pd.read_csv("arcos.csv", names=['xi', 'yi', 'xj', 'yj'])
print(arcos)

#print(dados)

for i in range(len(arcos.index)):

    x = [float(arcos.loc[i, 'xi']), float(arcos.loc[i, 'xj'])]
    y = [float(arcos.loc[i, 'yi']), float(arcos.loc[i, 'yj'])]
    plt.plot(x, y, color="#000000")
   # drawArrow(x[0], x[1], y[0], y[1], 0, "#000000", "-")
    
    

print('\n')
for i in range(len(dados.index)):
    
    x = float(dados.loc[i, 'x'])
    y = float(dados.loc[i, 'y'])
    no = int(dados.loc[i,'i'])
    tipo = str(dados.loc[i,'tipo'])
    tipo = tipo.replace(" ", "")   
    
    if dados.loc[i, 'cluster'] != -1:
        color = satColors[dados.loc[i, 'cluster']]
        
    else:
        color = "#000000"
        
    print(no, ': ', tipo, ' ', tipo)
    
    #if(tipo != 'X'):
    #    plt.annotate(no, (x,y), color='r', zorder=10)
    
    plt.scatter(x, y, c=color, marker=tipo, s=65, zorder= 5)
    
plt.savefig('fig_' + ''+ '.png')    
    
    

    
