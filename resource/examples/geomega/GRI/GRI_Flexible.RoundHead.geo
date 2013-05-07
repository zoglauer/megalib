
Include GRI_Flexible.Intro.geo

Constant CryoThickness 0.05
Constant DistanceGeCryo 1.0


Volume OuterCryo
OuterCryo.Material al6061
OuterCryo.Visibility 1
OuterCryo.Color 4
OuterCryo.Shape TUBS {Ge_Radius + DistanceGeCryo} {Ge_Radius + DistanceGeCryo + CryoThickness} { 0.5*(Ge_Number-1)*CryoThickness + 0.5*Ge_Number*Ge_Height+ DistanceGeCryo } 0.0 360.0


