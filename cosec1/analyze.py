import matplotlib.pyplot as plt
import numpy as np
import sys

shift = 10
data = np.array([int('0x' + line, base=16) for line in sys.stdin])
data.sort()

prev = 0
start = 0
for x in data:
    if x - prev > 1 << shift:
        print(f'{start:06x} -> {prev:06x}')
        start = x
    prev = x

data = data >> shift
plt.hist(data, bins=np.arange(data.min(), data.max()+1))
plt.savefig(sys.argv[1])
