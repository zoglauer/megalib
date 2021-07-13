# Atmosphere model created from file Atmosphere.BestCrabDay.geo

Name AtmosphereModel

# Tiny Surrounding Sphere above atomsphere @ 200 km
SurroundingSphere 0.1 0 0 20000000 0.1

Volume World
World.Material Vacuum
World.Shape BOX 10240000000.000000 10240000000.000000 10240000000.000000
World.Visibility 1
World.Position 0 0 0
World.Mother 0

Include $(MEGALIB)/resource/examples/geomega/materials/Materials.geo

Material MaterialSlice_0_1
MaterialSlice_0_1.Density 1.162000e-03
MaterialSlice_0_1.ComponentByAtoms He 2
MaterialSlice_0_1.ComponentByAtoms N 784737
MaterialSlice_0_1.ComponentByAtoms O 210565
MaterialSlice_0_1.ComponentByAtoms Ar 4694

Volume VolumeSlice_0_1
VolumeSlice_0_1.Material MaterialSlice_0_1
VolumeSlice_0_1.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_0_1.Visibility  1
VolumeSlice_0_1.Position 0 0 50000.000000
VolumeSlice_0_1.Mother World

Material MaterialSlice_1_2
MaterialSlice_1_2.Density 1.059000e-03
MaterialSlice_1_2.ComponentByAtoms He 2
MaterialSlice_1_2.ComponentByAtoms N 784807
MaterialSlice_1_2.ComponentByAtoms O 210497
MaterialSlice_1_2.ComponentByAtoms Ar 4692

Volume VolumeSlice_1_2
VolumeSlice_1_2.Material MaterialSlice_1_2
VolumeSlice_1_2.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_1_2.Visibility  1
VolumeSlice_1_2.Position 0 0 150000.000000
VolumeSlice_1_2.Mother World

Material MaterialSlice_2_3
MaterialSlice_2_3.Density 9.598000e-04
MaterialSlice_2_3.ComponentByAtoms He 2
MaterialSlice_2_3.ComponentByAtoms N 784773
MaterialSlice_2_3.ComponentByAtoms O 210530
MaterialSlice_2_3.ComponentByAtoms Ar 4693

Volume VolumeSlice_2_3
VolumeSlice_2_3.Material MaterialSlice_2_3
VolumeSlice_2_3.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_2_3.Visibility  1
VolumeSlice_2_3.Position 0 0 250000.000000
VolumeSlice_2_3.Mother World

Material MaterialSlice_3_4
MaterialSlice_3_4.Density 8.672000e-04
MaterialSlice_3_4.ComponentByAtoms He 2
MaterialSlice_3_4.ComponentByAtoms N 784724
MaterialSlice_3_4.ComponentByAtoms O 210577
MaterialSlice_3_4.ComponentByAtoms Ar 4694

Volume VolumeSlice_3_4
VolumeSlice_3_4.Material MaterialSlice_3_4
VolumeSlice_3_4.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_3_4.Visibility  1
VolumeSlice_3_4.Position 0 0 350000.000000
VolumeSlice_3_4.Mother World

Material MaterialSlice_4_5
MaterialSlice_4_5.Density 7.816000e-04
MaterialSlice_4_5.ComponentByAtoms He 2
MaterialSlice_4_5.ComponentByAtoms N 784723
MaterialSlice_4_5.ComponentByAtoms O 210577
MaterialSlice_4_5.ComponentByAtoms Ar 4695

Volume VolumeSlice_4_5
VolumeSlice_4_5.Material MaterialSlice_4_5
VolumeSlice_4_5.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_4_5.Visibility  1
VolumeSlice_4_5.Position 0 0 450000.000000
VolumeSlice_4_5.Mother World

Material MaterialSlice_5_6
MaterialSlice_5_6.Density 7.034000e-04
MaterialSlice_5_6.ComponentByAtoms He 2
MaterialSlice_5_6.ComponentByAtoms N 784734
MaterialSlice_5_6.ComponentByAtoms O 210566
MaterialSlice_5_6.ComponentByAtoms Ar 4696

Volume VolumeSlice_5_6
VolumeSlice_5_6.Material MaterialSlice_5_6
VolumeSlice_5_6.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_5_6.Visibility  1
VolumeSlice_5_6.Position 0 0 550000.000000
VolumeSlice_5_6.Mother World

Material MaterialSlice_6_7
MaterialSlice_6_7.Density 6.326000e-04
MaterialSlice_6_7.ComponentByAtoms He 2
MaterialSlice_6_7.ComponentByAtoms N 784760
MaterialSlice_6_7.ComponentByAtoms O 210541
MaterialSlice_6_7.ComponentByAtoms Ar 4694

Volume VolumeSlice_6_7
VolumeSlice_6_7.Material MaterialSlice_6_7
VolumeSlice_6_7.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_6_7.Visibility  1
VolumeSlice_6_7.Position 0 0 650000.000000
VolumeSlice_6_7.Mother World

Material MaterialSlice_7_8
MaterialSlice_7_8.Density 5.688000e-04
MaterialSlice_7_8.ComponentByAtoms He 2
MaterialSlice_7_8.ComponentByAtoms N 784782
MaterialSlice_7_8.ComponentByAtoms O 210520
MaterialSlice_7_8.ComponentByAtoms Ar 4694

Volume VolumeSlice_7_8
VolumeSlice_7_8.Material MaterialSlice_7_8
VolumeSlice_7_8.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_7_8.Visibility  1
VolumeSlice_7_8.Position 0 0 750000.000000
VolumeSlice_7_8.Mother World

Material MaterialSlice_8_9
MaterialSlice_8_9.Density 5.115000e-04
MaterialSlice_8_9.ComponentByAtoms He 2
MaterialSlice_8_9.ComponentByAtoms N 784782
MaterialSlice_8_9.ComponentByAtoms O 210521
MaterialSlice_8_9.ComponentByAtoms Ar 4693

Volume VolumeSlice_8_9
VolumeSlice_8_9.Material MaterialSlice_8_9
VolumeSlice_8_9.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_8_9.Visibility  1
VolumeSlice_8_9.Position 0 0 850000.000000
VolumeSlice_8_9.Mother World

Material MaterialSlice_9_10
MaterialSlice_9_10.Density 4.600000e-04
MaterialSlice_9_10.ComponentByAtoms He 2
MaterialSlice_9_10.ComponentByAtoms N 784761
MaterialSlice_9_10.ComponentByAtoms O 210543
MaterialSlice_9_10.ComponentByAtoms Ar 4693

Volume VolumeSlice_9_10
VolumeSlice_9_10.Material MaterialSlice_9_10
VolumeSlice_9_10.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_9_10.Visibility  1
VolumeSlice_9_10.Position 0 0 950000.000000
VolumeSlice_9_10.Mother World

Material MaterialSlice_10_11
MaterialSlice_10_11.Density 4.137000e-04
MaterialSlice_10_11.ComponentByAtoms He 2
MaterialSlice_10_11.ComponentByAtoms N 784758
MaterialSlice_10_11.ComponentByAtoms O 210545
MaterialSlice_10_11.ComponentByAtoms Ar 4693

Volume VolumeSlice_10_11
VolumeSlice_10_11.Material MaterialSlice_10_11
VolumeSlice_10_11.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_10_11.Visibility  1
VolumeSlice_10_11.Position 0 0 1050000.000000
VolumeSlice_10_11.Mother World

Material MaterialSlice_11_12
MaterialSlice_11_12.Density 3.716000e-04
MaterialSlice_11_12.ComponentByAtoms He 2
MaterialSlice_11_12.ComponentByAtoms N 784808
MaterialSlice_11_12.ComponentByAtoms O 210494
MaterialSlice_11_12.ComponentByAtoms Ar 4693

Volume VolumeSlice_11_12
VolumeSlice_11_12.Material MaterialSlice_11_12
VolumeSlice_11_12.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_11_12.Visibility  1
VolumeSlice_11_12.Position 0 0 1150000.000000
VolumeSlice_11_12.Mother World

Material MaterialSlice_12_13
MaterialSlice_12_13.Density 3.320000e-04
MaterialSlice_12_13.ComponentByAtoms He 2
MaterialSlice_12_13.ComponentByAtoms N 784725
MaterialSlice_12_13.ComponentByAtoms O 210578
MaterialSlice_12_13.ComponentByAtoms Ar 4693

Volume VolumeSlice_12_13
VolumeSlice_12_13.Material MaterialSlice_12_13
VolumeSlice_12_13.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_12_13.Visibility  1
VolumeSlice_12_13.Position 0 0 1250000.000000
VolumeSlice_12_13.Mother World

Material MaterialSlice_13_14
MaterialSlice_13_14.Density 2.939000e-04
MaterialSlice_13_14.ComponentByAtoms He 2
MaterialSlice_13_14.ComponentByAtoms N 784840
MaterialSlice_13_14.ComponentByAtoms O 210462
MaterialSlice_13_14.ComponentByAtoms Ar 4693

Volume VolumeSlice_13_14
VolumeSlice_13_14.Material MaterialSlice_13_14
VolumeSlice_13_14.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_13_14.Visibility  1
VolumeSlice_13_14.Position 0 0 1350000.000000
VolumeSlice_13_14.Mother World

Material MaterialSlice_14_15
MaterialSlice_14_15.Density 2.571000e-04
MaterialSlice_14_15.ComponentByAtoms He 2
MaterialSlice_14_15.ComponentByAtoms N 784788
MaterialSlice_14_15.ComponentByAtoms O 210516
MaterialSlice_14_15.ComponentByAtoms Ar 4692

Volume VolumeSlice_14_15
VolumeSlice_14_15.Material MaterialSlice_14_15
VolumeSlice_14_15.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_14_15.Visibility  1
VolumeSlice_14_15.Position 0 0 1450000.000000
VolumeSlice_14_15.Mother World

Material MaterialSlice_15_16
MaterialSlice_15_16.Density 2.218000e-04
MaterialSlice_15_16.ComponentByAtoms He 2
MaterialSlice_15_16.ComponentByAtoms N 784787
MaterialSlice_15_16.ComponentByAtoms O 210517
MaterialSlice_15_16.ComponentByAtoms Ar 4693

Volume VolumeSlice_15_16
VolumeSlice_15_16.Material MaterialSlice_15_16
VolumeSlice_15_16.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_15_16.Visibility  1
VolumeSlice_15_16.Position 0 0 1550000.000000
VolumeSlice_15_16.Mother World

Material MaterialSlice_16_17
MaterialSlice_16_17.Density 1.888000e-04
MaterialSlice_16_17.ComponentByAtoms He 2
MaterialSlice_16_17.ComponentByAtoms N 784798
MaterialSlice_16_17.ComponentByAtoms O 210506
MaterialSlice_16_17.ComponentByAtoms Ar 4692

Volume VolumeSlice_16_17
VolumeSlice_16_17.Material MaterialSlice_16_17
VolumeSlice_16_17.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_16_17.Visibility  1
VolumeSlice_16_17.Position 0 0 1650000.000000
VolumeSlice_16_17.Mother World

Material MaterialSlice_17_18
MaterialSlice_17_18.Density 1.591000e-04
MaterialSlice_17_18.ComponentByAtoms He 2
MaterialSlice_17_18.ComponentByAtoms N 784743
MaterialSlice_17_18.ComponentByAtoms O 210560
MaterialSlice_17_18.ComponentByAtoms Ar 4693

Volume VolumeSlice_17_18
VolumeSlice_17_18.Material MaterialSlice_17_18
VolumeSlice_17_18.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_17_18.Visibility  1
VolumeSlice_17_18.Position 0 0 1750000.000000
VolumeSlice_17_18.Mother World

Material MaterialSlice_18_19
MaterialSlice_18_19.Density 1.332000e-04
MaterialSlice_18_19.ComponentByAtoms He 2
MaterialSlice_18_19.ComponentByAtoms N 784745
MaterialSlice_18_19.ComponentByAtoms O 210558
MaterialSlice_18_19.ComponentByAtoms Ar 4693

Volume VolumeSlice_18_19
VolumeSlice_18_19.Material MaterialSlice_18_19
VolumeSlice_18_19.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_18_19.Visibility  1
VolumeSlice_18_19.Position 0 0 1850000.000000
VolumeSlice_18_19.Mother World

Material MaterialSlice_19_20
MaterialSlice_19_20.Density 1.113000e-04
MaterialSlice_19_20.ComponentByAtoms He 2
MaterialSlice_19_20.ComponentByAtoms N 784738
MaterialSlice_19_20.ComponentByAtoms O 210565
MaterialSlice_19_20.ComponentByAtoms Ar 4693

Volume VolumeSlice_19_20
VolumeSlice_19_20.Material MaterialSlice_19_20
VolumeSlice_19_20.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_19_20.Visibility  1
VolumeSlice_19_20.Position 0 0 1950000.000000
VolumeSlice_19_20.Mother World

Material MaterialSlice_20_21
MaterialSlice_20_21.Density 9.307000e-05
MaterialSlice_20_21.ComponentByAtoms He 2
MaterialSlice_20_21.ComponentByAtoms N 784810
MaterialSlice_20_21.ComponentByAtoms O 210493
MaterialSlice_20_21.ComponentByAtoms Ar 4694

Volume VolumeSlice_20_21
VolumeSlice_20_21.Material MaterialSlice_20_21
VolumeSlice_20_21.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_20_21.Visibility  1
VolumeSlice_20_21.Position 0 0 2050000.000000
VolumeSlice_20_21.Mother World

Material MaterialSlice_21_22
MaterialSlice_21_22.Density 7.806000e-05
MaterialSlice_21_22.ComponentByAtoms He 2
MaterialSlice_21_22.ComponentByAtoms N 784789
MaterialSlice_21_22.ComponentByAtoms O 210514
MaterialSlice_21_22.ComponentByAtoms Ar 4693

Volume VolumeSlice_21_22
VolumeSlice_21_22.Material MaterialSlice_21_22
VolumeSlice_21_22.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_21_22.Visibility  1
VolumeSlice_21_22.Position 0 0 2150000.000000
VolumeSlice_21_22.Mother World

Material MaterialSlice_22_23
MaterialSlice_22_23.Density 6.569000e-05
MaterialSlice_22_23.ComponentByAtoms He 2
MaterialSlice_22_23.ComponentByAtoms N 784839
MaterialSlice_22_23.ComponentByAtoms O 210466
MaterialSlice_22_23.ComponentByAtoms Ar 4692

Volume VolumeSlice_22_23
VolumeSlice_22_23.Material MaterialSlice_22_23
VolumeSlice_22_23.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_22_23.Visibility  1
VolumeSlice_22_23.Position 0 0 2250000.000000
VolumeSlice_22_23.Mother World

Material MaterialSlice_23_24
MaterialSlice_23_24.Density 5.545000e-05
MaterialSlice_23_24.ComponentByAtoms He 2
MaterialSlice_23_24.ComponentByAtoms N 784767
MaterialSlice_23_24.ComponentByAtoms O 210536
MaterialSlice_23_24.ComponentByAtoms Ar 4693

Volume VolumeSlice_23_24
VolumeSlice_23_24.Material MaterialSlice_23_24
VolumeSlice_23_24.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_23_24.Visibility  1
VolumeSlice_23_24.Position 0 0 2350000.000000
VolumeSlice_23_24.Mother World

Material MaterialSlice_24_25
MaterialSlice_24_25.Density 4.694000e-05
MaterialSlice_24_25.ComponentByAtoms He 2
MaterialSlice_24_25.ComponentByAtoms N 784742
MaterialSlice_24_25.ComponentByAtoms O 210560
MaterialSlice_24_25.ComponentByAtoms Ar 4693

Volume VolumeSlice_24_25
VolumeSlice_24_25.Material MaterialSlice_24_25
VolumeSlice_24_25.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_24_25.Visibility  1
VolumeSlice_24_25.Position 0 0 2450000.000000
VolumeSlice_24_25.Mother World

Material MaterialSlice_25_26
MaterialSlice_25_26.Density 3.984000e-05
MaterialSlice_25_26.ComponentByAtoms He 2
MaterialSlice_25_26.ComponentByAtoms N 784777
MaterialSlice_25_26.ComponentByAtoms O 210526
MaterialSlice_25_26.ComponentByAtoms Ar 4693

Volume VolumeSlice_25_26
VolumeSlice_25_26.Material MaterialSlice_25_26
VolumeSlice_25_26.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_25_26.Visibility  1
VolumeSlice_25_26.Position 0 0 2550000.000000
VolumeSlice_25_26.Mother World

Material MaterialSlice_26_27
MaterialSlice_26_27.Density 3.389000e-05
MaterialSlice_26_27.ComponentByAtoms He 2
MaterialSlice_26_27.ComponentByAtoms N 784731
MaterialSlice_26_27.ComponentByAtoms O 210572
MaterialSlice_26_27.ComponentByAtoms Ar 4693

Volume VolumeSlice_26_27
VolumeSlice_26_27.Material MaterialSlice_26_27
VolumeSlice_26_27.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_26_27.Visibility  1
VolumeSlice_26_27.Position 0 0 2650000.000000
VolumeSlice_26_27.Mother World

Material MaterialSlice_27_28
MaterialSlice_27_28.Density 2.889000e-05
MaterialSlice_27_28.ComponentByAtoms He 2
MaterialSlice_27_28.ComponentByAtoms N 784746
MaterialSlice_27_28.ComponentByAtoms O 210557
MaterialSlice_27_28.ComponentByAtoms Ar 4693

Volume VolumeSlice_27_28
VolumeSlice_27_28.Material MaterialSlice_27_28
VolumeSlice_27_28.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_27_28.Visibility  1
VolumeSlice_27_28.Position 0 0 2750000.000000
VolumeSlice_27_28.Mother World

Material MaterialSlice_28_29
MaterialSlice_28_29.Density 2.468000e-05
MaterialSlice_28_29.ComponentByAtoms He 2
MaterialSlice_28_29.ComponentByAtoms N 784777
MaterialSlice_28_29.ComponentByAtoms O 210526
MaterialSlice_28_29.ComponentByAtoms Ar 4693

Volume VolumeSlice_28_29
VolumeSlice_28_29.Material MaterialSlice_28_29
VolumeSlice_28_29.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_28_29.Visibility  1
VolumeSlice_28_29.Position 0 0 2850000.000000
VolumeSlice_28_29.Mother World

Material MaterialSlice_29_30
MaterialSlice_29_30.Density 2.112000e-05
MaterialSlice_29_30.ComponentByAtoms He 2
MaterialSlice_29_30.ComponentByAtoms N 784803
MaterialSlice_29_30.ComponentByAtoms O 210500
MaterialSlice_29_30.ComponentByAtoms Ar 4693

Volume VolumeSlice_29_30
VolumeSlice_29_30.Material MaterialSlice_29_30
VolumeSlice_29_30.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_29_30.Visibility  1
VolumeSlice_29_30.Position 0 0 2950000.000000
VolumeSlice_29_30.Mother World

Material MaterialSlice_30_31
MaterialSlice_30_31.Density 1.810000e-05
MaterialSlice_30_31.ComponentByAtoms He 2
MaterialSlice_30_31.ComponentByAtoms N 784762
MaterialSlice_30_31.ComponentByAtoms O 210541
MaterialSlice_30_31.ComponentByAtoms Ar 4693

Volume VolumeSlice_30_31
VolumeSlice_30_31.Material MaterialSlice_30_31
VolumeSlice_30_31.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_30_31.Visibility  1
VolumeSlice_30_31.Position 0 0 3050000.000000
VolumeSlice_30_31.Mother World

Material MaterialSlice_31_32
MaterialSlice_31_32.Density 1.554000e-05
MaterialSlice_31_32.ComponentByAtoms He 2
MaterialSlice_31_32.ComponentByAtoms N 784767
MaterialSlice_31_32.ComponentByAtoms O 210535
MaterialSlice_31_32.ComponentByAtoms Ar 4694

Volume VolumeSlice_31_32
VolumeSlice_31_32.Material MaterialSlice_31_32
VolumeSlice_31_32.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_31_32.Visibility  1
VolumeSlice_31_32.Position 0 0 3150000.000000
VolumeSlice_31_32.Mother World

Material MaterialSlice_32_33
MaterialSlice_32_33.Density 1.335000e-05
MaterialSlice_32_33.ComponentByAtoms He 2
MaterialSlice_32_33.ComponentByAtoms N 784756
MaterialSlice_32_33.ComponentByAtoms O 210546
MaterialSlice_32_33.ComponentByAtoms Ar 4694

Volume VolumeSlice_32_33
VolumeSlice_32_33.Material MaterialSlice_32_33
VolumeSlice_32_33.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_32_33.Visibility  1
VolumeSlice_32_33.Position 0 0 3250000.000000
VolumeSlice_32_33.Mother World

Material MaterialSlice_33_34
MaterialSlice_33_34.Density 1.149000e-05
MaterialSlice_33_34.ComponentByAtoms He 2
MaterialSlice_33_34.ComponentByAtoms N 784725
MaterialSlice_33_34.ComponentByAtoms O 210577
MaterialSlice_33_34.ComponentByAtoms Ar 4694

Volume VolumeSlice_33_34
VolumeSlice_33_34.Material MaterialSlice_33_34
VolumeSlice_33_34.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_33_34.Visibility  1
VolumeSlice_33_34.Position 0 0 3350000.000000
VolumeSlice_33_34.Mother World

Material MaterialSlice_34_35
MaterialSlice_34_35.Density 9.896000e-06
MaterialSlice_34_35.ComponentByAtoms He 2
MaterialSlice_34_35.ComponentByAtoms N 784830
MaterialSlice_34_35.ComponentByAtoms O 210474
MaterialSlice_34_35.ComponentByAtoms Ar 4692

Volume VolumeSlice_34_35
VolumeSlice_34_35.Material MaterialSlice_34_35
VolumeSlice_34_35.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_34_35.Visibility  1
VolumeSlice_34_35.Position 0 0 3450000.000000
VolumeSlice_34_35.Mother World

Material MaterialSlice_35_36
MaterialSlice_35_36.Density 8.533000e-06
MaterialSlice_35_36.ComponentByAtoms He 2
MaterialSlice_35_36.ComponentByAtoms N 784768
MaterialSlice_35_36.ComponentByAtoms O 210535
MaterialSlice_35_36.ComponentByAtoms Ar 4693

Volume VolumeSlice_35_36
VolumeSlice_35_36.Material MaterialSlice_35_36
VolumeSlice_35_36.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_35_36.Visibility  1
VolumeSlice_35_36.Position 0 0 3550000.000000
VolumeSlice_35_36.Mother World

Material MaterialSlice_36_37
MaterialSlice_36_37.Density 7.367000e-06
MaterialSlice_36_37.ComponentByAtoms He 2
MaterialSlice_36_37.ComponentByAtoms N 784730
MaterialSlice_36_37.ComponentByAtoms O 210572
MaterialSlice_36_37.ComponentByAtoms Ar 4693

Volume VolumeSlice_36_37
VolumeSlice_36_37.Material MaterialSlice_36_37
VolumeSlice_36_37.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_36_37.Visibility  1
VolumeSlice_36_37.Position 0 0 3650000.000000
VolumeSlice_36_37.Mother World

Material MaterialSlice_37_38
MaterialSlice_37_38.Density 6.368000e-06
MaterialSlice_37_38.ComponentByAtoms He 2
MaterialSlice_37_38.ComponentByAtoms N 784748
MaterialSlice_37_38.ComponentByAtoms O 210555
MaterialSlice_37_38.ComponentByAtoms Ar 4693

Volume VolumeSlice_37_38
VolumeSlice_37_38.Material MaterialSlice_37_38
VolumeSlice_37_38.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_37_38.Visibility  1
VolumeSlice_37_38.Position 0 0 3750000.000000
VolumeSlice_37_38.Mother World

Material MaterialSlice_38_39
MaterialSlice_38_39.Density 5.513000e-06
MaterialSlice_38_39.ComponentByAtoms He 2
MaterialSlice_38_39.ComponentByAtoms N 784751
MaterialSlice_38_39.ComponentByAtoms O 210551
MaterialSlice_38_39.ComponentByAtoms Ar 4694

Volume VolumeSlice_38_39
VolumeSlice_38_39.Material MaterialSlice_38_39
VolumeSlice_38_39.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_38_39.Visibility  1
VolumeSlice_38_39.Position 0 0 3850000.000000
VolumeSlice_38_39.Mother World

Material MaterialSlice_39_40
MaterialSlice_39_40.Density 4.779000e-06
MaterialSlice_39_40.ComponentByAtoms He 2
MaterialSlice_39_40.ComponentByAtoms N 784745
MaterialSlice_39_40.ComponentByAtoms O 210558
MaterialSlice_39_40.ComponentByAtoms Ar 4693

Volume VolumeSlice_39_40
VolumeSlice_39_40.Material MaterialSlice_39_40
VolumeSlice_39_40.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_39_40.Visibility  1
VolumeSlice_39_40.Position 0 0 3950000.000000
VolumeSlice_39_40.Mother World

Material MaterialSlice_40_41
MaterialSlice_40_41.Density 4.149000e-06
MaterialSlice_40_41.ComponentByAtoms He 2
MaterialSlice_40_41.ComponentByAtoms N 784792
MaterialSlice_40_41.ComponentByAtoms O 210511
MaterialSlice_40_41.ComponentByAtoms Ar 4693

Volume VolumeSlice_40_41
VolumeSlice_40_41.Material MaterialSlice_40_41
VolumeSlice_40_41.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_40_41.Visibility  1
VolumeSlice_40_41.Position 0 0 4050000.000000
VolumeSlice_40_41.Mother World

Material MaterialSlice_41_42
MaterialSlice_41_42.Density 3.608000e-06
MaterialSlice_41_42.ComponentByAtoms He 2
MaterialSlice_41_42.ComponentByAtoms N 784786
MaterialSlice_41_42.ComponentByAtoms O 210516
MaterialSlice_41_42.ComponentByAtoms Ar 4694

Volume VolumeSlice_41_42
VolumeSlice_41_42.Material MaterialSlice_41_42
VolumeSlice_41_42.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_41_42.Visibility  1
VolumeSlice_41_42.Position 0 0 4150000.000000
VolumeSlice_41_42.Mother World

Material MaterialSlice_42_43
MaterialSlice_42_43.Density 3.143000e-06
MaterialSlice_42_43.ComponentByAtoms He 2
MaterialSlice_42_43.ComponentByAtoms N 784811
MaterialSlice_42_43.ComponentByAtoms O 210491
MaterialSlice_42_43.ComponentByAtoms Ar 4693

Volume VolumeSlice_42_43
VolumeSlice_42_43.Material MaterialSlice_42_43
VolumeSlice_42_43.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_42_43.Visibility  1
VolumeSlice_42_43.Position 0 0 4250000.000000
VolumeSlice_42_43.Mother World

Material MaterialSlice_43_44
MaterialSlice_43_44.Density 2.742000e-06
MaterialSlice_43_44.ComponentByAtoms He 2
MaterialSlice_43_44.ComponentByAtoms N 784728
MaterialSlice_43_44.ComponentByAtoms O 210575
MaterialSlice_43_44.ComponentByAtoms Ar 4693

Volume VolumeSlice_43_44
VolumeSlice_43_44.Material MaterialSlice_43_44
VolumeSlice_43_44.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_43_44.Visibility  1
VolumeSlice_43_44.Position 0 0 4350000.000000
VolumeSlice_43_44.Mother World

Material MaterialSlice_44_45
MaterialSlice_44_45.Density 2.396000e-06
MaterialSlice_44_45.ComponentByAtoms He 2
MaterialSlice_44_45.ComponentByAtoms N 784758
MaterialSlice_44_45.ComponentByAtoms O 210545
MaterialSlice_44_45.ComponentByAtoms Ar 4693

Volume VolumeSlice_44_45
VolumeSlice_44_45.Material MaterialSlice_44_45
VolumeSlice_44_45.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_44_45.Visibility  1
VolumeSlice_44_45.Position 0 0 4450000.000000
VolumeSlice_44_45.Mother World

Material MaterialSlice_45_46
MaterialSlice_45_46.Density 2.098000e-06
MaterialSlice_45_46.ComponentByAtoms He 2
MaterialSlice_45_46.ComponentByAtoms N 784760
MaterialSlice_45_46.ComponentByAtoms O 210543
MaterialSlice_45_46.ComponentByAtoms Ar 4693

Volume VolumeSlice_45_46
VolumeSlice_45_46.Material MaterialSlice_45_46
VolumeSlice_45_46.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_45_46.Visibility  1
VolumeSlice_45_46.Position 0 0 4550000.000000
VolumeSlice_45_46.Mother World

Material MaterialSlice_46_47
MaterialSlice_46_47.Density 1.839000e-06
MaterialSlice_46_47.ComponentByAtoms He 2
MaterialSlice_46_47.ComponentByAtoms N 784799
MaterialSlice_46_47.ComponentByAtoms O 210504
MaterialSlice_46_47.ComponentByAtoms Ar 4693

Volume VolumeSlice_46_47
VolumeSlice_46_47.Material MaterialSlice_46_47
VolumeSlice_46_47.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_46_47.Visibility  1
VolumeSlice_46_47.Position 0 0 4650000.000000
VolumeSlice_46_47.Mother World

Material MaterialSlice_47_48
MaterialSlice_47_48.Density 1.615000e-06
MaterialSlice_47_48.ComponentByAtoms He 2
MaterialSlice_47_48.ComponentByAtoms N 784790
MaterialSlice_47_48.ComponentByAtoms O 210512
MaterialSlice_47_48.ComponentByAtoms Ar 4693

Volume VolumeSlice_47_48
VolumeSlice_47_48.Material MaterialSlice_47_48
VolumeSlice_47_48.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_47_48.Visibility  1
VolumeSlice_47_48.Position 0 0 4750000.000000
VolumeSlice_47_48.Mother World

Material MaterialSlice_48_49
MaterialSlice_48_49.Density 1.421000e-06
MaterialSlice_48_49.ComponentByAtoms He 2
MaterialSlice_48_49.ComponentByAtoms N 784800
MaterialSlice_48_49.ComponentByAtoms O 210503
MaterialSlice_48_49.ComponentByAtoms Ar 4693

Volume VolumeSlice_48_49
VolumeSlice_48_49.Material MaterialSlice_48_49
VolumeSlice_48_49.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_48_49.Visibility  1
VolumeSlice_48_49.Position 0 0 4850000.000000
VolumeSlice_48_49.Mother World

Material MaterialSlice_49_50
MaterialSlice_49_50.Density 1.252000e-06
MaterialSlice_49_50.ComponentByAtoms He 2
MaterialSlice_49_50.ComponentByAtoms N 784781
MaterialSlice_49_50.ComponentByAtoms O 210522
MaterialSlice_49_50.ComponentByAtoms Ar 4693

Volume VolumeSlice_49_50
VolumeSlice_49_50.Material MaterialSlice_49_50
VolumeSlice_49_50.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_49_50.Visibility  1
VolumeSlice_49_50.Position 0 0 4950000.000000
VolumeSlice_49_50.Mother World

Material MaterialSlice_50_51
MaterialSlice_50_51.Density 1.104000e-06
MaterialSlice_50_51.ComponentByAtoms He 2
MaterialSlice_50_51.ComponentByAtoms N 784753
MaterialSlice_50_51.ComponentByAtoms O 210550
MaterialSlice_50_51.ComponentByAtoms Ar 4693

Volume VolumeSlice_50_51
VolumeSlice_50_51.Material MaterialSlice_50_51
VolumeSlice_50_51.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_50_51.Visibility  1
VolumeSlice_50_51.Position 0 0 5050000.000000
VolumeSlice_50_51.Mother World

Material MaterialSlice_51_52
MaterialSlice_51_52.Density 9.756000e-07
MaterialSlice_51_52.ComponentByAtoms He 2
MaterialSlice_51_52.ComponentByAtoms N 784802
MaterialSlice_51_52.ComponentByAtoms O 210501
MaterialSlice_51_52.ComponentByAtoms Ar 4693

Volume VolumeSlice_51_52
VolumeSlice_51_52.Material MaterialSlice_51_52
VolumeSlice_51_52.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_51_52.Visibility  1
VolumeSlice_51_52.Position 0 0 5150000.000000
VolumeSlice_51_52.Mother World

Material MaterialSlice_52_53
MaterialSlice_52_53.Density 8.628000e-07
MaterialSlice_52_53.ComponentByAtoms He 2
MaterialSlice_52_53.ComponentByAtoms N 784735
MaterialSlice_52_53.ComponentByAtoms O 210568
MaterialSlice_52_53.ComponentByAtoms Ar 4693

Volume VolumeSlice_52_53
VolumeSlice_52_53.Material MaterialSlice_52_53
VolumeSlice_52_53.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_52_53.Visibility  1
VolumeSlice_52_53.Position 0 0 5250000.000000
VolumeSlice_52_53.Mother World

Material MaterialSlice_53_54
MaterialSlice_53_54.Density 7.638000e-07
MaterialSlice_53_54.ComponentByAtoms He 2
MaterialSlice_53_54.ComponentByAtoms N 784683
MaterialSlice_53_54.ComponentByAtoms O 210618
MaterialSlice_53_54.ComponentByAtoms Ar 4694

Volume VolumeSlice_53_54
VolumeSlice_53_54.Material MaterialSlice_53_54
VolumeSlice_53_54.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_53_54.Visibility  1
VolumeSlice_53_54.Position 0 0 5350000.000000
VolumeSlice_53_54.Mother World

Material MaterialSlice_54_55
MaterialSlice_54_55.Density 6.768000e-07
MaterialSlice_54_55.ComponentByAtoms He 2
MaterialSlice_54_55.ComponentByAtoms N 784770
MaterialSlice_54_55.ComponentByAtoms O 210532
MaterialSlice_54_55.ComponentByAtoms Ar 4694

Volume VolumeSlice_54_55
VolumeSlice_54_55.Material MaterialSlice_54_55
VolumeSlice_54_55.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_54_55.Visibility  1
VolumeSlice_54_55.Position 0 0 5450000.000000
VolumeSlice_54_55.Mother World

Material MaterialSlice_55_56
MaterialSlice_55_56.Density 6.001000e-07
MaterialSlice_55_56.ComponentByAtoms He 2
MaterialSlice_55_56.ComponentByAtoms N 784753
MaterialSlice_55_56.ComponentByAtoms O 210549
MaterialSlice_55_56.ComponentByAtoms Ar 4694

Volume VolumeSlice_55_56
VolumeSlice_55_56.Material MaterialSlice_55_56
VolumeSlice_55_56.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_55_56.Visibility  1
VolumeSlice_55_56.Position 0 0 5550000.000000
VolumeSlice_55_56.Mother World

Material MaterialSlice_56_57
MaterialSlice_56_57.Density 5.323000e-07
MaterialSlice_56_57.ComponentByAtoms He 2
MaterialSlice_56_57.ComponentByAtoms N 784777
MaterialSlice_56_57.ComponentByAtoms O 210525
MaterialSlice_56_57.ComponentByAtoms Ar 4693

Volume VolumeSlice_56_57
VolumeSlice_56_57.Material MaterialSlice_56_57
VolumeSlice_56_57.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_56_57.Visibility  1
VolumeSlice_56_57.Position 0 0 5650000.000000
VolumeSlice_56_57.Mother World

Material MaterialSlice_57_58
MaterialSlice_57_58.Density 4.722000e-07
MaterialSlice_57_58.ComponentByAtoms He 2
MaterialSlice_57_58.ComponentByAtoms N 784770
MaterialSlice_57_58.ComponentByAtoms O 210533
MaterialSlice_57_58.ComponentByAtoms Ar 4693

Volume VolumeSlice_57_58
VolumeSlice_57_58.Material MaterialSlice_57_58
VolumeSlice_57_58.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_57_58.Visibility  1
VolumeSlice_57_58.Position 0 0 5750000.000000
VolumeSlice_57_58.Mother World

Material MaterialSlice_58_59
MaterialSlice_58_59.Density 4.187000e-07
MaterialSlice_58_59.ComponentByAtoms He 2
MaterialSlice_58_59.ComponentByAtoms N 784734
MaterialSlice_58_59.ComponentByAtoms O 210569
MaterialSlice_58_59.ComponentByAtoms Ar 4693

Volume VolumeSlice_58_59
VolumeSlice_58_59.Material MaterialSlice_58_59
VolumeSlice_58_59.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_58_59.Visibility  1
VolumeSlice_58_59.Position 0 0 5850000.000000
VolumeSlice_58_59.Mother World

Material MaterialSlice_59_60
MaterialSlice_59_60.Density 3.709000e-07
MaterialSlice_59_60.ComponentByAtoms He 2
MaterialSlice_59_60.ComponentByAtoms N 784813
MaterialSlice_59_60.ComponentByAtoms O 210489
MaterialSlice_59_60.ComponentByAtoms Ar 4694

Volume VolumeSlice_59_60
VolumeSlice_59_60.Material MaterialSlice_59_60
VolumeSlice_59_60.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_59_60.Visibility  1
VolumeSlice_59_60.Position 0 0 5950000.000000
VolumeSlice_59_60.Mother World

Material MaterialSlice_60_61
MaterialSlice_60_61.Density 3.282000e-07
MaterialSlice_60_61.ComponentByAtoms He 2
MaterialSlice_60_61.ComponentByAtoms N 784798
MaterialSlice_60_61.ComponentByAtoms O 210505
MaterialSlice_60_61.ComponentByAtoms Ar 4693

Volume VolumeSlice_60_61
VolumeSlice_60_61.Material MaterialSlice_60_61
VolumeSlice_60_61.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_60_61.Visibility  1
VolumeSlice_60_61.Position 0 0 6050000.000000
VolumeSlice_60_61.Mother World

Material MaterialSlice_61_62
MaterialSlice_61_62.Density 2.899000e-07
MaterialSlice_61_62.ComponentByAtoms He 2
MaterialSlice_61_62.ComponentByAtoms N 784819
MaterialSlice_61_62.ComponentByAtoms O 210484
MaterialSlice_61_62.ComponentByAtoms Ar 4693

Volume VolumeSlice_61_62
VolumeSlice_61_62.Material MaterialSlice_61_62
VolumeSlice_61_62.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_61_62.Visibility  1
VolumeSlice_61_62.Position 0 0 6150000.000000
VolumeSlice_61_62.Mother World

Material MaterialSlice_62_63
MaterialSlice_62_63.Density 2.557000e-07
MaterialSlice_62_63.ComponentByAtoms He 2
MaterialSlice_62_63.ComponentByAtoms N 784721
MaterialSlice_62_63.ComponentByAtoms O 210581
MaterialSlice_62_63.ComponentByAtoms Ar 4694

Volume VolumeSlice_62_63
VolumeSlice_62_63.Material MaterialSlice_62_63
VolumeSlice_62_63.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_62_63.Visibility  1
VolumeSlice_62_63.Position 0 0 6250000.000000
VolumeSlice_62_63.Mother World

Material MaterialSlice_63_64
MaterialSlice_63_64.Density 2.249000e-07
MaterialSlice_63_64.ComponentByAtoms He 2
MaterialSlice_63_64.ComponentByAtoms N 784923
MaterialSlice_63_64.ComponentByAtoms O 210379
MaterialSlice_63_64.ComponentByAtoms Ar 4694

Volume VolumeSlice_63_64
VolumeSlice_63_64.Material MaterialSlice_63_64
VolumeSlice_63_64.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_63_64.Visibility  1
VolumeSlice_63_64.Position 0 0 6350000.000000
VolumeSlice_63_64.Mother World

Material MaterialSlice_64_65
MaterialSlice_64_65.Density 1.974000e-07
MaterialSlice_64_65.ComponentByAtoms He 2
MaterialSlice_64_65.ComponentByAtoms N 785282
MaterialSlice_64_65.ComponentByAtoms O 210020
MaterialSlice_64_65.ComponentByAtoms Ar 4694

Volume VolumeSlice_64_65
VolumeSlice_64_65.Material MaterialSlice_64_65
VolumeSlice_64_65.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_64_65.Visibility  1
VolumeSlice_64_65.Position 0 0 6450000.000000
VolumeSlice_64_65.Mother World

Material MaterialSlice_65_66
MaterialSlice_65_66.Density 1.727000e-07
MaterialSlice_65_66.ComponentByAtoms He 2
MaterialSlice_65_66.ComponentByAtoms N 785609
MaterialSlice_65_66.ComponentByAtoms O 209691
MaterialSlice_65_66.ComponentByAtoms Ar 4696

Volume VolumeSlice_65_66
VolumeSlice_65_66.Material MaterialSlice_65_66
VolumeSlice_65_66.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_65_66.Visibility  1
VolumeSlice_65_66.Position 0 0 6550000.000000
VolumeSlice_65_66.Mother World

Material MaterialSlice_66_67
MaterialSlice_66_67.Density 1.508000e-07
MaterialSlice_66_67.ComponentByAtoms He 2
MaterialSlice_66_67.ComponentByAtoms N 785931
MaterialSlice_66_67.ComponentByAtoms O 209368
MaterialSlice_66_67.ComponentByAtoms Ar 4697

Volume VolumeSlice_66_67
VolumeSlice_66_67.Material MaterialSlice_66_67
VolumeSlice_66_67.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_66_67.Visibility  1
VolumeSlice_66_67.Position 0 0 6650000.000000
VolumeSlice_66_67.Mother World

Material MaterialSlice_67_68
MaterialSlice_67_68.Density 1.313000e-07
MaterialSlice_67_68.ComponentByAtoms He 2
MaterialSlice_67_68.ComponentByAtoms N 786235
MaterialSlice_67_68.ComponentByAtoms O 209062
MaterialSlice_67_68.ComponentByAtoms Ar 4699

Volume VolumeSlice_67_68
VolumeSlice_67_68.Material MaterialSlice_67_68
VolumeSlice_67_68.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_67_68.Visibility  1
VolumeSlice_67_68.Position 0 0 6750000.000000
VolumeSlice_67_68.Mother World

Material MaterialSlice_68_69
MaterialSlice_68_69.Density 1.140000e-07
MaterialSlice_68_69.ComponentByAtoms He 2
MaterialSlice_68_69.ComponentByAtoms N 786564
MaterialSlice_68_69.ComponentByAtoms O 208733
MaterialSlice_68_69.ComponentByAtoms Ar 4699

Volume VolumeSlice_68_69
VolumeSlice_68_69.Material MaterialSlice_68_69
VolumeSlice_68_69.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_68_69.Visibility  1
VolumeSlice_68_69.Position 0 0 6850000.000000
VolumeSlice_68_69.Mother World

Material MaterialSlice_69_70
MaterialSlice_69_70.Density 9.868000e-08
MaterialSlice_69_70.ComponentByAtoms He 2
MaterialSlice_69_70.ComponentByAtoms N 786945
MaterialSlice_69_70.ComponentByAtoms O 208352
MaterialSlice_69_70.ComponentByAtoms Ar 4700

Volume VolumeSlice_69_70
VolumeSlice_69_70.Material MaterialSlice_69_70
VolumeSlice_69_70.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_69_70.Visibility  1
VolumeSlice_69_70.Position 0 0 6950000.000000
VolumeSlice_69_70.Mother World

Material MaterialSlice_70_71
MaterialSlice_70_71.Density 8.520000e-08
MaterialSlice_70_71.ComponentByAtoms He 2
MaterialSlice_70_71.ComponentByAtoms N 787310
MaterialSlice_70_71.ComponentByAtoms O 207985
MaterialSlice_70_71.ComponentByAtoms Ar 4701

Volume VolumeSlice_70_71
VolumeSlice_70_71.Material MaterialSlice_70_71
VolumeSlice_70_71.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_70_71.Visibility  1
VolumeSlice_70_71.Position 0 0 7050000.000000
VolumeSlice_70_71.Mother World

Material MaterialSlice_71_72
MaterialSlice_71_72.Density 7.336000e-08
MaterialSlice_71_72.ComponentByAtoms He 2
MaterialSlice_71_72.ComponentByAtoms N 787575
MaterialSlice_71_72.ComponentByAtoms O 207717
MaterialSlice_71_72.ComponentByAtoms Ar 4704

Volume VolumeSlice_71_72
VolumeSlice_71_72.Material MaterialSlice_71_72
VolumeSlice_71_72.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_71_72.Visibility  1
VolumeSlice_71_72.Position 0 0 7150000.000000
VolumeSlice_71_72.Mother World

Material MaterialSlice_72_73
MaterialSlice_72_73.Density 6.300000e-08
MaterialSlice_72_73.ComponentByAtoms He 2
MaterialSlice_72_73.ComponentByAtoms N 787996
MaterialSlice_72_73.ComponentByAtoms O 207298
MaterialSlice_72_73.ComponentByAtoms Ar 4701

Volume VolumeSlice_72_73
VolumeSlice_72_73.Material MaterialSlice_72_73
VolumeSlice_72_73.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_72_73.Visibility  1
VolumeSlice_72_73.Position 0 0 7250000.000000
VolumeSlice_72_73.Mother World

Material MaterialSlice_73_74
MaterialSlice_73_74.Density 5.408000e-08
MaterialSlice_73_74.ComponentByAtoms He 2
MaterialSlice_73_74.ComponentByAtoms N 788257
MaterialSlice_73_74.ComponentByAtoms O 207036
MaterialSlice_73_74.ComponentByAtoms Ar 4702

Volume VolumeSlice_73_74
VolumeSlice_73_74.Material MaterialSlice_73_74
VolumeSlice_73_74.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_73_74.Visibility  1
VolumeSlice_73_74.Position 0 0 7350000.000000
VolumeSlice_73_74.Mother World

Material MaterialSlice_74_75
MaterialSlice_74_75.Density 4.638000e-08
MaterialSlice_74_75.ComponentByAtoms He 2
MaterialSlice_74_75.ComponentByAtoms N 788555
MaterialSlice_74_75.ComponentByAtoms O 206737
MaterialSlice_74_75.ComponentByAtoms Ar 4704

Volume VolumeSlice_74_75
VolumeSlice_74_75.Material MaterialSlice_74_75
VolumeSlice_74_75.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_74_75.Visibility  1
VolumeSlice_74_75.Position 0 0 7450000.000000
VolumeSlice_74_75.Mother World

Material MaterialSlice_75_76
MaterialSlice_75_76.Density 3.980000e-08
MaterialSlice_75_76.ComponentByAtoms He 2
MaterialSlice_75_76.ComponentByAtoms N 788814
MaterialSlice_75_76.ComponentByAtoms O 206478
MaterialSlice_75_76.ComponentByAtoms Ar 4704

Volume VolumeSlice_75_76
VolumeSlice_75_76.Material MaterialSlice_75_76
VolumeSlice_75_76.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_75_76.Visibility  1
VolumeSlice_75_76.Position 0 0 7550000.000000
VolumeSlice_75_76.Mother World

Material MaterialSlice_76_77
MaterialSlice_76_77.Density 3.415000e-08
MaterialSlice_76_77.ComponentByAtoms He 2
MaterialSlice_76_77.ComponentByAtoms N 789253
MaterialSlice_76_77.ComponentByAtoms O 206039
MaterialSlice_76_77.ComponentByAtoms Ar 4704

Volume VolumeSlice_76_77
VolumeSlice_76_77.Material MaterialSlice_76_77
VolumeSlice_76_77.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_76_77.Visibility  1
VolumeSlice_76_77.Position 0 0 7650000.000000
VolumeSlice_76_77.Mother World

Material MaterialSlice_77_78
MaterialSlice_77_78.Density 2.928000e-08
MaterialSlice_77_78.ComponentByAtoms He 2
MaterialSlice_77_78.ComponentByAtoms N 789608
MaterialSlice_77_78.ComponentByAtoms O 205684
MaterialSlice_77_78.ComponentByAtoms Ar 4704

Volume VolumeSlice_77_78
VolumeSlice_77_78.Material MaterialSlice_77_78
VolumeSlice_77_78.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_77_78.Visibility  1
VolumeSlice_77_78.Position 0 0 7750000.000000
VolumeSlice_77_78.Mother World

Material MaterialSlice_78_79
MaterialSlice_78_79.Density 2.508000e-08
MaterialSlice_78_79.ComponentByAtoms He 2
MaterialSlice_78_79.ComponentByAtoms N 790040
MaterialSlice_78_79.ComponentByAtoms O 205253
MaterialSlice_78_79.ComponentByAtoms Ar 4703

Volume VolumeSlice_78_79
VolumeSlice_78_79.Material MaterialSlice_78_79
VolumeSlice_78_79.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_78_79.Visibility  1
VolumeSlice_78_79.Position 0 0 7850000.000000
VolumeSlice_78_79.Mother World

Material MaterialSlice_79_80
MaterialSlice_79_80.Density 2.145000e-08
MaterialSlice_79_80.ComponentByAtoms He 2
MaterialSlice_79_80.ComponentByAtoms N 790415
MaterialSlice_79_80.ComponentByAtoms O 204880
MaterialSlice_79_80.ComponentByAtoms Ar 4702

Volume VolumeSlice_79_80
VolumeSlice_79_80.Material MaterialSlice_79_80
VolumeSlice_79_80.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_79_80.Visibility  1
VolumeSlice_79_80.Position 0 0 7950000.000000
VolumeSlice_79_80.Mother World

Material MaterialSlice_80_81
MaterialSlice_80_81.Density 1.832000e-08
MaterialSlice_80_81.ComponentByAtoms He 2
MaterialSlice_80_81.ComponentByAtoms N 790892
MaterialSlice_80_81.ComponentByAtoms O 204404
MaterialSlice_80_81.ComponentByAtoms Ar 4701

Volume VolumeSlice_80_81
VolumeSlice_80_81.Material MaterialSlice_80_81
VolumeSlice_80_81.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_80_81.Visibility  1
VolumeSlice_80_81.Position 0 0 8050000.000000
VolumeSlice_80_81.Mother World

Material MaterialSlice_81_82
MaterialSlice_81_82.Density 1.561000e-08
MaterialSlice_81_82.ComponentByAtoms He 2
MaterialSlice_81_82.ComponentByAtoms N 791382
MaterialSlice_81_82.ComponentByAtoms O 203915
MaterialSlice_81_82.ComponentByAtoms Ar 4698

Volume VolumeSlice_81_82
VolumeSlice_81_82.Material MaterialSlice_81_82
VolumeSlice_81_82.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_81_82.Visibility  1
VolumeSlice_81_82.Position 0 0 8150000.000000
VolumeSlice_81_82.Mother World

Material MaterialSlice_82_83
MaterialSlice_82_83.Density 1.326000e-08
MaterialSlice_82_83.ComponentByAtoms He 2
MaterialSlice_82_83.ComponentByAtoms N 791974
MaterialSlice_82_83.ComponentByAtoms O 203325
MaterialSlice_82_83.ComponentByAtoms Ar 4696

Volume VolumeSlice_82_83
VolumeSlice_82_83.Material MaterialSlice_82_83
VolumeSlice_82_83.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_82_83.Visibility  1
VolumeSlice_82_83.Position 0 0 8250000.000000
VolumeSlice_82_83.Mother World

Material MaterialSlice_83_84
MaterialSlice_83_84.Density 1.124000e-08
MaterialSlice_83_84.ComponentByAtoms He 2
MaterialSlice_83_84.ComponentByAtoms N 792513
MaterialSlice_83_84.ComponentByAtoms O 202789
MaterialSlice_83_84.ComponentByAtoms Ar 4694

Volume VolumeSlice_83_84
VolumeSlice_83_84.Material MaterialSlice_83_84
VolumeSlice_83_84.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_83_84.Visibility  1
VolumeSlice_83_84.Position 0 0 8350000.000000
VolumeSlice_83_84.Mother World

Material MaterialSlice_84_85
MaterialSlice_84_85.Density 9.504000e-09
MaterialSlice_84_85.ComponentByAtoms He 2
MaterialSlice_84_85.ComponentByAtoms N 793186
MaterialSlice_84_85.ComponentByAtoms O 202120
MaterialSlice_84_85.ComponentByAtoms Ar 4689

Volume VolumeSlice_84_85
VolumeSlice_84_85.Material MaterialSlice_84_85
VolumeSlice_84_85.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_84_85.Visibility  1
VolumeSlice_84_85.Position 0 0 8450000.000000
VolumeSlice_84_85.Mother World

Material MaterialSlice_85_86
MaterialSlice_85_86.Density 8.012000e-09
MaterialSlice_85_86.ComponentByAtoms He 2
MaterialSlice_85_86.ComponentByAtoms N 793872
MaterialSlice_85_86.ComponentByAtoms O 201442
MaterialSlice_85_86.ComponentByAtoms Ar 4682

Volume VolumeSlice_85_86
VolumeSlice_85_86.Material MaterialSlice_85_86
VolumeSlice_85_86.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_85_86.Visibility  1
VolumeSlice_85_86.Position 0 0 8550000.000000
VolumeSlice_85_86.Mother World

Material MaterialSlice_86_87
MaterialSlice_86_87.Density 6.735000e-09
MaterialSlice_86_87.ComponentByAtoms He 2
MaterialSlice_86_87.ComponentByAtoms N 794577
MaterialSlice_86_87.ComponentByAtoms O 200744
MaterialSlice_86_87.ComponentByAtoms Ar 4674

Volume VolumeSlice_86_87
VolumeSlice_86_87.Material MaterialSlice_86_87
VolumeSlice_86_87.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_86_87.Visibility  1
VolumeSlice_86_87.Position 0 0 8650000.000000
VolumeSlice_86_87.Mother World

Material MaterialSlice_87_88
MaterialSlice_87_88.Density 5.645000e-09
MaterialSlice_87_88.ComponentByAtoms He 2
MaterialSlice_87_88.ComponentByAtoms N 795297
MaterialSlice_87_88.ComponentByAtoms O 200031
MaterialSlice_87_88.ComponentByAtoms Ar 4667

Volume VolumeSlice_87_88
VolumeSlice_87_88.Material MaterialSlice_87_88
VolumeSlice_87_88.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_87_88.Visibility  1
VolumeSlice_87_88.Position 0 0 8750000.000000
VolumeSlice_87_88.Mother World

Material MaterialSlice_88_89
MaterialSlice_88_89.Density 4.719000e-09
MaterialSlice_88_89.ComponentByAtoms He 2
MaterialSlice_88_89.ComponentByAtoms N 795987
MaterialSlice_88_89.ComponentByAtoms O 199356
MaterialSlice_88_89.ComponentByAtoms Ar 4653

Volume VolumeSlice_88_89
VolumeSlice_88_89.Material MaterialSlice_88_89
VolumeSlice_88_89.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_88_89.Visibility  1
VolumeSlice_88_89.Position 0 0 8850000.000000
VolumeSlice_88_89.Mother World

Material MaterialSlice_89_90
MaterialSlice_89_90.Density 3.934000e-09
MaterialSlice_89_90.ComponentByAtoms He 3
MaterialSlice_89_90.ComponentByAtoms N 796664
MaterialSlice_89_90.ComponentByAtoms O 198695
MaterialSlice_89_90.ComponentByAtoms Ar 4636

Volume VolumeSlice_89_90
VolumeSlice_89_90.Material MaterialSlice_89_90
VolumeSlice_89_90.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_89_90.Visibility  1
VolumeSlice_89_90.Position 0 0 8950000.000000
VolumeSlice_89_90.Mother World

Material MaterialSlice_90_91
MaterialSlice_90_91.Density 3.271000e-09
MaterialSlice_90_91.ComponentByAtoms He 3
MaterialSlice_90_91.ComponentByAtoms N 797345
MaterialSlice_90_91.ComponentByAtoms O 198033
MaterialSlice_90_91.ComponentByAtoms Ar 4617

Volume VolumeSlice_90_91
VolumeSlice_90_91.Material MaterialSlice_90_91
VolumeSlice_90_91.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_90_91.Visibility  1
VolumeSlice_90_91.Position 0 0 9050000.000000
VolumeSlice_90_91.Mother World

Material MaterialSlice_91_92
MaterialSlice_91_92.Density 2.715000e-09
MaterialSlice_91_92.ComponentByAtoms He 3
MaterialSlice_91_92.ComponentByAtoms N 797903
MaterialSlice_91_92.ComponentByAtoms O 197501
MaterialSlice_91_92.ComponentByAtoms Ar 4592

Volume VolumeSlice_91_92
VolumeSlice_91_92.Material MaterialSlice_91_92
VolumeSlice_91_92.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_91_92.Visibility  1
VolumeSlice_91_92.Position 0 0 9150000.000000
VolumeSlice_91_92.Mother World

Material MaterialSlice_92_93
MaterialSlice_92_93.Density 2.248000e-09
MaterialSlice_92_93.ComponentByAtoms He 3
MaterialSlice_92_93.ComponentByAtoms N 798420
MaterialSlice_92_93.ComponentByAtoms O 197013
MaterialSlice_92_93.ComponentByAtoms Ar 4562

Volume VolumeSlice_92_93
VolumeSlice_92_93.Material MaterialSlice_92_93
VolumeSlice_92_93.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_92_93.Visibility  1
VolumeSlice_92_93.Position 0 0 9250000.000000
VolumeSlice_92_93.Mother World

Material MaterialSlice_93_94
MaterialSlice_93_94.Density 1.860000e-09
MaterialSlice_93_94.ComponentByAtoms He 3
MaterialSlice_93_94.ComponentByAtoms N 798890
MaterialSlice_93_94.ComponentByAtoms O 196579
MaterialSlice_93_94.ComponentByAtoms Ar 4526

Volume VolumeSlice_93_94
VolumeSlice_93_94.Material MaterialSlice_93_94
VolumeSlice_93_94.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_93_94.Visibility  1
VolumeSlice_93_94.Position 0 0 9350000.000000
VolumeSlice_93_94.Mother World

Material MaterialSlice_94_95
MaterialSlice_94_95.Density 1.537000e-09
MaterialSlice_94_95.ComponentByAtoms He 3
MaterialSlice_94_95.ComponentByAtoms N 799264
MaterialSlice_94_95.ComponentByAtoms O 196247
MaterialSlice_94_95.ComponentByAtoms Ar 4483

Volume VolumeSlice_94_95
VolumeSlice_94_95.Material MaterialSlice_94_95
VolumeSlice_94_95.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_94_95.Visibility  1
VolumeSlice_94_95.Position 0 0 9450000.000000
VolumeSlice_94_95.Mother World

Material MaterialSlice_95_96
MaterialSlice_95_96.Density 1.269000e-09
MaterialSlice_95_96.ComponentByAtoms He 3
MaterialSlice_95_96.ComponentByAtoms N 799594
MaterialSlice_95_96.ComponentByAtoms O 195967
MaterialSlice_95_96.ComponentByAtoms Ar 4434

Volume VolumeSlice_95_96
VolumeSlice_95_96.Material MaterialSlice_95_96
VolumeSlice_95_96.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_95_96.Visibility  1
VolumeSlice_95_96.Position 0 0 9550000.000000
VolumeSlice_95_96.Mother World

Material MaterialSlice_96_97
MaterialSlice_96_97.Density 1.048000e-09
MaterialSlice_96_97.ComponentByAtoms He 3
MaterialSlice_96_97.ComponentByAtoms N 799808
MaterialSlice_96_97.ComponentByAtoms O 195810
MaterialSlice_96_97.ComponentByAtoms Ar 4375

Volume VolumeSlice_96_97
VolumeSlice_96_97.Material MaterialSlice_96_97
VolumeSlice_96_97.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_96_97.Visibility  1
VolumeSlice_96_97.Position 0 0 9650000.000000
VolumeSlice_96_97.Mother World

Material MaterialSlice_97_98
MaterialSlice_97_98.Density 8.653000e-10
MaterialSlice_97_98.ComponentByAtoms He 4
MaterialSlice_97_98.ComponentByAtoms N 800094
MaterialSlice_97_98.ComponentByAtoms O 195591
MaterialSlice_97_98.ComponentByAtoms Ar 4309

Volume VolumeSlice_97_98
VolumeSlice_97_98.Material MaterialSlice_97_98
VolumeSlice_97_98.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_97_98.Visibility  1
VolumeSlice_97_98.Position 0 0 9750000.000000
VolumeSlice_97_98.Mother World

Material MaterialSlice_98_99
MaterialSlice_98_99.Density 7.148000e-10
MaterialSlice_98_99.ComponentByAtoms He 4
MaterialSlice_98_99.ComponentByAtoms N 800284
MaterialSlice_98_99.ComponentByAtoms O 195476
MaterialSlice_98_99.ComponentByAtoms Ar 4233

Volume VolumeSlice_98_99
VolumeSlice_98_99.Material MaterialSlice_98_99
VolumeSlice_98_99.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_98_99.Visibility  1
VolumeSlice_98_99.Position 0 0 9850000.000000
VolumeSlice_98_99.Mother World

Material MaterialSlice_99_100
MaterialSlice_99_100.Density 5.909000e-10
MaterialSlice_99_100.ComponentByAtoms He 4
MaterialSlice_99_100.ComponentByAtoms N 800494
MaterialSlice_99_100.ComponentByAtoms O 195349
MaterialSlice_99_100.ComponentByAtoms Ar 4150

Volume VolumeSlice_99_100
VolumeSlice_99_100.Material MaterialSlice_99_100
VolumeSlice_99_100.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_99_100.Visibility  1
VolumeSlice_99_100.Position 0 0 9950000.000000
VolumeSlice_99_100.Mother World

Material MaterialSlice_100_101
MaterialSlice_100_101.Density 4.889000e-10
MaterialSlice_100_101.ComponentByAtoms He 5
MaterialSlice_100_101.ComponentByAtoms N 800623
MaterialSlice_100_101.ComponentByAtoms O 195313
MaterialSlice_100_101.ComponentByAtoms Ar 4057

Volume VolumeSlice_100_101
VolumeSlice_100_101.Material MaterialSlice_100_101
VolumeSlice_100_101.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_100_101.Visibility  1
VolumeSlice_100_101.Position 0 0 10050000.000000
VolumeSlice_100_101.Mother World

Material MaterialSlice_101_102
MaterialSlice_101_102.Density 4.050000e-10
MaterialSlice_101_102.ComponentByAtoms He 5
MaterialSlice_101_102.ComponentByAtoms N 800859
MaterialSlice_101_102.ComponentByAtoms O 195177
MaterialSlice_101_102.ComponentByAtoms Ar 3956

Volume VolumeSlice_101_102
VolumeSlice_101_102.Material MaterialSlice_101_102
VolumeSlice_101_102.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_101_102.Visibility  1
VolumeSlice_101_102.Position 0 0 10150000.000000
VolumeSlice_101_102.Mother World

Material MaterialSlice_102_103
MaterialSlice_102_103.Density 3.360000e-10
MaterialSlice_102_103.ComponentByAtoms H 1
MaterialSlice_102_103.ComponentByAtoms He 5
MaterialSlice_102_103.ComponentByAtoms N 800994
MaterialSlice_102_103.ComponentByAtoms O 195151
MaterialSlice_102_103.ComponentByAtoms Ar 3846

Volume VolumeSlice_102_103
VolumeSlice_102_103.Material MaterialSlice_102_103
VolumeSlice_102_103.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_102_103.Visibility  1
VolumeSlice_102_103.Position 0 0 10250000.000000
VolumeSlice_102_103.Mother World

Material MaterialSlice_103_104
MaterialSlice_103_104.Density 2.792000e-10
MaterialSlice_103_104.ComponentByAtoms H 1
MaterialSlice_103_104.ComponentByAtoms He 6
MaterialSlice_103_104.ComponentByAtoms N 801178
MaterialSlice_103_104.ComponentByAtoms O 195083
MaterialSlice_103_104.ComponentByAtoms Ar 3730

Volume VolumeSlice_103_104
VolumeSlice_103_104.Material MaterialSlice_103_104
VolumeSlice_103_104.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_103_104.Visibility  1
VolumeSlice_103_104.Position 0 0 10350000.000000
VolumeSlice_103_104.Mother World

Material MaterialSlice_104_105
MaterialSlice_104_105.Density 2.324000e-10
MaterialSlice_104_105.ComponentByAtoms H 1
MaterialSlice_104_105.ComponentByAtoms He 7
MaterialSlice_104_105.ComponentByAtoms N 801310
MaterialSlice_104_105.ComponentByAtoms O 195072
MaterialSlice_104_105.ComponentByAtoms Ar 3608

Volume VolumeSlice_104_105
VolumeSlice_104_105.Material MaterialSlice_104_105
VolumeSlice_104_105.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_104_105.Visibility  1
VolumeSlice_104_105.Position 0 0 10450000.000000
VolumeSlice_104_105.Mother World

Material MaterialSlice_105_106
MaterialSlice_105_106.Density 1.939000e-10
MaterialSlice_105_106.ComponentByAtoms H 1
MaterialSlice_105_106.ComponentByAtoms He 7
MaterialSlice_105_106.ComponentByAtoms N 801443
MaterialSlice_105_106.ComponentByAtoms O 195066
MaterialSlice_105_106.ComponentByAtoms Ar 3480

Volume VolumeSlice_105_106
VolumeSlice_105_106.Material MaterialSlice_105_106
VolumeSlice_105_106.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_105_106.Visibility  1
VolumeSlice_105_106.Position 0 0 10550000.000000
VolumeSlice_105_106.Mother World

Material MaterialSlice_106_107
MaterialSlice_106_107.Density 1.621000e-10
MaterialSlice_106_107.ComponentByAtoms H 1
MaterialSlice_106_107.ComponentByAtoms He 8
MaterialSlice_106_107.ComponentByAtoms N 801524
MaterialSlice_106_107.ComponentByAtoms O 195117
MaterialSlice_106_107.ComponentByAtoms Ar 3347

Volume VolumeSlice_106_107
VolumeSlice_106_107.Material MaterialSlice_106_107
VolumeSlice_106_107.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_106_107.Visibility  1
VolumeSlice_106_107.Position 0 0 10650000.000000
VolumeSlice_106_107.Mother World

Material MaterialSlice_107_108
MaterialSlice_107_108.Density 1.358000e-10
MaterialSlice_107_108.ComponentByAtoms H 1
MaterialSlice_107_108.ComponentByAtoms He 9
MaterialSlice_107_108.ComponentByAtoms N 801523
MaterialSlice_107_108.ComponentByAtoms O 195249
MaterialSlice_107_108.ComponentByAtoms Ar 3214

Volume VolumeSlice_107_108
VolumeSlice_107_108.Material MaterialSlice_107_108
VolumeSlice_107_108.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_107_108.Visibility  1
VolumeSlice_107_108.Position 0 0 10750000.000000
VolumeSlice_107_108.Mother World

Material MaterialSlice_108_109
MaterialSlice_108_109.Density 1.141000e-10
MaterialSlice_108_109.ComponentByAtoms H 1
MaterialSlice_108_109.ComponentByAtoms He 10
MaterialSlice_108_109.ComponentByAtoms N 801467
MaterialSlice_108_109.ComponentByAtoms O 195439
MaterialSlice_108_109.ComponentByAtoms Ar 3080

Volume VolumeSlice_108_109
VolumeSlice_108_109.Material MaterialSlice_108_109
VolumeSlice_108_109.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_108_109.Visibility  1
VolumeSlice_108_109.Position 0 0 10850000.000000
VolumeSlice_108_109.Mother World

Material MaterialSlice_109_110
MaterialSlice_109_110.Density 9.603000e-11
MaterialSlice_109_110.ComponentByAtoms H 2
MaterialSlice_109_110.ComponentByAtoms He 12
MaterialSlice_109_110.ComponentByAtoms N 801273
MaterialSlice_109_110.ComponentByAtoms O 195766
MaterialSlice_109_110.ComponentByAtoms Ar 2946

Volume VolumeSlice_109_110
VolumeSlice_109_110.Material MaterialSlice_109_110
VolumeSlice_109_110.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_109_110.Visibility  1
VolumeSlice_109_110.Position 0 0 10950000.000000
VolumeSlice_109_110.Mother World

Material MaterialSlice_110_111
MaterialSlice_110_111.Density 8.107000e-11
MaterialSlice_110_111.ComponentByAtoms H 2
MaterialSlice_110_111.ComponentByAtoms He 13
MaterialSlice_110_111.ComponentByAtoms N 801006
MaterialSlice_110_111.ComponentByAtoms O 196163
MaterialSlice_110_111.ComponentByAtoms Ar 2813

Volume VolumeSlice_110_111
VolumeSlice_110_111.Material MaterialSlice_110_111
VolumeSlice_110_111.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_110_111.Visibility  1
VolumeSlice_110_111.Position 0 0 11050000.000000
VolumeSlice_110_111.Mother World

Material MaterialSlice_111_112
MaterialSlice_111_112.Density 6.863000e-11
MaterialSlice_111_112.ComponentByAtoms H 2
MaterialSlice_111_112.ComponentByAtoms He 15
MaterialSlice_111_112.ComponentByAtoms N 800610
MaterialSlice_111_112.ComponentByAtoms O 196688
MaterialSlice_111_112.ComponentByAtoms Ar 2683

Volume VolumeSlice_111_112
VolumeSlice_111_112.Material MaterialSlice_111_112
VolumeSlice_111_112.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_111_112.Visibility  1
VolumeSlice_111_112.Position 0 0 11150000.000000
VolumeSlice_111_112.Mother World

Material MaterialSlice_112_113
MaterialSlice_112_113.Density 5.826000e-11
MaterialSlice_112_113.ComponentByAtoms H 2
MaterialSlice_112_113.ComponentByAtoms He 17
MaterialSlice_112_113.ComponentByAtoms N 800180
MaterialSlice_112_113.ComponentByAtoms O 197243
MaterialSlice_112_113.ComponentByAtoms Ar 2556

Volume VolumeSlice_112_113
VolumeSlice_112_113.Material MaterialSlice_112_113
VolumeSlice_112_113.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_112_113.Visibility  1
VolumeSlice_112_113.Position 0 0 11250000.000000
VolumeSlice_112_113.Mother World

Material MaterialSlice_113_114
MaterialSlice_113_114.Density 4.961000e-11
MaterialSlice_113_114.ComponentByAtoms H 2
MaterialSlice_113_114.ComponentByAtoms He 19
MaterialSlice_113_114.ComponentByAtoms N 799564
MaterialSlice_113_114.ComponentByAtoms O 197979
MaterialSlice_113_114.ComponentByAtoms Ar 2434

Volume VolumeSlice_113_114
VolumeSlice_113_114.Material MaterialSlice_113_114
VolumeSlice_113_114.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_113_114.Visibility  1
VolumeSlice_113_114.Position 0 0 11350000.000000
VolumeSlice_113_114.Mother World

Material MaterialSlice_114_115
MaterialSlice_114_115.Density 4.239000e-11
MaterialSlice_114_115.ComponentByAtoms H 3
MaterialSlice_114_115.ComponentByAtoms He 21
MaterialSlice_114_115.ComponentByAtoms N 798806
MaterialSlice_114_115.ComponentByAtoms O 198852
MaterialSlice_114_115.ComponentByAtoms Ar 2316

Volume VolumeSlice_114_115
VolumeSlice_114_115.Material MaterialSlice_114_115
VolumeSlice_114_115.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_114_115.Visibility  1
VolumeSlice_114_115.Position 0 0 11450000.000000
VolumeSlice_114_115.Mother World

Material MaterialSlice_115_116
MaterialSlice_115_116.Density 3.634000e-11
MaterialSlice_115_116.ComponentByAtoms H 3
MaterialSlice_115_116.ComponentByAtoms He 23
MaterialSlice_115_116.ComponentByAtoms N 798003
MaterialSlice_115_116.ComponentByAtoms O 199764
MaterialSlice_115_116.ComponentByAtoms Ar 2204

Volume VolumeSlice_115_116
VolumeSlice_115_116.Material MaterialSlice_115_116
VolumeSlice_115_116.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_115_116.Visibility  1
VolumeSlice_115_116.Position 0 0 11550000.000000
VolumeSlice_115_116.Mother World

Material MaterialSlice_116_117
MaterialSlice_116_117.Density 3.128000e-11
MaterialSlice_116_117.ComponentByAtoms H 3
MaterialSlice_116_117.ComponentByAtoms He 26
MaterialSlice_116_117.ComponentByAtoms N 797070
MaterialSlice_116_117.ComponentByAtoms O 200801
MaterialSlice_116_117.ComponentByAtoms Ar 2097

Volume VolumeSlice_116_117
VolumeSlice_116_117.Material MaterialSlice_116_117
VolumeSlice_116_117.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_116_117.Visibility  1
VolumeSlice_116_117.Position 0 0 11650000.000000
VolumeSlice_116_117.Mother World

Material MaterialSlice_117_118
MaterialSlice_117_118.Density 2.702000e-11
MaterialSlice_117_118.ComponentByAtoms H 3
MaterialSlice_117_118.ComponentByAtoms He 29
MaterialSlice_117_118.ComponentByAtoms N 796060
MaterialSlice_117_118.ComponentByAtoms O 201909
MaterialSlice_117_118.ComponentByAtoms Ar 1997

Volume VolumeSlice_117_118
VolumeSlice_117_118.Material MaterialSlice_117_118
VolumeSlice_117_118.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_117_118.Visibility  1
VolumeSlice_117_118.Position 0 0 11750000.000000
VolumeSlice_117_118.Mother World

Material MaterialSlice_118_119
MaterialSlice_118_119.Density 2.344000e-11
MaterialSlice_118_119.ComponentByAtoms H 4
MaterialSlice_118_119.ComponentByAtoms He 31
MaterialSlice_118_119.ComponentByAtoms N 794961
MaterialSlice_118_119.ComponentByAtoms O 203100
MaterialSlice_118_119.ComponentByAtoms Ar 1901

Volume VolumeSlice_118_119
VolumeSlice_118_119.Material MaterialSlice_118_119
VolumeSlice_118_119.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_118_119.Visibility  1
VolumeSlice_118_119.Position 0 0 11850000.000000
VolumeSlice_118_119.Mother World

Material MaterialSlice_119_120
MaterialSlice_119_120.Density 2.043000e-11
MaterialSlice_119_120.ComponentByAtoms H 4
MaterialSlice_119_120.ComponentByAtoms He 35
MaterialSlice_119_120.ComponentByAtoms N 793742
MaterialSlice_119_120.ComponentByAtoms O 204403
MaterialSlice_119_120.ComponentByAtoms Ar 1813

Volume VolumeSlice_119_120
VolumeSlice_119_120.Material MaterialSlice_119_120
VolumeSlice_119_120.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_119_120.Visibility  1
VolumeSlice_119_120.Position 0 0 11950000.000000
VolumeSlice_119_120.Mother World

Material MaterialSlice_120_121
MaterialSlice_120_121.Density 1.788000e-11
MaterialSlice_120_121.ComponentByAtoms H 4
MaterialSlice_120_121.ComponentByAtoms He 38
MaterialSlice_120_121.ComponentByAtoms N 792495
MaterialSlice_120_121.ComponentByAtoms O 205731
MaterialSlice_120_121.ComponentByAtoms Ar 1729

Volume VolumeSlice_120_121
VolumeSlice_120_121.Material MaterialSlice_120_121
VolumeSlice_120_121.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_120_121.Visibility  1
VolumeSlice_120_121.Position 0 0 12050000.000000
VolumeSlice_120_121.Mother World

Material MaterialSlice_121_122
MaterialSlice_121_122.Density 1.574000e-11
MaterialSlice_121_122.ComponentByAtoms H 4
MaterialSlice_121_122.ComponentByAtoms He 42
MaterialSlice_121_122.ComponentByAtoms N 791187
MaterialSlice_121_122.ComponentByAtoms O 207114
MaterialSlice_121_122.ComponentByAtoms Ar 1651

Volume VolumeSlice_121_122
VolumeSlice_121_122.Material MaterialSlice_121_122
VolumeSlice_121_122.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_121_122.Visibility  1
VolumeSlice_121_122.Position 0 0 12150000.000000
VolumeSlice_121_122.Mother World

Material MaterialSlice_122_123
MaterialSlice_122_123.Density 1.392000e-11
MaterialSlice_122_123.ComponentByAtoms H 5
MaterialSlice_122_123.ComponentByAtoms He 45
MaterialSlice_122_123.ComponentByAtoms N 789815
MaterialSlice_122_123.ComponentByAtoms O 208553
MaterialSlice_122_123.ComponentByAtoms Ar 1579

Volume VolumeSlice_122_123
VolumeSlice_122_123.Material MaterialSlice_122_123
VolumeSlice_122_123.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_122_123.Visibility  1
VolumeSlice_122_123.Position 0 0 12250000.000000
VolumeSlice_122_123.Mother World

Material MaterialSlice_123_124
MaterialSlice_123_124.Density 1.239000e-11
MaterialSlice_123_124.ComponentByAtoms H 5
MaterialSlice_123_124.ComponentByAtoms He 49
MaterialSlice_123_124.ComponentByAtoms N 788349
MaterialSlice_123_124.ComponentByAtoms O 210083
MaterialSlice_123_124.ComponentByAtoms Ar 1511

Volume VolumeSlice_123_124
VolumeSlice_123_124.Material MaterialSlice_123_124
VolumeSlice_123_124.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_123_124.Visibility  1
VolumeSlice_123_124.Position 0 0 12350000.000000
VolumeSlice_123_124.Mother World

Material MaterialSlice_124_125
MaterialSlice_124_125.Density 1.109000e-11
MaterialSlice_124_125.ComponentByAtoms H 5
MaterialSlice_124_125.ComponentByAtoms He 53
MaterialSlice_124_125.ComponentByAtoms N 786815
MaterialSlice_124_125.ComponentByAtoms O 211676
MaterialSlice_124_125.ComponentByAtoms Ar 1448

Volume VolumeSlice_124_125
VolumeSlice_124_125.Material MaterialSlice_124_125
VolumeSlice_124_125.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_124_125.Visibility  1
VolumeSlice_124_125.Position 0 0 12450000.000000
VolumeSlice_124_125.Mother World

Material MaterialSlice_125_126
MaterialSlice_125_126.Density 9.977000e-12
MaterialSlice_125_126.ComponentByAtoms H 5
MaterialSlice_125_126.ComponentByAtoms He 57
MaterialSlice_125_126.ComponentByAtoms N 785288
MaterialSlice_125_126.ComponentByAtoms O 213257
MaterialSlice_125_126.ComponentByAtoms Ar 1390

Volume VolumeSlice_125_126
VolumeSlice_125_126.Material MaterialSlice_125_126
VolumeSlice_125_126.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_125_126.Visibility  1
VolumeSlice_125_126.Position 0 0 12550000.000000
VolumeSlice_125_126.Mother World

Material MaterialSlice_126_127
MaterialSlice_126_127.Density 9.010000e-12
MaterialSlice_126_127.ComponentByAtoms H 5
MaterialSlice_126_127.ComponentByAtoms He 62
MaterialSlice_126_127.ComponentByAtoms N 783590
MaterialSlice_126_127.ComponentByAtoms O 215005
MaterialSlice_126_127.ComponentByAtoms Ar 1335

Volume VolumeSlice_126_127
VolumeSlice_126_127.Material MaterialSlice_126_127
VolumeSlice_126_127.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_126_127.Visibility  1
VolumeSlice_126_127.Position 0 0 12650000.000000
VolumeSlice_126_127.Mother World

Material MaterialSlice_127_128
MaterialSlice_127_128.Density 8.168000e-12
MaterialSlice_127_128.ComponentByAtoms H 5
MaterialSlice_127_128.ComponentByAtoms He 66
MaterialSlice_127_128.ComponentByAtoms N 781937
MaterialSlice_127_128.ComponentByAtoms O 216705
MaterialSlice_127_128.ComponentByAtoms Ar 1283

Volume VolumeSlice_127_128
VolumeSlice_127_128.Material MaterialSlice_127_128
VolumeSlice_127_128.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_127_128.Visibility  1
VolumeSlice_127_128.Position 0 0 12750000.000000
VolumeSlice_127_128.Mother World

Material MaterialSlice_128_129
MaterialSlice_128_129.Density 7.430000e-12
MaterialSlice_128_129.ComponentByAtoms H 6
MaterialSlice_128_129.ComponentByAtoms He 71
MaterialSlice_128_129.ComponentByAtoms N 780206
MaterialSlice_128_129.ComponentByAtoms O 218480
MaterialSlice_128_129.ComponentByAtoms Ar 1235

Volume VolumeSlice_128_129
VolumeSlice_128_129.Material MaterialSlice_128_129
VolumeSlice_128_129.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_128_129.Visibility  1
VolumeSlice_128_129.Position 0 0 12850000.000000
VolumeSlice_128_129.Mother World

Material MaterialSlice_129_130
MaterialSlice_129_130.Density 6.780000e-12
MaterialSlice_129_130.ComponentByAtoms H 6
MaterialSlice_129_130.ComponentByAtoms He 76
MaterialSlice_129_130.ComponentByAtoms N 778259
MaterialSlice_129_130.ComponentByAtoms O 220467
MaterialSlice_129_130.ComponentByAtoms Ar 1190

Volume VolumeSlice_129_130
VolumeSlice_129_130.Material MaterialSlice_129_130
VolumeSlice_129_130.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_129_130.Visibility  1
VolumeSlice_129_130.Position 0 0 12950000.000000
VolumeSlice_129_130.Mother World

Material MaterialSlice_130_131
MaterialSlice_130_131.Density 6.206000e-12
MaterialSlice_130_131.ComponentByAtoms H 6
MaterialSlice_130_131.ComponentByAtoms He 81
MaterialSlice_130_131.ComponentByAtoms N 776490
MaterialSlice_130_131.ComponentByAtoms O 222274
MaterialSlice_130_131.ComponentByAtoms Ar 1147

Volume VolumeSlice_130_131
VolumeSlice_130_131.Material MaterialSlice_130_131
VolumeSlice_130_131.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_130_131.Visibility  1
VolumeSlice_130_131.Position 0 0 13050000.000000
VolumeSlice_130_131.Mother World

Material MaterialSlice_131_132
MaterialSlice_131_132.Density 5.697000e-12
MaterialSlice_131_132.ComponentByAtoms H 6
MaterialSlice_131_132.ComponentByAtoms He 86
MaterialSlice_131_132.ComponentByAtoms N 774542
MaterialSlice_131_132.ComponentByAtoms O 224257
MaterialSlice_131_132.ComponentByAtoms Ar 1107

Volume VolumeSlice_131_132
VolumeSlice_131_132.Material MaterialSlice_131_132
VolumeSlice_131_132.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_131_132.Visibility  1
VolumeSlice_131_132.Position 0 0 13150000.000000
VolumeSlice_131_132.Mother World

Material MaterialSlice_132_133
MaterialSlice_132_133.Density 5.243000e-12
MaterialSlice_132_133.ComponentByAtoms H 6
MaterialSlice_132_133.ComponentByAtoms He 91
MaterialSlice_132_133.ComponentByAtoms N 772562
MaterialSlice_132_133.ComponentByAtoms O 226269
MaterialSlice_132_133.ComponentByAtoms Ar 1069

Volume VolumeSlice_132_133
VolumeSlice_132_133.Material MaterialSlice_132_133
VolumeSlice_132_133.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_132_133.Visibility  1
VolumeSlice_132_133.Position 0 0 13250000.000000
VolumeSlice_132_133.Mother World

Material MaterialSlice_133_134
MaterialSlice_133_134.Density 4.837000e-12
MaterialSlice_133_134.ComponentByAtoms H 6
MaterialSlice_133_134.ComponentByAtoms He 96
MaterialSlice_133_134.ComponentByAtoms N 770487
MaterialSlice_133_134.ComponentByAtoms O 228374
MaterialSlice_133_134.ComponentByAtoms Ar 1033

Volume VolumeSlice_133_134
VolumeSlice_133_134.Material MaterialSlice_133_134
VolumeSlice_133_134.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_133_134.Visibility  1
VolumeSlice_133_134.Position 0 0 13350000.000000
VolumeSlice_133_134.Mother World

Material MaterialSlice_134_135
MaterialSlice_134_135.Density 4.472000e-12
MaterialSlice_134_135.ComponentByAtoms H 6
MaterialSlice_134_135.ComponentByAtoms He 102
MaterialSlice_134_135.ComponentByAtoms N 768417
MaterialSlice_134_135.ComponentByAtoms O 230473
MaterialSlice_134_135.ComponentByAtoms Ar 999

Volume VolumeSlice_134_135
VolumeSlice_134_135.Material MaterialSlice_134_135
VolumeSlice_134_135.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_134_135.Visibility  1
VolumeSlice_134_135.Position 0 0 13450000.000000
VolumeSlice_134_135.Mother World

Material MaterialSlice_135_136
MaterialSlice_135_136.Density 4.145000e-12
MaterialSlice_135_136.ComponentByAtoms H 7
MaterialSlice_135_136.ComponentByAtoms He 108
MaterialSlice_135_136.ComponentByAtoms N 766236
MaterialSlice_135_136.ComponentByAtoms O 232680
MaterialSlice_135_136.ComponentByAtoms Ar 967

Volume VolumeSlice_135_136
VolumeSlice_135_136.Material MaterialSlice_135_136
VolumeSlice_135_136.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_135_136.Visibility  1
VolumeSlice_135_136.Position 0 0 13550000.000000
VolumeSlice_135_136.Mother World

Material MaterialSlice_136_137
MaterialSlice_136_137.Density 3.849000e-12
MaterialSlice_136_137.ComponentByAtoms H 7
MaterialSlice_136_137.ComponentByAtoms He 114
MaterialSlice_136_137.ComponentByAtoms N 764021
MaterialSlice_136_137.ComponentByAtoms O 234919
MaterialSlice_136_137.ComponentByAtoms Ar 937

Volume VolumeSlice_136_137
VolumeSlice_136_137.Material MaterialSlice_136_137
VolumeSlice_136_137.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_136_137.Visibility  1
VolumeSlice_136_137.Position 0 0 13650000.000000
VolumeSlice_136_137.Mother World

Material MaterialSlice_137_138
MaterialSlice_137_138.Density 3.581000e-12
MaterialSlice_137_138.ComponentByAtoms H 7
MaterialSlice_137_138.ComponentByAtoms He 120
MaterialSlice_137_138.ComponentByAtoms N 761776
MaterialSlice_137_138.ComponentByAtoms O 237186
MaterialSlice_137_138.ComponentByAtoms Ar 909

Volume VolumeSlice_137_138
VolumeSlice_137_138.Material MaterialSlice_137_138
VolumeSlice_137_138.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_137_138.Visibility  1
VolumeSlice_137_138.Position 0 0 13750000.000000
VolumeSlice_137_138.Mother World

Material MaterialSlice_138_139
MaterialSlice_138_139.Density 3.338000e-12
MaterialSlice_138_139.ComponentByAtoms H 7
MaterialSlice_138_139.ComponentByAtoms He 126
MaterialSlice_138_139.ComponentByAtoms N 759510
MaterialSlice_138_139.ComponentByAtoms O 239474
MaterialSlice_138_139.ComponentByAtoms Ar 881

Volume VolumeSlice_138_139
VolumeSlice_138_139.Material MaterialSlice_138_139
VolumeSlice_138_139.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_138_139.Visibility  1
VolumeSlice_138_139.Position 0 0 13850000.000000
VolumeSlice_138_139.Mother World

Material MaterialSlice_139_140
MaterialSlice_139_140.Density 3.117000e-12
MaterialSlice_139_140.ComponentByAtoms H 7
MaterialSlice_139_140.ComponentByAtoms He 133
MaterialSlice_139_140.ComponentByAtoms N 757164
MaterialSlice_139_140.ComponentByAtoms O 241839
MaterialSlice_139_140.ComponentByAtoms Ar 855

Volume VolumeSlice_139_140
VolumeSlice_139_140.Material MaterialSlice_139_140
VolumeSlice_139_140.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_139_140.Visibility  1
VolumeSlice_139_140.Position 0 0 13950000.000000
VolumeSlice_139_140.Mother World

Material MaterialSlice_140_141
MaterialSlice_140_141.Density 2.915000e-12
MaterialSlice_140_141.ComponentByAtoms H 7
MaterialSlice_140_141.ComponentByAtoms He 139
MaterialSlice_140_141.ComponentByAtoms N 754792
MaterialSlice_140_141.ComponentByAtoms O 244229
MaterialSlice_140_141.ComponentByAtoms Ar 830

Volume VolumeSlice_140_141
VolumeSlice_140_141.Material MaterialSlice_140_141
VolumeSlice_140_141.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_140_141.Visibility  1
VolumeSlice_140_141.Position 0 0 14050000.000000
VolumeSlice_140_141.Mother World

Material MaterialSlice_141_142
MaterialSlice_141_142.Density 2.731000e-12
MaterialSlice_141_142.ComponentByAtoms H 7
MaterialSlice_141_142.ComponentByAtoms He 146
MaterialSlice_141_142.ComponentByAtoms N 752331
MaterialSlice_141_142.ComponentByAtoms O 246707
MaterialSlice_141_142.ComponentByAtoms Ar 806

Volume VolumeSlice_141_142
VolumeSlice_141_142.Material MaterialSlice_141_142
VolumeSlice_141_142.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_141_142.Visibility  1
VolumeSlice_141_142.Position 0 0 14150000.000000
VolumeSlice_141_142.Mother World

Material MaterialSlice_142_143
MaterialSlice_142_143.Density 2.562000e-12
MaterialSlice_142_143.ComponentByAtoms H 7
MaterialSlice_142_143.ComponentByAtoms He 153
MaterialSlice_142_143.ComponentByAtoms N 749884
MaterialSlice_142_143.ComponentByAtoms O 249170
MaterialSlice_142_143.ComponentByAtoms Ar 784

Volume VolumeSlice_142_143
VolumeSlice_142_143.Material MaterialSlice_142_143
VolumeSlice_142_143.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_142_143.Visibility  1
VolumeSlice_142_143.Position 0 0 14250000.000000
VolumeSlice_142_143.Mother World

Material MaterialSlice_143_144
MaterialSlice_143_144.Density 2.406000e-12
MaterialSlice_143_144.ComponentByAtoms H 7
MaterialSlice_143_144.ComponentByAtoms He 161
MaterialSlice_143_144.ComponentByAtoms N 747381
MaterialSlice_143_144.ComponentByAtoms O 251688
MaterialSlice_143_144.ComponentByAtoms Ar 762

Volume VolumeSlice_143_144
VolumeSlice_143_144.Material MaterialSlice_143_144
VolumeSlice_143_144.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_143_144.Visibility  1
VolumeSlice_143_144.Position 0 0 14350000.000000
VolumeSlice_143_144.Mother World

Material MaterialSlice_144_145
MaterialSlice_144_145.Density 2.264000e-12
MaterialSlice_144_145.ComponentByAtoms H 7
MaterialSlice_144_145.ComponentByAtoms He 168
MaterialSlice_144_145.ComponentByAtoms N 744906
MaterialSlice_144_145.ComponentByAtoms O 254175
MaterialSlice_144_145.ComponentByAtoms Ar 741

Volume VolumeSlice_144_145
VolumeSlice_144_145.Material MaterialSlice_144_145
VolumeSlice_144_145.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_144_145.Visibility  1
VolumeSlice_144_145.Position 0 0 14450000.000000
VolumeSlice_144_145.Mother World

Material MaterialSlice_145_146
MaterialSlice_145_146.Density 2.132000e-12
MaterialSlice_145_146.ComponentByAtoms H 7
MaterialSlice_145_146.ComponentByAtoms He 176
MaterialSlice_145_146.ComponentByAtoms N 742285
MaterialSlice_145_146.ComponentByAtoms O 256809
MaterialSlice_145_146.ComponentByAtoms Ar 721

Volume VolumeSlice_145_146
VolumeSlice_145_146.Material MaterialSlice_145_146
VolumeSlice_145_146.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_145_146.Visibility  1
VolumeSlice_145_146.Position 0 0 14550000.000000
VolumeSlice_145_146.Mother World

Material MaterialSlice_146_147
MaterialSlice_146_147.Density 2.011000e-12
MaterialSlice_146_147.ComponentByAtoms H 7
MaterialSlice_146_147.ComponentByAtoms He 183
MaterialSlice_146_147.ComponentByAtoms N 739684
MaterialSlice_146_147.ComponentByAtoms O 259421
MaterialSlice_146_147.ComponentByAtoms Ar 702

Volume VolumeSlice_146_147
VolumeSlice_146_147.Material MaterialSlice_146_147
VolumeSlice_146_147.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_146_147.Visibility  1
VolumeSlice_146_147.Position 0 0 14650000.000000
VolumeSlice_146_147.Mother World

Material MaterialSlice_147_148
MaterialSlice_147_148.Density 1.898000e-12
MaterialSlice_147_148.ComponentByAtoms H 7
MaterialSlice_147_148.ComponentByAtoms He 191
MaterialSlice_147_148.ComponentByAtoms N 737043
MaterialSlice_147_148.ComponentByAtoms O 262072
MaterialSlice_147_148.ComponentByAtoms Ar 683

Volume VolumeSlice_147_148
VolumeSlice_147_148.Material MaterialSlice_147_148
VolumeSlice_147_148.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_147_148.Visibility  1
VolumeSlice_147_148.Position 0 0 14750000.000000
VolumeSlice_147_148.Mother World

Material MaterialSlice_148_149
MaterialSlice_148_149.Density 1.794000e-12
MaterialSlice_148_149.ComponentByAtoms H 8
MaterialSlice_148_149.ComponentByAtoms He 200
MaterialSlice_148_149.ComponentByAtoms N 734423
MaterialSlice_148_149.ComponentByAtoms O 264702
MaterialSlice_148_149.ComponentByAtoms Ar 665

Volume VolumeSlice_148_149
VolumeSlice_148_149.Material MaterialSlice_148_149
VolumeSlice_148_149.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_148_149.Visibility  1
VolumeSlice_148_149.Position 0 0 14850000.000000
VolumeSlice_148_149.Mother World

Material MaterialSlice_149_150
MaterialSlice_149_150.Density 1.698000e-12
MaterialSlice_149_150.ComponentByAtoms H 8
MaterialSlice_149_150.ComponentByAtoms He 208
MaterialSlice_149_150.ComponentByAtoms N 731758
MaterialSlice_149_150.ComponentByAtoms O 267375
MaterialSlice_149_150.ComponentByAtoms Ar 648

Volume VolumeSlice_149_150
VolumeSlice_149_150.Material MaterialSlice_149_150
VolumeSlice_149_150.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_149_150.Visibility  1
VolumeSlice_149_150.Position 0 0 14950000.000000
VolumeSlice_149_150.Mother World

Material MaterialSlice_150_151
MaterialSlice_150_151.Density 1.608000e-12
MaterialSlice_150_151.ComponentByAtoms H 8
MaterialSlice_150_151.ComponentByAtoms He 217
MaterialSlice_150_151.ComponentByAtoms N 728984
MaterialSlice_150_151.ComponentByAtoms O 270157
MaterialSlice_150_151.ComponentByAtoms Ar 632

Volume VolumeSlice_150_151
VolumeSlice_150_151.Material MaterialSlice_150_151
VolumeSlice_150_151.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_150_151.Visibility  1
VolumeSlice_150_151.Position 0 0 15050000.000000
VolumeSlice_150_151.Mother World

Material MaterialSlice_151_152
MaterialSlice_151_152.Density 1.524000e-12
MaterialSlice_151_152.ComponentByAtoms H 8
MaterialSlice_151_152.ComponentByAtoms He 226
MaterialSlice_151_152.ComponentByAtoms N 726372
MaterialSlice_151_152.ComponentByAtoms O 272777
MaterialSlice_151_152.ComponentByAtoms Ar 616

Volume VolumeSlice_151_152
VolumeSlice_151_152.Material MaterialSlice_151_152
VolumeSlice_151_152.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_151_152.Visibility  1
VolumeSlice_151_152.Position 0 0 15150000.000000
VolumeSlice_151_152.Mother World

Material MaterialSlice_152_153
MaterialSlice_152_153.Density 1.446000e-12
MaterialSlice_152_153.ComponentByAtoms H 8
MaterialSlice_152_153.ComponentByAtoms He 234
MaterialSlice_152_153.ComponentByAtoms N 723558
MaterialSlice_152_153.ComponentByAtoms O 275597
MaterialSlice_152_153.ComponentByAtoms Ar 601

Volume VolumeSlice_152_153
VolumeSlice_152_153.Material MaterialSlice_152_153
VolumeSlice_152_153.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_152_153.Visibility  1
VolumeSlice_152_153.Position 0 0 15250000.000000
VolumeSlice_152_153.Mother World

Material MaterialSlice_153_154
MaterialSlice_153_154.Density 1.374000e-12
MaterialSlice_153_154.ComponentByAtoms H 8
MaterialSlice_153_154.ComponentByAtoms He 244
MaterialSlice_153_154.ComponentByAtoms N 720749
MaterialSlice_153_154.ComponentByAtoms O 278411
MaterialSlice_153_154.ComponentByAtoms Ar 586

Volume VolumeSlice_153_154
VolumeSlice_153_154.Material MaterialSlice_153_154
VolumeSlice_153_154.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_153_154.Visibility  1
VolumeSlice_153_154.Position 0 0 15350000.000000
VolumeSlice_153_154.Mother World

Material MaterialSlice_154_155
MaterialSlice_154_155.Density 1.306000e-12
MaterialSlice_154_155.ComponentByAtoms H 8
MaterialSlice_154_155.ComponentByAtoms He 253
MaterialSlice_154_155.ComponentByAtoms N 717950
MaterialSlice_154_155.ComponentByAtoms O 281214
MaterialSlice_154_155.ComponentByAtoms Ar 571

Volume VolumeSlice_154_155
VolumeSlice_154_155.Material MaterialSlice_154_155
VolumeSlice_154_155.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_154_155.Visibility  1
VolumeSlice_154_155.Position 0 0 15450000.000000
VolumeSlice_154_155.Mother World

Material MaterialSlice_155_156
MaterialSlice_155_156.Density 1.243000e-12
MaterialSlice_155_156.ComponentByAtoms H 8
MaterialSlice_155_156.ComponentByAtoms He 263
MaterialSlice_155_156.ComponentByAtoms N 715173
MaterialSlice_155_156.ComponentByAtoms O 283996
MaterialSlice_155_156.ComponentByAtoms Ar 557

Volume VolumeSlice_155_156
VolumeSlice_155_156.Material MaterialSlice_155_156
VolumeSlice_155_156.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_155_156.Visibility  1
VolumeSlice_155_156.Position 0 0 15550000.000000
VolumeSlice_155_156.Mother World

Material MaterialSlice_156_157
MaterialSlice_156_157.Density 1.183000e-12
MaterialSlice_156_157.ComponentByAtoms H 8
MaterialSlice_156_157.ComponentByAtoms He 273
MaterialSlice_156_157.ComponentByAtoms N 712302
MaterialSlice_156_157.ComponentByAtoms O 286870
MaterialSlice_156_157.ComponentByAtoms Ar 544

Volume VolumeSlice_156_157
VolumeSlice_156_157.Material MaterialSlice_156_157
VolumeSlice_156_157.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_156_157.Visibility  1
VolumeSlice_156_157.Position 0 0 15650000.000000
VolumeSlice_156_157.Mother World

Material MaterialSlice_157_158
MaterialSlice_157_158.Density 1.127000e-12
MaterialSlice_157_158.ComponentByAtoms H 8
MaterialSlice_157_158.ComponentByAtoms He 283
MaterialSlice_157_158.ComponentByAtoms N 709540
MaterialSlice_157_158.ComponentByAtoms O 289635
MaterialSlice_157_158.ComponentByAtoms Ar 531

Volume VolumeSlice_157_158
VolumeSlice_157_158.Material MaterialSlice_157_158
VolumeSlice_157_158.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_157_158.Visibility  1
VolumeSlice_157_158.Position 0 0 15750000.000000
VolumeSlice_157_158.Mother World

Material MaterialSlice_158_159
MaterialSlice_158_159.Density 1.075000e-12
MaterialSlice_158_159.ComponentByAtoms H 8
MaterialSlice_158_159.ComponentByAtoms He 293
MaterialSlice_158_159.ComponentByAtoms N 706659
MaterialSlice_158_159.ComponentByAtoms O 292518
MaterialSlice_158_159.ComponentByAtoms Ar 519

Volume VolumeSlice_158_159
VolumeSlice_158_159.Material MaterialSlice_158_159
VolumeSlice_158_159.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_158_159.Visibility  1
VolumeSlice_158_159.Position 0 0 15850000.000000
VolumeSlice_158_159.Mother World

Material MaterialSlice_159_160
MaterialSlice_159_160.Density 1.026000e-12
MaterialSlice_159_160.ComponentByAtoms H 8
MaterialSlice_159_160.ComponentByAtoms He 304
MaterialSlice_159_160.ComponentByAtoms N 703836
MaterialSlice_159_160.ComponentByAtoms O 295343
MaterialSlice_159_160.ComponentByAtoms Ar 507

Volume VolumeSlice_159_160
VolumeSlice_159_160.Material MaterialSlice_159_160
VolumeSlice_159_160.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_159_160.Visibility  1
VolumeSlice_159_160.Position 0 0 15950000.000000
VolumeSlice_159_160.Mother World

Material MaterialSlice_160_161
MaterialSlice_160_161.Density 9.795000e-13
MaterialSlice_160_161.ComponentByAtoms H 8
MaterialSlice_160_161.ComponentByAtoms He 315
MaterialSlice_160_161.ComponentByAtoms N 700970
MaterialSlice_160_161.ComponentByAtoms O 298210
MaterialSlice_160_161.ComponentByAtoms Ar 495

Volume VolumeSlice_160_161
VolumeSlice_160_161.Material MaterialSlice_160_161
VolumeSlice_160_161.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_160_161.Visibility  1
VolumeSlice_160_161.Position 0 0 16050000.000000
VolumeSlice_160_161.Mother World

Material MaterialSlice_161_162
MaterialSlice_161_162.Density 9.358000e-13
MaterialSlice_161_162.ComponentByAtoms H 9
MaterialSlice_161_162.ComponentByAtoms He 326
MaterialSlice_161_162.ComponentByAtoms N 697918
MaterialSlice_161_162.ComponentByAtoms O 301262
MaterialSlice_161_162.ComponentByAtoms Ar 483

Volume VolumeSlice_161_162
VolumeSlice_161_162.Material MaterialSlice_161_162
VolumeSlice_161_162.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_161_162.Visibility  1
VolumeSlice_161_162.Position 0 0 16150000.000000
VolumeSlice_161_162.Mother World

Material MaterialSlice_162_163
MaterialSlice_162_163.Density 8.948000e-13
MaterialSlice_162_163.ComponentByAtoms H 9
MaterialSlice_162_163.ComponentByAtoms He 337
MaterialSlice_162_163.ComponentByAtoms N 695008
MaterialSlice_162_163.ComponentByAtoms O 304171
MaterialSlice_162_163.ComponentByAtoms Ar 472

Volume VolumeSlice_162_163
VolumeSlice_162_163.Material MaterialSlice_162_163
VolumeSlice_162_163.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_162_163.Visibility  1
VolumeSlice_162_163.Position 0 0 16250000.000000
VolumeSlice_162_163.Mother World

Material MaterialSlice_163_164
MaterialSlice_163_164.Density 8.561000e-13
MaterialSlice_163_164.ComponentByAtoms H 9
MaterialSlice_163_164.ComponentByAtoms He 349
MaterialSlice_163_164.ComponentByAtoms N 692093
MaterialSlice_163_164.ComponentByAtoms O 307085
MaterialSlice_163_164.ComponentByAtoms Ar 461

Volume VolumeSlice_163_164
VolumeSlice_163_164.Material MaterialSlice_163_164
VolumeSlice_163_164.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_163_164.Visibility  1
VolumeSlice_163_164.Position 0 0 16350000.000000
VolumeSlice_163_164.Mother World

Material MaterialSlice_164_165
MaterialSlice_164_165.Density 8.195000e-13
MaterialSlice_164_165.ComponentByAtoms H 9
MaterialSlice_164_165.ComponentByAtoms He 361
MaterialSlice_164_165.ComponentByAtoms N 689068
MaterialSlice_164_165.ComponentByAtoms O 310109
MaterialSlice_164_165.ComponentByAtoms Ar 451

Volume VolumeSlice_164_165
VolumeSlice_164_165.Material MaterialSlice_164_165
VolumeSlice_164_165.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_164_165.Visibility  1
VolumeSlice_164_165.Position 0 0 16450000.000000
VolumeSlice_164_165.Mother World

Material MaterialSlice_165_166
MaterialSlice_165_166.Density 7.850000e-13
MaterialSlice_165_166.ComponentByAtoms H 9
MaterialSlice_165_166.ComponentByAtoms He 373
MaterialSlice_165_166.ComponentByAtoms N 686162
MaterialSlice_165_166.ComponentByAtoms O 313013
MaterialSlice_165_166.ComponentByAtoms Ar 441

Volume VolumeSlice_165_166
VolumeSlice_165_166.Material MaterialSlice_165_166
VolumeSlice_165_166.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_165_166.Visibility  1
VolumeSlice_165_166.Position 0 0 16550000.000000
VolumeSlice_165_166.Mother World

Material MaterialSlice_166_167
MaterialSlice_166_167.Density 7.523000e-13
MaterialSlice_166_167.ComponentByAtoms H 9
MaterialSlice_166_167.ComponentByAtoms He 385
MaterialSlice_166_167.ComponentByAtoms N 683229
MaterialSlice_166_167.ComponentByAtoms O 315944
MaterialSlice_166_167.ComponentByAtoms Ar 431

Volume VolumeSlice_166_167
VolumeSlice_166_167.Material MaterialSlice_166_167
VolumeSlice_166_167.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_166_167.Visibility  1
VolumeSlice_166_167.Position 0 0 16650000.000000
VolumeSlice_166_167.Mother World

Material MaterialSlice_167_168
MaterialSlice_167_168.Density 7.214000e-13
MaterialSlice_167_168.ComponentByAtoms H 9
MaterialSlice_167_168.ComponentByAtoms He 398
MaterialSlice_167_168.ComponentByAtoms N 680333
MaterialSlice_167_168.ComponentByAtoms O 318836
MaterialSlice_167_168.ComponentByAtoms Ar 421

Volume VolumeSlice_167_168
VolumeSlice_167_168.Material MaterialSlice_167_168
VolumeSlice_167_168.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_167_168.Visibility  1
VolumeSlice_167_168.Position 0 0 16750000.000000
VolumeSlice_167_168.Mother World

Material MaterialSlice_168_169
MaterialSlice_168_169.Density 6.921000e-13
MaterialSlice_168_169.ComponentByAtoms H 9
MaterialSlice_168_169.ComponentByAtoms He 411
MaterialSlice_168_169.ComponentByAtoms N 677295
MaterialSlice_168_169.ComponentByAtoms O 321871
MaterialSlice_168_169.ComponentByAtoms Ar 412

Volume VolumeSlice_168_169
VolumeSlice_168_169.Material MaterialSlice_168_169
VolumeSlice_168_169.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_168_169.Visibility  1
VolumeSlice_168_169.Position 0 0 16850000.000000
VolumeSlice_168_169.Mother World

Material MaterialSlice_169_170
MaterialSlice_169_170.Density 6.643000e-13
MaterialSlice_169_170.ComponentByAtoms H 9
MaterialSlice_169_170.ComponentByAtoms He 424
MaterialSlice_169_170.ComponentByAtoms N 674330
MaterialSlice_169_170.ComponentByAtoms O 324831
MaterialSlice_169_170.ComponentByAtoms Ar 403

Volume VolumeSlice_169_170
VolumeSlice_169_170.Material MaterialSlice_169_170
VolumeSlice_169_170.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_169_170.Visibility  1
VolumeSlice_169_170.Position 0 0 16950000.000000
VolumeSlice_169_170.Mother World

Material MaterialSlice_170_171
MaterialSlice_170_171.Density 6.380000e-13
MaterialSlice_170_171.ComponentByAtoms H 10
MaterialSlice_170_171.ComponentByAtoms He 437
MaterialSlice_170_171.ComponentByAtoms N 671332
MaterialSlice_170_171.ComponentByAtoms O 327825
MaterialSlice_170_171.ComponentByAtoms Ar 394

Volume VolumeSlice_170_171
VolumeSlice_170_171.Material MaterialSlice_170_171
VolumeSlice_170_171.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_170_171.Visibility  1
VolumeSlice_170_171.Position 0 0 17050000.000000
VolumeSlice_170_171.Mother World

Material MaterialSlice_171_172
MaterialSlice_171_172.Density 6.130000e-13
MaterialSlice_171_172.ComponentByAtoms H 10
MaterialSlice_171_172.ComponentByAtoms He 451
MaterialSlice_171_172.ComponentByAtoms N 668327
MaterialSlice_171_172.ComponentByAtoms O 330825
MaterialSlice_171_172.ComponentByAtoms Ar 385

Volume VolumeSlice_171_172
VolumeSlice_171_172.Material MaterialSlice_171_172
VolumeSlice_171_172.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_171_172.Visibility  1
VolumeSlice_171_172.Position 0 0 17150000.000000
VolumeSlice_171_172.Mother World

Material MaterialSlice_172_173
MaterialSlice_172_173.Density 5.892000e-13
MaterialSlice_172_173.ComponentByAtoms H 10
MaterialSlice_172_173.ComponentByAtoms He 465
MaterialSlice_172_173.ComponentByAtoms N 665308
MaterialSlice_172_173.ComponentByAtoms O 333838
MaterialSlice_172_173.ComponentByAtoms Ar 377

Volume VolumeSlice_172_173
VolumeSlice_172_173.Material MaterialSlice_172_173
VolumeSlice_172_173.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_172_173.Visibility  1
VolumeSlice_172_173.Position 0 0 17250000.000000
VolumeSlice_172_173.Mother World

Material MaterialSlice_173_174
MaterialSlice_173_174.Density 5.666000e-13
MaterialSlice_173_174.ComponentByAtoms H 10
MaterialSlice_173_174.ComponentByAtoms He 479
MaterialSlice_173_174.ComponentByAtoms N 662305
MaterialSlice_173_174.ComponentByAtoms O 336835
MaterialSlice_173_174.ComponentByAtoms Ar 369

Volume VolumeSlice_173_174
VolumeSlice_173_174.Material MaterialSlice_173_174
VolumeSlice_173_174.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_173_174.Visibility  1
VolumeSlice_173_174.Position 0 0 17350000.000000
VolumeSlice_173_174.Mother World

Material MaterialSlice_174_175
MaterialSlice_174_175.Density 5.451000e-13
MaterialSlice_174_175.ComponentByAtoms H 10
MaterialSlice_174_175.ComponentByAtoms He 494
MaterialSlice_174_175.ComponentByAtoms N 659278
MaterialSlice_174_175.ComponentByAtoms O 339855
MaterialSlice_174_175.ComponentByAtoms Ar 361

Volume VolumeSlice_174_175
VolumeSlice_174_175.Material MaterialSlice_174_175
VolumeSlice_174_175.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_174_175.Visibility  1
VolumeSlice_174_175.Position 0 0 17450000.000000
VolumeSlice_174_175.Mother World

Material MaterialSlice_175_176
MaterialSlice_175_176.Density 5.247000e-13
MaterialSlice_175_176.ComponentByAtoms H 10
MaterialSlice_175_176.ComponentByAtoms He 509
MaterialSlice_175_176.ComponentByAtoms N 656231
MaterialSlice_175_176.ComponentByAtoms O 342894
MaterialSlice_175_176.ComponentByAtoms Ar 353

Volume VolumeSlice_175_176
VolumeSlice_175_176.Material MaterialSlice_175_176
VolumeSlice_175_176.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_175_176.Visibility  1
VolumeSlice_175_176.Position 0 0 17550000.000000
VolumeSlice_175_176.Mother World

Material MaterialSlice_176_177
MaterialSlice_176_177.Density 5.052000e-13
MaterialSlice_176_177.ComponentByAtoms H 11
MaterialSlice_176_177.ComponentByAtoms He 524
MaterialSlice_176_177.ComponentByAtoms N 653237
MaterialSlice_176_177.ComponentByAtoms O 345881
MaterialSlice_176_177.ComponentByAtoms Ar 345

Volume VolumeSlice_176_177
VolumeSlice_176_177.Material MaterialSlice_176_177
VolumeSlice_176_177.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_176_177.Visibility  1
VolumeSlice_176_177.Position 0 0 17650000.000000
VolumeSlice_176_177.Mother World

Material MaterialSlice_177_178
MaterialSlice_177_178.Density 4.866000e-13
MaterialSlice_177_178.ComponentByAtoms H 11
MaterialSlice_177_178.ComponentByAtoms He 539
MaterialSlice_177_178.ComponentByAtoms N 650157
MaterialSlice_177_178.ComponentByAtoms O 348953
MaterialSlice_177_178.ComponentByAtoms Ar 338

Volume VolumeSlice_177_178
VolumeSlice_177_178.Material MaterialSlice_177_178
VolumeSlice_177_178.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_177_178.Visibility  1
VolumeSlice_177_178.Position 0 0 17750000.000000
VolumeSlice_177_178.Mother World

Material MaterialSlice_178_179
MaterialSlice_178_179.Density 4.689000e-13
MaterialSlice_178_179.ComponentByAtoms H 11
MaterialSlice_178_179.ComponentByAtoms He 555
MaterialSlice_178_179.ComponentByAtoms N 647107
MaterialSlice_178_179.ComponentByAtoms O 351994
MaterialSlice_178_179.ComponentByAtoms Ar 331

Volume VolumeSlice_178_179
VolumeSlice_178_179.Material MaterialSlice_178_179
VolumeSlice_178_179.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_178_179.Visibility  1
VolumeSlice_178_179.Position 0 0 17850000.000000
VolumeSlice_178_179.Mother World

Material MaterialSlice_179_180
MaterialSlice_179_180.Density 4.521000e-13
MaterialSlice_179_180.ComponentByAtoms H 11
MaterialSlice_179_180.ComponentByAtoms He 571
MaterialSlice_179_180.ComponentByAtoms N 644081
MaterialSlice_179_180.ComponentByAtoms O 355011
MaterialSlice_179_180.ComponentByAtoms Ar 323

Volume VolumeSlice_179_180
VolumeSlice_179_180.Material MaterialSlice_179_180
VolumeSlice_179_180.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_179_180.Visibility  1
VolumeSlice_179_180.Position 0 0 17950000.000000
VolumeSlice_179_180.Mother World

Material MaterialSlice_180_181
MaterialSlice_180_181.Density 4.359000e-13
MaterialSlice_180_181.ComponentByAtoms H 11
MaterialSlice_180_181.ComponentByAtoms He 587
MaterialSlice_180_181.ComponentByAtoms N 640979
MaterialSlice_180_181.ComponentByAtoms O 358104
MaterialSlice_180_181.ComponentByAtoms Ar 317

Volume VolumeSlice_180_181
VolumeSlice_180_181.Material MaterialSlice_180_181
VolumeSlice_180_181.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_180_181.Visibility  1
VolumeSlice_180_181.Position 0 0 18050000.000000
VolumeSlice_180_181.Mother World

Material MaterialSlice_181_182
MaterialSlice_181_182.Density 4.205000e-13
MaterialSlice_181_182.ComponentByAtoms H 12
MaterialSlice_181_182.ComponentByAtoms He 604
MaterialSlice_181_182.ComponentByAtoms N 637936
MaterialSlice_181_182.ComponentByAtoms O 361135
MaterialSlice_181_182.ComponentByAtoms Ar 310

Volume VolumeSlice_181_182
VolumeSlice_181_182.Material MaterialSlice_181_182
VolumeSlice_181_182.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_181_182.Visibility  1
VolumeSlice_181_182.Position 0 0 18150000.000000
VolumeSlice_181_182.Mother World

Material MaterialSlice_182_183
MaterialSlice_182_183.Density 4.058000e-13
MaterialSlice_182_183.ComponentByAtoms H 12
MaterialSlice_182_183.ComponentByAtoms He 621
MaterialSlice_182_183.ComponentByAtoms N 634855
MaterialSlice_182_183.ComponentByAtoms O 364206
MaterialSlice_182_183.ComponentByAtoms Ar 303

Volume VolumeSlice_182_183
VolumeSlice_182_183.Material MaterialSlice_182_183
VolumeSlice_182_183.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_182_183.Visibility  1
VolumeSlice_182_183.Position 0 0 18250000.000000
VolumeSlice_182_183.Mother World

Material MaterialSlice_183_184
MaterialSlice_183_184.Density 3.918000e-13
MaterialSlice_183_184.ComponentByAtoms H 12
MaterialSlice_183_184.ComponentByAtoms He 639
MaterialSlice_183_184.ComponentByAtoms N 631822
MaterialSlice_183_184.ComponentByAtoms O 367227
MaterialSlice_183_184.ComponentByAtoms Ar 297

Volume VolumeSlice_183_184
VolumeSlice_183_184.Material MaterialSlice_183_184
VolumeSlice_183_184.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_183_184.Visibility  1
VolumeSlice_183_184.Position 0 0 18350000.000000
VolumeSlice_183_184.Mother World

Material MaterialSlice_184_185
MaterialSlice_184_185.Density 3.783000e-13
MaterialSlice_184_185.ComponentByAtoms H 12
MaterialSlice_184_185.ComponentByAtoms He 657
MaterialSlice_184_185.ComponentByAtoms N 628699
MaterialSlice_184_185.ComponentByAtoms O 370338
MaterialSlice_184_185.ComponentByAtoms Ar 291

Volume VolumeSlice_184_185
VolumeSlice_184_185.Material MaterialSlice_184_185
VolumeSlice_184_185.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_184_185.Visibility  1
VolumeSlice_184_185.Position 0 0 18450000.000000
VolumeSlice_184_185.Mother World

Material MaterialSlice_185_186
MaterialSlice_185_186.Density 3.654000e-13
MaterialSlice_185_186.ComponentByAtoms H 12
MaterialSlice_185_186.ComponentByAtoms He 675
MaterialSlice_185_186.ComponentByAtoms N 625660
MaterialSlice_185_186.ComponentByAtoms O 373366
MaterialSlice_185_186.ComponentByAtoms Ar 285

Volume VolumeSlice_185_186
VolumeSlice_185_186.Material MaterialSlice_185_186
VolumeSlice_185_186.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_185_186.Visibility  1
VolumeSlice_185_186.Position 0 0 18550000.000000
VolumeSlice_185_186.Mother World

Material MaterialSlice_186_187
MaterialSlice_186_187.Density 3.531000e-13
MaterialSlice_186_187.ComponentByAtoms H 13
MaterialSlice_186_187.ComponentByAtoms He 693
MaterialSlice_186_187.ComponentByAtoms N 622554
MaterialSlice_186_187.ComponentByAtoms O 376459
MaterialSlice_186_187.ComponentByAtoms Ar 279

Volume VolumeSlice_186_187
VolumeSlice_186_187.Material MaterialSlice_186_187
VolumeSlice_186_187.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_186_187.Visibility  1
VolumeSlice_186_187.Position 0 0 18650000.000000
VolumeSlice_186_187.Mother World

Material MaterialSlice_187_188
MaterialSlice_187_188.Density 3.413000e-13
MaterialSlice_187_188.ComponentByAtoms H 13
MaterialSlice_187_188.ComponentByAtoms He 712
MaterialSlice_187_188.ComponentByAtoms N 619425
MaterialSlice_187_188.ComponentByAtoms O 379575
MaterialSlice_187_188.ComponentByAtoms Ar 273

Volume VolumeSlice_187_188
VolumeSlice_187_188.Material MaterialSlice_187_188
VolumeSlice_187_188.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_187_188.Visibility  1
VolumeSlice_187_188.Position 0 0 18750000.000000
VolumeSlice_187_188.Mother World

Material MaterialSlice_188_189
MaterialSlice_188_189.Density 3.300000e-13
MaterialSlice_188_189.ComponentByAtoms H 13
MaterialSlice_188_189.ComponentByAtoms He 731
MaterialSlice_188_189.ComponentByAtoms N 616342
MaterialSlice_188_189.ComponentByAtoms O 382644
MaterialSlice_188_189.ComponentByAtoms Ar 267

Volume VolumeSlice_188_189
VolumeSlice_188_189.Material MaterialSlice_188_189
VolumeSlice_188_189.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_188_189.Visibility  1
VolumeSlice_188_189.Position 0 0 18850000.000000
VolumeSlice_188_189.Mother World

Material MaterialSlice_189_190
MaterialSlice_189_190.Density 3.191000e-13
MaterialSlice_189_190.ComponentByAtoms H 13
MaterialSlice_189_190.ComponentByAtoms He 750
MaterialSlice_189_190.ComponentByAtoms N 613274
MaterialSlice_189_190.ComponentByAtoms O 385698
MaterialSlice_189_190.ComponentByAtoms Ar 262

Volume VolumeSlice_189_190
VolumeSlice_189_190.Material MaterialSlice_189_190
VolumeSlice_189_190.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_189_190.Visibility  1
VolumeSlice_189_190.Position 0 0 18950000.000000
VolumeSlice_189_190.Mother World

Material MaterialSlice_190_191
MaterialSlice_190_191.Density 3.087000e-13
MaterialSlice_190_191.ComponentByAtoms H 14
MaterialSlice_190_191.ComponentByAtoms He 770
MaterialSlice_190_191.ComponentByAtoms N 610125
MaterialSlice_190_191.ComponentByAtoms O 388832
MaterialSlice_190_191.ComponentByAtoms Ar 256

Volume VolumeSlice_190_191
VolumeSlice_190_191.Material MaterialSlice_190_191
VolumeSlice_190_191.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_190_191.Visibility  1
VolumeSlice_190_191.Position 0 0 19050000.000000
VolumeSlice_190_191.Mother World

Material MaterialSlice_191_192
MaterialSlice_191_192.Density 2.987000e-13
MaterialSlice_191_192.ComponentByAtoms H 14
MaterialSlice_191_192.ComponentByAtoms He 790
MaterialSlice_191_192.ComponentByAtoms N 607068
MaterialSlice_191_192.ComponentByAtoms O 391874
MaterialSlice_191_192.ComponentByAtoms Ar 251

Volume VolumeSlice_191_192
VolumeSlice_191_192.Material MaterialSlice_191_192
VolumeSlice_191_192.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_191_192.Visibility  1
VolumeSlice_191_192.Position 0 0 19150000.000000
VolumeSlice_191_192.Mother World

Material MaterialSlice_192_193
MaterialSlice_192_193.Density 2.891000e-13
MaterialSlice_192_193.ComponentByAtoms H 14
MaterialSlice_192_193.ComponentByAtoms He 811
MaterialSlice_192_193.ComponentByAtoms N 603945
MaterialSlice_192_193.ComponentByAtoms O 394982
MaterialSlice_192_193.ComponentByAtoms Ar 246

Volume VolumeSlice_192_193
VolumeSlice_192_193.Material MaterialSlice_192_193
VolumeSlice_192_193.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_192_193.Visibility  1
VolumeSlice_192_193.Position 0 0 19250000.000000
VolumeSlice_192_193.Mother World

Material MaterialSlice_193_194
MaterialSlice_193_194.Density 2.799000e-13
MaterialSlice_193_194.ComponentByAtoms H 15
MaterialSlice_193_194.ComponentByAtoms He 832
MaterialSlice_193_194.ComponentByAtoms N 600816
MaterialSlice_193_194.ComponentByAtoms O 398094
MaterialSlice_193_194.ComponentByAtoms Ar 241

Volume VolumeSlice_193_194
VolumeSlice_193_194.Material MaterialSlice_193_194
VolumeSlice_193_194.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_193_194.Visibility  1
VolumeSlice_193_194.Position 0 0 19350000.000000
VolumeSlice_193_194.Mother World

Material MaterialSlice_194_195
MaterialSlice_194_195.Density 2.711000e-13
MaterialSlice_194_195.ComponentByAtoms H 15
MaterialSlice_194_195.ComponentByAtoms He 853
MaterialSlice_194_195.ComponentByAtoms N 597731
MaterialSlice_194_195.ComponentByAtoms O 401162
MaterialSlice_194_195.ComponentByAtoms Ar 236

Volume VolumeSlice_194_195
VolumeSlice_194_195.Material MaterialSlice_194_195
VolumeSlice_194_195.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_194_195.Visibility  1
VolumeSlice_194_195.Position 0 0 19450000.000000
VolumeSlice_194_195.Mother World

Material MaterialSlice_195_196
MaterialSlice_195_196.Density 2.626000e-13
MaterialSlice_195_196.ComponentByAtoms H 15
MaterialSlice_195_196.ComponentByAtoms He 875
MaterialSlice_195_196.ComponentByAtoms N 594572
MaterialSlice_195_196.ComponentByAtoms O 404304
MaterialSlice_195_196.ComponentByAtoms Ar 231

Volume VolumeSlice_195_196
VolumeSlice_195_196.Material MaterialSlice_195_196
VolumeSlice_195_196.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_195_196.Visibility  1
VolumeSlice_195_196.Position 0 0 19550000.000000
VolumeSlice_195_196.Mother World

Material MaterialSlice_196_197
MaterialSlice_196_197.Density 2.544000e-13
MaterialSlice_196_197.ComponentByAtoms H 16
MaterialSlice_196_197.ComponentByAtoms He 897
MaterialSlice_196_197.ComponentByAtoms N 591476
MaterialSlice_196_197.ComponentByAtoms O 407382
MaterialSlice_196_197.ComponentByAtoms Ar 226

Volume VolumeSlice_196_197
VolumeSlice_196_197.Material MaterialSlice_196_197
VolumeSlice_196_197.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_196_197.Visibility  1
VolumeSlice_196_197.Position 0 0 19650000.000000
VolumeSlice_196_197.Mother World

Material MaterialSlice_197_198
MaterialSlice_197_198.Density 2.466000e-13
MaterialSlice_197_198.ComponentByAtoms H 16
MaterialSlice_197_198.ComponentByAtoms He 920
MaterialSlice_197_198.ComponentByAtoms N 588370
MaterialSlice_197_198.ComponentByAtoms O 410470
MaterialSlice_197_198.ComponentByAtoms Ar 222

Volume VolumeSlice_197_198
VolumeSlice_197_198.Material MaterialSlice_197_198
VolumeSlice_197_198.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_197_198.Visibility  1
VolumeSlice_197_198.Position 0 0 19750000.000000
VolumeSlice_197_198.Mother World

Material MaterialSlice_198_199
MaterialSlice_198_199.Density 2.390000e-13
MaterialSlice_198_199.ComponentByAtoms H 16
MaterialSlice_198_199.ComponentByAtoms He 943
MaterialSlice_198_199.ComponentByAtoms N 585169
MaterialSlice_198_199.ComponentByAtoms O 413652
MaterialSlice_198_199.ComponentByAtoms Ar 217

Volume VolumeSlice_198_199
VolumeSlice_198_199.Material MaterialSlice_198_199
VolumeSlice_198_199.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_198_199.Visibility  1
VolumeSlice_198_199.Position 0 0 19850000.000000
VolumeSlice_198_199.Mother World

Material MaterialSlice_199_200
MaterialSlice_199_200.Density 2.317000e-13
MaterialSlice_199_200.ComponentByAtoms H 17
MaterialSlice_199_200.ComponentByAtoms He 966
MaterialSlice_199_200.ComponentByAtoms N 582112
MaterialSlice_199_200.ComponentByAtoms O 416690
MaterialSlice_199_200.ComponentByAtoms Ar 213

Volume VolumeSlice_199_200
VolumeSlice_199_200.Material MaterialSlice_199_200
VolumeSlice_199_200.Shape BOX 51200000.000000 51200000.000000 50000.000000
VolumeSlice_199_200.Visibility  1
VolumeSlice_199_200.Position 0 0 19950000.000000
VolumeSlice_199_200.Mother World

Volume SphereBox_1
SphereBox_1.Material MaterialSlice_33_34
SphereBox_1.Shape BOX 100000.000000 100000.000000 50000.000000
SphereBox_1.Visibility 1
SphereBox_1.Mother VolumeSlice_33_34
Volume SphereBox_0
SphereBox_0.Material MaterialSlice_33_34
SphereBox_0.Shape BOX 49999.500000 49999.500000 49999.000000
SphereBox_0.Visibility 1
SphereBox_0.Copy SphereBox_pp_0
SphereBox_pp_0.Position 49999.500000 49999.500000 0.0
SphereBox_pp_0.Mother SphereBox_1
SphereBox_0.Copy SphereBox_pm_0
SphereBox_pm_0.Position 49999.500000 -49999.500000 0.0
SphereBox_pm_0.Mother SphereBox_1
SphereBox_0.Copy SphereBox_mp_0
SphereBox_mp_0.Position -49999.500000 49999.500000 0.0
SphereBox_mp_0.Mother SphereBox_1
SphereBox_0.Copy SphereBox_mm_0
SphereBox_mm_0.Position -49999.500000 -49999.500000 0.0
SphereBox_mm_0.Mother SphereBox_1

Volume TestSphere
TestSphere.Material MaterialSlice_33_34
TestSphere.Shape Sphere 0 49998
TestSphere.Visibility 1
TestSphere.Position 0 0 0
TestSphere.Mother SphereBox_0

