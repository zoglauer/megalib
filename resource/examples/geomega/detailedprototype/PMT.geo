// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
//Include Materials.geo



// The Mother Volume of the Detector Configuration:
Volume PMT_Volume
PMT_Volume.Material Air
PMT_Volume.Visibility 0
PMT_Volume.Virtual 1
PMT_Volume.Shape BRIK 7.3 2.75 3.75
// NEEDS THIS LINE TO VIEW ALONE:
//PMT_Volume.Mother 0


// The Consisting Parts:

// Aluminium
Volume PMT_Aluminium
PMT_Aluminium.Material Aluminium
PMT_Aluminium.Shape BRIK 7.3 1.166 3.75
PMT_Aluminium.Position 0.0 -1.584 0.0
PMT_Aluminium.Visibility 1
PMT_Aluminium.Color 2
PMT_Aluminium.Mother PMT_Volume

// Glass
Volume PMT_Glass
PMT_Glass.Material Glass
PMT_Glass.Shape BRIK 7.3 1.2595 3.75
PMT_Glass.Position 0.0 0.8415 0.0
PMT_Glass.Visibility 1
PMT_Glass.Color 1
PMT_Glass.Mother PMT_Volume

// Steel
Volume PMT_Steel
PMT_Steel.Material Steel
PMT_Steel.Shape BRIK 7.3 0.0935 3.75
PMT_Steel.Position 0.0 2.1945 0.0
PMT_Steel.Visibility 1
PMT_Steel.Color 1
PMT_Steel.Mother PMT_Volume

// Plastic
Volume PMT_Plastic
PMT_Plastic.Material PMTPlastic
PMT_Plastic.Shape BRIK 7.3 0.22 3.75
PMT_Plastic.Position 0.0 2.508 0.0
PMT_Plastic.Visibility 1
PMT_Plastic.Color 1
PMT_Plastic.Mother PMT_Volume

// Tin
Volume PMT_Tin
PMT_Tin.Material Tin
PMT_Tin.Shape BRIK 7.3 0.011 3.75
PMT_Tin.Position 0.0 2.739 0.0
PMT_Tin.Visibility 1
PMT_Tin.Color 1
PMT_Tin.Mother PMT_Volume