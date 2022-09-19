###This file defines a single PMT. It's modeled as an Aluminum tube (0.2cm thick, 3.4cm radius) filled with "pmtmix" with a aluminum 1cm thick cap at the top.
###This was not edited from MassModel_0.9 -> MassModel_1.0. I'm not sure the original author, but Carolyn checked all dimensions

Include Intro.geo


Volume SinglePMT
SinglePMT.Visibility 1
SinglePMT.Material Vacuum
SinglePMT.Shape TUBS 0 3.4 8.25 0 360 
SinglePMT.Virtual true


Volume PMTHousing
PMTHousing.Visibility 1
PMTHousing.Material al6061
PMTHousing.Shape TUBS 0.0 2.8 7.25 0 360 
PMTHousing.Position 0 0 -0.5
PMTHousing.Mother SinglePMT

# Acording to SPI it's filled with "PMTMIX"
Volume PMTInside
PMTInside.Visibility 1
PMTInside.Material pmtmix
PMTInside.Shape TUBS 0.0 2.6 7.25 0 360 
PMTInside.Position 0 0 0.0
PMTInside.Mother PMTHousing


# Cap: Aluminium for now...
Volume PMTCap
PMTCap.Visibility 1
PMTCap.Material al6061
PMTCap.Shape TUBS 0.0 3.4 0.5 0 360 
PMTCap.Position 0 0 7.25
PMTCap.Mother SinglePMT



