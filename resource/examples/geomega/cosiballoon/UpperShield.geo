#File defines the "UpperShield", ie. the four shields that lie on their sides surrounding the Cryostat. "LowerShield" contains the two shields that sit underneath the cryostat.


Include CsIShield.geo
###CsIShield defines a single CSI shield, PMTs and all.

Constant xoffset 0.9525
###The shields don't sit in a perfect square with their corners touching, the two shields that are offset in the x direction (facing front and back) are shifted in 0.9525 cm. So the distance between the shields in along the x direction is 45.695 cm, where the shield length is actually 47.6 cm.

Volume UpperShield
UpperShield.Visibility 1
UpperShield.Material Vacuum
UpperShield.Shape BRIK 1000.0 1000.0 1000.0 
UpperShield.Virtual true

###The shields are rotated such that the PMT end is closer to the cryocooler, as this end has better response.

RectangularShieldModule.Copy RectangularShieldModule_Copy1
RectangularShieldModule_Copy1.Mother UpperShield
RectangularShieldModule_Copy1.Position {(recL+recW)-xoffset} 0 Zpos
###Zpos, recL and recW are defined in csishieldslab.geo 
RectangularShieldModule_Copy1.Rotation 0 0 -90
RectangularShieldModule_Copy1.Visibility 1

RectangularShieldModule.Copy RectangularShieldModule_Copy2
RectangularShieldModule_Copy2.Mother UpperShield
RectangularShieldModule_Copy2.Position {-(recL+recW)+xoffset} 0 Zpos
RectangularShieldModule_Copy2.Rotation 0 0 90
RectangularShieldModule_Copy2.Visibility 1

RectangularShieldModule.Copy RectangularShieldModule_Copy3
RectangularShieldModule_Copy3.Mother UpperShield
RectangularShieldModule_Copy3.Position 0 {-(recL+recW)} Zpos
RectangularShieldModule_Copy3.Rotation 0 0 180
RectangularShieldModule_Copy3.Visibility 1


RectangularShieldModule.Copy RectangularShieldModule_Copy4
RectangularShieldModule_Copy4.Mother UpperShield
RectangularShieldModule_Copy4.Position 0 {recL+recW} Zpos
RectangularShieldModule_Copy4.Rotation 0 180 0 
####ck (2/14) rotated so that the pmt's are closest to the cryocooler
RectangularShieldModule_Copy4.Visibility 1





