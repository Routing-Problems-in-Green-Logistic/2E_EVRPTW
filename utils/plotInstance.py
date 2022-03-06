import sys
import pandas as pd
import matplotlib.pyplot as plt
import random
import math
from glob import glob

satColors = ['#2ff3e0', '#f8d210', '#fa26a0', '#f51720']

def mean(p1, p2):
    return ((p1[0] + p2[0])/2, (p1[1] + p2[1])/2)

def drawArrow(x1, x2, y1, y2, width, color, ls):
    dist = math.dist((x1, y1), (x2, y2))
    dx = 1*(x2 - x1)
    dy = 1*(y2 - y1)
    #dx = 0.95*(x2 - x1)
    #dy = 0.95*(y2 - y1)
    plt.arrow(x1, y1, dx, dy, head_width=0, length_includes_head=True, edgecolor=None, color=color, zorder=1, label=dist, ls=ls, aa=True, alpha=0.8)
    return dist

plt.figure()
plt.grid(visible=True, alpha=0.3)
plt.legend(prop={'size': 6})
plt.legend()
plt.xlabel("x")
plt.ylabel("y")

path = sys.argv[1]
if not path:
    exit(255)
coord = pd.read_csv(path, header=None, names=['code', 'x', 'y', 'demand'], delim_whitespace=True);
# extracting
#   the type ((D)epot, (S)at, (C)lient, (F)Recharging Station
#   the number (id)
#   the x, y and the demand
print(coord['code'].values)
print(coord['x'].values)
print(coord['y'].values)
print(coord['demand'].values)
nodeId = [int(x[1:]) for x in coord['code'].values[:-3]]
nodeType = [x[0] for x in coord['code'].values[:-3]]
#nodeType = coord['code'].values[:-3]
x = [float(x) for x in coord['x'].values[:-3]]
y = [float(x) for x in coord['y'].values[:-3]]
demand = [float(x) for x in coord['demand'].values[:-3]]
print("nodeId", ' ')
print(nodeId)
print("nodeType", ' ')
print(nodeType)
print("x", ' ')
print(x)
print("y", ' ')
print(y)
print("demand", ' ')
print(demand)

#could have converted to float on the go, but...

for i in range(len(nodeId)):
    if demand[i] != 0:
        plt.annotate(round(demand[i]), (x[i], y[i]), color='r')

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
    for j in range(i, len(nodeId)):
        if i != j:
            if nodeType[i] == 'S' and (nodeType[j] == 'C' or nodeType[j] == 'F'):
                dist = drawArrow(x[i], x[j], y[i], y[j], width=0.01, color=color, ls='-')
            if (nodeType[i] == 'D' and nodeType[j] == 'S'):
                dist = drawArrow(x[i], x[j], y[i], y[j], width=0.01, color='#c1c1c1', ls=':')
                #plt.annotate(round(dist), mean((x[i], y[i]), (y[j],y[j])), color='b')
    plt.scatter(x[i], y[i], c=color, marker=marker, s=25, zorder= 5, edgecolors='#222222')
#plt.plot(x, y)
plt.savefig('instance' + ''+ '.png')

"""
for name in glob(path + "/route*.csv"):
    col = (random.random(), random.random(), random.random())
    coord = pd.read_csv(name)

    node = coord['node'].values
    x = coord['x'].values
    y = coord['y'].values
    flag = coord['type'].values
    demand = coord['demand'].values

    for i in range(1, len(node)):
        drawArrow(x[i-1], x[i], y[i-1], y[i], 0.3, col)
        """

plt.savefig('plotInstance' + '.png')
