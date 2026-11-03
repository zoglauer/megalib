# Instructions for AI agents working on MEGAlib

MEGAlib is a C++17 library and a set of programs for simulating, calibrating, and analyzing Compton telescope data (Geant4 and ROOT based). This file is the entry point for AI agents. The details are in two documents at the repository root, which you must read before changing code or tests:

* `CodingConventions.md`: naming, comments, formatting, error handling, filesystem safety, the ChangeLog rule.
* `TestingConventions.md`: unit tests (`UT*`), end-to-end tests (`ET*`), the test driver, reference campaigns.

## Working agreement

* Never commit, amend, or push - even if asked. Leave new files untracked or staged for the user to review.
* Before you write or edit production code, show the proposed change as a diff and wait for approval. New unit tests do not need this approval. Prepare the change in a scratch copy, build and test it there, and apply it only after the user says so. Always list the full path of the diff for review.
* Fix one problem at a time, make the smallest change that fixes it, and do not mix bug fixes with restyling, reordering, or renaming. Refuse to mix them and split the work.
* Say explicitly which files you will delete before you delete anything.
* Every bug fix gets a regression test that fails without the fix. A fix that can change results gets an entry in `doc/ChangeLog.txt` (see `CodingConventions.md`).
* Report outcomes as they are: a failing test is reported with its output, a skipped step is reported as skipped, and what you did not run or check is said so.
* Before adding a new function, check if a similar one existing in MEGAlib. If yes, use that instead.


## Layout

* `src/<module>/inc`, `src/<module>/src`, `src/<module>/unittests`: headers, sources, and unit tests of each module (`global`, `geomega`, `cosima`, `revan`, `mimrec`, `response`, `sivan`, `endtoend`, ...). Headers are linked into `include/`, and programs are built into `bin/` and libraries into `lib/`.
* `src/endtoend/framework`: the library `libEndToEnd` with the `MEndToEndTest` base class, the report and reference classes, the reference campaign, and the test driver (`MTestDriver`), the programs `testdrive` and `testdrivercalibration`, and the unit tests of the framework. `src/endtoend/tests`: the end-to-end tests (`ET*`), which link the library.
* `resource/endtoend/<Test>.references.tsv`: reference tables of the statistical end-to-end tests.
* `external/`: ROOT and Geant4 as installed by `setup.sh`. Do not edit anything there.
* The repository root contains many loose data and log files. They are not part of the project: do not add them, edit them, or rely on them.

## Code layout

The gneeral MEGAlib data flow data is: geometry (`.geo.setup`) and source (`.source`) → `cosima` simulates → `.sim` file → `revan` reconstructs events → `.tra` file → `mimrec` makes images, spectra, ARM, and polarization results. Class names start with `M` followed by a module prefix. Look for a class in the module that matches its prefix first.

* `src/global/misc`: the foundation that everything uses. `MString` (use its `Is<T>()`, `IsNumber()`, `Tokenize()` before writing parsers), `MFile`, `MSystem`, `MProcessManager` (child processes), `MStreams` (`mout`, `merr`, `show`), `MTime`, `MVector`, `MRotation`, `MXml*`, the settings base classes (`MSettings*`), the physical events (`MPhysicalEvent`, `MComptonEvent`, `MPhotoEvent`, `MPairEvent`, ...), the readers of `.tra` files (`MFileEventsTra`), response matrices (`MResponseMatrix*`), binners, and the unit-test base class `MUnitTest`.
* `src/global/gui`: ROOT GUI helper classes (`MGUI*`). `src/global/neuralnet`: a small neural network library (`MNeuralNetwork*`).
* `src/geomega` (`MD*`): detector geometry. `MDGeometry` and `MDGeometryQuest` read a `.geo.setup`; volumes, shapes (`MDShape*`), materials, detector types (`MDStrip2D`, `MDStrip3D`, `MDCalorimeter`, `MDVoxel3D`, ...), and triggers. The program is `geomega`.
* `src/cosima` (`MC*`): the Geant4 simulation. `MCCosima` and `MCMain` are the program, `MCSource` and `MCPrimaryGeneratorAction` make the particles, `MCPhysicsList*` selects the physics, `MCDetectorConstruction` builds the geometry, `MC*SD` and `MC*Hit` are the sensitive detectors and their hits, `MCActivator` and `MCIsotopeStore` handle activation and decay. Files are `.cc`, not `.cxx`.
* `src/sivan` (`MSim*`, `MFileEventsSim`): reading and analyzing `.sim` files. `MSimEvent` holds an event with its interactions (`MSimIA`), hits (`MSimHT`), and clusters.
* `src/revan` (`MER*`): event reconstruction. Hit clustering, event clustering, Compton sequence reconstruction (`MERCSR*`), tracking (`MERTrack*`), noising (`MERNoising`), coincidence, and decay handling. The program is `revan`.
* `src/mimrec`: high-level analysis of `.tra` files. Back projection and image reconstruction (`MBackprojection*`, `MImage*`, `MBPData*`), event selection (`MEventSelector`), ARM fitting, exposure, polarization, and the GUI. The program is `mimrec`.
* `src/response` (`MResponse*`): the response creator and response manipulator programs (`responsecreator`, `responsemanipulator`) and the classes that make imaging, ARM, efficiency, and Compton-sequence responses.
* `src/spectralyze`, `src/realta`, `src/eview`: spectral analysis and isotope identification, real-time analysis with network transport, and the event viewer. Each is a small program with its own `M*Main` and settings class.
* `src/fretalon` (`framework`, `base`, `melinator`): calibration framework for read-out data (`MReadOut*`, `MCalibration*`) and the calibration program `melinator`.
* `src/addon`: many small standalone tools, one `.cxx` each (file converters, `SimCombiner`, `TraMerger`, `SimAnalyzer`, `CompareHistograms`, ...).
* `src/endtoend` (`framework`, `tests`): the end-to-end testing framework with the test driver, and the end-to-end tests (see "End-to-end tests" below).
* `bin/`: built programs plus the tracked shell scripts (`dcosima*`, `mcosima`, `megalib-config`, ...); `source-megalib.sh` is made by `setup.sh`. `lib/` holds the built libraries, `include/` has links to all headers.
* `resource/examples`: example geometries (`geomega/`, with `special/Max.geo.setup` as the standard test geometry and `materials/Materials.geo` for the materials), sources, and configurations for each program. `resource/endtoend`: reference tables. `resource/catalogs`: source catalogs. `resource/libraries`: isotope libraries.
* `config/`: build system (`Makefile.*`, `configure_*`, allowed ROOT and Geant4 versions, `Version.txt`). `doc/ChangeLog.txt`: the change log.

## Build and test

Set up the environment first, then build:

```
source bin/source-megalib.sh
make -o info -j$(nproc) tests GEANT4INSTALLED=1
```

Every unit test is a program named after its file (`UTString.cxx` becomes `bin/UTString`). A unit test prints `Passed tests: N` and `Failed tests: M`. A test passes only with `Failed tests: 0`. The build must have no warnings.

Use the test driver to run many tests at once:

```
testdrive                                        # all unit tests
testdrive --endtoend                             # all end-to-end tests
testdrive --endtoend ETCosimaToMimrecThresholds  # selected tests
testdrive -r ETCosimaToMimrecThresholds          # reference campaign (CPU heavy, see TestingConventions.md)
```

Only one `testdrive` runs at a time per user. End-to-end tests need `cosima`, `revan`, and `mimrec` at run time and fail, not skip, without them.



