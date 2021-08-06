import sys
import pandas as pd
import matplotlib.pyplot as plt
from glob import glob

i = 0
#for name in glob("cmake-build-debug/2optSolRoute*.csv"):
name = sys.argv[1]
if not name:
    print("missing file path")
    exit()
coord = pd.read_csv(name, header=None, delim_whitespace=True)
print(coord)
#
flag = coord[0].values
x = coord[1].values
y = coord[2].values
#new_x, new_y = zip(*sorted(zip(x, y)))
print(y)
demand = coord[3].values
#y = coord['y'].values
#no = coord['no'].values
#estacao = coord['estacao'].values
tiltedX = []
tiltedY = []
plt.figure()
def connectpoints(x,y,p1,p2):
    x1, x2 = x[p1], x[p2]
    y1, y2 = y[p1], y[p2]
    plt.plot([x1,x2],[y1,y2],'-', color='lightgray')

floatx = [float(i) for i in x[1:15]]
floaty = [float(i) for i in y[1:15]]
#plt.plot([float(i) for i in x[:15]],[float(i) for i in y[:15]], 'ro-')
# for i in range(len(floatx)):
    # for j in range(len(floaty)):
        # connectpoints(floatx, floaty, i, j)

plt.legend(prop={'size': 6})
plt.legend()
plt.xlabel("x")
plt.ylabel("y")
demand_float = [float(x) for x in demand[1:15]]
for j in range(len(demand_float)):
    if demand_float[j] != 0:
        plt.annotate(demand_float[j],(floatx[j],floaty[j]),color='r')
for i,j in enumerate(x):
    if flag[i].startswith('S'):
        color = 'green'
        marker = 'D'
    elif flag[i].startswith('C'):
        color = 'black'
        marker = 'o'
    elif flag[i].startswith('F'):
        color = 'blue'
        marker = '^'
    elif flag[i].startswith('D'):
        color = 'red'
        marker = 's'
    else:
        continue;
    plt.scatter(float(x[i]), float(y[i]), color=color, s=100, marker=marker)
plt.savefig('test' + 'vai'+ '.png')
