This example simulates the activation of a set of materials for the given orbit and summarizes the activation in a report pdf.

(1) Create your orbit files

The exmaple files are for a NuSTARlike orbit.

(2) Create a materials list file called MaterialsList.txt

The format is one material by line

Name
Density
Amount of materials in the material as in a for by atoms, m for by mass
Element 1
Amount 1
Element 2
Amount 2
.
.
.

For example:
CsI 4.5 a Cs 1 I 1
Si 2.33 a Si 1
Alu6061 2.6989 m Si 0.0060 Fe 0.0070 Cu 0.003 Mn 0.0015 Mg 0.001 Cr 0.002 Zn 0.0025 Ti 0.0015 Al 0.9665

(3) Run the geometruy and sim file creator

python3 CreateSourceFiles.py

The created geometry is a sphere with 1 meter radius and 1 cm thickness.
For normalization prurposes, the density of the material is set to 1 g/cm3

The cosima source files correspond to the 3 steps in activation simulation:
- hadron interactions
- build-uop after 1 year in orbit
- decay

(4) Launch the simulation

bash launch.sh

(5) Create a suimmary pdf file

python3 PlotActivations.py















