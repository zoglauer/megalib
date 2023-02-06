Include Intro.geo

#The ColdfingerExtension2 piece extends through the CryostatCryocoolerHole hole. I need to add the extension piece here and also inside the doghouse...There is also the piece that extends up to the cryocooler tip (Chris refers to it as the whistle), which I need to add here.
#The ColdfingerExtension is 1.791 cm above the InnerSide of the Cryostat Bottom, and 0.531 cm away from the cryostat wall in the +y direction

Volume ColdFingerExtension_WithinCryostatShell
ColdFingerExtension_WithinCryostatShell.Visibility 0
ColdFingerExtension_WithinCryostatShell.Virtual True
ColdFingerExtension_WithinCryostatShell.Material Vacuum
ColdFingerExtension_WithinCryostatShell.Shape BRIK 1 {4.45/2} 4

Volume ColdFingerExtension6
ColdFingerExtension6.Visibility 1
ColdFingerExtension6.Material al6061
ColdFingerExtension6.Shape BRIK {1.2196/2} {4.45/2} {1.27/2}
##The 1.2674 cm thickness of this piece is the same as the cryostat wall thickness, so it fits perfectly inside.
ColdFingerExtension6.Position 0 0 {-4.496/2-1.27/2}
ColdFingerExtension6.Color 6
ColdFingerExtension6.Mother ColdFingerExtension_WithinCryostatShell
##This piece isn't perfect. I think the position of the ColdfingerExtension_WithinCryostatShell is off. I'm pretty sure i should have an extra +0.5538 cm in the z direction on this piece, but then it's way above the piece within the cryostat wall, which isn't right.


Volume ColdFingerExtension_CopperConnectionToWhistle
ColdFingerExtension_CopperConnectionToWhistle.Visibility 1
ColdFingerExtension_CopperConnectionToWhistle.Material Copper
ColdFingerExtension_CopperConnectionToWhistle.Shape BRIK {1.2196/2} {1.32/2} {4.496/2}
ColdFingerExtension_CopperConnectionToWhistle.Position 0 0 0
ColdFingerExtension_CopperConnectionToWhistle.Color 4
ColdFingerExtension_CopperConnectionToWhistle.Mother ColdFingerExtension_WithinCryostatShell

Volume ColdFingerExtension_AlConnectionToWhistle
ColdFingerExtension_AlConnectionToWhistle.Visibility 1
ColdFingerExtension_AlConnectionToWhistle.Material al6061
ColdFingerExtension_AlConnectionToWhistle.Shape BRIK {1.2196/2} {2.972/2} {1.412/2}
ColdFingerExtension_AlConnectionToWhistle.Position 0 0 {4.496/2+1.412/2}
ColdFingerExtension_AlConnectionToWhistle.Color 5
ColdFingerExtension_AlConnectionToWhistle.Mother ColdFingerExtension_WithinCryostatShell
##I don't actually model the "whistle" part, cause it's not that massive and it's a complicated shape. I get the bulk of the material in the whistle with the piece above (once I extend it into the Doghouse)
