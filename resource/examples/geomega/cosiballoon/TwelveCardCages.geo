#This file describes the cardcage structure containing the 12 card cages.
#Mass of each card cage according to solidworks is 44.5 lbs. Density is 0.02814 lbs/in^3 = 0.7805 g/cm^3. I've attained that here by creating a new material "Cardcage" which is made up of aluminum but has the correct denisty. The final mass of each cardcage here is 44.3 lbs.
#I could still add the aluminum mounting panels that the cardcages are bolted to.

Include Intro.geo

###definitions of the cardcage dimensions, as obtained from Solidworks
Constant CC_halfwidth {32.86/2}
Constant CC_halfdepth {26.459/2}
Constant CC_halfheight {29.700/2}

Volume TwelveCardCages
TwelveCardCages.Virtual True
TwelveCardCages.Visibility 0
TwelveCardCages.Material Vacuum
TwelveCardCages.Shape BRIK 65 65 15


#First define one CC
Volume OneCardCage
OneCardCage.Visibility 1
OneCardCage.Material Cardcage
OneCardCage.Shape BRIK CC_halfwidth CC_halfdepth CC_halfheight
OneCardCage.Position 0 0 0

#Then three CC's in a row...
Volume ThreeCardCages
ThreeCardCages.Virtual True
ThreeCardCages.Visibility 1
ThreeCardCages.Material Vacuum
ThreeCardCages.Shape BRIK 50 15 15

For X 3 {-CC_halfwidth*2} {CC_halfwidth*2}
  OneCardCage.Copy CardCage_%X
  CardCage_%X.Position $X 0 0
  CardCage_%X.Mother ThreeCardCages
Done


#Now orient the four sides with three CCs each so that it makes up the 12 cardcages that sit in the top of the electronic bay.

ThreeCardCages.Copy ThreeCCs_1
ThreeCCs_1.Position {-14.6585} {65.34/2+CC_halfdepth} 0
ThreeCCs_1.Color 5
ThreeCCs_1.Mother TwelveCardCages

ThreeCardCages.Copy ThreeCCs_2
ThreeCCs_2.Position {14.6585} {-65.34/2-CC_halfdepth} 0
ThreeCCs_2.Color 6
ThreeCCs_2.Mother TwelveCardCages

ThreeCardCages.Copy ThreeCCs_3
ThreeCCs_3.Position {-65.34/2-CC_halfdepth} {-14.6585} 0
ThreeCCs_3.Color 7
ThreeCCs_3.Rotation 0 0 90
ThreeCCs_3.Mother TwelveCardCages

ThreeCardCages.Copy ThreeCCs_4
ThreeCCs_4.Position {+65.34/2+CC_halfdepth} {14.6585} 0
ThreeCCs_4.Color 8
ThreeCCs_4.Rotation 0 0 90
ThreeCCs_4.Mother TwelveCardCages





