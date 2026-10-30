# Tessellated mesh solids in Geomega and Cosima

Geomega's `TESSELLATED` shape imports a closed solid mesh from STL (ASCII or
binary) or OBJ. Coordinates are interpreted in the same length units as the
rest of the geometry.

The local implementation builds a ROOT `TGeoTessellated` for Geomega and a
Geant4 `G4TessellatedSolid` for Cosima. Both use the same validated triangle
mesh. No intermediate ROOT file or GDML conversion is required. Cosima
converts centimeters to Geant4 length units and applies the usual volume
placement, rotation, material and copy handling.

## Importing a mesh

Use the shape type directly on a volume:

```text
MeshVolume.Shape TESSELLATED models/part.stl
```

Paths are resolved relative to the geometry file. OBJ files can also be used:

```text
MeshVolume.Shape TESSELLATED models/part.obj
```

The filename extension selects OBJ or STL, case-insensitively. OBJ polygons are
triangulated as a fan; face indices may be positive or negative and may include
texture/normal suffixes. STL facets are read as triangles.

The same shape can be declared by name and used by one or more volumes:

```text
Shape TESSELLATED ImportedPart
ImportedPart.MeshFile models/part.obj
MeshVolume.Shape ImportedPart
```

The first implementation accepts only finite, non-degenerate meshes whose
triangles form a closed, two-manifold surface with consistent outward winding.
Open, non-manifold, inconsistently wound, zero-volume, or unsupported mesh files
are rejected with an error; automatic repair is not attempted.
ROOT facet checks are also enforced, including rejection of tiny nonzero-area
triangles. A facet rejected by Geant4 aborts geometry construction and reports
the shape name and zero-based facet index. Closing a Geant4 solid finalizes
its navigation data; it does not independently prove that the mesh is valid.

## Self-contained Geomega output

`MDShapeTessellated::GetGeomega()` writes the mesh inline as a vertex count,
indexed vertices, facet count, and indexed triangular facets. This output does
not refer back to the original STL or OBJ file:

```text
MeshVolume.Shape TESSELLATED 4 4
MeshVolume_Shape.Vertices 4
MeshVolume_Shape.Vertex 0 0 0
MeshVolume_Shape.Vertex 1 0 0
MeshVolume_Shape.Vertex 0 1 0
MeshVolume_Shape.Vertex 0 0 1
MeshVolume_Shape.Facets 4
MeshVolume_Shape.Facet 0 2 1
MeshVolume_Shape.Facet 0 1 3
MeshVolume_Shape.Facet 1 2 3
MeshVolume_Shape.Facet 2 0 3
```

The facet indices are zero-based. The mesh data must follow the volume shape
declaration in the same geometry file.

## Build and run

Activate the environment for the MEGAlib checkout being built, then run from
that checkout:

```sh
source bin/source-megalib.sh
make cos
cosima -s 12345 resource/examples/cosima/source/TessellatedBox.source
```

The example fires 200 photons into an aluminium tessellated box and writes
simulation output in the current directory. `TessellatedBox.geo.setup` can
also be opened in Geomega. To simulate a CAD assembly, use its `.geo.setup`
as the `Geometry` in a Cosima `.source` file, choose the desired source, and
replace any placeholder materials with the actual material definitions.
Use a far-field source or put a local source inside the world volume.

The initial supported use is passive geometry. Defining a mesh as a sensitive
detector also requires a compatible MEGAlib detector description; detectors
with analytic-shape or segmentation assumptions are not generalized here.
Closedness and winding checks do not establish absence of self-intersections
or assembly overlaps. Check those separately for the mass model in use.

## Regression tests

```sh
make test-tessellated
# Keep fixtures, logs and simulation output in a new directory:
bash src/cosima/tests/run-tessellated.sh --output-dir /tmp/tessellated-results
# Additionally construct an existing assembly in ROOT and Geant4:
bash src/cosima/tests/run-tessellated.sh --geometry /path/to/model.geo.setup
```

The suite loads geometry through the real Geomega parser and Cosima detector
constructor. It checks ROOT dictionary availability; embedded, OBJ, ASCII STL
and binary STL meshes; named shapes; scaling; copied and rotated placements;
analytic box volume, containment, safety and ray distances; concave geometry;
sampled fixture overlaps; and rejection of malformed or too-small meshes.
It also runs the actual Cosima executable with a fixed seed, checks 200 stored
events for each of a mesh box and an equivalent `BRIK`, verifies physical
interaction positions and compares interaction counts.

The optional assembly check verifies construction, facet counts and volume
agreement for every tessellated volume. It is not an exhaustive overlap or
self-intersection check. Large meshes require additional initialization time
and memory for the ROOT and Geant4 navigation structures.

Validated locally with ROOT 6.40.04 and Geant4 10.02.p03. Other versions should
run this suite before use; the mere presence of `TGeoTessellated` in an older
ROOT installation does not guarantee its navigation capabilities.
See [the validation report](tessellated-validation.md) for the regression
results, the reduced COSI assembly transport run and its reproduction setup.
