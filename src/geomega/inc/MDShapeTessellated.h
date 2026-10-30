/*
 * MDShapeTessellated.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDShapeTessellated__
#define __MDShapeTessellated__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <array>
#include <map>
#include <vector>

// MEGAlib libs:
#include "MDShape.h"


////////////////////////////////////////////////////////////////////////////////


//! Class representing a closed triangle-mesh solid
class MDShapeTessellated : public MDShape
{
 public:
  //! Standard constructor
  MDShapeTessellated(const MString& Name);
  //! Default destructor
  virtual ~MDShapeTessellated();

  //! Load a mesh from an STL or OBJ file
  bool LoadMesh(const MString& FileName, const MString& GeometryFile = "");

  //! Validate the mesh and create its ROOT representation
  bool Validate();

  //! Mesh coordinates are local to the shape, in Geomega centimeters.
  //! Call Validate() before using these arrays; validation can weld vertices.
  const std::vector<std::array<double, 3>>& GetVertices() const { return m_Vertices; }
  const std::vector<std::array<unsigned int, 3>>& GetFacets() const { return m_Facets; }

  //! Parse mesh file or embedded tessellation data
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  MVector GetSize();
  MString ToString();
  MString GetGeomega() const;
  MString GetGeomega(const MString& ShapeName) const;
  double GetVolume();
  virtual bool Scale(const double Scaler, const MString Axes = "XYZ");
  virtual MVector GetUniquePosition() const;
  virtual vector<MVector> CreateSurfacePattern(const unsigned int Detail = 0) const;

 private:
  bool AddTriangle(const std::array<double, 3>& A,
                   const std::array<double, 3>& B,
                   const std::array<double, 3>& C);
  bool CheckMesh(MString& Error);
  bool ParseEmbeddedData(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  std::vector<std::array<double, 3>> m_Vertices;
  std::vector<std::array<unsigned int, 3>> m_Facets;
  std::map<std::array<double, 3>, unsigned int> m_VertexIndices;
  unsigned int m_ExpectedVertices;
  unsigned int m_ExpectedFacets;

#ifdef ___CLING___
 public:
  ClassDef(MDShapeTessellated, 0) // closed triangle-mesh solid
#endif
};

#endif


////////////////////////////////////////////////////////////////////////////////
