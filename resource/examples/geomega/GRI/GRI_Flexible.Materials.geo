#


Name materials
Version 0.1

## This was here originally, too many materials & had problems running 
## protons so kicked this out - 060410
## Include $(MEGALIB)/resource/geometries/materials/MGeantDefaultMaterials.geo
AbsorptionFileDirectory ./absorptions

## **************************************************************************
## Stuff from the original default material file I had to take back ...:

Material vacuum
vacuum.Density                  1E-16
vacuum.Component                H 1

Material aluminum
aluminum.Density               2.7
aluminum.Component             Al 1

Material beryllium
beryllium.Density               1.858
beryllium.Component             Be 1

Material Densimet180
Densimet180.Density               18
Densimet180.ComponentByMass       W  0.95
Densimet180.ComponentByMass       Fe  0.16
Densimet180.ComponentByMass       Ni  0.44



## ****************************************************
## MATERIALS from Georg's MAX with TGRS-Ge Monolith

# Material definitions: 
Material al_2024_5056
al_2024_5056.Density 0.48
al_2024_5056.ComponentByMass Mg 0.03275
al_2024_5056.ComponentByMass Al 0.9292
al_2024_5056.ComponentByMass Si 0.004
al_2024_5056.ComponentByMass Ti 0.00075
al_2024_5056.ComponentByMass Cr 0.00115
al_2024_5056.ComponentByMass Mn 0.00365
al_2024_5056.ComponentByMass Fe 0.0045
al_2024_5056.ComponentByMass Cu 0.02225
al_2024_5056.ComponentByMass Zn 0.00175

Material ti_6
ti_6.Density 0.888
ti_6.ComponentByMass Al 0.06
ti_6.ComponentByMass Fe 0.0025
ti_6.ComponentByMass O 0.002
ti_6.ComponentByMass V 0.04
ti_6.ComponentByMass Ti 0.8955

Material prop_mixt
prop_mixt.Density 0.127
prop_mixt.ComponentByMass H 0.12538
prop_mixt.ComponentByMass He 0.00341
prop_mixt.ComponentByMass N 0.87121

Material cold_gas
cold_gas.Density 0.149
cold_gas.ComponentByMass N 1

Material inox1810
inox1810.Density 9.147
inox1810.ComponentByMass Cr 0.18
inox1810.ComponentByMass Ni 0.1
inox1810.ComponentByMass Fe 0.72

Material al6061
al6061.Density 2.6989
al6061.ComponentByMass Si 0.006
al6061.ComponentByMass Fe 0.0035
al6061.ComponentByMass Cu 0.0028
al6061.ComponentByMass Mn 0.0008
al6061.ComponentByMass Mg 0.01
al6061.ComponentByMass Cr 0.002
al6061.ComponentByMass Zn 0.0013
al6061.ComponentByMass Ti 0.0008
al6061.ComponentByMass Al 0.9728

Material nicd_cells
nicd_cells.Density 10
nicd_cells.ComponentByMass H 0.015
nicd_cells.ComponentByMass C 0.011
nicd_cells.ComponentByMass N 0.002
nicd_cells.ComponentByMass O 0.153
nicd_cells.ComponentByMass K 0.021
nicd_cells.ComponentByMass Fe 0.317
nicd_cells.ComponentByMass Co 0.004
nicd_cells.ComponentByMass Ni 0.331
nicd_cells.ComponentByMass Cd 0.146

Material isot_cu
isot_cu.Density 8.933
isot_cu.ComponentByMass Cu 1

Material electr
electr.Density 10
electr.ComponentByMass H 0.0124
electr.ComponentByMass C 0.1969
electr.ComponentByMass N 0.018
electr.ComponentByMass O 0.1349
electr.ComponentByMass Mg 0.005
electr.ComponentByMass Al 0.2578
electr.ComponentByMass Si 0.005
electr.ComponentByMass Ca 0.0056
electr.ComponentByMass Ti 0.0135
electr.ComponentByMass Cr 0.0003
electr.ComponentByMass Fe 0.0693
electr.ComponentByMass Ni 0.0364
electr.ComponentByMass Cu 0.1245
electr.ComponentByMass Zn 0.0406
electr.ComponentByMass Sn 0.0032
electr.ComponentByMass Ta 0.0766

Material c_epoxy
c_epoxy.Density 1.628
c_epoxy.ComponentByMass C 0.917
c_epoxy.ComponentByMass H 0.02
c_epoxy.ComponentByMass O 0.063

Material bgo
bgo.Density 7.13
bgo.ComponentByAtoms Bi 4
bgo.ComponentByAtoms Ge 3
bgo.ComponentByAtoms C 12

Material bgopmtmix
bgopmtmix.Density 1
bgopmtmix.ComponentByMass C 0.05
bgopmtmix.ComponentByMass Cu 0.1
bgopmtmix.ComponentByMass Si 0.05
bgopmtmix.ComponentByMass Al 0.8

Material mg_az31b
mg_az31b.Density 1.77
mg_az31b.ComponentByMass Al 0.03
mg_az31b.ComponentByMass Fe 3e-05
mg_az31b.ComponentByMass Ni 3e-05
mg_az31b.ComponentByMass Zn 0.01
mg_az31b.ComponentByMass Ca 0.0002
mg_az31b.ComponentByMass Mg 0.95699
mg_az31b.ComponentByMass Cu 0.00025
mg_az31b.ComponentByMass Mn 0.002
mg_az31b.ComponentByMass Si 0.0005

Material plastic
plastic.Density 1.032
plastic.ComponentByMass H 0.0853
plastic.ComponentByMass C 0.9147

Material cal_src
cal_src.Density 2.662
cal_src.ComponentByAtoms K 2
cal_src.ComponentByAtoms S 1
cal_src.ComponentByAtoms O 4

Material molsv
molsv.Density 1
molsv.ComponentByMass O 0.493
molsv.ComponentByMass Si 0.234
molsv.ComponentByMass Al 0.185
molsv.ComponentByMass Na 0.088

Material hget
hget.Density 8.7
hget.ComponentByMass O 0.131
hget.ComponentByMass Pd 0.869

Material inor
inor.Density 7.31
inor.ComponentByMass In 1

Material steel_302
steel_302.Density 7.86
steel_302.ComponentByMass C 0.0015
steel_302.ComponentByMass Fe 0.69775
steel_302.ComponentByMass Ni 0.09
steel_302.ComponentByMass S 0.0003
steel_302.ComponentByMass Cr 0.18
steel_302.ComponentByMass Mn 0.02
steel_302.ComponentByMass P 0.00045
steel_302.ComponentByMass Si 0.01

Material teflon
teflon.Density 2.2
teflon.ComponentByAtoms F 4
teflon.ComponentByAtoms C 2

Material alumina
alumina.Density 3.97
alumina.ComponentByAtoms Al  2
alumina.ComponentByAtoms O  3

Material ti_5
ti_5.Density 4.48
ti_5.ComponentByMass Al 0.05
ti_5.ComponentByMass O 0.002
ti_5.ComponentByMass Ti 0.918
ti_5.ComponentByMass Fe 0.005
ti_5.ComponentByMass Sn 0.025

Material nit_fil
nit_fil.Density 0.001197
nit_fil.ComponentByMass N 1

Material gold
gold.Density 19.3
gold.ComponentByMass Au 1

Material steel_a286
steel_a286.Density 7.92
steel_a286.ComponentByMass Al 0.0018
steel_a286.ComponentByMass B 6e-05
steel_a286.ComponentByMass C 0.0004
steel_a286.ComponentByMass Cr 0.148
steel_a286.ComponentByMass Fe 0.54209
steel_a286.ComponentByMass Mn 0.01
steel_a286.ComponentByMass Mo 0.013
steel_a286.ComponentByMass Ni 0.255
steel_a286.ComponentByMass P 0.0002
steel_a286.ComponentByMass S 0.00015
steel_a286.ComponentByMass Si 0.005
steel_a286.ComponentByMass Ti 0.0213
steel_a286.ComponentByMass V 0.003

Material germanium
germanium.Density 5.323
germanium.ComponentByMass Ge 1

Material active_ge_recoil
active_ge_recoil.Density 5.323
active_ge_recoil.ComponentByMass Ge 1

Material active_si_recoil
active_si_recoil.Density 2.33
active_si_recoil.ComponentByMass Si 1

Material active_czt_recoil
active_czt_recoil.Density 5.78
active_czt_recoil.ComponentByAtoms            Zn 1
active_czt_recoil.ComponentByAtoms            Cd 9
active_czt_recoil.ComponentByAtoms            Te 10

// PEEK from MEGA
// Values of Peek are not correct, except for the density (=1.31) !!!
Material Peek
Peek.Density                     1.31
Peek.Component                   H  8   // H
Peek.Component                   C  5   // C
Peek.Component                   O  2   // O

## ****************************************************
## MATERIALS from NCT file

Material ro4003   // one of circuit board materials by Rogers Corporation
                  // used in NCT
ro4003.Density            1.79
ro4003.ComponentByMass    C    0.1520        // C
ro4003.ComponentByMass    O    0.4290        // O
ro4003.ComponentByMass     H    0.0180        // H
ro4003.ComponentByMass    Si    0.3300        // Si
ro4003.ComponentByMass    Ca    0.0370        // Ca
ro4003.ComponentByMass    Al    0.0190        // Al
ro4003.ComponentByMass    B    0.0110        // B
ro4003.ComponentByMass    K    0.0010        // K
ro4003.ComponentByMass    Mg    0.0050        // Mg

Material roTMM3   // one of circuit board materials by Rogers Corporation
                  // used in NCT
roTMM3.Density            1.78
roTMM3.ComponentByMass    C    0.1201        // C
roTMM3.ComponentByMass    O    0.4270        // O
roTMM3.ComponentByMass     H    0.0198        // H
roTMM3.ComponentByMass    Si    0.3616        // Si
roTMM3.ComponentByMass    Ti    0.0153        // Ti
roTMM3.ComponentByMass   Ba    0.0098        // Ba
roTMM3.ComponentByMass    Ca    0.0022        // Ca
roTMM3.ComponentByMass    Al    0.0011        // Al
roTMM3.ComponentByMass    B    0.0003        // B
roTMM3.ComponentByMass    Mg    0.0002        // Mg
roTMM3.ComponentByMass    S    0.0010        // S

Material indium
indium.Density                  7.310
indium.Component                In  1

// stainless steel according to P.Turin for NCT screws
Material Steel_18_8
Steel_18_8.Density              7.87
Steel_18_8.ComponentByMass      Cr  0.18   // Cr
Steel_18_8.ComponentByMass      Ni  0.09   // Ni
Steel_18_8.ComponentByMass      C  0.013  // C
Steel_18_8.ComponentByMass      Fe  0.717   // Fe

Material INST_ACsupport    // Support for AC including wiring (carbon fiber + misc)
INST_ACsupport.Density               0.144
INST_ACsupport.ComponentByMass       C  0.98     // C
INST_ACsupport.ComponentByMass       Cu 0.01     // Cu
INST_ACsupport.ComponentByMass       Si 0.01     // Al


