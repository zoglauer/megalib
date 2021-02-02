# Plot activations

import ROOT as M

# Load MEGAlib into ROOT
M.gSystem.Load("$(MEGALIB)/lib/libMEGAlib.so")

# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()

# We are good to go ...

# Nuclear lines to watch
NuclearLineWidth = 2
NuclearLines = [ 511.0, 1157.0, 1173.2, 1332.5, 1808.7 ]

# Create at summary latex file
LatexFileName = "Summary.tex" 
Out = open(LatexFileName, 'w')
print("\\documentclass{article}", file=Out)
print("\\usepackage{graphicx}", file=Out)
print("", file=Out)
print("\\begin{document}", file=Out)
print("", file=Out)
print("\\title{Activation comparison for different space craft materials}", file=Out)
print("\\author{Andreas Zoglauer}", file=Out)
print("", file=Out)
print("\\maketitle", file=Out)
print("", file=Out)
print("\\begin{abstract}", file=Out)
print("\\begin{center}**** This paper has been automatically generated ****\end{center}", file=Out)
print("\\end{abstract}", file=Out)
print("", file=Out)
print("\\section{Simulation assumptions}", file=Out)
print("The assumed orbit for the particle background was low-earth equatorial, with an altitude of 650~km and an inclination of 6~degrees, similar to NuSTAR. The simulation mass-model assumed a hallow sphere of 1~m radius with a thickness corresponding to 1~g/cm$^2$ line density. For the long-term activation we assumed 1~year in orbit, the activation and subsequently the decays have been orbit-averaged.", file=Out)
print("\\section{Materials}", file=Out)



# Open the materials file
MaterialListFile = open('MaterialList.txt', 'r') 
Lines = MaterialListFile.readlines() 

for Line in Lines:
  # Retrieve the material data from the file 
  Tokens = Line.split()
  
  if len(Tokens) < 5:
    continue

  Name = Tokens.pop(0)
  MaterialDensity = float(Tokens.pop(0))
  Type = Tokens.pop(0)
  Materials = []
  Amount = []
  while len(Tokens) >= 2:
    Materials.append(Tokens.pop(0))
    Amount.append(Tokens.pop(0))
 
  GeometryName = f"{Name}.geo.setup"
  FileName = f"{Name}_Decay.inc1.id1.sim.gz"

  # Load geometry:
  Geometry = M.MDGeometryQuest()  
  if Geometry.ScanSetupFile(M.MString(GeometryName)) == True:
    print("Geometry " + GeometryName + " loaded!")
  else:
    print("Unable to load geometry " + GeometryName + " - Aborting!")
    quit()
    
  Reader = M.MFileEventsSim(Geometry)
  if Reader.Open(M.MString(FileName)) == False:
    print("Unable to open file " + FileName + ". Aborting!")
    quit()
  Reader.ShowProgress(True)

  NBins = 1000
  EMax = 5000
  Hist = M.TH1D(f"{Name}", f"{Name}", NBins, 0, EMax)
  Hist.SetXTitle("Energy [keV]")
  Hist.SetYTitle("created gamma rays / kg / s")

  ElapsedTime = 0

  MaxCounts = 10000000
  
  TotalCounts = 0
  LineCounts = []
  for N in NuclearLines:
    LineCounts.append(0)
  
  while True: 
    Event = Reader.GetNextEvent()
    if not Event:
      break
    M.SetOwnership(Event, True)
  
    # Add all photons created by decay or annihilation to the list
    for i in range(0, Event.GetNIAs()):
      if Event.GetIAAt(i).GetProcess() == M.MString("DECA") or Event.GetIAAt(i).GetProcess() == M.MString("ANNI"):
        if Event.GetIAAt(i).GetSecondaryParticleID() == 1:
          Energy = Event.GetIAAt(i).GetSecondaryEnergy()
          Hist.Fill(Energy)
          
          TotalCounts += 1
          for i in range(0, len(NuclearLines)):
            if Energy >= NuclearLines[i]-NuclearLineWidth and Energy <= NuclearLines[i]+NuclearLineWidth:
              LineCounts[i] += 1
          
          
          #print("Adding {}".format(Event.GetIAAt(i).GetSecondaryEnergy()))

    ElapsedTime = Event.GetTime().GetAsSeconds()

    MaxCounts -= 1
    if MaxCounts == 0:
      break

  # Normalize
  Volume = 4.0/3.0*M.c_Pi*(100*100*100-pow(100-1.0/MaterialDensity,3))
  Mass = MaterialDensity*Volume / 1000 # kg

  for i in range(1, NBins+1):
    Hist.SetBinContent(i, Hist.GetBinContent(i)/ElapsedTime/Mass)

  TotalCounts = TotalCounts / ElapsedTime / Mass
  for i in range(0, len(LineCounts)):
    LineCounts[i] = LineCounts[i] / ElapsedTime / Mass

  # Plot
  Canvas = M.TCanvas(f"Canvas_{Name}", f"Canvas_{Name}")
  Canvas.cd()
  Canvas.SetLogy()
  Hist.Draw()
  Canvas.Update() 
  Canvas.SaveAs(f"{Name}.pdf")
  
  print("\\pagebreak", file=Out)
  print("\n", file=Out)
  
  print("\\subsection{" + Name + "}", file=Out)
  print("\n", file=Out)
  
  print("\\begin{figure}[h!]", file=Out)
  print("  \\centering", file=Out)
  print("  \\includegraphics[width=6.0in]{" + Name + ".pdf}", file=Out)
  print("  \\caption{Generated gamma rays due to decay and positron annilation in one kg of material in one second.}", file=Out)
  print("\\end{figure}", file=Out)
  print("\n", file=Out)
    
  print("\\begin{center}", file=Out)
  print("\\begin{tabular}{ l l }", file=Out)
  print("  \\hline", file=Out)
  print("  Material & {} \\\\".format(Name), file=Out)
  print("  Density & {} \\\\".format(MaterialDensity), file=Out)
  print("  \\hline", file=Out)
  print("  Composition &  \\\\", file=Out)
  for i in range(0, len(Materials)):
    print("  Element: {} & {} {} \\\\".format(Materials[i], Amount[i], ("atoms" if Type == "a" else "u")), file=Out)
  print("  \\hline", file=Out)
  print("  Total generated gamma's & " + str(round(TotalCounts, 6)) + " $\gamma$ / kg /s  \\\\", file=Out) 
  for i in range(0, len(LineCounts)):
    print("  Generated in " + str(NuclearLines[i]) + "$\pm$" + str(NuclearLineWidth) + " keV & " + "%.6f" % LineCounts[i] + " $\gamma$ / kg /s \\\\", file=Out)   
  print("  \\hline", file=Out)
  print("\\end{tabular}", file=Out)
  print("\\end{center}", file=Out)
  

  
print("\\end{document}", file=Out)
Out.close()

import subprocess
result = subprocess.run(['pdflatex', LatexFileName], stdout=subprocess.PIPE)
result.stdout
 
# Prevent the canvases from being closed
import os
print("ATTENTION: Please exit by clicking: File -> Close ROOT! Do not just close the window by clicking \"x\"")
print("           ... and if you didn't honor this warning, and are stuck, execute the following in a new terminal: kill " + str(os.getpid()))
#M.gApplication.Run()

