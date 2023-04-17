#File to describe all of the materials used in COSI. This has been copied from the old NCTMaterials.geo and reformatted using the new Geomega material parameters. I commented out each type of material before running geomega to see which ones my geometery actually uses and deleted all of the obsolete ones. 
###I'm not sure why some materials have a lower case "m" infront of the name...

Include $(MEGALIB)/resource/examples/geomega/materials/Materials.geo

### Material definitions:

Material al6061
al6061.Density 2.6989
al6061.ComponentByMass Si 0.006
al6061.ComponentByMass Fe 0.007
al6061.ComponentByMass Cu 0.003
al6061.ComponentByMass Mn 0.0015
al6061.ComponentByMass Mg 0.01
al6061.ComponentByMass Cr 0.002
al6061.ComponentByMass Zn 0.0025
al6061.ComponentByMass Ti 0.0015
al6061.ComponentByMass Al 0.9665

# From SPI...
Material pmtmix
pmtmix.Density 1.0
pmtmix.ComponentByMass  C   0.05
pmtmix.ComponentByMass  Cu  0.1
pmtmix.ComponentByMass  Si  0.05
pmtmix.ComponentByMass  Al  0.8

#Material Vacuum                          
Vacuum.Density                  1E-16                
Vacuum.Component                H 1 

Material maluminium
maluminium.Density 2.7
maluminium.ComponentByMass Al  1

Material mcopper
mcopper.Density 8.954
mcopper.ComponentByMass Cu 1

Material mcsi
mcsi.Density 4.5
mcsi.ComponentByAtoms Cs 1
mcsi.ComponentByAtoms I  1

Material mgepassive
mgepassive.Density 5.323
mgepassive.ComponentByMass Ge  1

Material mindium
mindium.Density 7.31
mindium.ComponentByMass In  1

Material mro3003
mro3003.Density 2.1
mro3003.ComponentByMass C   0.1604
mro3003.ComponentByMass F   0.2544
mro3003.ComponentByMass O   0.3061
mro3003.ComponentByMass Si  0.2522
mro3003.ComponentByMass Tl  0.0269

Material mro4003
mro4003.Density 1.79
mro4003.ComponentByMass C  0.151697
mro4003.ComponentByMass O  0.428144
mro4003.ComponentByMass H  0.0179641
mro4003.ComponentByMass Si 0.329341
mro4003.ComponentByMass Ca 0.0369261
mro4003.ComponentByMass Al 0.0189621
mro4003.ComponentByMass B  0.010978
mro4003.ComponentByMass K  0.000998004
mro4003.ComponentByMass Mg 0.00499002

Material mrotmm3
mrotmm3.Density 1.78
mrotmm3.ComponentByMass C  0.125313
mrotmm3.ComponentByMass O  0.445534
mrotmm3.ComponentByMass H  0.0206594
mrotmm3.ComponentByMass Si 0.377295
mrotmm3.ComponentByMass Ti 0.0159641
mrotmm3.ComponentByMass Ba 0.0102254
mrotmm3.ComponentByMass Ca 0.00229549
mrotmm3.ComponentByMass Al 0.00114775
mrotmm3.ComponentByMass B  0.000313022
mrotmm3.ComponentByMass Mg 0.000208681
mrotmm3.ComponentByMass S  0.00104341

Material msteel_18_8
msteel_18_8.Density 7.87
msteel_18_8.ComponentByMass Cr 0.18
msteel_18_8.ComponentByMass Ni 0.09
msteel_18_8.ComponentByMass C  0.013
msteel_18_8.ComponentByMass Fe 0.717

Material mtin
mtin.Density 7.31
mtin.ComponentByMass Sn 1


Material mvacuum
mvacuum.Density 1e-12
mvacuum.ComponentByMass H 1

Material mgermanium_ge_recoil
mgermanium_ge_recoil.Density 5.32
mgermanium_ge_recoil.ComponentByMass Ge 1

Material mal6061
mal6061.Density 2.6989
mal6061.ComponentByMass Si 0.006
mal6061.ComponentByMass Fe 0.007
mal6061.ComponentByMass Cu 0.003
mal6061.ComponentByMass Mn 0.0015
mal6061.ComponentByMass Mg 0.01
mal6061.ComponentByMass Cr 0.002
mal6061.ComponentByMass Zn 0.0025
mal6061.ComponentByMass Ti 0.0015
mal6061.ComponentByMass Al 0.9665

Material mdetector_al6061
mdetector_al6061.Density 2.6989
mdetector_al6061.ComponentByMass Si 0.006
mdetector_al6061.ComponentByMass Fe 0.007
mdetector_al6061.ComponentByMass Cu 0.003
mdetector_al6061.ComponentByMass Mn 0.0015
mdetector_al6061.ComponentByMass Mg 0.01
mdetector_al6061.ComponentByMass Cr 0.002
mdetector_al6061.ComponentByMass Zn 0.0025
mdetector_al6061.ComponentByMass Ti 0.0015
mdetector_al6061.ComponentByMass Al 0.9665

Material Cardcage
###defined to be made of Aluminum but to have the correct density such that a solid card cage made of this material will have the correct mass
Cardcage.Density 0.7805
Cardcage.ComponentByAtoms Al 1

Material SourcePlastic
SourcePlastic.Density 2
SourcePlastic.ComponentByAtoms C 2
SourcePlastic.ComponentByAtoms H 4

Material Polyethylene
Polyethylene.Density 0.95
Polyethylene.ComponentByAtoms C 2
Polyethylene.ComponentByAtoms H 4

Material SourceSaltCs137
SourceSaltCs137.Density 3.99
SourceSaltCs137.ComponentByAtoms Cs 1
SourceSaltCs137.ComponentByAtoms Cl 1



# Note: set this air to the correct location to preperly model atmospheric absorption!
#Constant AirDummy AirSSL
Constant AirDummy AirFtSumner
#Constant AirDummy AirFloat

Material AirSeaLevel
# note: density calculated for altitude 0 m above sea level
AirSeaLevel.Density 0.0012250
AirSeaLevel.ComponentByMass N  0.7551
AirSeaLevel.ComponentByMass O  0.2318
AirSeaLevel.ComponentByMass Ar 0.0129
AirSeaLevel.ComponentByMass C  0.0002

Material AirSSL
# note: density calculated for altitude 500 m above sea level
AirSSL.Density 0.0011786
AirSSL.ComponentByMass N  0.7551
AirSSL.ComponentByMass O  0.2318
AirSSL.ComponentByMass Ar 0.0129
AirSSL.ComponentByMass C  0.0002

Material AirFtSumner
# note: density calculated for altitude 1250 m above sea level
AirFtSumner.Density 0.0010846
AirFtSumner.ComponentByMass N  0.7551
AirFtSumner.ComponentByMass O  0.2318
AirFtSumner.ComponentByMass Ar 0.0129
AirFtSumner.ComponentByMass C  0.0002

Material AirFloat
# note: atmospheric absorption should be calculated elsewhere
AirFloat.Density 1e-12
AirFloat.ComponentByMass N  0.7551
AirFloat.ComponentByMass O  0.2318
AirFloat.ComponentByMass Ar 0.0129
AirFloat.ComponentByMass C  0.0002

Material AirFloat_8gmcm2
# note: atmospheric absorption should be calculated elsewhere
AirFloat_8gmcm2.Density 8.0
AirFloat_8gmcm2.ComponentByMass N 0.7551
AirFloat_8gmcm2.ComponentByMass O 0.2318
AirFloat_8gmcm2.ComponentByMass Ar 0.0129
AirFloat_8gmcm2.ComponentByMass C 0.0002

Material AirFloat_Antarctica_130000kft
#the density here is equal to the depth in gm/cm^2 that I got from running the following command:
#./integrate_density_profile.py 39.624 -77.85 166.666 360
# in the directory (on Cobalt) /home/alowell/sync/nct/exposure_maps/mcmurdo/with_resp_and_extinction/atmospheric_density_profile_log
# this corresponds to heigh = 130 kft = 39.624 km, coords. for mcmurdo, on day 360/365
AirFloat_Antarctica_130000kft.Density 4.05288324089
AirFloat_Antarctica_130000kft.ComponentByMass N 0.7551
AirFloat_Antarctica_130000kft.ComponentByMass O  0.2318
AirFloat_Antarctica_130000kft.ComponentByMass Ar 0.0129
AirFloat_Antarctica_130000kft.ComponentByMass C  0.0002





