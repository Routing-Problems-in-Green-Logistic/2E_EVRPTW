import sys
from csv import reader
import pandas as pd
import matplotlib.pyplot as plt
import math
import numpy
from glob import glob

def mod(x):
    if x < 0:
        return -x
    else:
        return x

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
    return dist

satColors = ['#2ff3e0', '#f8d210', '#fa26a0', '#f51720', '#2ff3e0', '#2ff3e0', '#2ff3e0', '#2ff3e0', '#2ff3e0', '#2ff3e0']
fig = plt.figure(dpi=800)
ax = fig.gca()


plt.grid(visible=True, alpha=0.3)
#plt.legend(prop={'size': 6})
#plt.legend()
#plt.xlabel("x")
#plt.ylabel("y")

if len(sys.argv) < 3:
    print('Utilizacao:\npython3 plotSolution.py instancia.txt solucao.txt <nos para excluir>')
    exit(-1)

solutionPath = sys.argv[2]
instancePath = sys.argv[1]
remover = sys.argv[3:]
remover = [int(x) for x in remover]
removereRota = []


print(remover)

if not solutionPath or not instancePath:
    print("usage: plotSolution solution.txt instance.txt' \nE.g.: plotSolution solution.txt ./instancias/Set1/....1.txt")
    exit(0)

coord = pd.read_csv(instancePath, header=0, delim_whitespace=True)

removeNo = [int(1) for i in range(coord.size)]


for i in remover:
    if coord.iloc[i].code == 'S':
        removereRota.append(i)

print(removereRota)

file = open(solutionPath, 'r')
routes = []

for linha in file:

    rota = []
    split = linha.split()    
    split = [int(i) for i in split]

    next = False

    for i in split:

        if i in removereRota:

            next = False
            break

        if i in remover:    
            continue            
        next = True
        removeNo[i] = 0
        rota.append(int(i))

    if next:
        routes.append(rota)

nodeId = coord.index.values



nodeType = [x[0] for x in coord['code'].values]



#nodeType = coord['code'].values[:-3]
x = [float(x) for x in coord['x'].values]


y = [float(x) for x in coord['y'].values]


demand = [float(x) for x in coord['demanda'].values]
demand = demand[1:]

plt.gca().set_aspect('equal', adjustable='box')


for i in range(len(nodeId)):
    
    if removeNo[i] == 1:
        continue

    plt.annotate(nodeId[i], (x[i], y[i]), color='r', zorder=10)
    print(nodeId[i], ": ", x[i], " ", y[i])

    color = ''
    marker = ''
    if nodeType[i] == 'C':
        marker = 'o'
        color = '#000000'
    if nodeType[i] == 'F':
        marker = '^'
        color = '#0000ff'
    if nodeType[i] == 'S':
        marker = 'D'
        #color = '#00ff00'
        color = satColors[i-1]
    if nodeType[i] == 'D':
        marker = 's'
        color = '#ff0000'
    plt.scatter(x[i], y[i], c=color, marker=marker, s=65, zorder= 5, edgecolors='#222222')


#print('*******************************************')

for route in routes:
    print(route)
    if len(route) == 0:
       continue
    if nodeType[route[0]] == 'S':
        color = satColors[route[0]-1]
    else:
        color = satColors[3]

    for i in range(0, len(route)-1):
        #print('color: ', str(color))
        print(route[i], ' ', route[i+1])
        drawArrow(x[route[i]], x[route[i+1]], y[route[i]], y[route[i+1]], 0, color, '-')

plt.savefig('solution__' + ''+ '.png')
