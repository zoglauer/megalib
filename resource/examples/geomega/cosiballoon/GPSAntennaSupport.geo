#These are the three beams that support the dGPS system. They should not be included in the model for the majority of the calibrations, as they probably won't be added until later...

Include Intro.geo
Include Gondola.geo


Volume GPSAntennaSupport
GPSAntennaSupport.Visibility 0
GPSAntennaSupport.Virtual True
GPSAntennaSupport.Material Vacuum
GPSAntennaSupport.Shape BRIK 150 150 10

Volume GPS_LongBeam
GPS_LongBeam.Visibility 1
GPS_LongBeam.Material al6061
GPS_LongBeam.Shape BRIK {274.3/2} GondolaBeam_HalfWidth GondolaBeam_HalfWidth

Volume GPS_LongBeamInside
GPS_LongBeamInside.Visibility 0
GPS_LongBeamInside.Material Vacuum
GPS_LongBeamInside.Shape BRIK {274.3/2} {GondolaBeam_HalfWidth-Beam_Thickness} {GondolaBeam_HalfWidth-Beam_Thickness}
GPS_LongBeamInside.Mother GPS_LongBeam
GPS_LongBeamInside.Position 0 0 0

#This boom sits 124.6 cm measured from the back end of the top layer gondola beam
Volume GPS_ShortBeam
GPS_ShortBeam.Visibility 1
GPS_ShortBeam.Material al6061
GPS_ShortBeam.Shape BRIK {3.8/2} {264.2/2} {3.8/2}
GPS_ShortBeam.Mother GPSAntennaSupport
GPS_ShortBeam.Position {-274.3/2+3.8/2} 0 GondolaBeam_HalfWidth

Volume GPS_ShortBeamInside
GPS_ShortBeamInside.Visibility 0
GPS_ShortBeamInside.Material Vacuum
GPS_ShortBeamInside.Shape BRIK {3.8/2-Beam_Thickness} {264.2/2} {3.8/2-Beam_Thickness}
GPS_ShortBeamInside.Mother GPS_ShortBeam
GPS_ShortBeamInside.Position 0 0 0

For Y 2 {-GondolaHorizontalBeam_HalfLength+GondolaBeam_HalfWidth} {GondolaHorizontalBeam_HalfLength*2 - GondolaBeam_HalfWidth*2}
  GPS_LongBeam.Copy GPS_LongBeam_%Y
  GPS_LongBeam_%Y.Position 0 $Y 0
  GPS_LongBeam_%Y.Mother GPSAntennaSupport
Done




