Include Intro.geo

#The Doghouse is the Cryocooler aluminum box that attaches to the cryostat and houses the cryocooler tip. Chris and the engineers refer to it as the dog house and that's stuck.

#The ColdfingerExtension2 piece extends through the CryostatCryocoolerHole hole. I already added the piece inside the cryostat wall, now I need to add it inside the doghouse...There is also the piece that extends up to the cryocooler tip (Chris refers to it as the whistle), which I need to add here.


Volume ColdFingerExtension_WithinTheDoghouse
ColdFingerExtension_WithinTheDoghouse.Visibility 0
ColdFingerExtension_WithinTheDoghouse.Virtual True
ColdFingerExtension_WithinTheDoghouse.Material Vacuum
ColdFingerExtension_WithinTheDoghouse.Shape BRIK {2.165/2} {4.45/2} 4

Volume ColdFingerExtension7
ColdFingerExtension7.Visibility 1
ColdFingerExtension7.Material al6061
ColdFingerExtension7.Shape BRIK {2.165/2} {4.45/2} {1.27/2}
##The 1.2674 cm thickness of this piece is the same as the cryostat wall thickness, so it fits perfectly inside.
ColdFingerExtension7.Position 0 0 {-4.496/2-1.27/2}
ColdFingerExtension7.Color 6
ColdFingerExtension7.Mother ColdFingerExtension_WithinTheDoghouse

Volume ColdFingerExtension_CopperConnectionToWhistle2
ColdFingerExtension_CopperConnectionToWhistle2.Visibility 1
ColdFingerExtension_CopperConnectionToWhistle2.Material Copper
ColdFingerExtension_CopperConnectionToWhistle2.Shape BRIK {2.165/2} {1.32/2} {4.496/2}
ColdFingerExtension_CopperConnectionToWhistle2.Position 0 0 0
ColdFingerExtension_CopperConnectionToWhistle2.Color 4
ColdFingerExtension_CopperConnectionToWhistle2.Mother ColdFingerExtension_WithinTheDoghouse

Volume ColdFingerExtension_AlConnectionToWhistle2
ColdFingerExtension_AlConnectionToWhistle2.Visibility 1
ColdFingerExtension_AlConnectionToWhistle2.Material al6061
ColdFingerExtension_AlConnectionToWhistle2.Shape BRIK {2.165/2} {2.972/2} {1.412/2}
ColdFingerExtension_AlConnectionToWhistle2.Position 0 0 {4.496/2+1.412/2}
ColdFingerExtension_AlConnectionToWhistle2.Color 5
ColdFingerExtension_AlConnectionToWhistle2.Mother ColdFingerExtension_WithinTheDoghouse

##I don't actually model the "whistle" part, cause it's not that massive and it's a complicated shape. I get the bulk of the material in the whistle with the piece above (once I extend it into the Doghouse)
