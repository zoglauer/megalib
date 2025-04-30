# Histogram this system and the reference systems

import glob
import os
import numpy as np
import argparse
import matplotlib
import matplotlib.pyplot as plt

Mode="c"

parser = argparse.ArgumentParser(description='View MEGAlib benchmarks')
parser.add_argument('-m', '--mode', default='c',help='c: Cosima, r: Revan, m: Mimrec, a: ResponseCreator')
parser.add_argument('-r', '--referencecount', type=int, default=7, help='Number of references to show (default: 7)')

args = parser.parse_args()

Mode = (args.mode).lower()
if Mode != 'c' and Mode != 'r' and Mode != 'm' and Mode != 'a':
  print("Error: The mode must be either \'c\', \'r\', \'m\', or \'a\'.")
  sys.exit(0)

ReferenceCount = args.referencecount
if ReferenceCount < 0:
  ReferenceCount = 0

FileNames = []
Reference = []


# Find the latest benchmark from this system

FileList = glob.glob('*.bm')

if not FileList:
  print("Error: No benchmark results found")
  exit()


YoungestFile = max(FileList, key=os.path.getctime)

if YoungestFile != "":
  print("Found latest benchmark file: {}".format(YoungestFile))
  FileNames.append(YoungestFile)
  Reference.append("*** this ***")
  
  
# Find all reference systems
FileList = glob.glob('References/*.bm')
print(FileList)
for File in FileList:
  FileNames.append(File)
  Reference.append("(reference)") # {}".format(File))

print(FileNames)


# Extract all data
IsGood = []
HostNames = []
CPUSockets = []
CPUNames = []
OSNames = []
OSVersions = []

RunSingle = []
RunMulti = []

import re
for f in range(0, len(FileNames)):
  FileName = FileNames[f]
  IsGood.append(True)
  HostNames.append("-")
  CPUSockets.append("-")
  CPUNames.append("-")
  OSNames.append("-")
  OSVersions.append("-")
  RunSingle.append(0)
  RunMulti.append(0)
  with open(FileName) as File:
    CPUSocket = ""
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
      if "CPUSockets" in Line:
        Split = Line.split(':')
        if len(Split) > 1:
          CPUSocket = Split[1].strip()
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
      if Mode == 'c':
        if "EventsPerSecondCosimaSingle" in Line:
          Split = Line.split(':')
          if len(Split) > 1:
            Single = Split[1].strip()
        if "EventsPerSecondCosimaMultiple" in Line:
          Split = Line.split(':')
          if len(Split) > 1:
            Multiple = Split[1].strip()
      elif Mode == 'r': 
        if "EventsPerSecondRevanSingle" in Line:
          Split = Line.split(':')
          if len(Split) > 1:
            Single = Split[1].strip()
        if "EventsPerSecondRevanMultiple" in Line:
          Split = Line.split(':')
          if len(Split) > 1:
            Multiple = Split[1].strip()
      elif Mode == 'm': 
        if "EventsPerSecondMimrecImagingSingle" in Line:
          Split = Line.split(':')
          if len(Split) > 1:
            Single = Split[1].strip()
        if "EventsPerSecondMimrecImagingMultiple" in Line:
          Split = Line.split(':')
          if len(Split) > 1:
            Multiple = Split[1].strip()
      elif Mode == 'a': 
        if "EventsPerSecondResponseCreatorSingle" in Line:
          Split = Line.split(':')
          if len(Split) > 1:
            Single = Split[1].strip()
        if "EventsPerSecondResponseCreatorMultiple" in Line:
          Split = Line.split(':')
          if len(Split) > 1:
            Multiple = Split[1].strip()
      
    
    if HostName:
      HostNames[f] = HostName
    else:
      IsGood[f] = False
      
    if CPUName:
      CPUNames[f] = CPUName
    else:
      IsGood[f] = False
      
    if CPUSocket:
      CPUSockets[f] = CPUSocket
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
    
    if Single:
      RunSingle[f] = int(float(Single))
    else:
      IsGood[f] = False
    
    if Multiple:
      RunMulti[f] = int(float(Multiple))
    else:
      IsGood[f] = False
      
      
print(len(Reference))
print(len(IsGood))


# Sorting:
#IsGood.sort(key=dict(zip(IsGood, RunMulti)).get)

IsGood = [x for _,x in sorted(zip(RunMulti, IsGood))]
Reference = [x for _,x in sorted(zip(RunMulti, Reference))]
HostNames = [x for _,x in sorted(zip(RunMulti, HostNames))]
CPUSockets = [x for _,x in sorted(zip(RunMulti, CPUSockets))]
CPUNames = [x for _,x in sorted(zip(RunMulti, CPUNames))]
OSNames = [x for _,x in sorted(zip(RunMulti, OSNames))]
OSVersions = [x for _,x in sorted(zip(RunMulti, OSVersions))]
RunSingle = [x for _,x in sorted(zip(RunMulti, RunSingle))]
RunMulti = [x for _,x in sorted(zip(RunMulti, RunMulti))]

# If we have more than (RefernceCount+1) first cull results which are within 5% of an higher value
HighestRef=-1
for i in range(len(Reference) - 1, -1, -1):
  if HighestRef < 0:
    if IsGood[i] == True:
      HighestRef = i
  else:
    if Reference[0] == "*** this ***":
      HighestRef = i
      continue

    if RunMulti[i] > 0.95*RunMulti[HighestRef]:
      IsGood[i] = False
      print("Deleting {}".format(CPUNames[i]))
    else: 
      HighestRef = i

Reference = [a for a, keep in zip(Reference, IsGood) if keep]
HostNames = [a for a, keep in zip(HostNames, IsGood) if keep]
CPUSockets = [a for a, keep in zip(CPUSockets, IsGood) if keep]
CPUNames = [a for a, keep in zip(CPUNames, IsGood) if keep]
OSNames = [a for a, keep in zip(OSNames, IsGood) if keep]
OSVersions = [a for a, keep in zip(OSVersions, IsGood) if keep]
RunSingle = [a for a, keep in zip(RunSingle, IsGood) if keep]
RunMulti = [a for a, keep in zip(RunMulti, IsGood) if keep]
IsGood = [a for a, keep in zip(IsGood, IsGood) if keep]


# If we have more than (RefernceCount+1) delete the weakest references
while len(Reference) > ReferenceCount+1:
  # Find which one to delete
  ToDelete = 0
  if Reference[0] == "*** this ***":
    ToDelete = 1
  print("Deleting {}".format(CPUNames[ToDelete]))
  del IsGood[ToDelete]
  del Reference[ToDelete]
  del HostNames[ToDelete]
  del CPUSockets[ToDelete]
  del CPUNames[ToDelete]
  del OSNames[ToDelete]
  del OSVersions[ToDelete]
  del RunSingle[ToDelete]
  del RunMulti[ToDelete]

# Some beautification
CPUNames = [ s.replace("Ryzen Threadripper", "Threadripper") for s in CPUNames ]
CPUNames = [ s.replace("Mobile Intel(R) Pentium(R) 4", "P4") for s in CPUNames ]


print(IsGood)
print(Reference)
print(CPUSockets)
print(CPUNames)
print(OSNames)
print(OSVersions)
print(RunSingle)
print(RunMulti)


# Plot histogram

Labels = []
for f in range(0, len(Reference)):
  if int(CPUSockets[f]) > 1:
    Labels.append("{}\n{}x {}\n{} {}\n{}".format(HostNames[f], CPUSockets[f], CPUNames[f], OSNames[f], OSVersions[f], Reference[f]))
  else:
    Labels.append("{}\n{}\n{} {}\n{}".format(HostNames[f], CPUNames[f], OSNames[f], OSVersions[f], Reference[f]))

FontScaler = 0.5

matplotlib.rcParams.update({'font.size': FontScaler*18})
matplotlib.rcParams.update({'axes.titlesize': FontScaler*32})
matplotlib.rcParams.update({'axes.labelsize': FontScaler*24}) 
if len(Reference) <= 8:
  matplotlib.rcParams.update({'xtick.labelsize': FontScaler*18})
else:
  matplotlib.rcParams.update({'xtick.labelsize': FontScaler*18*8/len(Reference)})
matplotlib.rcParams.update({'figure.figsize': [12.0, 6.0]})

x = np.arange(len(Labels))  # the label locations
width = 0.35  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(x - width/2, RunSingle, width, label='Single Core')
rects2 = ax.bar(x + width/2, RunMulti, width, label='All Cores')

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('(Started) Events / second')
if Mode == 'c':
  ax.set_title('Cosima Simulation Benchmark')
elif Mode == 'r':
  ax.set_title('Event Reconstruction Benchmark')
elif Mode == 'm':
  ax.set_title('Mimrec Imaging Benchmark')
elif Mode == 'a':
  ax.set_title('Response Creation Benchmark')
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

fig.tight_layout()

plt.grid(True, axis='y')
plt.show()



exit()  


