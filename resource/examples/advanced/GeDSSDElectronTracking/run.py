###################################################################################################
#
# run.py
#
# Copyright (C) by Andreas Zoglauer.
# All rights reserved.
#
# Please see the file License.txt in the main repository for the copyright-notice. 
#  
###################################################################################################

  
  
###################################################################################################


import os
import sys
import argparse
import ROOT
from GeDSSDElectronTracking import GeDSSDElectronTracking
  
  
###################################################################################################


"""
This is the main test program for COSI electron tracking in python.
For all the command line options, try:

python3 run.py --help

"""


parser = argparse.ArgumentParser(description='Perform training and testing of COSI electron tracking learning tools.')
parser.add_argument('-f', '--file', default='COSIElectronTracking.p1.x3.y3.electrontrackingwithcrossstripdetector.tmva.root', help='File name used for training/testing')
parser.add_argument('-o', '--output', default='Results', help='Prefix for the output filename and directory')
parser.add_argument('-l', '--layout', default='N+N,10', help='Layout of the hidden layer. Default: 30,15')
parser.add_argument('-m', '--maxevents', default='100000', help='Maximum number of events to use')
parser.add_argument('-e', '--onlyevaluate', action='store_true', help='Only test the approach')
parser.add_argument('-t', '--onlytrain', action='store_true', help='Only train the approach')
parser.add_argument('-a', '--all', action='store_true', help='Independently of what file was given operate on all')

args = parser.parse_args()

AI = GeDSSDElectronTracking(args.file, args.output, args.layout, int(args.maxevents), args.all)

if args.onlyevaluate == False:
  if AI.train() == False:
    sys.exit()

if args.onlytrain == False:
  if AI.test() == False:
    sys.exit()


# prevent Canvases from closing

List = ROOT.gROOT.GetListOfCanvases()
if List.LastIndex() > 0:
  print("ATTENTION: Please exit by clicking: File -> Close ROOT! Do not just close the window by clicking \"x\"")
  print("           ... and if you didn't honor this warning, and are stuck, execute the following in a new terminal: kill " + str(os.getpid()))
  ROOT.gApplication.Run()


# END
###################################################################################################
