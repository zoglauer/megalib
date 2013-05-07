

// One layer of the tracker
Volume SiStripDetector 
SiStripDetector.Material PCB
SiStripDetector.Visibility 0
SiStripDetector.Shape BRIK 11.7  11.7  0.025


// One Si-Wafer (6x6) of the tracker
Volume Wafer    
Wafer.Material Silicon
Wafer.Visibility 1
#Wafer.LineWidth 3
Wafer.Color 0
Wafer.Shape BRIK 3.15 3.15 0.025


// Some plastic around the wafer (Readout-electronics...)
Volume PCBLong                 
PCBLong.Material PCB
PCBLong.Color 30
PCBLong.Visibility 0
PCBLong.Shape BRIK 2.0 11.6 0.025


// Again some plastic around the wafer (Readout-electronics...)
Volume PCBSmall               
PCBSmall.Material PCB
PCBSmall.Color 30
PCBSmall.Visibility 0
PCBSmall.Shape BRIK 9.6 2.0 0.025




Wafer.Copy WaferN01
WaferN01.Position  8.45 8.45 0.0
WaferN01.Mother SiStripDetector

Wafer.Copy WaferN02
WaferN02.Position  8.45 2.05 0.0
WaferN02.Mother SiStripDetector

Wafer.Copy WaferN03
WaferN03.Position  8.45 -4.35 0.0
WaferN03.Mother SiStripDetector

Wafer.Copy WaferN04
WaferN04.Position  2.05 8.45 0.0
WaferN04.Mother SiStripDetector

Wafer.Copy WaferN05
WaferN05.Position  2.05 2.05 0.0
WaferN05.Mother SiStripDetector

Wafer.Copy WaferN06
WaferN06.Position  2.05 -4.35 0.0
WaferN06.Mother SiStripDetector

Wafer.Copy WaferN07
WaferN07.Position  -4.35 8.45 0.0
WaferN07.Mother SiStripDetector

Wafer.Copy WaferN08
WaferN08.Position  -4.35 2.05 0.0
WaferN08.Mother SiStripDetector

Wafer.Copy WaferN09
WaferN09.Position  -4.35 -4.35 0.0
WaferN09.Mother SiStripDetector


PCBSmall.Copy lp1
lp1.Position 2.0 -9.6 0.0
lp1.Mother SiStripDetector

PCBLong.Copy lp2
lp2.Position -9.6 0.0 0.0
lp2.Mother SiStripDetector