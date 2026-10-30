/*
 * MDShapeTessellated.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


////////////////////////////////////////////////////////////////////////////////


#include "MDShapeTessellated.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <utility>

#include "TGeoTessellated.h"

#include "MFile.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDShapeTessellated)
#endif


////////////////////////////////////////////////////////////////////////////////


namespace
{
using Vertex = std::array<double, 3>;
using Facet = std::array<unsigned int, 3>;

bool IsUnsignedInteger(const MString& Token, bool AllowZero)
{
  const std::string Text = Token.Data();
  if (Text.empty()) return false;
  for (unsigned char C : Text) {
    if (!std::isdigit(C)) return false;
  }
  errno = 0;
  char* End = nullptr;
  const unsigned long long Value = std::strtoull(Text.c_str(), &End, 10);
  return errno == 0 && End != Text.c_str() && *End == '\0' &&
         Value <= std::numeric_limits<unsigned int>::max() &&
         (AllowZero || Value > 0);
}

bool IsFiniteDouble(const MString& Token)
{
  const std::string Text = Token.Data();
  if (Text.empty()) return false;
  errno = 0;
  char* End = nullptr;
  const double Value = std::strtod(Text.c_str(), &End);
  return errno == 0 && End != Text.c_str() && *End == '\0' && std::isfinite(Value);
}


bool ReadBinarySTL(const std::string& FileName,
                   std::vector<std::array<Vertex, 3>>& Triangles,
                   MString& Error)
{
  std::ifstream Input(FileName.c_str(), std::ios::binary);
  if (!Input) {
    Error = "Unable to open mesh file";
    return false;
  }

  Input.seekg(80);
  unsigned char CountBytes[4];
  if (!Input.read(reinterpret_cast<char*>(CountBytes), 4)) {
    Error = "Binary STL header is incomplete";
    return false;
  }
  const std::uint32_t Count =
    static_cast<std::uint32_t>(CountBytes[0]) |
    (static_cast<std::uint32_t>(CountBytes[1]) << 8) |
    (static_cast<std::uint32_t>(CountBytes[2]) << 16) |
    (static_cast<std::uint32_t>(CountBytes[3]) << 24);
  const std::uint64_t ExpectedSize = 84ULL + 50ULL*Count;
  Input.seekg(0, std::ios::end);
  if (Input.tellg() < 0 || static_cast<std::uint64_t>(Input.tellg()) != ExpectedSize) {
    Error = "Binary STL size does not match its triangle count";
    return false;
  }
  Input.seekg(84);

  Triangles.clear();
  Triangles.reserve(Count);
  for (std::uint32_t i = 0; i < Count; ++i) {
    unsigned char Record[50];
    if (!Input.read(reinterpret_cast<char*>(Record), sizeof(Record))) {
      Error = "Binary STL contains a truncated triangle";
      return false;
    }
    std::array<Vertex, 3> Triangle;
    for (unsigned int VertexIndex = 0; VertexIndex < 3; ++VertexIndex) {
      for (unsigned int Axis = 0; Axis < 3; ++Axis) {
        const unsigned int Offset = 12 + VertexIndex*12 + Axis*4;
        const std::uint32_t Bits =
          static_cast<std::uint32_t>(Record[Offset]) |
          (static_cast<std::uint32_t>(Record[Offset + 1]) << 8) |
          (static_cast<std::uint32_t>(Record[Offset + 2]) << 16) |
          (static_cast<std::uint32_t>(Record[Offset + 3]) << 24);
        float Coordinate;
        std::memcpy(&Coordinate, &Bits, sizeof(Coordinate));
        Triangle[VertexIndex][Axis] = Coordinate;
      }
    }
    Triangles.push_back(Triangle);
  }
  return true;
}


bool ReadASCIISTL(const std::string& FileName,
                  std::vector<std::array<Vertex, 3>>& Triangles,
                  MString& Error)
{
  std::ifstream Input(FileName.c_str());
  if (!Input) {
    Error = "Unable to open mesh file";
    return false;
  }

  std::vector<Vertex> Vertices;
  std::string Line;
  while (std::getline(Input, Line)) {
    std::istringstream LineInput(Line);
    std::string Word;
    while (LineInput >> Word) {
      if (Word == "vertex") {
        Vertex V;
        if (!(LineInput >> V[0] >> V[1] >> V[2])) {
          Error = "ASCII STL has an incomplete vertex";
          return false;
        }
        Vertices.push_back(V);
        break;
      }
    }
  }
  if (Vertices.empty() || Vertices.size() % 3 != 0) {
    Error = "ASCII STL must contain complete triangular facets";
    return false;
  }

  Triangles.clear();
  Triangles.reserve(Vertices.size()/3);
  for (std::size_t i = 0; i < Vertices.size(); i += 3) {
    Triangles.push_back({Vertices[i], Vertices[i + 1], Vertices[i + 2]});
  }
  return true;
}


bool ParseOBJIndex(const std::string& Token, std::size_t NVertices, unsigned int& Index)
{
  const std::size_t Slash = Token.find('/');
  const std::string Number = Token.substr(0, Slash);
  if (Number.empty()) return false;

  char* End = nullptr;
  const long Parsed = std::strtol(Number.c_str(), &End, 10);
  if (End == Number.c_str() || *End != '\0' || Parsed == 0) return false;

  const long Resolved = Parsed > 0 ? Parsed - 1 : static_cast<long>(NVertices) + Parsed;
  if (Resolved < 0 || static_cast<std::size_t>(Resolved) >= NVertices ||
      static_cast<unsigned long>(Resolved) > std::numeric_limits<unsigned int>::max()) {
    return false;
  }
  Index = static_cast<unsigned int>(Resolved);
  return true;
}


bool ReadOBJ(const std::string& FileName,
             std::vector<Vertex>& Vertices,
             std::vector<Facet>& Facets,
             MString& Error)
{
  std::ifstream Input(FileName.c_str());
  if (!Input) {
    Error = "Unable to open mesh file";
    return false;
  }

  Vertices.clear();
  Facets.clear();
  std::string Line;
  unsigned int LineNumber = 0;
  while (std::getline(Input, Line)) {
    ++LineNumber;
    std::istringstream LineInput(Line);
    std::string Kind;
    if (!(LineInput >> Kind) || Kind[0] == '#') continue;

    if (Kind == "v") {
      Vertex V;
      if (!(LineInput >> V[0] >> V[1] >> V[2])) {
        Error = MString("OBJ has an invalid vertex on line ") + LineNumber;
        return false;
      }
      Vertices.push_back(V);
    } else if (Kind == "f") {
      std::vector<unsigned int> Indices;
      std::string Token;
      while (LineInput >> Token) {
        if (!Token.empty() && Token[0] == '#') break;
        unsigned int Index = 0;
        if (!ParseOBJIndex(Token, Vertices.size(), Index)) {
          Error = MString("OBJ has an invalid face index on line ") + LineNumber;
          return false;
        }
        Indices.push_back(Index);
      }
      if (Indices.size() < 3) {
        Error = MString("OBJ face has fewer than three vertices on line ") + LineNumber;
        return false;
      }
      for (std::size_t i = 1; i + 1 < Indices.size(); ++i) {
        Facets.push_back({Indices[0], Indices[i], Indices[i + 1]});
      }
    }
  }
  if (Facets.empty()) {
    Error = "OBJ contains no faces";
    return false;
  }
  return true;
}
}


////////////////////////////////////////////////////////////////////////////////


MDShapeTessellated::MDShapeTessellated(const MString& Name) : MDShape(Name)
{
  m_Type = "TESSELLATED";
  m_ExpectedVertices = 0;
  m_ExpectedFacets = 0;
}


MDShapeTessellated::~MDShapeTessellated()
{
}


bool MDShapeTessellated::AddTriangle(const Vertex& A, const Vertex& B, const Vertex& C)
{
  const Vertex Points[3] = {A, B, C};
  Facet Triangle;
  for (unsigned int i = 0; i < 3; ++i) {
    Vertex V = Points[i];
    for (unsigned int Axis = 0; Axis < 3; ++Axis) {
      if (!std::isfinite(V[Axis])) return false;
      if (V[Axis] == 0.0) V[Axis] = 0.0;
    }
    auto Found = m_VertexIndices.find(V);
    if (Found == m_VertexIndices.end()) {
      Triangle[i] = static_cast<unsigned int>(m_Vertices.size());
      m_VertexIndices[V] = Triangle[i];
      m_Vertices.push_back(V);
    } else {
      Triangle[i] = Found->second;
    }
  }
  m_Facets.push_back(Triangle);
  return true;
}


bool MDShapeTessellated::LoadMesh(const MString& FileName, const MString& GeometryFile)
{
  MString Expanded = FileName;
  if (MFile::ExpandFileName(Expanded, GeometryFile) == false) {
    merr<<"Error expanding tessellated mesh filename: "<<FileName<<endl;
    return false;
  }
  if (MFile::Exists(Expanded) == false) {
    merr<<"Tessellated mesh file does not exist: "<<Expanded<<endl;
    return false;
  }

  delete m_Geo;
  m_Geo = nullptr;
  m_IsValidated = false;
  m_ExpectedVertices = 0;
  m_ExpectedFacets = 0;
  const std::string Path = Expanded.Data();
  const std::size_t Dot = Path.find_last_of('.');
  std::string Extension = Dot == std::string::npos ? "" : Path.substr(Dot);
  std::transform(Extension.begin(), Extension.end(), Extension.begin(),
                 [](unsigned char C) { return std::tolower(C); });

  m_Vertices.clear();
  m_Facets.clear();
  m_VertexIndices.clear();
  MString Error;
  if (Extension == ".obj") {
    if (!ReadOBJ(Path, m_Vertices, m_Facets, Error)) {
      merr<<"   ***  Error  ***  in tessellated shape "<<m_Name<<": "<<Error<<endl;
      return false;
    }
  } else if (Extension == ".stl") {
    std::ifstream Probe(Path.c_str(), std::ios::binary);
    Probe.seekg(0, std::ios::end);
    const std::streamoff Length = Probe.tellg();
    bool IsBinary = false;
    if (Length >= 84) {
      Probe.seekg(80);
      unsigned char CountBytes[4];
      if (Probe.read(reinterpret_cast<char*>(CountBytes), 4)) {
        const std::uint32_t Count =
          static_cast<std::uint32_t>(CountBytes[0]) |
          (static_cast<std::uint32_t>(CountBytes[1]) << 8) |
          (static_cast<std::uint32_t>(CountBytes[2]) << 16) |
          (static_cast<std::uint32_t>(CountBytes[3]) << 24);
        IsBinary = 84ULL + 50ULL*Count == static_cast<std::uint64_t>(Length);
      }
    }

    std::vector<std::array<Vertex, 3>> Triangles;
    const bool Read = IsBinary ? ReadBinarySTL(Path, Triangles, Error) :
                                ReadASCIISTL(Path, Triangles, Error);
    if (!Read) {
      merr<<"   ***  Error  ***  in tessellated shape "<<m_Name<<": "<<Error<<endl;
      return false;
    }
    for (const auto& Triangle : Triangles) {
      if (!AddTriangle(Triangle[0], Triangle[1], Triangle[2])) {
        merr<<"   ***  Error  ***  in tessellated shape "<<m_Name
            <<": vertex coordinate is not finite"<<endl;
        return false;
      }
    }
  } else {
    merr<<"   ***  Error  ***  in tessellated shape "<<m_Name
        <<": mesh file extension must be .stl or .obj"<<endl;
    return false;
  }

  m_ExpectedVertices = static_cast<unsigned int>(m_Vertices.size());
  m_ExpectedFacets = static_cast<unsigned int>(m_Facets.size());
  m_IsValidated = false;
  return true;
}


bool MDShapeTessellated::CheckMesh(MString& Error)
{
  if (m_Vertices.size() < 4 || m_Facets.size() < 4) {
    Error = "A closed solid needs at least four vertices and four facets";
    return false;
  }
  if ((m_ExpectedVertices != 0 && m_Vertices.size() != m_ExpectedVertices) ||
      (m_ExpectedFacets != 0 && m_Facets.size() != m_ExpectedFacets)) {
    Error = "Embedded vertex or facet count does not match its header";
    return false;
  }

  std::vector<bool> Used(m_Vertices.size(), false);
  for (const Facet& F : m_Facets) {
    for (unsigned int Index : F) {
      if (Index >= m_Vertices.size()) {
        Error = "Mesh contains an invalid facet vertex index";
        return false;
      }
      Used[Index] = true;
    }
  }

  std::map<Vertex, unsigned int> UniqueVertices;
  std::vector<Vertex> WeldedVertices;
  std::vector<unsigned int> Remap(m_Vertices.size());
  for (unsigned int i = 0; i < m_Vertices.size(); ++i) {
    if (!Used[i]) continue;
    Vertex V = m_Vertices[i];
    for (unsigned int Axis = 0; Axis < 3; ++Axis) {
      if (!std::isfinite(V[Axis])) {
        Error = "Mesh contains a non-finite coordinate";
        return false;
      }
      if (V[Axis] == 0.0) V[Axis] = 0.0;
    }
    auto Found = UniqueVertices.find(V);
    if (Found == UniqueVertices.end()) {
      Remap[i] = static_cast<unsigned int>(WeldedVertices.size());
      UniqueVertices[V] = Remap[i];
      WeldedVertices.push_back(V);
    } else {
      Remap[i] = Found->second;
    }
  }
  for (Facet& F : m_Facets) {
    for (unsigned int& Index : F) {
      if (Index >= Remap.size()) {
        Error = "Mesh contains an invalid facet vertex index";
        return false;
      }
      Index = Remap[Index];
    }
  }
  m_Vertices.swap(WeldedVertices);
  m_VertexIndices.clear();
  m_ExpectedVertices = static_cast<unsigned int>(m_Vertices.size());
  if (m_Vertices.size() < 4) {
    Error = "A closed solid needs at least four distinct vertices";
    return false;
  }

  using Edge = std::pair<unsigned int, unsigned int>;
  std::map<Edge, std::pair<unsigned int, int>> Edges;
  Vertex Min = m_Vertices.front();
  Vertex Max = m_Vertices.front();
  for (const Vertex& V : m_Vertices) {
    for (unsigned int Axis = 0; Axis < 3; ++Axis) {
      if (!std::isfinite(V[Axis])) {
        Error = "Mesh contains a non-finite coordinate";
        return false;
      }
      Min[Axis] = std::min(Min[Axis], V[Axis]);
      Max[Axis] = std::max(Max[Axis], V[Axis]);
    }
  }

  double Reference[3] = {0.0, 0.0, 0.0};
  for (const Vertex& V : m_Vertices) {
    for (unsigned int Axis = 0; Axis < 3; ++Axis) Reference[Axis] += V[Axis]/m_Vertices.size();
  }
  long double SixVolume = 0.0L;
  for (const Facet& F : m_Facets) {
    if (F[0] >= m_Vertices.size() || F[1] >= m_Vertices.size() || F[2] >= m_Vertices.size() ||
        F[0] == F[1] || F[1] == F[2] || F[2] == F[0]) {
      Error = "Mesh contains an invalid or repeated facet vertex index";
      return false;
    }
    const Vertex& A = m_Vertices[F[0]];
    const Vertex& B = m_Vertices[F[1]];
    const Vertex& C = m_Vertices[F[2]];
    const double AB[3] = {B[0] - A[0], B[1] - A[1], B[2] - A[2]};
    const double AC[3] = {C[0] - A[0], C[1] - A[1], C[2] - A[2]};
    const double Cross[3] = {
      AB[1]*AC[2] - AB[2]*AC[1],
      AB[2]*AC[0] - AB[0]*AC[2],
      AB[0]*AC[1] - AB[1]*AC[0]
    };
    if (Cross[0] == 0.0 && Cross[1] == 0.0 && Cross[2] == 0.0) {
      Error = "Mesh contains a degenerate triangle";
      return false;
    }

    const long double X[3] = {A[0] - Reference[0], A[1] - Reference[1], A[2] - Reference[2]};
    const long double Y[3] = {B[0] - Reference[0], B[1] - Reference[1], B[2] - Reference[2]};
    const long double Z[3] = {C[0] - Reference[0], C[1] - Reference[1], C[2] - Reference[2]};
    SixVolume += X[0]*(Y[1]*Z[2] - Y[2]*Z[1]) +
                 X[1]*(Y[2]*Z[0] - Y[0]*Z[2]) +
                 X[2]*(Y[0]*Z[1] - Y[1]*Z[0]);

    for (unsigned int i = 0; i < 3; ++i) {
      const unsigned int From = F[i];
      const unsigned int To = F[(i + 1) % 3];
      const Edge Key(std::min(From, To), std::max(From, To));
      auto& Entry = Edges[Key];
      ++Entry.first;
      Entry.second += From < To ? 1 : -1;
    }
  }

  for (const auto& Entry : Edges) {
    if (Entry.second.first != 2 || Entry.second.second != 0) {
      Error = "Mesh is open, non-manifold, or has inconsistent facet winding";
      return false;
    }
  }
  if (!(SixVolume > 0.0L) || !std::isfinite(static_cast<double>(SixVolume))) {
    Error = "Mesh facets must have a consistent outward orientation and enclose positive volume";
    return false;
  }
  if (Max[0] == Min[0] || Max[1] == Min[1] || Max[2] == Min[2]) {
    Error = "Mesh does not enclose a three-dimensional volume";
    return false;
  }
  return true;
}


bool MDShapeTessellated::Validate()
{
  if (m_IsValidated) return true;

  MString Error;
  if (!CheckMesh(Error)) {
    merr<<"   ***  Error  ***  in tessellated shape "<<m_Name<<": "<<Error<<endl;
    return false;
  }

  TGeoTessellated* Tessellated = new TGeoTessellated(m_Name.Data());
  unsigned int FacetIndex = 0;
  for (const Facet& F : m_Facets) {
    const Vertex& A = m_Vertices[F[0]];
    const Vertex& B = m_Vertices[F[1]];
    const Vertex& C = m_Vertices[F[2]];
    if (!Tessellated->AddFacet(TGeoTessellated::Vertex_t(A[0], A[1], A[2]),
                               TGeoTessellated::Vertex_t(B[0], B[1], B[2]),
                               TGeoTessellated::Vertex_t(C[0], C[1], C[2]))) {
      delete Tessellated;
      merr<<"   ***  Error  ***  in tessellated shape "<<m_Name
          <<": ROOT rejected triangle facet "<<FacetIndex<<endl;
      return false;
    }
    // CloseShape reports facet errors, but does not propagate their status.
    // Reject them before ROOT builds its navigation acceleration structure.
    if (!Tessellated->FacetCheck(FacetIndex)) {
      delete Tessellated;
      merr<<"   ***  Error  ***  in tessellated shape "<<m_Name
          <<": ROOT rejected degenerate or too-small facet "<<FacetIndex<<endl;
      return false;
    }
    ++FacetIndex;
  }
  Tessellated->CloseShape(true, false, false);
  if (!Tessellated->IsClosedBody() || !(Tessellated->Capacity() > 0.0)) {
    delete Tessellated;
    merr<<"   ***  Error  ***  in tessellated shape "<<m_Name
        <<": ROOT could not validate the closed solid"<<endl;
    return false;
  }
  delete m_Geo;
  m_Geo = Tessellated;
  m_IsValidated = true;
  return true;
}


bool MDShapeTessellated::ParseEmbeddedData(const MTokenizer& Tokenizer, const MDDebugInfo& Info)
{
  const unsigned int Count = Tokenizer.GetNTokens();
  const MString Descriptor = Tokenizer.GetTokenAt(1);
  if (Descriptor == "Vertices") {
    if (Count != 3 || !m_Vertices.empty() || !m_Facets.empty()) {
      Info.Error("TESSELLATED Vertices must declare the vertex count before mesh records");
      return false;
    }
    if (!IsUnsignedInteger(Tokenizer.GetTokenAt(2), false)) {
      Info.Error("TESSELLATED vertex count must be a positive integer");
      return false;
    }
    m_ExpectedVertices = Tokenizer.GetTokenAtAsUnsignedInt(2);
    if (m_ExpectedVertices < 4) {
      Info.Error("TESSELLATED requires at least four vertices");
      return false;
    }
  } else if (Descriptor == "Vertex") {
    if (Count != 5 || m_ExpectedVertices == 0 || m_Vertices.size() >= m_ExpectedVertices) {
      Info.Error("TESSELLATED Vertex needs x, y, z and a declared vertex count");
      return false;
    }
    if (!IsFiniteDouble(Tokenizer.GetTokenAt(2)) ||
        !IsFiniteDouble(Tokenizer.GetTokenAt(3)) ||
        !IsFiniteDouble(Tokenizer.GetTokenAt(4))) {
      Info.Error("TESSELLATED vertex coordinates must be finite numbers");
      return false;
    }
    m_Vertices.push_back({Tokenizer.GetTokenAtAsDouble(2),
                          Tokenizer.GetTokenAtAsDouble(3),
                          Tokenizer.GetTokenAtAsDouble(4)});
  } else if (Descriptor == "Facets") {
    if (Count != 3 || m_ExpectedVertices == 0 ||
        m_Vertices.size() != m_ExpectedVertices || m_Facets.size() != 0) {
      Info.Error("TESSELLATED Facets must declare the facet count after all vertices");
      return false;
    }
    if (!IsUnsignedInteger(Tokenizer.GetTokenAt(2), false)) {
      Info.Error("TESSELLATED facet count must be a positive integer");
      return false;
    }
    m_ExpectedFacets = Tokenizer.GetTokenAtAsUnsignedInt(2);
    if (m_ExpectedFacets < 4) {
      Info.Error("TESSELLATED requires at least four facets");
      return false;
    }
  } else if (Descriptor == "Facet") {
    if (Count != 5 || m_ExpectedFacets == 0 || m_Facets.size() >= m_ExpectedFacets) {
      Info.Error("TESSELLATED Facet needs three indices and a declared facet count");
      return false;
    }
    for (unsigned int i = 2; i < 5; ++i) {
      if (!IsUnsignedInteger(Tokenizer.GetTokenAt(i), true)) {
        Info.Error("TESSELLATED facet indices must be non-negative integers");
        return false;
      }
    }
    m_Facets.push_back({Tokenizer.GetTokenAtAsUnsignedInt(2),
                        Tokenizer.GetTokenAtAsUnsignedInt(3),
                        Tokenizer.GetTokenAtAsUnsignedInt(4)});
  } else {
    Info.Error(MString("Unhandled descriptor in shape TESSELLATED: ") + Descriptor);
    return false;
  }
  m_IsValidated = false;
  return true;
}


bool MDShapeTessellated::Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info)
{
  if (Tokenizer.GetNTokens() < 2) {
    Info.Error("Incomplete TESSELLATED shape description");
    return false;
  }

  unsigned int Offset = 0;
  if (Tokenizer.IsTokenAt(1, "Shape")) {
    Offset = 1;
    if (Tokenizer.GetNTokens() < 3 ||
        Tokenizer.GetTokenAt(2).AreIdentical("TESSELLATED") == false) {
      Info.Error("Expected TESSELLATED after Shape");
      return false;
    }
  }

  const unsigned int DescriptorIndex = Offset == 1 ? 3 : 1;
  if (Tokenizer.GetTokenAt(DescriptorIndex).AreIdentical("Parameters") ||
      Tokenizer.GetTokenAt(DescriptorIndex).AreIdentical("MeshFile")) {
    if (Tokenizer.GetNTokens() != DescriptorIndex + 2) {
      Info.Error("TESSELLATED MeshFile needs one filename");
      return false;
    }
    if (!LoadMesh(Tokenizer.GetTokenAt(DescriptorIndex + 1), Info.GetFileName())) {
      Info.Error("Unable to read TESSELLATED mesh file");
      return false;
    }
    return true;
  }

  if (Tokenizer.GetTokenAt(DescriptorIndex).AreIdentical("Vertices")) {
    MTokenizer Adjusted;
    MString Text = MString("Mesh ") + Tokenizer.GetTokenAt(DescriptorIndex) + " " +
                   Tokenizer.GetTokenAt(DescriptorIndex + 1);
    Adjusted.Analyse(Text, false);
    return ParseEmbeddedData(Adjusted, Info);
  }
  if (Tokenizer.GetTokenAt(DescriptorIndex).AreIdentical("Vertex") ||
      Tokenizer.GetTokenAt(DescriptorIndex).AreIdentical("Facets") ||
      Tokenizer.GetTokenAt(DescriptorIndex).AreIdentical("Facet")) {
    MString Text = Tokenizer.GetTokenAfterAsString(DescriptorIndex - 1);
    MTokenizer Adjusted(Text);
    return ParseEmbeddedData(Adjusted, Info);
  }

  if (Offset == 1 && Tokenizer.GetNTokens() == 5 &&
      IsUnsignedInteger(Tokenizer.GetTokenAt(3), false) &&
      IsUnsignedInteger(Tokenizer.GetTokenAt(4), false)) {
    m_ExpectedVertices = Tokenizer.GetTokenAtAsUnsignedInt(3);
    m_ExpectedFacets = Tokenizer.GetTokenAtAsUnsignedInt(4);
    m_Vertices.clear();
    m_Facets.clear();
    m_VertexIndices.clear();
    m_IsValidated = false;
    return true;
  }

  if (Offset == 1 && Tokenizer.GetNTokens() == 4) {
    if (!LoadMesh(Tokenizer.GetTokenAt(3), Info.GetFileName())) {
      Info.Error("Unable to read TESSELLATED mesh file");
      return false;
    }
    return true;
  }
  if (Offset == 1 && Tokenizer.GetNTokens() == 5 &&
      Tokenizer.GetTokenAt(3).AreIdentical("MeshFile")) {
    if (!LoadMesh(Tokenizer.GetTokenAt(4), Info.GetFileName())) {
      Info.Error("Unable to read TESSELLATED mesh file");
      return false;
    }
    return true;
  }

  if (Offset == 0 && Tokenizer.GetNTokens() == 2) {
    if (!LoadMesh(Tokenizer.GetTokenAt(1), Info.GetFileName())) {
      Info.Error("Unable to read TESSELLATED mesh file");
      return false;
    }
    return true;
  }

  Info.Error("TESSELLATED expects a mesh filename or embedded vertex/facet counts");
  return false;
}


MVector MDShapeTessellated::GetSize()
{
  if (m_Vertices.empty()) return MVector(0, 0, 0);
  Vertex Extent = {0.0, 0.0, 0.0};
  for (const Vertex& V : m_Vertices) {
    for (unsigned int Axis = 0; Axis < 3; ++Axis) {
      Extent[Axis] = std::max(Extent[Axis], std::fabs(V[Axis]));
    }
  }
  return MVector(Extent[0], Extent[1], Extent[2]);
}


MString MDShapeTessellated::ToString()
{
  std::ostringstream Out;
  Out<<"TESSELLATED ("<<m_Vertices.size()<<" vertices, "<<m_Facets.size()<<" facets)"<<std::endl;
  return Out.str().c_str();
}


MString MDShapeTessellated::GetGeomega() const
{
  return GetGeomega(m_Name);
}


MString MDShapeTessellated::GetGeomega(const MString& ShapeName) const
{
  std::ostringstream Out;
  Out.precision(std::numeric_limits<double>::max_digits10);
  Out<<"TESSELLATED "<<m_Vertices.size()<<" "<<m_Facets.size()<<std::endl;
  Out<<ShapeName<<".Vertices "<<m_Vertices.size()<<std::endl;
  for (const Vertex& V : m_Vertices) {
    Out<<ShapeName<<".Vertex "<<V[0]<<" "<<V[1]<<" "<<V[2]<<std::endl;
  }
  Out<<ShapeName<<".Facets "<<m_Facets.size()<<std::endl;
  for (const Facet& F : m_Facets) {
    Out<<ShapeName<<".Facet "<<F[0]<<" "<<F[1]<<" "<<F[2]<<std::endl;
  }
  return Out.str().c_str();
}


double MDShapeTessellated::GetVolume()
{
  if (!Validate()) return 0.0;
  return m_Geo->Capacity();
}


bool MDShapeTessellated::Scale(const double Scaler, const MString Axes)
{
  if (Scaler <= 0.0) {
    merr<<"   ***  Error  ***  in shape "<<m_Name<<": scale factor must be positive"<<endl;
    return false;
  }
  if (Scaler == m_Scaler && Axes == m_ScalingAxis) return true;
  if (!MDShape::Scale(Scaler, Axes)) return false;
  if (!IsScaled()) return true;

  for (Vertex& V : m_Vertices) {
    if (m_ScalingAxis.Contains("X")) V[0] *= m_Scaler;
    if (m_ScalingAxis.Contains("Y")) V[1] *= m_Scaler;
    if (m_ScalingAxis.Contains("Z")) V[2] *= m_Scaler;
  }
  m_IsValidated = false;
  return Validate();
}


MVector MDShapeTessellated::GetUniquePosition() const
{
  long double SixVolume = 0.0L;
  long double Centroid[3] = {0.0L, 0.0L, 0.0L};
  if (m_Vertices.empty()) return MVector(0, 0, 0);
  double Reference[3] = {0.0, 0.0, 0.0};
  for (const Vertex& V : m_Vertices) {
    for (unsigned int Axis = 0; Axis < 3; ++Axis) Reference[Axis] += V[Axis]/m_Vertices.size();
  }
  for (const Facet& F : m_Facets) {
    const Vertex& A = m_Vertices[F[0]];
    const Vertex& B = m_Vertices[F[1]];
    const Vertex& C = m_Vertices[F[2]];
    const long double Ax = A[0] - Reference[0], Ay = A[1] - Reference[1], Az = A[2] - Reference[2];
    const long double Bx = B[0] - Reference[0], By = B[1] - Reference[1], Bz = B[2] - Reference[2];
    const long double Cx = C[0] - Reference[0], Cy = C[1] - Reference[1], Cz = C[2] - Reference[2];
    const long double V6 = Ax*(By*Cz - Bz*Cy) + Ay*(Bz*Cx - Bx*Cz) + Az*(Bx*Cy - By*Cx);
    SixVolume += V6;
    Centroid[0] += V6*(A[0] + B[0] + C[0]);
    Centroid[1] += V6*(A[1] + B[1] + C[1]);
    Centroid[2] += V6*(A[2] + B[2] + C[2]);
  }
  if (SixVolume == 0.0L) return MVector(Reference[0], Reference[1], Reference[2]);
  return MVector(static_cast<double>((Reference[0]*SixVolume + Centroid[0])/(4.0L*SixVolume)),
                 static_cast<double>((Reference[1]*SixVolume + Centroid[1])/(4.0L*SixVolume)),
                 static_cast<double>((Reference[2]*SixVolume + Centroid[2])/(4.0L*SixVolume)));
}


vector<MVector> MDShapeTessellated::CreateSurfacePattern(const unsigned int Detail) const
{
  vector<MVector> Points;
  Points.reserve(m_Vertices.size());
  for (const Vertex& V : m_Vertices) Points.emplace_back(V[0], V[1], V[2]);
  return Points;
}


////////////////////////////////////////////////////////////////////////////////
