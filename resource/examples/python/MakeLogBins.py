# Extract the data space parameters from the event data

import argparse
import os
import math

# Parse the arguments
parser = argparse.ArgumentParser(description='Make logarithmic bins for the responsecreator including lines')
parser.add_argument('-i', '--min', default='150', type=float, help='Minimum energy in keV (default: 150)')
parser.add_argument('-a', '--max', default='5000', type=float, help='Maximum energy in keV (default: 5000)')
parser.add_argument('-b', '--bin', default='10', type=int, help='Number of bins (default: 10)')
parser.add_argument('-l', '--line', default=[], type=float, action='append', help='Bin around a specific energy in keV (can appear multiple times)')
parser.add_argument('-s', '--safetymin', default='20', type=float, help='The lower bound of the energy bin around lines in keV (default: 20)')
parser.add_argument('-t', '--safetymax', default='10', type=float, help='The upper bound of the energy bin around lines in keV (default: 10)')

args = parser.parse_args()

MinEnergy = args.min
MaxEnergy = args.max
Lines = args.line
Bins = args.bin
LowerSafety = args.safetymin
UpperSafety = args.safetymax

if MinEnergy <= 0:
  print("Input error: Minimum energy ({} keV) must be larger than 0!".format(MinEnergy))
  quit()

if MaxEnergy <= MinEnergy:
  print("Input error: Maximum energy ({} keV) must be larger than minimum energy!".format(MaxEnergy))
  quit()

if LowerSafety <= 0:
  print("Input error: Minimum safety ({} keV) must be larger than 0!".format(LowerSafety))
  quit()

if UpperSafety <= 0:
  print("Input error: Minimum safety ({} keV) must be larger than 0!".format(UpperSafety))
  quit()

if Bins < 1 + len(Lines):
  print("Input error: The number of bins must be at least 1 + number of lines, i.e. at least {}!".format(1 + len(Lines)))
  quit()

for E in Lines:
  if E - LowerSafety < MinEnergy:
    print("Input error: Line energy ({} keV) minus safety ({} keV) is smaller than minimum energy ({} keV)!".format(E, LowerSafety, MinEnergy))
    quit()
  if E + UpperSafety > MaxEnergy:
    print("Input error: Line energy ({} keV) plus safety ({} keV) is larger than maximum energy ({} keV)!".format(E, UpperSafety, MaxEnergy))
    quit()

for i in range(0, len(Lines)-1):
  if Lines[i] + UpperSafety > Lines[i+1] - LowerSafety:
    print("Input error: Overlapping safeties: {} + {} > {} - {}".format(Lines[i], UpperSafety, Lines[i+1], LowerSafety))
    quit()

LogMinEnergy = math.log(MinEnergy)
LogMaxEnergy = math.log(MaxEnergy)
LogAverageBinWidth = ( LogMaxEnergy - LogMinEnergy ) / ( Bins - len(Lines) )

LogStart = LogMinEnergy
LogBinEdges = [ LogStart ]

for E in Lines:
  LogStop = math.log(E - LowerSafety)
  LogWidth = (LogStop - LogStart)
  Bins = round(LogWidth / LogAverageBinWidth)
  LogBinWidth = LogWidth / Bins
  for Bin in range(1, Bins+1):
    LogBinEdges.append(LogStart + Bin*LogBinWidth)
  LogStart = math.log(E + UpperSafety)
  LogBinEdges.append(LogStart)

LogStop = LogMaxEnergy
LogWidth = (LogStop - LogStart)
Bins = round(LogWidth / LogAverageBinWidth)
LogBinWidth = LogWidth / Bins
for Bin in range(1, Bins+1):
  LogBinEdges.append(LogStart + Bin*LogBinWidth)

BinEdges = [ round(math.exp(x)) for x in LogBinEdges ]

print("Bin edges: {}".format(BinEdges))


