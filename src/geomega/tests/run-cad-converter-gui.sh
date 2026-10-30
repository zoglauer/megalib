#!/usr/bin/env bash
set -euo pipefail
repo=$(cd "$(dirname "$0")/../../.." && pwd)
scratch=$(mktemp -d)
trap 'rm -rf "$scratch"' EXIT
# root-config emits compiler/linker words, intentionally split here.
read -r -a root_cflags <<< "$(root-config --cflags)"
read -r -a root_libs <<< "$(root-config --glibs)"
"${CXX:-g++}" "${root_cflags[@]}" -D___CLING___ -I"$repo/include" \
  "$repo/src/geomega/tests/cad_converter_gui.cxx" -L"$repo/lib" \
  -lGeomegaGui -lGeomega -lCommonGui -lCommonMisc "${root_libs[@]}" -lGeom \
  -o "$scratch/cad_converter_gui"
"$scratch/cad_converter_gui" "$repo/src/geomega/cad-conversion-dev/cad_converter/tests/data" "$scratch"
