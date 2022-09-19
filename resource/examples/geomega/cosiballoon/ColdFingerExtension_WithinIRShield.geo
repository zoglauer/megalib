
#Building up the coldfinger extension out of two assymetric trapazoids. But, the piece extends beyond the inside of the cryostat shell, so I need to make additional volumes in the Cryostat and Cryocool Doghouse...
#What I refer to as the ColdfingerExtension* is called the "Inner Heat Strap Coldfinger Attachement Bar" in Solidworks.
##There are three trapizodial shape here which, summed up, give an approximation for the coldfinger extension volume that lives within the boundaries of the IRShield. Outside of the IRShield, there are another two trapizoids living within the boundaries of the cryostat inner wall. I refer to these as ColdfingerExtension1-3 within the IRShield and ColdfingerExtension4-5 within the Cryostat Inner walls. There are also addtional pieces within the wall of the cryostat and within the dog house.
### I did a ton of trig to get these trapazoids the right size. I started with the angle and the length of one side and the width, then calculated the rest. 
Include Intro.geo
Include IRShieldsAndGe.geo


Volume ColdFingerExtension_WithinIRShield
ColdFingerExtension_WithinIRShield.Material Vacuum
ColdFingerExtension_WithinIRShield.Virtual True
ColdFingerExtension_WithinIRShield.Visibility 0
ColdFingerExtension_WithinIRShield.Shape BRIK {11.3881 - IRShield_thickness} {13.0224 - IRShield_thickness} {5.4674 - IRShield_thickness}
#I made the ColdFingerExtension_WithinIRShield virtual volume to be the same as the inside of the IRShield because I had already figured out the position of this extension relative to the IRShield before I gave it it's own .geo file.


Volume ColdfingerExtension1
ColdfingerExtension1.Material al6061
ColdfingerExtension1.Visibility 1
ColdfingerExtension1.Color 6
ColdfingerExtension1.Shape TRAP {1.828/2} 0 0 {7.62/2} {9.4/2+6.083/2} {9.4/2} {21.76} {7.62/2} {9.4/2+6.083/2} {9.4/2} {21.76}
#ColdfingerExtension1.Shape TRAP {1.828/2} 0 0 {7.62/2} {9.5/2+6.083/2} {9.5/2+1} {15} {7.62/2} {9.5/2+6.083/2} {9.5/2+1} {15}
### The widths: 1.828 and 7.62 are from the solidworks model, and an angle of 51.4 degrees between the coldfinger and this first extension piece was used. The length 11cm was chosen as an approximation, and the rest was calculated.
ColdfingerExtension1.Mother ColdFingerExtension_WithinIRShield
ColdfingerExtension1.Rotation 0 0 {90+38.6}
ColdfingerExtension1.Position {-4+0.643} 5.7 {-5.4674 + IRShield_thickness + 1.828/2} 
###This position was found by inspection...the rotation and weird shape it too hard to find mathematically. I checked to make sure that the final position is right.
#But now looking abck at this, I'm not convinced that the z position is correct. Nothing is flawed here, but maybe in the positioing of the iRShield?





##The two next pieces were found using trig and inspection to fill the gaps left by the previous piece.
Volume ColdfingerExtension2
ColdfingerExtension2.Material al6061
ColdfingerExtension2.Visibility 1
ColdfingerExtension2.Color 6
ColdfingerExtension2.Shape TRAP {1.828/2} 0 0 {4.45/2 - 1.0301/2 - IRShield_thickness/2} {7.1088/2 - IRShield_thickness/2} {7.0585/2 - 4.221/2} {32.06} {4.45/2 - 1.0301/2 - IRShield_thickness/2} {7.1088/2 - IRShield_thickness/2} {7.0585/2 - 4.221/2} {32.06}
ColdfingerExtension2.Mother ColdFingerExtension_WithinIRShield
ColdfingerExtension2.Rotation 0 0 180
ColdfingerExtension2.Position -8.85 11.25 {-5.4674 + IRShield_thickness + 1.828/2}


Volume ColdfingerExtension3
ColdfingerExtension3.Material al6061
ColdfingerExtension3.Visibility 1
ColdfingerExtension3.Color 6
ColdfingerExtension3.Shape TRAP {1.828/2} 0 0 {1.7677/2} {2.219/2} {0.001} {32.1} {1.7677/2} {2.219/2} {0.001} {32.1}
ColdfingerExtension3.Mother ColdFingerExtension_WithinIRShield
ColdfingerExtension3.Rotation 180 0 {180+38.6}
ColdfingerExtension3.Position -10.35 9.215 {-5.4674 + IRShield_thickness + 1.828/2}


