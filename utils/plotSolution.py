import sys
from csv import reader
import pandas as pd
import matplotlib.pyplot as plt
import math
from glob import glob

def drawArrow(x1, x2, y1, y2, width, color, ls):
    dist = math.dist((x1, y1), (x2, y2))
    dx = 1*(x2 - x1)
    dy = 1*(y2 - y1)
    if not dx or not dy:
        return;
    dx *= (1 - 0.5/abs(dx)) #assuming length != 0
    dy *= (1 - 0.5/abs(dy))
    plt.arrow(x1, y1, dx, dy, head_width=1.0, length_includes_head=True, edgecolor=None, color=color, zorder=1, label=dist, ls=ls, aa=True, alpha=0.8, overhang=0.5)
    return dist

satColors = ['#2ff3e0', '#f8d210', '#fa26a0', '#f51720']
plt.figure()
plt.grid(visible=True, alpha=0.3)
plt.legend(prop={'size': 6})
plt.legend()
plt.xlabel("x")
plt.ylabel("y")

solutionPath = sys.argv[1]
instancePath = sys.argv[2]
if not solutionPath or not instancePath:
    print("usage: plotSolution solution.txt instance.txt' \nE.g.: plotSolution solution.txt ./instancias/Set1/....1.txt")
    exit(0)

#df = pd.read_csv(solutionPath, delimiter=',', header=None, skipfooter=3)
# User list comprehension to create a list of lists from Dataframe rows
#routes = [list(row) for row in df.values]
# read csv file as a list of lists
routes = []
with open(solutionPath, 'r') as read_obj:
    # pass the file object to reader() to get the reader object
    csv_reader = reader(read_obj)
    # Pass reader object to list() to get a list of lists
    routes = list(csv_reader)
    print(routes)
routes = [[int(x) for x in route if x] for route in routes]
print(routes)

coord = pd.read_csv(instancePath, header=None, names=['code', 'x', 'y', 'demand'], delim_whitespace=True, skipfooter=3);
# extracting
#   the type ((D)epot, (S)at, (C)lient, (F)Recharging Station
#   the number (id)
#   the x, y and the demand
nodeId = [int(x[1:]) for x in coord['code'].values]
nodeType = [x[0] for x in coord['code'].values]
#nodeType = coord['code'].values[:-3]
x = [float(x) for x in coord['x'].values]
y = [float(x) for x in coord['y'].values]
demand = [float(x) for x in coord['demand'].values]
for i in range(len(nodeId)):
    if demand[i] != 0:
        plt.annotate(round(demand[i]), (x[i], y[i]), color='r', zorder=10)
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
for route in routes:
    if len(route) == 0:
       continue
    if nodeType[route[0]] == 'S':
        color = satColors[route[0]-1]
    else:
        color = '#111111'
    for i in range(0, len(route)-1):
        drawArrow(x[route[i]], x[route[i+1]], y[route[i]], y[route[i+1]], 0, color, '-')

plt.savefig('solution' + ''+ '.png')
