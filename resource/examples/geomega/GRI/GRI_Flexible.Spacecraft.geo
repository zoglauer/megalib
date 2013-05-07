!***********************************
!***   MAX Detector Spacecraft   ***
!***********************************


Include GRI_Flexible.Intro.geo


Volume Spacecraft
Spacecraft.Material vacuum
Spacecraft.Shape TUBS 0 60 185 0 360 
Spacecraft.Position 0 0 0
Spacecraft.Mother VAC0
!Spacecraft.Virtual true
Spacecraft.Visibility 0


!--- s/c body: outside wall and plates ---!

! outside of s/c cylinder
Volume SCSD
SCSD.Visibility 1
SCSD.Material al_2024_5056
SCSD.Shape TUBS 44 46.5 86 0 360 
SCSD.Position 0 0 0
SCSD.Mother Spacecraft

! top, bottom, and interior panels
Volume SCPT
SCPT.Visibility 1
SCPT.Material al_2024_5056
SCPT.Shape TUBS 0 44 1.25 0 360 

SCPT.Copy SCPT_Copy1
SCPT_Copy1.Position 0 0 84.75
SCPT_Copy2.Mother Spacecraft

SCPT.Copy SCPT_Copy2
SCPT_Copy2.Position 0 0 -84.75
SCPT_Copy2.Mother Spacecraft

SCPT.Copy SCPT_Copy3
SCPT_Copy3.Position 0 0 -40.25
SCPT_Copy3.Mother Spacecraft

! miscellanous support
Volume SCMS
SCMS.Visibility 1
SCMS.Material al_2024_5056
SCMS.Shape TUBS 0 44 1.456 0 360 
SCMS.Position 0 0 41.056
SCMS.Mother Spacecraft

!--- hydrazine tanks ---!

! mother volume
Volume HYTK
HYTK.Visibility 1
HYTK.Material vacuum
HYTK.Shape SPHE 0 39.3 0 180 0 360
HYTK.Position 0 0 0.3
HYTK.Mother Spacecraft

! tank walls
Volume HYTW
HYTW.Visibility 1
HYTW.Material ti_6
HYTW.Shape SPHE 38.3 39.3 0 180 0 360
HYTW.Position 0 0 0
HYTW.Mother HYTK

! hydrazine
Volume HYTH
HYTH.Visibility 1
HYTH.Material prop_mixt
HYTH.Shape SPHE 0 38.3 0 180 0 360
HYTH.Position 0 0 0
HYTH.Mother HYTK

!--- cold gas tanks ---!

! mother volume
Volume CGTK
CGTK.Visibility 1
CGTK.Material vacuum
CGTK.Shape SPHE 0 21 0 180 0 360

CGTK.Copy CGTK_Copy1
CGTK_Copy1.Position 22 0 -62.5
CGTK_Copy1.Mother Spacecraft

CGTK.Copy CGTK_Copy2
CGTK_Copy2.Position -22 0 -62.5
CGTK_Copy2.Mother Spacecraft

Volume CGTW
CGTW.Visibility 1
CGTW.Material c_epoxy
CGTW.Shape SPHE 20 21 0 180 0 360
CGTW.Position 0 0 0
CGTW.Mother CGTK

Volume CGTG
CGTG.Visibility 1
CGTG.Material cold_gas
CGTG.Shape SPHE 0 20 0 180 0 360
CGTG.Position 0 0 0
CGTG.Mother CGTK

Volume PPPL
PPPL.Visibility 1
PPPL.Material inox1810
PPPL.Shape TUBS 43 44 0.5 0 360 


PPPL.Copy PPPL_Copy1
PPPL_Copy1.Position 0 0 83
PPPL_Copy1.Mother Spacecraft

PPPL.Copy PPPL_Copy2
PPPL_Copy2.Position 0 0 -83
PPPL_Copy2.Mother Spacecraft


!--- miscellanoues electronics:                 ---!
!--- battery, harness, power control drive unit ---!

! mother volume
Volume MSEL
MSEL.Visibility 1
MSEL.Material vacuum
MSEL.Shape BRIK 10 10 10

Volume MEAH
MEAH.Visibility 1
MEAH.Material al6061
MEAH.Shape BRIK 10 10 1.839

Volume MEBY
MEBY.Visibility 1
MEBY.Material nicd_cells
MEBY.Shape BRIK 10 10 3.474

Volume MEHS
MEHS.Visibility 1
MEHS.Material isot_cu
MEHS.Shape BRIK 10 10 0.278

Volume MECU
MECU.Visibility 1
MECU.Material electr
MECU.Shape BRIK 10 10 0.744

Volume AELB
AELB.Visibility 1
AELB.Material vacuum
AELB.Shape BRIK 10 10 10

Volume AEAH
AEAH.Visibility 1
AEAH.Material al6061
AEAH.Shape BRIK 10 10 1.019

Volume AEEL
AEEL.Visibility 1
AEEL.Material electr
AEEL.Shape BRIK 10 10 2.475

MSEL.Position 25 0 52.512
MSEL.Mother Spacecraft

MEAH.Position 0 0 -8.161
MEAH.Mother MSEL

MEBY.Position 0 0 -1
MEBY.Mother MSEL

MEHS.Position 0 0 3
MEHS.Mother MSEL

MECU.Position 0 0 9.256
MECU.Mother MSEL

AELB.Position -25 0 52.512
AELB.Mother Spacecraft

AEAH.Position 0 0 -8.981
AEAH.Mother AELB

AEEL.Position 0 0 7.525
AEEL.Mother AELB


Volume TCEB
TCEB.Visibility 1
TCEB.Material vacuum
TCEB.Shape BRIK 10 10 10

Volume TCAH
TCAH.Visibility 1
TCAH.Material al6061
TCAH.Shape BRIK 10 10 0.446

Volume TCEL
TCEL.Visibility 1
TCEL.Material electr
TCEL.Shape BRIK 10 10 1.082

TCEB.Position 0 25 52.512
TCEB.Mother Spacecraft

TCAH.Position 0 0 -9.554
TCAH.Mother TCEB

TCEL.Position 0 0 8.918
TCEL.Mother TCEB



!--- ACS thrusters ---!

! mother volume
Volume ACST
ACST.Visibility 1
ACST.Material vacuum
ACST.Shape TUBS 0 5 5 0 360 

ACST.Copy ACST_Copy1
ACST_Copy1.Position 40 0 91
ACST_Copy1.Mother Spacecraft

ACST.Copy ACST_Copy2
ACST_Copy2.Position 20 34.64 91
ACST_Copy2.Mother Spacecraft

ACST.Copy ACST_Copy3
ACST_Copy3.Position -20 34.64 91
ACST_Copy3.Mother Spacecraft

ACST.Copy ACST_Copy4
ACST_Copy4.Position -40 0 91
ACST_Copy4.Mother Spacecraft

ACST.Copy ACST_Copy5
ACST_Copy5.Position -20 -34.64 91
ACST_Copy5.Mother Spacecraft

ACST.Copy ACST_Copy6
ACST_Copy6.Position 20 -34.64 91
ACST_Copy6.Mother Spacecraft

ACST.Copy ACST_Copy7
ACST_Copy7.Position 40 0 -91
ACST_Copy7.Mother Spacecraft

ACST.Copy ACST_Copy8
ACST_Copy8.Position 20 34.64 -91
ACST_Copy8.Mother Spacecraft

ACST.Copy ACST_Copy9
ACST_Copy9.Position -20 34.64 -91
ACST_Copy9.Mother Spacecraft

ACST.Copy ACST_Copy10
ACST_Copy10.Position -40 0 -91
ACST_Copy10.Mother Spacecraft

ACST.Copy ACST_Copy11
ACST_Copy11.Position -20 -34.64 -91
ACST_Copy11.Mother Spacecraft

ACST.Copy ACST_Copy12
ACST_Copy12.Position 20 -34.64 -91
ACST_Copy12.Mother Spacecraft


Volume THRU
THRU.Visibility 1
THRU.Material inox1810
THRU.Shape TUBS 4.726 5 5 0 360 
THRU.Position 0 0 0
THRU.Mother ACST

!--- reaction wheels ---!

! mother volume
Volume RCWL
RCWL.Visibility 1
RCWL.Material vacuum
RCWL.Shape TUBS 0 15 10 0 360 
RCWL.Position 0 0 73.5
RCWL.Mother Spacecraft

Volume RWLS
RWLS.Visibility 1
RWLS.Material inox1810
RWLS.Shape TUBS 14.804 15 10 0 360 
RWLS.Position 0 0 0
RWLS.Mother RCWL

!--- star trackers ---!

! mother volume
Volume STTK
STTK.Visibility 1
STTK.Material vacuum
STTK.Shape BRIK 5 10 15

STTK.Copy STTK_Copy1
STTK_Copy1.Position 51.5 0 71
STTK_Copy1.Mother Spacecraft

STTK.Copy STTK_Copy2
STTK_Copy2.Position -51.5 0 71
STTK_Copy2.Mother Spacecraft



Volume STAH
STAH.Visibility 1
STAH.Material al6061
STAH.Shape BRIK 5 10 0.387
STAH.Position 0 0 -14.613
STAH.Mother STTK

Volume STEL
STEL.Visibility 1
STEL.Material electr
STEL.Shape BRIK 5 10 0.941
STEL.Position 0 0 14.059
STEL.Mother STTK

!-------------------------------------------------------------------------


!*******************************************
!*******************************************
!***   Ge detector including radiator,   ***
!***   and tower                         ***
!*******************************************
!*******************************************

!*************
!*   tower   *
!*************

!--- mother volume ---!
Volume TOWR
TOWR.Material vacuum
TOWR.Shape TUBS 0 75 80 0 360 
TOWR.Position 0 0 166
TOWR.Mother Spacecraft
TOWR.Virtual true
TOWR.Visibility 0

!--- carbon fiber structure ---!

! tower
Volume TRCS
TRCS.Visibility 1
TRCS.Material c_epoxy
//TRCS.Shape PCON 0 360 4 0 12 13 99 12 13 99 0 13 100 0 13 
//TRCS.Shape PCON 0 360 8   0 12 13   93 12 13  93 12 20  94 12 20   94 19 20  99 19 20  99 0 20   100 0 20 
//TRCS.Shape PCON 0 360 2 0 45 46.5 99 45 46.5 
TRCS.Shape PCON 0 360 2 0 12 13 99 12 13 
TRCS.Position 0 0 -80
TRCS.Mother TOWR


! BGO PMTs (based on SPI mass model volume SPMU)
! mother volume
Volume BPMT
BPMT.Visibility 1
BPMT.Material al6061
BPMT.Shape TUBS 0 2.1 3.75 0 360 



BPMT.Copy BPMT_Copy1
BPMT.Copy BPMT_Copy2
BPMT.Copy BPMT_Copy3
BPMT.Copy BPMT_Copy4
BPMT_Copy1.Position 5. 5. 15.25
BPMT_Copy2.Position -5. 5. 15.25
BPMT_Copy3.Position -5. -5. 15.25
BPMT_Copy4.Position 5. -5. 15.25
BPMT_Copy1.Mother TOWR
BPMT_Copy2.Mother TOWR
BPMT_Copy3.Mother TOWR
BPMT_Copy4.Mother TOWR



! PMT proper
Volume BPTP
BPTP.Visibility 1
BPTP.Material bgopmtmix
BPTP.Shape TUBS 0 1.9 3.5 0 360 
BPTP.Position 0 0 0.25
BPTP.Mother BPMT



!********************************************
!    Electronics/Alu analoguous to Georg's BST1,BST2,BST3
!    close to the detectors ...
!********************************************

! ** take Georg's masses, double them since this is
! ** more complex than a TGRS detector, and change
! ** their shapes to boxes ...
! ** BST1 was 80cm3 -> 160cm3   (45.0%)
! ** BST2 was 58.5cm3 -> 117cm3 (32.5%)
! ** BST3 was 40.5cm3 -> 81cm3  (22.5%)

! Segment 1 - Magnesium Alloy
Volume BST1   
BST1.Visibility 1
BST1.Material mg_az31b
BST1.Shape BRIK  8  {0.45 * 8}  2
BST1.Position 0.0  {0.55 * 8}  -15
BST1.Mother TOWR

! Segment 2 - Al6061
Volume BST2
BST2.Visibility 1
BST2.Material al6061
BST2.Shape BRIK  8 {0.325 * 8} 2
BST2.Position 0.0  {-0.225 * 8}  -15
BST2.Mother TOWR

! Segment 3 - electronics
Volume BST3
BST3.Visibility 1
BST3.Material electr
BST3.Shape BRIK  8 {0.225 * 8} 2
BST3.Position 0.0 {-0.775*8} -15
BST3.Mother TOWR






!--- APU-DPU electronics "box" ---!

! mother volume
Volume DPU
DPU.Visibility 1
DPU.Material vacuum
DPU.Shape TUBS 0 8 2 0 360 
DPU.Position 0 0 0
DPU.Mother TOWR

Volume DPUA
DPUA.Visibility 1
DPUA.Material al6061
DPUA.Shape TUBS 0 8 1.201 0 360 
DPUA.Position 0 0 0.799
DPUA.Mother DPU

Volume DPUE
DPUE.Visibility 1
DPUE.Material electr
DPUE.Shape TUBS 0 8 0.554 0 360 
DPUE.Position 0 0 -1.446
DPUE.Mother DPU

Volume DPUS
DPUS.Visibility 1
DPUS.Material al6061
DPUS.Shape TUBS 0 12 1 0 360 
DPUS.Position 0 0 -3
DPUS.Mother TOWR

Volume DPUH
DPUH.Visibility 1
DPUH.Material isot_cu
DPUH.Shape TUBS 11.6 12 0.2 0 360 
DPUH.Position 0 0 -1.8
DPUH.Mother TOWR









