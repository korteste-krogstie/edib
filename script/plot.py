from netCDF4 import Dataset
import numpy as np
from matplotlib import pylab as plt
from mpl_toolkits.mplot3d import Axes3D
import datetime as dt

def eit_plot(X,Y,T,N,idx0,idx1):
	fig = plt.figure()
	ax = fig.gca(projection='3d')

	plt.xlabel("xpos[m]")
	plt.ylabel("ypos[m]")
	for i in range(idx0,idx1):
		n = N[i]
		ax.plot(X[i][0:n],Y[i][0:n],((T[i][0:n]-T[i][0])/3600)) #[dt.datetime.fromtimestamp(T[i][j]) for j in range(0,n)])
	plt.show()
	plt.close()

if __name__ == "__main__":
	import sys
	with Dataset(sys.argv[1]) as ds:
		X = ds.variables.get("xpos")
		Y = ds.variables.get("ypos")
		T = ds.variables.get("tpos")
		N = ds.variables.get("num")
		eit_plot(X,Y,T,N,int(sys.argv[2]),int(sys.argv[3]))
