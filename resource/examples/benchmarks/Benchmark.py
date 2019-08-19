# Histogram this system and the reference systems

import glob
import os
import numpy as np
import matplotlib
import matplotlib.pyplot as plt

FileNames = []
This = []

# Find the latest benchmark from this system

FileList = glob.glob('*.bm')

if not FileList:
  print("Error: No benchmark results found")
  exit()


YoungestFile = max(FileList, key=os.path.getctime)

if YoungestFile != "":
  print("Found latest benchmark file: {}".format(YoungestFile))
  FileNames.append(YoungestFile)
  This.append("*** this ***")
  
  
# Find all reference systems
FileList = glob.glob('References/*.bm')
print(FileList)
for File in FileList:
  FileNames.append(File)
  This.append("")

print(FileNames)


# Extract all data
IsGood = []
HostNames = []
CPUNames = []
OSNames = []
OSVersions = []
CosimaSingles = []
CosimaMultiples = []

import re
for f in range(0, len(FileNames)):
  FileName = FileNames[f]
  IsGood.append(True)
  HostNames.append("-")
  CPUNames.append("-")
  OSNames.append("-")
  OSVersions.append("-")
  CosimaSingles.append(0)
  CosimaMultiples.append(0)
  with open(FileName) as File:
    CPUName = ""
    HostName = ""
    OSName = ""
    OSVersion = ""
    CosimaSingle = ""
    CosimaMultiple = ""
    for Line in File:
      if "HostName" in Line:
        Split = Line.split(':')
        if len(Split) > 1:
          HostName = Split[1].strip()
      if "CPUName" in Line:
        Split = Line.split(':')
        if len(Split) > 1:
          CPUName = Split[1].strip()
      if "OSFlavour" in Line:
        Split = Line.split(':')
        if len(Split) > 1:
          OSName = Split[1].strip()
      if "OSVersion" in Line:
        Split = Line.split(':')
        if len(Split) > 1:
          OSVersion = Split[1].strip()
      if "EventsPerSecondCosimaSingle" in Line:
        Split = Line.split(':')
        if len(Split) > 1:
          CosimaSingle = Split[1].strip()
      if "EventsPerSecondCosimaMultiple" in Line:
        Split = Line.split(':')
        if len(Split) > 1:
          CosimaMultiple = Split[1].strip()
    
    if HostName:
      HostNames[f] = HostName
    else:
      IsGood[f] = False
      
    if CPUName:
      CPUNames[f] = CPUName
    else:
      IsGood[f] = False
      
    if OSName:
      OSNames[f] = OSName
    else:
      IsGood[f] = False
    
    if OSVersion:
      OSVersions[f] = OSVersion
    else:
      IsGood[f] = False
    
    if CosimaSingle:
      CosimaSingles[f] = int(float(CosimaSingle))
    else:
      IsGood[f] = False
    
    if CosimaMultiple:
      CosimaMultiples[f] = int(float(CosimaMultiple))
    else:
      IsGood[f] = False
      
      

# Sorting:
IsGood.sort(key=dict(zip(IsGood, CosimaMultiples)).get)
This.sort(key=dict(zip(This, CosimaMultiples)).get)
HostNames.sort(key=dict(zip(HostNames, CosimaMultiples)).get)
CPUNames.sort(key=dict(zip(CPUNames, CosimaMultiples)).get)
OSNames.sort(key=dict(zip(OSNames, CosimaMultiples)).get)
OSVersions.sort(key=dict(zip(OSVersions, CosimaMultiples)).get)
CosimaSingles.sort(key=dict(zip(CosimaSingles, CosimaMultiples)).get)
CosimaMultiples.sort(key=dict(zip(CosimaMultiples, CosimaMultiples)).get)
      

print(IsGood)
print(CPUNames)
print(OSNames)
print(OSVersions)
print(CosimaSingles)
print(CosimaMultiples)


# Plot histogram

Labels = []
for f in range(0, len(FileNames)):
  Labels.append("{}\n{}\n{} {}\n{}".format(HostNames[f], CPUNames[f], OSNames[f], OSVersions[f], This[f]))

matplotlib.rcParams.update({'font.size': 20})

x = np.arange(len(Labels))  # the label locations
width = 0.35  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(x - width/2, CosimaSingles, width, label='Single Core')
rects2 = ax.bar(x + width/2, CosimaMultiples, width, label='All Cores')

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Events / second')
ax.set_title('Cosima Simulation Benchmark')
ax.set_xticks(x)
ax.set_xticklabels(Labels)
ax.legend()

def autolabel(rects):
  for rect in rects:
    height = rect.get_height()
    ax.annotate('{}'.format(height),
      xy=(rect.get_x() + rect.get_width() / 2, height),
      xytext=(0, 3),  # 3 points vertical offset
      textcoords="offset points",
      ha='center', va='bottom')


autolabel(rects1)
autolabel(rects2)

fig.set_size_inches(20, 14)
#fig.tight_layout()

plt.grid(True, axis='y')
plt.show()



exit()  


