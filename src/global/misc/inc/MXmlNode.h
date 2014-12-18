/*
 * MXmlNode.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MXmlNode__
#define __MXmlNode__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cstdlib>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlData.h"
#include "MXmlAttribute.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MXmlNode : public MXmlData
{
  // public interface:
 public:
  //! Default constructor
  MXmlNode();
  //! Constructor -- no values
  MXmlNode(MXmlNode* MotherNode, MString Name);
  //! Constructor -- content is string
  MXmlNode(MXmlNode* MotherNode, MString Name, MString Value);
  //! Constructor -- content is integer
  MXmlNode(MXmlNode* MotherNode, MString Name, int Value);
  //! Constructor -- content is long
  MXmlNode(MXmlNode* MotherNode, MString Name, long Value);
  //! Constructor -- content is unsigned integer
  MXmlNode(MXmlNode* MotherNode, MString Name, unsigned int Value);
  //! Constructor -- content is double
  MXmlNode(MXmlNode* MotherNode, MString Name, double Value);
  //! Constructor -- content is a vector
  MXmlNode(MXmlNode* MotherNode, MString Name, MVector Value);
  //! Constructor -- content is a boolean
  MXmlNode(MXmlNode* MotherNode, MString Name, bool Value);
  //! Constructor -- content is an integer min/max-value
  MXmlNode(MXmlNode* MotherNode, MString Name, int ValueMin, int ValueMax);
  //! Constructor -- content is a long min/max-value
  MXmlNode(MXmlNode* MotherNode, MString Name, long ValueMin, long ValueMax);
  //! Constructor content is a double min/max-value
  MXmlNode(MXmlNode* MotherNode, MString Name, double ValueMin, double ValueMax);
  //! Default destructor
  virtual ~MXmlNode();

  //! Reset the node
  virtual void Clear();

  //! Return the value of the node as vector
  MVector GetValueAsVector() const;
  //! Return the minimum value of the node as int
  int GetMinValueAsInt() const;
  //! Return the maximum value of the node as int
  int GetMaxValueAsInt() const;
  //! Return the minimum value of the node as long
  long GetMinValueAsLong() const;
  //! Return the maximum value of the node as long
  long GetMaxValueAsLong() const;
  //! Return the minimum value of the node as double
  double GetMinValueAsDouble() const;
  //! Return the maximum value of the node as double
  double GetMaxValueAsDouble() const;


  //! Return the number of nodes
  unsigned int GetNNodes() { return m_Nodes.size(); }
  //! Return a given node
  MXmlNode* GetNode(unsigned int i);
  //! Return a given node
  MXmlNode* GetNode(MString Name);
  //! Add a node
  void AddNode(MXmlNode* Node);


  //! Return the number of attributes
  unsigned int GetNAttributes() { return m_Attributes.size(); }
  //! Return a given attribute
  MXmlAttribute* GetAttribute(unsigned int i);
  //! Return a given attribute
  MXmlAttribute* GetAttribute(MString Name);
  //! Add a attribute
  void AddAttribute(MXmlAttribute* Attribute);


  //! Returns the XML text
  virtual MString ToString(unsigned int Indent = 0);


  // protected methods:
 protected:
  //! Parse text into this node
  virtual bool Parse(MString Text);
  //! Return true if the next tag is closed
  virtual bool IsClosed(MString Text);
  
  // private methods:
 private:



  // protected members:
 protected:
  //! All sub nodes
  vector<MXmlNode*> m_Nodes; 
  //! All attributes
  vector<MXmlAttribute*> m_Attributes; 


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MXmlNode, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
