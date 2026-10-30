# STEP to Geomega

Build Geomega with `make geo` from the MEGAlib root. The converter dialog is
available under **File → Convert STEP CAD...**. Its sources are
`src/geomega/src/MGUICADConverter.cxx` and
`src/geomega/inc/MGUICADConverter.h`, included in `libGeomegaGui` and its ROOT
dictionary.

The conversion engine is the separate executable in
`src/geomega/cad-conversion-dev/cad_converter`. On Linux, build it from the
MEGAlib root with:

```bash
export VCPKG_ROOT=/path/to/vcpkg
make cad-converter
```

This uses the converter's existing CMake/vcpkg release preset (CMake >= 3.21,
C++17, OpenCASCADE and Manifold). See the converter's README for initial
dependency setup and its Windows CLI build. The ROOT GUI uses POSIX process
APIs, as supported by MEGAlib's Linux/macOS builds; the convenience Make target
uses the Linux preset.

The dialog locates the converter in this order:

1. `MEGALIB_CAD_CONVERTER`, if set, must name an executable file.
2. `$MEGALIB/bin/cad_converter`.
3. The bundled converter's `install/linux-release/bin/cad_converter`.
4. The bundled converter's `build/linux-release/cad_converter`.
5. `cad_converter` on `PATH`.

Use the override for a different build or installation. Keep a CMake-built
executable with its dependency libraries; copying the executable alone may
break its runtime library paths.

Select a `.step` or `.stp` file (uppercase extensions also work), optionally
choose an output directory, then click **Convert**. Geomega is selected by
default, and is also the fallback if no format is selected. OBJ and STL can
be selected in any combination. The settings match the CLI:

- Angular deflection: degrees, default 20, greater than 0 and at most 180.
- Maximum deviation: mm; 0 uses the converter's automatic model-size default.
- Minimum volume: mm³; 0 keeps all solids.

Conversion runs in a child process while the GUI displays its log. Cancel or
closing the dialog stops that process. Each run leaves a
`<model>.conversion-XXXXXX` log in the output directory. Failed/cancelled runs
can leave partial files; rerun conversion before using them. Existing outputs
for the same model name are overwritten.

Geomega output consists of `<model>.geo.setup` and `<model>_geo/` mesh files.
Unsafe characters in the mesh directory name are percent-encoded so Geomega's
single-token `Include` syntax can read paths derived from names with spaces.
Keep the setup and mesh directory together. **View in Geomega** becomes
available after a successful Geomega conversion and loads the setup into the
current Geomega window, reloading even if the filename is unchanged.
Preview does not generate simulation cross sections.

Coordinates are converted from mm to cm. Materials are **Aluminium
placeholders**: assign the real materials in the setup before simulation.
Conversion warnings remain visible in the log and completion status.

## Verification

```bash
cd src/geomega/cad-conversion-dev/cad_converter
ctest --preset linux-release
```

From the MEGAlib root, with the MEGAlib/ROOT environment loaded and an X display
available (or using `xvfb-run`):

```bash
src/geomega/tests/run-cad-converter-gui.sh
```

The integration test runs the real dialog/converter, loads all ten small STEP
fixtures through `MDGeometry`, checks filename/format handling, and exercises
failure and cancellation. It uses temporary output files and cleans them up.
