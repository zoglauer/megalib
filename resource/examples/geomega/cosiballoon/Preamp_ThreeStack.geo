#File defines three pre-amp boxes stacked verically. The pre-amps boxes are approximated as aluminum boxes with four boards (material = ro3003), to model the individuale pre-amp circuit boards.

Include Intro.geo

Constant preamps_zoffset 2.6162
###The distance between the centers of two neighbouring pre-amp boxes.

Volume Preamp_ThreeStack
Preamp_ThreeStack.Visibility 1
Preamp_ThreeStack.Material Vacuum
Preamp_ThreeStack.Virtual True
Preamp_ThreeStack.Shape BRIK 2.6 6 4

Volume PreamplifierBox
PreamplifierBox.Visibility 1
PreamplifierBox.Material al6061
PreamplifierBox.Shape BRIK 2.54 5.715 1.27
###The volume of the pre-amp boxes is off compared to the solid works file, solidworks says: BRIK 2.3005 5.6515 1.257, but that doesn't include the preampt connectors and extra bits of aluminum out to the sides, so I think it's probably a pretty good approximation.

Volume PreamplifierBoxVacuum
PreamplifierBoxVacuum.Visibility 1
PreamplifierBoxVacuum.Material Vacuum
PreamplifierBoxVacuum.Shape BRIK 2.44 5.615 1.17
PreamplifierBoxVacuum.Position 0 0 0
PreamplifierBoxVacuum.Mother PreamplifierBox

Volume PreamplifierBoxBoard
PreamplifierBoxBoard.Visibility 0
PreamplifierBoxBoard.Material ro3003
PreamplifierBoxBoard.Shape BRIK 2.44 5.615 0.05

For Z 4 -0.702 0.468
  PreamplifierBoxBoard.Copy PreamplifierBoxBoard_%Z
  PreamplifierBoxBoard_%Z.Position 0 0 $Z
  PreamplifierBoxBoard_%Z.Mother PreamplifierBoxVacuum
Done

For Z 3 -preamps_zoffset preamps_zoffset
  PreamplifierBox.Copy PreamplifierBox_%Z
  PreamplifierBox_%Z.Position 0 0 $Z
  PreamplifierBox_%Z.Mother Preamp_ThreeStack
Done



