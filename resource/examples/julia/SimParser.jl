# SimParser.jl
#
# Copyright (C) by the MEGAlib contributors.
#
# This file is part of MEGAlib.
#
# MEGAlib is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# MEGAlib is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
# License (License.md) for more details.
#
# SPDX-License-Identifier: LGPL-3.0-or-later


# Define file paths`
GeometryName = "Test.geo.setup"
FileName = "Test.inc1.id1.sim.gz"


# Load ROOT and integrate MEGAlib
using PyCall
M = pyimport("ROOT")
MEGAlibPath = ENV["MEGALIB"]  # Get the value of the environment variable M
MEGAlibPath = joinpath(MEGAlibPath, "lib/libMEGAlib.so")  # Safely join the path
M.gSystem.Load(MEGAlibPath)


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
Geometry.ActivateNoising(false)

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


