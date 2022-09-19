#This was not edited in MassModel_1.0, as I have not worked with airslabs yet, but I figured I should keep this .geo file for when taht time comes.

Include COSIMaterials.geo

Constant atmos_depth 3.45 
#Constant airslabthickness 10.0
Constant airslabthickness 1.0

Material AirFloat_kiruna_average
AirFloat_kiruna_average.ComponentByMass N 0.7551
AirFloat_kiruna_average.ComponentByMass O 0.2318
AirFloat_kiruna_average.ComponentByMass Ar 0.0129
AirFloat_kiruna_average.ComponentByMass C 0.0002
#AirFloat_kiruna_average.Density {atmos_depth/airslabthickness}
AirFloat_kiruna_average.Density 0.345

Volume AirSlab
AirSlab.Shape BRIK 100 100 {airslabthickness}
AirSlab.Virtual False
AirSlab.Material AirFloat_Antarctica_130000kft


