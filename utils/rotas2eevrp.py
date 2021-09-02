import pandas as pd
import matplotlib.pyplot as plt
from glob import glob

i = 0
for name in glob("route*.csv"):
    coord = pd.read_csv(name)
      
    node = coord['node'].values
    x = coord['x'].values
    y = coord['y'].values
    type = coord['type'].values
    tiltedX = []
    tiltedY = []
    for n in range(len(node)):
        #if type[n]:
            #tiltedX.append(x[n]+2)
            #tiltedY.append(y[n]+2)
        #else:
        tiltedX.append(x[n])
        tiltedY.append(y[n])



    #plt.figure()
    if node[0] == 0:
        plt.plot(tiltedX,tiltedY,'D--',label='Veículo ' + str(i))
    else:
        plt.plot(tiltedX,tiltedY,'o-',label='Veículo ' + str(i))
    plt.legend(prop={'size': 6})
    plt.xlabel("x")
    plt.ylabel("y")
    for j in range(len(node)):
        #if estacao[j] == 1:
            #plt.annotate(no[j],(tiltedX[j],tiltedY[j]),color='r')
        #else:
        plt.annotate(node[j],(x[j],y[j]))
        if type[j] == "rs":
            print("uai");
            plt.scatter(x[j], y[j], color='r', s=100, marker='^')
    i+=1
    #plt.savefig('test' + str(i) + '.png')
plt.savefig('test' + 'vai'+ '.png')
