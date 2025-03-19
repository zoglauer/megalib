 #
 # SimParser.jl
 #
 # Copyright (C) by Andreas Zoglauer.
 # All rights reserved.
 #
 # This code implementation is the intellectual property of
 # Andreas Zoglauer.
 #
 # By copying, distributing or modifying the Program (or any work
 # based on the Program) you indicate your acceptance of this statement,
 # and all its terms.
 #

# Define file paths`
GeometryName = "Test.geo.setup"
FileName = "Test.inc1.id1.sim.gz"


# Load ROOT and integrate MEGAlib
using PyCall
M = pyimport("ROOT")
MEGAlibPath = ENV["MEGALIB"]  # Get the value of the environment variable M
MEGAlibPath = joinpath(MEGAlibPath, "lib/libMEGAlib.so")  # Safely join the path
println(MEGAlibPath)
M.gSystem.Load(MEGAlibPath)
#M.gSystem.Load("/Users/andreas/Documents/Science/Software/COSItools/megalib/lib/libMEGAlib.so")


# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()


# Load geometry
Geometry = M.MDGeometryQuest()
if Geometry.ScanSetupFile(M.MString(GeometryName)) == true
    println("Geometry ", GeometryName, " loaded!")
else
    println("Unable to load geometry ", GeometryName, " - Aborting!")
    exit()
end
# Deactivate noising


# Open the sim file
Reader = M.MFileEventsSim(Geometry)  # Create an instance of MFileEventsSim with the loaded geometry
if Reader.Open(M.MString(FileName)) == false
    println("Unable to open file ", FileName, ". Aborting!")
    exit()
end


# Loop over all events
while true
    Event = Reader.GetNextEvent()  # Get the next event from the reader
    if Event == nothing  # No more events
        break
    end
    M.SetOwnership(Event, true)  # Hold-over from Python - don't know if we need that here


    # Retrieve the Hit data
    for i in 0:Event.GetNHTs()-1
        Energy = Event.GetHTAt(i).GetEnergy()
        Position = Event.GetHTAt(i).GetPosition()
        
        println("Hit ", i, ": E=", Energy, " P=(", Position.GetX(), "/", Position.GetY(), "/", Position.GetZ(), ")")
    end
end

# Clean up
Reader.Close()


