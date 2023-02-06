#File defines the "LowerShield", ie. the two shields that sit underneath the cryostat

Include CsIShield.geo
###CsIShield defines a single CSI shield, PMTs and all

Volume LowerShield
LowerShield.Visibility 0
LowerShield.Material Vacuum
LowerShield.Shape BRIK 1000.0 1000.0 1000.0 
LowerShield.Virtual true

RectangularShieldModule.Copy RectangularShieldModule_Copy5
RectangularShieldModule_Copy5.Mother LowerShield
RectangularShieldModule_Copy5.Position 0 recH {Zpos-(recH+recW)}
###Zpos, recH and recW are defined in csishieldslab.geo 
RectangularShieldModule_Copy5.Rotation -90 0 0
RectangularShieldModule_Copy5.Visibility 1

RectangularShieldModule.Copy RectangularShieldModule_Copy6
RectangularShieldModule_Copy6.Mother LowerShield
RectangularShieldModule_Copy6.Position 0 {-recH} {Zpos-(recH+recW)}
RectangularShieldModule_Copy6.Rotation -90 0 180
RectangularShieldModule_Copy6.Visibility 1



