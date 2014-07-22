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
vacuum.Component                1 1 1
vacuum.Sensitivity              1

Material aluminum
aluminum.Density               2.7
aluminum.Component             27.0 13.0 1
aluminum.Sensitivity           1

Material beryllium
beryllium.Density               1.858
beryllium.Component             9.01  4.0 1
beryllium.Sensitivity           1

Material Densimet180
Densimet180.Density               18
Densimet180.ComponentByMass       183.8  74  0.95
Densimet180.ComponentByMass       55.84  26  0.16
Densimet180.ComponentByMass       58.69  28  0.44
Densimet180.Sensitivity           1




## ****************************************************
## MATERIALS from Georg's MAX with TGRS-Ge Monolith

# Material definitions: 
Material al_2024_5056
al_2024_5056.Density 0.48
al_2024_5056.ComponentByMass 24.305 12 0.03275
al_2024_5056.ComponentByMass 26.9815 13 0.9292
al_2024_5056.ComponentByMass 28.0855 14 0.004
al_2024_5056.ComponentByMass 47.867 22 0.00075
al_2024_5056.ComponentByMass 51.996 24 0.00115
al_2024_5056.ComponentByMass 54.938 25 0.00365
al_2024_5056.ComponentByMass 55.845 26 0.0045
al_2024_5056.ComponentByMass 63.546 29 0.02225
al_2024_5056.ComponentByMass 65.39 30 0.00175

Material ti_6
ti_6.Density 0.888
ti_6.ComponentByMass 26.9815 13 0.06
ti_6.ComponentByMass 55.845 26 0.0025
ti_6.ComponentByMass 15.994 8 0.002
ti_6.ComponentByMass 50.9415 23 0.04
ti_6.ComponentByMass 47.867 22 0.8955

Material prop_mixt
prop_mixt.Density 0.127
prop_mixt.ComponentByMass 1.00794 1 0.12538
prop_mixt.ComponentByMass 4.0026 2 0.00341
prop_mixt.ComponentByMass 14.0067 7 0.87121

Material cold_gas
cold_gas.Density 0.149
cold_gas.ComponentByMass 14.0067 7 1
cold_gas.RadiationLength 254.953

Material inox1810
inox1810.Density 9.147
inox1810.ComponentByMass 51.996 24 0.18
inox1810.ComponentByMass 58.6933 28 0.1
inox1810.ComponentByMass 55.845 26 0.72

Material al6061
al6061.Density 2.6989
al6061.ComponentByMass 28.0855 14 0.006
al6061.ComponentByMass 55.845 26 0.0035
al6061.ComponentByMass 63.546 29 0.0028
al6061.ComponentByMass 54.938 25 0.0008
al6061.ComponentByMass 24.305 12 0.01
al6061.ComponentByMass 51.996 24 0.002
al6061.ComponentByMass 65.39 30 0.0013
al6061.ComponentByMass 47.867 22 0.0008
al6061.ComponentByMass 26.9815 13 0.9728

Material nicd_cells
nicd_cells.Density 10
nicd_cells.ComponentByMass 1.0079 1 0.015
nicd_cells.ComponentByMass 12.0107 6 0.011
nicd_cells.ComponentByMass 14.0067 7 0.002
nicd_cells.ComponentByMass 15.994 8 0.153
nicd_cells.ComponentByMass 39.0983 19 0.021
nicd_cells.ComponentByMass 55.845 26 0.317
nicd_cells.ComponentByMass 58.9332 27 0.004
nicd_cells.ComponentByMass 58.6933 28 0.331
nicd_cells.ComponentByMass 112.41 48 0.146

Material isot_cu
isot_cu.Density 8.933
isot_cu.ComponentByMass 63.546 29 1
isot_cu.RadiationLength 1.43993

Material electr
electr.Density 10
electr.ComponentByMass 1.0079 1 0.0124
electr.ComponentByMass 12.0107 6 0.1969
electr.ComponentByMass 14.0067 7 0.018
electr.ComponentByMass 15.994 8 0.1349
electr.ComponentByMass 24.305 12 0.005
electr.ComponentByMass 26.9815 13 0.2578
electr.ComponentByMass 28.0855 14 0.005
electr.ComponentByMass 40.078 20 0.0056
electr.ComponentByMass 47.867 22 0.0135
electr.ComponentByMass 51.996 24 0.0003
electr.ComponentByMass 55.845 26 0.0693
electr.ComponentByMass 58.6933 28 0.0364
electr.ComponentByMass 63.546 29 0.1245
electr.ComponentByMass 65.39 30 0.0406
electr.ComponentByMass 118.71 50 0.0032
electr.ComponentByMass 180.948 73 0.0766

Material c_epoxy
c_epoxy.Density 1.628
c_epoxy.ComponentByMass 12.0107 6 0.917
c_epoxy.ComponentByMass 1.0079 1 0.02
c_epoxy.ComponentByMass 15.994 8 0.063

Material bgo
bgo.Density 7.13
bgo.ComponentByAtoms 208.98 83 4
bgo.ComponentByAtoms 72.64 32 3
bgo.ComponentByAtoms 12.0107 6 12

Material bgopmtmix
bgopmtmix.Density 1
bgopmtmix.ComponentByMass 12.0107 6 0.05
bgopmtmix.ComponentByMass 63.546 29 0.1
bgopmtmix.ComponentByMass 28.0855 14 0.05
bgopmtmix.ComponentByMass 26.9815 13 0.8

Material mg_az31b
mg_az31b.Density 1.77
mg_az31b.ComponentByMass 26.9815 13 0.03
mg_az31b.ComponentByMass 55.845 26 3e-05
mg_az31b.ComponentByMass 58.6933 28 3e-05
mg_az31b.ComponentByMass 65.39 30 0.01
mg_az31b.ComponentByMass 40.078 20 0.0002
mg_az31b.ComponentByMass 24.305 12 0.95699
mg_az31b.ComponentByMass 63.546 29 0.00025
mg_az31b.ComponentByMass 54.938 25 0.002
mg_az31b.ComponentByMass 28.0855 14 0.0005

Material plastic
plastic.Density 1.032
plastic.ComponentByMass 1.0079 1 0.0853
plastic.ComponentByMass 12.011 6 0.9147

Material cal_src
cal_src.Density 2.662
cal_src.ComponentByAtoms 39.102 19 2
cal_src.ComponentByAtoms 32.064 16 1
cal_src.ComponentByAtoms 15.9994 8 4

Material molsv
molsv.Density 1
molsv.ComponentByMass 15.994 8 0.493
molsv.ComponentByMass 28.0855 14 0.234
molsv.ComponentByMass 26.9815 13 0.185
molsv.ComponentByMass 22.9898 11 0.088

Material hget
hget.Density 8.7
hget.ComponentByMass 15.9994 8 0.131
hget.ComponentByMass 106.4 46 0.869

Material inor
inor.Density 7.31
inor.ComponentByMass 114.82 49 1
inor.RadiationLength 1.21056

Material steel_302
steel_302.Density 7.86
steel_302.ComponentByMass 12.0107 6 0.0015
steel_302.ComponentByMass 55.845 26 0.69775
steel_302.ComponentByMass 58.6933 28 0.09
steel_302.ComponentByMass 32.065 16 0.0003
steel_302.ComponentByMass 51.996 24 0.18
steel_302.ComponentByMass 54.938 25 0.02
steel_302.ComponentByMass 30.9738 15 0.00045
steel_302.ComponentByMass 28.0855 14 0.01

Material teflon
teflon.Density 2.2
teflon.ComponentByAtoms 18.9984 9 4
teflon.ComponentByAtoms 12.0107 6 2

Material alumina
alumina.Density 3.97
alumina.ComponentByAtoms 26.98154  13  2
alumina.ComponentByAtoms 15.9994    8  3

Material ti_5
ti_5.Density 4.48
ti_5.ComponentByMass 26.9815 13 0.05
ti_5.ComponentByMass 15.994 8 0.002
ti_5.ComponentByMass 47.867 22 0.918
ti_5.ComponentByMass 55.845 26 0.005
ti_5.ComponentByMass 118.71 50 0.025

Material nit_fil
nit_fil.Density 0.001197
nit_fil.ComponentByMass 14.0067 7 1

Material gold
gold.Density 19.3
gold.ComponentByMass 196.967 79 1
gold.RadiationLength 0.334783

Material steel_a286
steel_a286.Density 7.92
steel_a286.ComponentByMass 26.9815 13 0.0018
steel_a286.ComponentByMass 10.811 5 6e-05
steel_a286.ComponentByMass 12.0107 6 0.0004
steel_a286.ComponentByMass 51.996 24 0.148
steel_a286.ComponentByMass 55.845 26 0.54209
steel_a286.ComponentByMass 54.938 25 0.01
steel_a286.ComponentByMass 95.94 42 0.013
steel_a286.ComponentByMass 58.6933 28 0.255
steel_a286.ComponentByMass 30.9738 15 0.0002
steel_a286.ComponentByMass 32.065 16 0.00015
steel_a286.ComponentByMass 28.0855 14 0.005
steel_a286.ComponentByMass 47.867 22 0.0213
steel_a286.ComponentByMass 50.9415 23 0.003

Material germanium
germanium.Density 5.323
germanium.ComponentByMass 72.64 32 1
germanium.RadiationLength 2.30216

Material active_ge_recoil
active_ge_recoil.Density 5.323
active_ge_recoil.ComponentByMass 72.64 32 1
active_ge_recoil.RadiationLength 2.30216

Material active_si_recoil
active_si_recoil.Density 2.33
active_si_recoil.ComponentByMass 28.08 14.0 1
active_si_recoil.Sensitivity 1

Material active_czt_recoil
active_czt_recoil.Density 5.78
active_czt_recoil.ComponentByAtoms            65.39 30.0 1
active_czt_recoil.ComponentByAtoms            112.411 48.0 9
active_czt_recoil.ComponentByAtoms            127.6 52.0 10

// PEEK from MEGA
// Values of Peek are not correct, except for the density (=1.31) !!!
Material Peek
Peek.Density                     1.31
Peek.Component                   1.0   1.0  8   // H
Peek.Component                   12.1  6.0  5   // C
Peek.Component                   16.0  8.0  2   // O
Peek.Sensitivity                 1

## ****************************************************
## MATERIALS from NCT file

Material ro4003   // one of circuit board materials by Rogers Corporation
                  // used in NCT
ro4003.Density            1.79
ro4003.ComponentByMass    12.011   6.0    0.1520        // C
ro4003.ComponentByMass    16.0     8.0    0.4290        // O
ro4003.ComponentByMass     1.0     1.0    0.0180        // H
ro4003.ComponentByMass    28.086  14.0    0.3300        // Si
ro4003.ComponentByMass    40.078  20.0    0.0370        // Ca
ro4003.ComponentByMass    26.982  13.0    0.0190        // Al
ro4003.ComponentByMass    10.811   5.0    0.0110        // B
ro4003.ComponentByMass    39.098  19.0    0.0010        // K
ro4003.ComponentByMass    24.305  12.0    0.0050        // Mg
ro4003.Sensitivity        1

Material roTMM3   // one of circuit board materials by Rogers Corporation
                  // used in NCT
roTMM3.Density            1.78
roTMM3.ComponentByMass    12.011   6.0    0.1201        // C
roTMM3.ComponentByMass    16.0     8.0    0.4270        // O
roTMM3.ComponentByMass     1.0     1.0    0.0198        // H
roTMM3.ComponentByMass    28.086  14.0    0.3616        // Si
roTMM3.ComponentByMass    47.88   22.0    0.0153        // Ti
roTMM3.ComponentByMass   137.327  56.0    0.0098        // Ba
roTMM3.ComponentByMass    40.078  20.0    0.0022        // Ca
roTMM3.ComponentByMass    26.982  13.0    0.0011        // Al
roTMM3.ComponentByMass    10.811   5.0    0.0003        // B
roTMM3.ComponentByMass    24.305  12.0    0.0002        // Mg
roTMM3.ComponentByMass    32.066  16.0    0.0010        // S
roTMM3.Sensitivity        1

Material indium
indium.Density                  7.310
indium.Component                114.818     49.0  1
indium.Sensitivity              1
indium.RadiationLength          1.21054

// stainless steel according to P.Turin for NCT screws
Material Steel_18_8
Steel_18_8.Density              7.87
Steel_18_8.ComponentByMass      51.99 24.0  0.18   // Cr
Steel_18_8.ComponentByMass      58.69 28.0  0.09   // Ni
Steel_18_8.ComponentByMass      12.01  6.0  0.013  // C
Steel_18_8.ComponentByMass      55.85 26.0  0.717   // Fe
Steel_18_8.Sensitivity          1

Material INST_ACsupport    // Support for AC including wiring (carbon fiber + misc)
INST_ACsupport.Density               0.144
INST_ACsupport.ComponentByMass       12.01  6.0  0.98     // C
INST_ACsupport.ComponentByMass       63.546 29.0 0.01     // Cu
INST_ACsupport.ComponentByMass       28.086 14.0 0.01     // Al
INST_ACsupport.Sensitivity               1


