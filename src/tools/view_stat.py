import numpy as np
import matplotlib.pyplot as plt
import sys
import math
from matplotlib.ticker import MaxNLocator

if(len(sys.argv)<2):
	sys.exit(0)

result = np.loadtxt(sys.argv[1])
print(result)

step_max = 19

step = np.int_(result[0][0:step_max])
time = result[1][0:step_max]
volume_border = np.array(result[2][0:step_max]) - np.array(result[3][0:step_max])


print(volume_border)

fig, ax1 = plt.subplots()
color = 'tab:red'
ax1.set_xlabel('Iteration')
ax1.xaxis.set_major_locator(MaxNLocator(integer=True))

ax1.set_ylabel('Iteration processing time (in sec)', color=color)
ax1.semilogy(step, time, "-o", color=color)
# ax1.tick_params(axis='y', labelcolor=color)
# ax1.set_ylim([0, 7])

ax2 = ax1.twinx()  # instantiate a second axes that shares the same x-axis

color = 'tab:blue'
ax2.set_ylabel('Border volume', color=color)  # we already handled the x-label with ax1
ax2.semilogy(step, volume_border, "-o", color=color)
# ax2.set_ylim([0, 70])
ax2.axhline(y=0, color='k')

fig.tight_layout()  # otherwise the right y-label is slightly clipped
plt.show()
