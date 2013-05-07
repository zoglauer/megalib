

// One layer of the tracker
Volume SiStripDetector 
SiStripDetector.Material PCB
SiStripDetector.Visibility 0
SiStripDetector.Shape BRIK  23.2  23.2  0.025

// One Si-Wafer (6x6) of the tracker
Volume Wafer    
Wafer.Material Silicon
Wafer.Visibility 1
Wafer.Color 7
Wafer.Shape BRIK 3.15 3.15 0.025

//
Volume WaferLine
WaferLine.Material PCB
WaferLine.Visibility 0
WaferLine.Color 5
WaferLine.Shape BRIK 19.2  3.2 0.025

// Some plastic around the wafer (Readout-electronics...)
Volume PCBLong                 
PCBLong.Material PCB
PCBLong.Color 3
PCBLong.Visibility 0
PCBLong.Shape BRIK  23.2  2.0  0.025

// Again some plastic around the wafer (Readout-electronics...)
Volume PCBSmall               
PCBSmall.Material PCB
PCBSmall.Color 3
PCBSmall.Visibility 0
PCBSmall.Shape BRIK  2.0 19.1 0.025



Wafer.Copy w01
w01.Position  16.0  0.0  0.0
w01.Mother WaferLine

Wafer.Copy w02
w02.Position  9.6  0.0  0.0
w02.Mother WaferLine

Wafer.Copy w03
w03.Position  3.2  0.0  0.0
w03.Mother WaferLine

Wafer.Copy w04
w04.Position  -3.2  0.0  0.0
w04.Mother WaferLine

Wafer.Copy w05
w05.Position  -9.6  0.0  0.0
w05.Mother WaferLine

Wafer.Copy w06
w06.Position  -16.0  0.0   0.0
w06.Mother WaferLine


WaferLine.Copy WaferLineN01
WaferLineN01.Position  0.0  16.0  0.0
WaferLineN01.Mother SiStripDetector

WaferLine.Copy WaferLineN02
WaferLineN02.Position  0.0  9.6  0.0
WaferLineN02.Mother SiStripDetector

WaferLine.Copy WaferLineN03
WaferLineN03.Position  0.0  3.2  0.0
WaferLineN03.Mother SiStripDetector

WaferLine.Copy WaferLineN04
WaferLineN04.Position  0.0  -3.2  0.0
WaferLineN04.Mother SiStripDetector

WaferLine.Copy WaferLineN05
WaferLineN05.Position  0.0  -9.6  0.0
WaferLineN05.Mother SiStripDetector

WaferLine.Copy WaferLineN06
WaferLineN06.Position  0.0  -16.0  0.0
WaferLineN06.Mother SiStripDetector



// Add some plastic:

PCBLong.Copy pl1
pl1.Position 0.0 21.2 0.0
pl1.Mother SiStripDetector

PCBLong.Copy pl2
pl2.Position 0.0 -21.2 0.0
pl2.Mother SiStripDetector


PCBSmall.Copy pl3
pl3.Position 21.2  0.0  0.0
pl3.Mother SiStripDetector

PCBSmall.Copy pl4
pl4.Position -21.2  0.0  0.0
pl4.Mother SiStripDetector

