#Files defines the geometry of the graded-z shield used for COSI'14 and COSI'16.

Include Intro.geo

# 0.008 inch = 0.0203 cm tin with minor variations upward, so 0.021 as average
Constant HalfThicknessTin {0.021/2}
# Probably 16 MIL thickness = 0.016 inch = 0.0406 cm
Constant HalfThicknessCopper {0.0406/2}
Constant HalfThicknessShields {2.55}
Constant HalfWidthGradedZ_1 {18.74*2.54/2 + HalfThicknessShields - 4*2.54/2}
Constant HalfWidthGradedZ_2 {4*2.54/2}
Constant HalfLengthGradedZ_1 {17.99*2.54/2 + HalfThicknessShields}
Constant HalfLengthGradedZ_2 {17.99*2.54/2 + HalfThicknessShields - 6*2.54/2}
Constant GradedZThickness {HalfThicknessTin + HalfThicknessCopper}

Volume GradedZShield
GradedZShield.Visibility 0
GradedZShield.Virtual True
GradedZShield.Material Vacuum
GradedZShield.Shape BRIK 18 19 GradedZThickness

Volume CopperLayer_1
CopperLayer_1.Visibility 1
CopperLayer_1.Material mcopper
CopperLayer_1.Shape BRIK HalfLengthGradedZ_1 HalfWidthGradedZ_1 HalfThicknessCopper
CopperLayer_1.Mother GradedZShield
CopperLayer_1.Position 0 {HalfWidthGradedZ_2}  {-HalfThicknessTin}

Volume CopperLayer_2
CopperLayer_2.Visibility 1
CopperLayer_2.Material mcopper
CopperLayer_2.Shape BRIK HalfLengthGradedZ_2 HalfWidthGradedZ_2 HalfThicknessCopper
CopperLayer_2.Mother GradedZShield
CopperLayer_2.Position {6*2.54/2} {-HalfWidthGradedZ_1}  {-HalfThicknessTin}

Volume TinLayer_1
TinLayer_1.Visibility 1
TinLayer_1.Color 2
TinLayer_1.Material mtin
TinLayer_1.Shape BRIK HalfLengthGradedZ_1 HalfWidthGradedZ_1 HalfThicknessTin
TinLayer_1.Mother GradedZShield
TinLayer_1.Position 0 {HalfWidthGradedZ_2} HalfThicknessCopper

Volume TinLayer_2
TinLayer_2.Visibility 1
TinLayer_2.Color 2
TinLayer_2.Material mtin
TinLayer_2.Shape BRIK HalfLengthGradedZ_2 HalfWidthGradedZ_2 HalfThicknessTin
TinLayer_2.Mother GradedZShield
TinLayer_2.Position {6*2.54/2} {-HalfWidthGradedZ_1} HalfThicknessCopper


