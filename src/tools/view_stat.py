import numpy as np
import matplotlib.pyplot as plt
import sys
import math
from matplotlib.ticker import MaxNLocator

if(len(sys.argv)<2):
	sys.exit(0)

result = np.loadtxt(sys.argv[1])

step = np.int_(result[0])
time = result[1]
volume_border = np.array(result[2]) - np.array(result[3])

fig, ax1 = plt.subplots()
color = 'tab:red'
ax1.set_xlabel('Iteration')
ax1.xaxis.set_major_locator(MaxNLocator(integer=True))

ax1.set_ylabel('Time of the Iteration (in s)', color=color)
ax1.plot(step, time, "-o", color=color)
ax1.tick_params(axis='y', labelcolor=color)

ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

color = 'tab:blue'
ax2.set_ylabel('Volume of the border', color=color)  # we already handled the x-label with ax1
ax2.plot(step, volume_border, "-o", color=color)
ax2.tick_params(axis='y', labelcolor=color)

fig.tight_layout()  # otherwise the right y-label is slightly clipped
plt.show()
