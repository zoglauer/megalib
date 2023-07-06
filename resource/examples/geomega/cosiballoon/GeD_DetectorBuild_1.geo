Volume GeD_Mother_1
GeD_Mother_1.Visibility 1
GeD_Mother_1.Virtual True
GeD_Mother_1.Material mvacuum
GeD_Mother_1.Shape BRIK 10. 3. 7.

Volume GeWafer_1
GeWafer_1.Visibility 1
GeWafer_1.Material mgermanium_ge_recoil
GeWafer_1.Shape BRIK 4.025 4.025 0.725
GeWafer_1.Position 0. 0. -0.025000000000000022
GeWafer_1.Color 2
GeWafer_1.Mother GeD_Mother_1

Volume GeCorner_1
GeCorner_1.Visibility 1
GeCorner_1.Material mvacuum
GeCorner_1.Shape TRD1 0 0.805 0.725 0.403

GeCorner_1.Copy GeCorner_1_0
GeCorner_1_0.Position 3.73 3.73 0
GeCorner_1_0.Rotation 90 -45 0 45 90 -135
GeCorner_1_0.Mother GeWafer_1

GeCorner_1.Copy GeCorner_1_1
GeCorner_1_1.Position -3.73 3.73 0
GeCorner_1_1.Rotation 90 45 0 135 90 -45
GeCorner_1_1.Mother GeWafer_1

GeCorner_1.Copy GeCorner_1_2
GeCorner_1_2.Position -3.73 -3.73 0
GeCorner_1_2.Rotation 90 135 0 -135 90 45
GeCorner_1_2.Mother GeWafer_1

GeCorner_1.Copy GeCorner_1_3
GeCorner_1_3.Position 3.73 -3.73 0
GeCorner_1_3.Rotation 90 -135 0 -45 90 135
GeCorner_1_3.Mother GeWafer_1

#AWL: this is the template file from which each GeD_Mother is built.  the 1 is replaced by the detector number in the python script GeD_12Stack.py

#GeOutRigger 1 and 2 are germanium crystal parts, not sensitive detector, used for mounts on three of the four sides of the wafer. Two trapezoids are staked to make the right shape of 'out rigger'

Volume GeOutRigger1_1
GeOutRigger1_1.Material mgepassive
GeOutRigger1_1.Shape TRD1 1.21 2.88 0.076 0.39
GeOutRigger1_1.Visibility 1

Volume GeOutRigger2_1
GeOutRigger2_1.Material mgepassive
GeOutRigger2_1.Shape TRD1 1.21 2.4389 0.3 0.287
GeOutRigger2_1.Visibility 1

Volume IND0_Copy001_1
IND0_Copy001_1.Position 0 0.295 0
IND0_Copy001_1.Mother GeOutRigger2_1
IND0_Copy001_1.Color 8
IND0_Copy001_1.Material mindium
IND0_Copy001_1.Shape TRD1 1.21 2.4389 0.005 0.287

Volume IND0_Copy002_1
IND0_Copy002_1.Position 0 -0.295 0
IND0_Copy002_1.Mother GeOutRigger2_1
IND0_Copy002_1.Color 8
IND0_Copy002_1.Material mindium
IND0_Copy002_1.Shape TRD1 1.21 2.4389 0.005 0.287

##check the AC vs DC definitions of these below....
GeOutRigger1_1.Copy GeOutRigger_DCSide1_1
GeOutRigger_DCSide1_1.Position -4.415 0 0.6740
GeOutRigger_DCSide1_1.Rotation 90 90 0 180 90 0
GeOutRigger_DCSide1_1.Mother GeD_Mother_1
GeOutRigger_DCSide1_1.Material mgepassive
GeOutRigger_DCSide1_1.Shape TRD1 1.21 2.88 0.076 0.39

GeOutRigger1_1.Copy GeOutRigger_CFSide1_1
GeOutRigger_CFSide1_1.Position 4.4150 0 -0.6740
GeOutRigger_CFSide1_1.Rotation 90 -90 0 0 90 180
GeOutRigger_CFSide1_1.Mother GeD_Mother_1
GeOutRigger_CFSide1_1.Material mgepassive
GeOutRigger_CFSide1_1.Shape TRD1 1.21 2.88 0.076 0.39

GeOutRigger1_1.Copy GeOutRigger_ACSide1_1
GeOutRigger_ACSide1_1.Position -0.0005 -4.415 -0.6740
GeOutRigger_ACSide1_1.Rotation 90 180 0 -90 90 90
GeOutRigger_ACSide1_1.Mother GeD_Mother_1
GeOutRigger_ACSide1_1.Material mgepassive
GeOutRigger_ACSide1_1.Shape TRD1 1.21 2.88 0.076 0.39

GeOutRigger2_1.Copy GeOutRigger_DCSide2_1
GeOutRigger_DCSide2_1.Position -4.5135 0 -0.2980
GeOutRigger_DCSide2_1.Rotation 90 90 0 180 90 0
GeOutRigger_DCSide2_1.Mother GeD_Mother_1
GeOutRigger_DCSide2_1.Material mgepassive
GeOutRigger_DCSide2_1.Shape TRD1 1.21 2.4389 0.3 0.287

GeOutRigger2_1.Copy GeOutRigger_CFSide2_1
GeOutRigger_CFSide2_1.Position 4.5125 0 -0.2980
GeOutRigger_CFSide2_1.Rotation 90 -90 0 0 90 180
GeOutRigger_CFSide2_1.Mother GeD_Mother_1
GeOutRigger_CFSide2_1.Material mgepassive
GeOutRigger_CFSide2_1.Shape TRD1 1.21 2.4389 0.3 0.287

GeOutRigger2_1.Copy GeOutRigger_ACSide2_1
GeOutRigger_ACSide2_1.Position -0.0005 -4.5130 -0.2980
GeOutRigger_ACSide2_1.Rotation 90 180 0 -90 90 90
GeOutRigger_ACSide2_1.Mother GeD_Mother_1
GeOutRigger_ACSide2_1.Material mgepassive
GeOutRigger_ACSide2_1.Shape TRD1 1.21 2.4389 0.3 0.287

## PC Boards mounted to detectors:

##The PC Board on the surface of the Ge wafer on the high voltage side - not always on the top of the detector
Volume LVPCBoard_Top_1 
LVPCBoard_Top_1.Position 0.1445 -4.4000 {-0.75-0.0526-0.075} ##checked
LVPCBoard_Top_1.Rotation 90 180 90 90 180 90
LVPCBoard_Top_1.Mother GeD_Mother_1
LVPCBoard_Top_1.Material mro4003
LVPCBoard_Top_1.Shape BRIK 4.255 1.555 0.075


## The PC Board on the side of the Ge wafer on the high voltage side - the terminus.
Volume LVPCBoard_Side_1
LVPCBoard_Side_1.Position 0.1445 -6.1200 {-0.75-0.0526 + 0.978 - {0.075*2}}  ##checked
LVPCBoard_Side_1.Rotation 90 180 90 -90 0 0
LVPCBoard_Side_1.Mother GeD_Mother_1
LVPCBoard_Side_1.Material mrotmm3
LVPCBoard_Side_1.Shape BRIK 4.546 0.158 0.978

# Cutout in the LVPCBoard_Top
Volume LVPCBoard_Cutout_Copy_1
LVPCBoard_Cutout_Copy_1.Position 0.0985 1.13 0
LVPCBoard_Cutout_Copy_1.Rotation 90 0 0 90 90 -90
LVPCBoard_Cutout_Copy_1.Mother LVPCBoard_Top_1
LVPCBoard_Cutout_Copy_1.Material mvacuum
LVPCBoard_Cutout_Copy_1.Shape TRD1 3.715 2.8725 0.075 0.425

Volume HVPCBoard_Top_1
HVPCBoard_Top_1.Position -4.6940 -0.1455 0.9670
HVPCBoard_Top_1.Rotation 90 180 90 -90 0 0
HVPCBoard_Top_1.Mother GeD_Mother_1
HVPCBoard_Top_1.Material mro4003
HVPCBoard_Top_1.Shape BRIK 1.563 4.458 0.075

Volume HVPCBoard_Side_1
HVPCBoard_Side_1.Position -6.4210 -0.1455 0.0620
HVPCBoard_Side_1.Rotation 90 180 90 -90 0 0
HVPCBoard_Side_1.Mother GeD_Mother_1
HVPCBoard_Side_1.Material mrotmm3
HVPCBoard_Side_1.Shape BRIK 0.164 4.661 0.978


Volume HVPCBoard_Cutout_Copy_1
HVPCBoard_Cutout_Copy_1.Position -1.135 0.0935 0
HVPCBoard_Cutout_Copy_1.Rotation 90 90 0 180 90 0
HVPCBoard_Cutout_Copy_1.Mother HVPCBoard_Top_1
HVPCBoard_Cutout_Copy_1.Material mvacuum
HVPCBoard_Cutout_Copy_1.Shape TRD1 3.71 2.8665 0.075 0.428

Volume BASE_Copy1_1
BASE_Copy1_1.Position  4.6495 -0.8200  0.1100
BASE_Copy1_1.Rotation 90 180 90 -90 0 0
BASE_Copy1_1.Color 3
BASE_Copy1_1.Mother GeD_Mother_1
BASE_Copy1_1.Material mdetector_al6061
BASE_Copy1_1.Shape BRIK 0.425 4.53 0.1

Volume BAS0_Copy1_1
BAS0_Copy1_1.Position -4.6705 -0.4200  0.1100
BAS0_Copy1_1.Rotation 90 180 90 -90 0 0
BAS0_Copy1_1.Mother GeD_Mother_1
BAS0_Copy1_1.Color 3
BAS0_Copy1_1.Material mdetector_al6061
BAS0_Copy1_1.Shape BRIK 0.425 4.93 0.1

Volume BAS1_Copy1_1
BAS1_Copy1_1.Position -0.0105 -4.7900  0.1100
BAS1_Copy1_1.Rotation 90 180 90 -90 0 0
BAS1_Copy1_1.Color 4
BAS1_Copy1_1.Mother GeD_Mother_1
BAS1_Copy1_1.Material mdetector_al6061
BAS1_Copy1_1.Shape BRIK 4.235 0.56 0.1

##Obsolete volumes from NCT '09 detector mounts
Volume BOTT_Copy1_1
BOTT_Copy1_1.Position -5.0305 -0.4200 -0.1310
BOTT_Copy1_1.Rotation 90 180 90 -90 0 0
BOTT_Copy1_1.Color 5
BOTT_Copy1_1.Mother GeD_Mother_1
BOTT_Copy1_1.Material mdetector_al6061
BOTT_Copy1_1.Shape BRIK 0.065 4.93 0.141

Volume BOT0_Copy1_1
BOT0_Copy1_1.Position -0.0105 -5.1600 -0.1310
BOT0_Copy1_1.Rotation 90 180 90 -90 0 0
BOT0_Copy1_1.Color 6
BOT0_Copy1_1.Mother GeD_Mother_1
BOT0_Copy1_1.Material mdetector_al6061
BOT0_Copy1_1.Shape BRIK 4.955 0.19 0.141

Volume BOT1_Copy1_1
BOT1_Copy1_1.Position  5.0095 -0.8200 -0.1310
BOT1_Copy1_1.Rotation 90 180 90 -90 0 0
BOT1_Copy1_1.Color 7
BOT1_Copy1_1.Mother GeD_Mother_1
BOT1_Copy1_1.Material mdetector_al6061
BOT1_Copy1_1.Shape BRIK 0.065 4.53 0.141

Volume BOT2_Copy1_1
#BOT2_Copy1_1.Position -4.6055  3.6350 -0.1310
BOT2_Copy1_1.Position -4.6055  3.256 -0.1310
BOT2_Copy1_1.Rotation 90 90 90 180 0 0
BOT2_Copy1_1.Mother GeD_Mother_1
BOT2_Copy1_1.Color 8
BOT2_Copy1_1.Material mdetector_al6061
BOT2_Copy1_1.Shape TRAP 0.141 0 0 0.36 0.875 1.625 313.831 0.36 0.875 1.625 313.831

Volume BOT3_Copy1_1
BOT3_Copy1_1.Position -4.6055 -3.6950 -0.1310
BOT3_Copy1_1.Rotation 90 -90 90 -0 0 0
BOT3_Copy1_1.Color 9
BOT3_Copy1_1.Mother GeD_Mother_1
BOT3_Copy1_1.Material mdetector_al6061
BOT3_Copy1_1.Shape TRAP 0.141 0 0 0.36 1.65 0.9 46.1691 0.36 1.65 0.9 46.1691

Volume BOT4_Copy1_1
#BOT4_Copy1_1.Position  4.5845  3.2600 -0.1310
BOT4_Copy1_1.Position  4.5845  2.870 -0.1310
BOT4_Copy1_1.Rotation 90 -90 90 -0 0 0
BOT4_Copy1_1.Color 5
BOT4_Copy1_1.Mother GeD_Mother_1
BOT4_Copy1_1.Material mdetector_al6061
BOT4_Copy1_1.Shape TRAP 0.141 0 0 0.36 0.45 1.2 46.1691 0.36 0.45 1.2 46.1691

Volume BOT5_Copy1_1
BOT5_Copy1_1.Position  4.5845 -3.6950 -0.1310
BOT5_Copy1_1.Rotation 90 90 90 -0 180 -0
BOT5_Copy1_1.Color 3
BOT5_Copy1_1.Mother GeD_Mother_1
BOT5_Copy1_1.Material mdetector_al6061
BOT5_Copy1_1.Shape TRAP 0.141 0 0 0.36 0.9 1.65 46.1691 0.36 0.9 1.65 46.1691

Volume BOT6_Copy1_1
BOT6_Copy1_1.Position -3.2205 -4.6000 -0.1310
BOT6_Copy1_1.Rotation 90 -0 90 -90 180 180
BOT6_Copy1_1.Color 4
BOT6_Copy1_1.Mother GeD_Mother_1
BOT6_Copy1_1.Material mdetector_al6061
BOT6_Copy1_1.Shape TRAP 0.141 0 0 0.37 0.65 1.4 45.3845 0.37 0.65 1.4 45.3845

Volume BOT7_Copy1_1
BOT7_Copy1_1.Position  3.1995 -4.6000 -0.1310
BOT7_Copy1_1.Rotation 90 180 90 -90 0 0
BOT7_Copy1_1.Color 5
BOT7_Copy1_1.Mother GeD_Mother_1
BOT7_Copy1_1.Material mdetector_al6061
BOT7_Copy1_1.Shape TRAP 0.141 0 0 0.37 0.65 1.4 45.3845 0.37 0.65 1.4 45.3845

Volume BOT8_Copy1_1
BOT8_Copy1_1.Position -0.0105 -5.0200 -0.5070 ##checked
BOT8_Copy1_1.Rotation 90 180 90 -90 0 0
BOT8_Copy1_1.Color 6
BOT8_Copy1_1.Mother GeD_Mother_1
BOT8_Copy1_1.Material mdetector_al6061
BOT8_Copy1_1.Shape BRIK 4.235 0.05 0.235

Volume BO00_Copy1_1
BO00_Copy1_1.Position -4.3255 -4.3700 -0.5070
BO00_Copy1_1.Rotation 90 180 90 -90 0 0
BO00_Copy1_1.Color 7
BO00_Copy1_1.Mother GeD_Mother_1
BO00_Copy1_1.Material mdetector_al6061
BO00_Copy1_1.Shape BRIK 0.08 0.7 0.235

Volume BO01_Copy1_1
BO01_Copy1_1.Position -4.6955 -4.3700 -0.5070
BO01_Copy1_1.Rotation 90 180 90 -90 0 0
BO01_Copy1_1.Color 8
BO01_Copy1_1.Mother GeD_Mother_1
BO01_Copy1_1.Material msteel_18_8
BO01_Copy1_1.Shape TUBS 0 0.2 0.235 0 360

Volume BO02_1
BO02_1.Material mdetector_al6061
BO02_1.Shape BRIK 0.25 0.7 0.235

Volume BO03_1
BO03_1.Position 0 0 0
BO03_1.Mother BO02_1
BO03_1.Color 9
BO03_1.Material msteel_18_8
BO03_1.Shape TUBS 0 0.2 0.235 0 360

BO02_1.Copy BO02_Copy1_1
BO02_Copy1_1.Position  4.4745 -4.3700 -0.5070
BO02_Copy1_1.Rotation 90 180 90 -90 0 0
BO02_Copy1_1.Color 9
BO02_Copy1_1.Mother GeD_Mother_1

Volume BO04_Copy1_1
#BO04_Copy1_1.Position -4.5655  0.0200 -0.7800
BO04_Copy1_1.Position  -4.5655  0.0200 -1.1850
BO04_Copy1_1.Rotation 90 180 90 -90 0 0
BO04_Copy1_1.Color 4
BO04_Copy1_1.Mother GeD_Mother_1
BO04_Copy1_1.Material mdetector_al6061
BO04_Copy1_1.Shape BRIK 0.32 3.18 0.115

Volume BO04_Copy2_1
#BO04_Copy2_1.Position  4.5445  0.0200 -0.7800
BO04_Copy2_1.Position  4.5445  0.0200 -1.1850
BO04_Copy2_1.Rotation 90 180 90 -90 0 0
BO04_Copy2_1.Color 3
BO04_Copy2_1.Mother GeD_Mother_1
BO04_Copy2_1.Material mdetector_al6061
BO04_Copy2_1.Shape BRIK 0.32 3.18 0.115

Volume BO05_Copy1_1
#BO05_Copy1_1.Position -4.5655  0.0200 -0.8430
BO05_Copy1_1.Position -4.5655  0.0200 -1.3640
BO05_Copy1_1.Rotation 90 180 90 -90 0 0
BO05_Copy1_1.Color 4
BO05_Copy1_1.Mother GeD_Mother_1
BO05_Copy1_1.Material mdetector_al6061
BO05_Copy1_1.Shape BRIK 0.32 2.324 0.064

Volume BO05_Copy2_1
#BO05_Copy2_1.Position  4.5445  0.0200 -0.8430
BO05_Copy2_1.Position 4.5445 0.0200 -1.364
BO05_Copy2_1.Rotation 90 180 90 -90 0 0
BO05_Copy2_1.Color 5
BO05_Copy2_1.Mother GeD_Mother_1
BO05_Copy2_1.Material mdetector_al6061
BO05_Copy2_1.Shape BRIK 0.32 2.324 0.064

Volume BO06_Copy1_1
#BO06_Copy1_1.Position -4.6255 -2.8500 -0.5140
BO06_Copy1_1.Position -4.6255 -2.8500 {-0.5140-0.129}
BO06_Copy1_1.Rotation 90 180 90 -90 0 0
BO06_Copy1_1.Color 6
BO06_Copy1_1.Mother GeD_Mother_1
BO06_Copy1_1.Material msteel_18_8
BO06_Copy1_1.Shape TUBS 0 0.11 0.371 0 360

Volume BO06_Copy2_1
BO06_Copy2_1.Position -4.6255  2.9500 {-0.5140-0.129}
BO06_Copy2_1.Rotation 90 180 90 -90 0 0
BO06_Copy2_1.Color 7
BO06_Copy2_1.Mother GeD_Mother_1
BO06_Copy2_1.Material msteel_18_8
BO06_Copy2_1.Shape TUBS 0 0.11 0.371 0 360

Volume BO06_Copy3_1
BO06_Copy3_1.Position  4.5445 -2.8500 {-0.5140-0.129}
BO06_Copy3_1.Rotation 90 180 90 -90 0 0
BO06_Copy3_1.Color 8
BO06_Copy3_1.Mother GeD_Mother_1
BO06_Copy3_1.Material msteel_18_8
BO06_Copy3_1.Shape TUBS 0 0.11 0.371 0 360

Volume BO06_Copy4_1
BO06_Copy4_1.Position  4.5445  2.9500 {-0.5140-0.129}
BO06_Copy4_1.Rotation 90 180 90 -90 0 0
BO06_Copy4_1.Color 9
BO06_Copy4_1.Mother GeD_Mother_1
BO06_Copy4_1.Material msteel_18_8
BO06_Copy4_1.Shape TUBS 0 0.11 0.371 0 360

Volume BO07_Copy1_1
BO07_Copy1_1.Position  5.0015 -5.6680 -0.0310
BO07_Copy1_1.Rotation 90 180 90 -90 0 0
BO07_Copy1_1.Color 4
BO07_Copy1_1.Mother GeD_Mother_1
BO07_Copy1_1.Material mdetector_al6061
BO07_Copy1_1.Shape BRIK 0.073 0.318 0.241

Volume COOL_1
COOL_1.Material mdetector_al6061
COOL_1.Shape BRIK 0.177 1.749 1.201

Volume COO0_1
COO0_1.Position 0 0.68 0
COO0_1.Mother COOL_1
COO0_1.Color 4
COO0_1.Material mvacuum
COO0_1.Shape BRIK 0.177 0.58 0.95

Volume COO1_1
COO1_1.Position 0 -0.68 0
COO1_1.Mother COOL_1
COO1_1.Color 5
COO1_1.Material mvacuum
COO1_1.Shape BRIK 0.177 0.58 0.95

COOL_1.Copy COOL_Copy1_1
COOL_Copy1_1.Position  5.2515 -4.2370  0.9290
COOL_Copy1_1.Rotation 90 180 90 -90 0 0
COOL_Copy1_1.Color 3
COOL_Copy1_1.Mother GeD_Mother_1

Volume BO08_Copy1_1
BO08_Copy1_1.Position  5.0015  1.7620 -0.7220
BO08_Copy1_1.Rotation 90 180 90 -90 0 0
BO08_Copy1_1.Color 4
BO08_Copy1_1.Mother GeD_Mother_1
BO08_Copy1_1.Material mdetector_al6061
BO08_Copy1_1.Shape BRIK 0.073 0.26 0.45

Volume BO10_Copy1_1
BO10_Copy1_1.Position -5.0145 -5.0550 -0.7320
BO10_Copy1_1.Rotation 90 180 90 -90 0 0
BO10_Copy.Color 5
BO10_Copy1_1.Mother GeD_Mother_1
BO10_Copy1_1.Material mdetector_al6061
BO10_Copy1_1.Shape BRIK 0.081 0.295 0.46

Volume BO11_Copy1_1
BO11_Copy1_1.Position  4.8204 -4.7750 -0.7320
BO11_Copy1_1.Rotation 90 180 90 -90 0 0
BO11_Copy1_1.Color 6
BO11_Copy1_1.Mother GeD_Mother_1
BO11_Copy1_1.Material mdetector_al6061
BO11_Copy1_1.Shape BRIK 0.081 0.295 0.46

Volume BO12_Copy1_1
BO12_Copy1_1.Position -4.8455  4.2100 -0.7320
BO12_Copy1_1.Rotation 90 180 90 -90 0 0
BO12_Copy1_1.Color 7
BO12_Copy1_1.Mother GeD_Mother_1
BO12_Copy1_1.Material mdetector_al6061
BO12_Copy1_1.Shape BRIK 0.25 0.15 0.46

Volume TOPS_1
TOPS_1.Material mdetector_al6061
TOPS_1.Shape BRIK 0.425 0.71 0.315

Volume TOP0_1
TOP0_1.Position 0 0 0
TOP0_1.Mother TOPS_1
TOP0_1.Color 6
TOP0_1.Material msteel_18_8
TOP0_1.Shape TUBS 0 0.2 0.295 0 360

TOPS_1.Copy TOPS_Copy1_1
TOPS_Copy1_1.Position -4.6705 -4.6400  0.5250
TOPS_Copy1_1.Rotation 90 180 90 -90 0 0
TOPS_Copy1_1.Color 8
TOPS_Copy1_1.Mother GeD_Mother_1


Volume TOP1_Copy1_1
TOP1_Copy1_1.Position -4.7625 -5.0550  1.0700
TOP1_Copy1_1.Rotation 90 180 90 -90 0 0
TOP1_Copy1_1.Color 9
TOP1_Copy1_1.Mother GeD_Mother_1
TOP1_Copy1_1.Material mdetector_al6061
TOP1_Copy1_1.Shape BRIK 0.333 0.295 0.23


#This volume was causing an overlap. For now, I'm going to just comment it out. When I have more time, I'll redo the whole GeD volme definitions.
#AWL Aug 4, 2015: this comment ^^ says that TOP2 is causing the problem... running the overlap checker now says that TOP3 is causing the problem.  commenting out below...
Volume TOP2_Copy1_1
TOP2_Copy1_1.Position -4.6845 -5.0550  1.7200
TOP2_Copy1_1.Color 3
TOP2_Copy1_1.Rotation 90 180 90 -90 0 0
TOP2_Copy1_1.Mother GeD_Mother_1
TOP2_Copy1_1.Material mdetector_al6061
TOP2_Copy1_1.Shape BRIK 0.248 0.295 0.42

BeginComment
Volume TOP3_Copy1_1
TOP3_Copy1_1.Position -4.3185  0.2900  0.5250
TOP3_Copy1_1.Rotation 90 180 90 -90 0 0
TOP3_Copy1_1.Mother GeD_Mother_1
TOP3_Copy1_1.Color 4
TOP3_Copy1_1.Material mdetector_al6061
TOP3_Copy1_1.Shape BRIK 0.073 4.22 0.315
EndComment

Volume TOP4_Copy1_1
TOP4_Copy1_1.Position  0.3415 -5.2770  0.5250
TOP4_Copy1_1.Rotation 90 180 90 -90 0 0
TOP4_Copy1_1.Color 5
TOP4_Copy1_1.Mother GeD_Mother_1
TOP4_Copy1_1.Material mdetector_al6061
TOP4_Copy1_1.Shape BRIK 4.587 0.073 0.315

Volume TOP5_Copy1_1
TOP5_Copy1_1.Position  5.0015 -4.2370  1.1700
TOP5_Copy1_1.Rotation 90 180 90 -90 0 0
TOP5_Copy1_1.Color 6
TOP5_Copy1_1.Mother GeD_Mother_1
TOP5_Copy1_1.Material mdetector_al6061
TOP5_Copy1_1.Shape BRIK 0.073 1.749 0.96

Volume TOP6_Copy1_1
TOP6_Copy1_1.Position  5.0015 -0.0180  0.5250
TOP6_Copy1_1.Rotation 90 180 90 -90 0 0
TOP6_Copy1_1.Color 3
TOP6_Copy1_1.Mother GeD_Mother_1
TOP6_Copy1_1.Material mdetector_al6061
TOP6_Copy1_1.Shape BRIK 0.073 2.47 0.315

Volume TOP7_Copy1_1
TOP7_Copy1_1.Position  5.0015  1.5070  1.0800
TOP7_Copy1_1.Rotation 90 180 90 -90 0 0
TOP7_Copy1_1.Color 5
TOP7_Copy1_1.Mother GeD_Mother_1
TOP7_Copy1_1.Material mdetector_al6061
TOP7_Copy1_1.Shape BRIK 0.073 0.515 0.24

#This volume was causeing an overlap. For now, I'm going to just comment it out. When I have more time, I'll redo the whole GeD volume definitions.
BeginComment
Volume TOP8_Copy1_1
TOP8_Copy1_1.Position  5.0015  1.2470  1.7100
TOP8_Copy1_1.Rotation 90 180 90 -90 0 0
TOP8_Copy1_1.Color 4
TOP8_Copy1_1.Mother GeD_Mother_1
EndComment

Volume TO00_1
TO00_1.Material mdetector_al6061
TO00_1.Shape BRIK 0.352 0.325 0.315

Volume TO01_Copy001_1
TO01_Copy001_1.Position 0 0 0
TO01_Copy001_1.Mother TO00_1
TO01_Copy001_1.Color 7
TO01_Copy001_1.Material msteel_18_8
TO01_Copy001_1.Shape TUBS 0 0.15 0.315 0 360

TO00_1.Copy TO00_Copy1_1
TO00_Copy1_1.Position -4.7435  4.1850  0.5250
TO00_Copy1_1.Rotation 90 180 90 -90 0 0
TO00_Copy1_1.Color 6
TO00_Copy1_1.Mother GeD_Mother_1

Volume TO02_Copy1_1
TO02_Copy1_1.Position -4.8455  4.4350  1.4800
TO02_Copy1_1.Rotation 90 180 90 -90 0 0
TO02_Copy1_1.Color 7
TO02_Copy1_1.Mother GeD_Mother_1
TO02_Copy1_1.Material mdetector_al6061
TO02_Copy1_1.Shape BRIK 0.25 0.075 0.64

