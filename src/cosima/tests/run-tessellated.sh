#!/usr/bin/env bash
set -euo pipefail
repo=$(cd "$(dirname "$0")/../../.." && pwd)
scratch=$(mktemp -d)
trap 'rm -rf "$scratch"' EXIT
read -r -a root_cflags <<< "$(root-config --cflags)"
read -r -a root_libs <<< "$(root-config --glibs)"
read -r -a g4_cflags <<< "$(geant4-config --cflags)"
read -r -a g4_libs <<< "$(geant4-config --libs)"
"${CXX:-g++}" "${g4_cflags[@]}" "${root_cflags[@]}" -D___CLING___ -I"$repo/include" \
  "$repo/src/cosima/tests/tessellated_geometry.cc" -L"$repo/lib" \
  -lCosima -lSivan -lGeomega -lCommonMisc -lCommonGui \
  "${root_libs[@]}" -lGeom "${g4_libs[@]}" -o "$scratch/tessellated_geometry"
python3 "$repo/src/cosima/tests/test_tessellated.py" \
  --checker "$scratch/tessellated_geometry" --cosima "$repo/bin/cosima" "$@"
