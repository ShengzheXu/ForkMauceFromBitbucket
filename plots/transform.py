#!/usr/bin/env python

import sys
import math
from os import listdir
from os.path import isfile, join

if len(sys.argv) < 2:
    print("usage: " + sys.argv[0] + " input_file [input_file ...] output_file")
    sys.exit(0)

del sys.argv[0]

output = sys.argv[-1]
del sys.argv[-1]

files = sys.argv

avgdata = []

firstFile = True;
for f in files:
    with open(f, 'r') as file:
        # read a list of lines into data
        data = file.readlines()
    print(data[-1])

    if firstFile:
        firstFile = False
        for i in range(len(data)):
            x = data[i].split()
            avgdata.append([i] + [float(x[1])] + [float(v) ** 2 for v in x[2:]])
    else:
        if len(data) != len(avgdata):
            print("The length of file " + f + " is " + str(len(data)) +
                  ", different from the previously read " + str(len(avgdata)))
            sys.exit(1)
        for i in range(len(data)):
            x = data[i].split()
            avgdata[i][1] += float(x[1])
            avgdata[i][2:] = [v + float(v1) ** 2 for v, v1 in zip(avgdata[i][2:], x[2:])]

for i in range(len(avgdata)):
    avgdata[i][1:] = [v / len(files) for v in avgdata[i][1:]]
    avgdata[i][2:] = [math.sqrt(v) for v in avgdata[i][2:]]
    avgdata[i] = " ".join([str(v) for v in avgdata[i]])

# and write everything back
with open(output, 'w') as file:
    file.write("\n".join(avgdata))
