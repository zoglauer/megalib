###According to Solidworks, the mass of the coldfinger is 1.633 kg (not including the extension), here it's 1.629.
###According to Solidworks, the mass of the ColdfingerExtension (Extension1+2+3+4 from the other volumes) is 674 g, here it's 639+


Included Intro.geo


Volume ColdFinger
ColdFinger.Visibility 1
ColdFinger.Virtual True
ColdFinger.Material Vacuum
ColdFinger.Shape BRIK 10.585 0.7938 5.3022


Volume ColdFingerOutside
ColdFingerOutside.Material Copper
ColdFingerOutside.Visibility 1
ColdFingerOutside.Color 7
ColdFingerOutside.Shape BRIK 10.585 0.7938 5.3022
ColdFingerOutside.Mother ColdFinger
ColdFingerOutside.Position 0 0 0

Volume InnerColdfingerCut
InnerColdfingerCut.Material Vacuum
InnerColdfingerCut.Visibility 1
InnerColdfingerCut.Color 7
InnerColdfingerCut.Shape BRIK 7.093 0.7938 3.8735
InnerColdfingerCut.Mother ColdFingerOutside
InnerColdfingerCut.Position 0 0 1.4287



