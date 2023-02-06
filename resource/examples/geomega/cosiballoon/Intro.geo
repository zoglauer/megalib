
Name MassmodelCOSIBalloon
Version 4

#ShowVolumes false

# Material definitions
Include COSIMaterials.geo

AbsorptionFileDirectory crossections

# Master volumes
Volume WorldVolume
WorldVolume.Visibility 0
WorldVolume.Material Vacuum
WorldVolume.Shape BRIK 1000 1000 1000
WorldVolume.Mother 0



