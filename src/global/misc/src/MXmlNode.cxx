/*
 * MXmlNode.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MXmlNode
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MXmlNode.h"

// Standard libs:
#include <sstream>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MXmlNode.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MXmlNode)
#endif


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode() : MXmlData()
{
  // Construct an instance of MXmlNode
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name) : MXmlData(Name)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, const MString& Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, int Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, long Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, unsigned int Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, double Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, bool Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, const MTime& Value) : MXmlData(Name, Value)
{
  //! Constructor

  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, MVector Value)
{
  //! Constructor

  m_Name = Name;
  
  new MXmlNode(this, "X", Value.X());
  new MXmlNode(this, "Y", Value.Y());
  new MXmlNode(this, "Z", Value.Z());
  
  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, int ValueMin, int ValueMax)
{
  //! Constructor

  MString Temp;

  m_Name = Name;
  
  new MXmlNode(this, "Min", ValueMin);
  new MXmlNode(this, "Max", ValueMax);
  
  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, long ValueMin, long ValueMax)
{
  //! Constructor

  MString Temp;

  m_Name = Name;
  
  new MXmlNode(this, "Min", ValueMin);
  new MXmlNode(this, "Max", ValueMax);
  
  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, double ValueMin, double ValueMax)
{
  //! Constructor

  MString Temp;

  m_Name = Name;
  
  new MXmlNode(this, "Min", ValueMin);
  new MXmlNode(this, "Max", ValueMax);
  
  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::MXmlNode(MXmlNode* MotherNode, const MString& Name, const MTime& ValueMin, const MTime& ValueMax)
{
  //! Constructor

  MString Temp;

  m_Name = Name;
  
  new MXmlNode(this, "Min", ValueMin);
  new MXmlNode(this, "Max", ValueMax);
  
  if (MotherNode != 0) {
    MotherNode->AddNode(this);
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode::~MXmlNode()
{
  // Delete this instance of MXmlNode

  for (unsigned int n = 0; n < m_Nodes.size(); ++n) {
    delete m_Nodes[n];
  }
  for (unsigned int a = 0; a < m_Attributes.size(); ++a) {
    delete m_Attributes[a];
  }
}


////////////////////////////////////////////////////////////////////////////////


void MXmlNode::Clear()
{
  // Reset the node

  MXmlData::Clear();
  m_Nodes.clear();
  m_Attributes.clear();
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MXmlNode::GetNode(unsigned int i)
{
  //! Return a given node
  if (i < m_Nodes.size()) {
    return m_Nodes.at(i);
  } else {
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MXmlNode::GetNode(const MString& Name)
{
  //! Return a node

  for (unsigned int m = 0; m < m_Nodes.size(); ++m) {
    if (m_Nodes[m]->GetName() == Name) {
      return m_Nodes[m];      
    }
  }
 
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MXmlNode::AddNode(MXmlNode* Node)
{
  //! Add a node
  m_Value = "";
  return m_Nodes.push_back(Node);
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute* MXmlNode::GetAttribute(unsigned int i)
{
  //! Return a given attribute
  if (i < m_Attributes.size()) {
    return m_Attributes.at(i);
  } else {
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MXmlAttribute* MXmlNode::GetAttribute(const MString& Name)
{
  //! Return a attribute

  for (unsigned int m = 0; m < m_Attributes.size(); ++m) {
    if (m_Attributes[m]->GetName() == Name) {
      return m_Attributes[m];      
    }
  }
 
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MXmlNode::AddAttribute(MXmlAttribute* Attribute)
{
  //! Add a attribute
  return m_Attributes.push_back(Attribute);
}


////////////////////////////////////////////////////////////////////////////////


MVector MXmlNode::GetValueAsVector() const
{
  // Return as a vector

  bool IsGood = false;
  MVector Value;
  
  if (m_Nodes.size() == 3) {
    if (m_Nodes[0]->GetName() == "X" && m_Nodes[1]->GetName() == "Y" && m_Nodes[2]->GetName() == "Z") {
      Value.SetX(m_Nodes[0]->GetValueAsDouble());
      Value.SetY(m_Nodes[1]->GetValueAsDouble());
      Value.SetZ(m_Nodes[2]->GetValueAsDouble());
      IsGood = true;
    }
  }

  if (IsGood == false) {
    mout<<"Xml: Value error in node \""<<m_Name<<"\" --- Value is not a vector!"<<endl;
  }

  return Value;
}


////////////////////////////////////////////////////////////////////////////////


int MXmlNode::GetMinValueAsInt() const
{
  //! Return the minimum value of the node as int

  if (m_Nodes.size() == 2) {
    if (m_Nodes[0]->GetName() == "Min" && m_Nodes[1]->GetName() == "Max") {
      return m_Nodes[0]->GetValueAsInt();
    }
  }

  mout<<"Xml: Value error in node \""<<m_Name<<"\" --- Value is not a min/max node!"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


int MXmlNode::GetMaxValueAsInt() const
{
  //! Return the minimum value of the node as int

  if (m_Nodes.size() == 2) {
    if (m_Nodes[0]->GetName() == "Min" && m_Nodes[1]->GetName() == "Max") {
      return m_Nodes[1]->GetValueAsInt();
    }
  }

  mout<<"Xml: Value error in node \""<<m_Name<<"\" --- Value is not a min/max node!"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


long MXmlNode::GetMinValueAsLong() const
{
  //! Return the minimum value of the node as long

  if (m_Nodes.size() == 2) {
    if (m_Nodes[0]->GetName() == "Min" && m_Nodes[1]->GetName() == "Max") {
      return m_Nodes[0]->GetValueAsLong();
    }
  }

  mout<<"Xml: Value error in node \""<<m_Name<<"\" --- Value is not a min/max node!"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


long MXmlNode::GetMaxValueAsLong() const
{
  //! Return the minimum value of the node as long

  if (m_Nodes.size() == 2) {
    if (m_Nodes[0]->GetName() == "Min" && m_Nodes[1]->GetName() == "Max") {
      return m_Nodes[1]->GetValueAsLong();
    }
  }

  mout<<"Xml: Value error in node \""<<m_Name<<"\" --- Value is not a min/max node!"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


double MXmlNode::GetMinValueAsDouble() const
{
  //! Return the value of the node as double

  if (m_Nodes.size() == 2) {
    if (m_Nodes[0]->GetName() == "Min" && m_Nodes[1]->GetName() == "Max") {
      return m_Nodes[0]->GetValueAsDouble();
    }
  }

  mout<<"Xml: Value error in node \""<<m_Name<<"\" --- Value is not a min/max node!"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


double MXmlNode::GetMaxValueAsDouble() const
{
  //! Return the value of the node as double

  if (m_Nodes.size() == 2) {
    if (m_Nodes[0]->GetName() == "Min" && m_Nodes[1]->GetName() == "Max") {
      return m_Nodes[1]->GetValueAsDouble();
    }
  }

  mout<<"Xml: Value error in node \""<<m_Name<<"\" --- Value is not a min/max node!"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MTime MXmlNode::GetMinValueAsTime() const
{
  //! Return the value of the node as time

  if (m_Nodes.size() == 2) {
    if (m_Nodes[0]->GetName() == "Min" && m_Nodes[1]->GetName() == "Max") {
      return m_Nodes[0]->GetValueAsTime();
    }
  }

  mout<<"Xml: Value error in node \""<<m_Name<<"\" --- Value is not a min/max node!"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MTime MXmlNode::GetMaxValueAsTime() const
{
  //! Return the value of the node as double

  if (m_Nodes.size() == 2) {
    if (m_Nodes[0]->GetName() == "Min" && m_Nodes[1]->GetName() == "Max") {
      return m_Nodes[1]->GetValueAsTime();
    }
  }

  mout<<"Xml: Value error in node \""<<m_Name<<"\" --- Value is not a min/max node!"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MString MXmlNode::ToString(unsigned int Indent)
{
  //! Returns the XML text

  MString Ind;
  for (unsigned int i = 0; i < Indent; ++i) Ind += " ";

  MString Xml;
  Xml += Ind;
  Xml += "<";
  Xml += m_Name;
  for (unsigned int a = 0; a < m_Attributes.size(); ++a) {
    Xml += " ";
    Xml += m_Attributes[a]->ToString();
  }
  if (m_Nodes.size() == 0 && m_Value == "") {
    Xml += " />";    
  } else {
    Xml += ">";
    if (m_Nodes.size() > 0) {
      for (unsigned int n = 0; n < m_Nodes.size(); ++n) {
        Xml += "\n";
        Xml += m_Nodes[n]->ToString(Indent+2);
      }
      Xml += "\n";
      Xml += Ind;
    } else {
      Xml += m_Value;
    } 
    Xml += "</";
    Xml += m_Name;
    Xml += ">";
  }

  return Xml;
}


////////////////////////////////////////////////////////////////////////////////


bool MXmlNode::IsClosed(MString Text)
{
  //! Parse text into this node

  if (Text.Length() >= 2 && Text[0] == '<' && Text[Text.Length()-1] == '>') {
  
    size_t FirstBegin = Text.Index("<", 0);
    size_t FirstEnd = Text.Index(">", 0);
  
    // Check for comments "<!--"
    if (Text[1] == '!' && Text.Length() >= 4 && Text[2] == '-' && Text[3] == '-') {
      FirstEnd = Text.Index("-->", 0);
      Text = Text.Replace(FirstBegin, FirstEnd-FirstBegin+3, "");
      return false;
    }

    // Check for specials "<?"
    if (Text[1] == '?') {
      FirstEnd = Text.Index("?>", 0);
      Text = Text.Replace(FirstBegin, FirstEnd-FirstBegin+2, "");
      return false;
    }

    MString Name = Text.GetSubString(FirstBegin+1, FirstEnd-FirstBegin-1);
    Name.Strip();
    // Check if it is an empty node
    size_t Empty = Name.Index("/", 0);
    // Make sure "/" is really the last entry (has to be after the above strip) and not part of some other text
    if (Empty != MString::npos) {
      if (Empty != Name.Length()-1) Empty = MString::npos;
    }
   
    if (Empty != MString::npos) return true;
  }
  
  return false;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MXmlNode::Parse(MString Text)
{
  //! Parse text into this node

  Text = Text.Strip();
  
  // For all nodes with:
  while (Text.Length() >= 2 && Text[0] == '<' && Text[Text.Length()-1] == '>') {
    size_t FirstBegin = Text.Index("<", 0);
    size_t FirstEnd = Text.Index(">", 0);

    // Check for comments "<!--"
    if (Text[1] == '!' && Text.Length() >= 4 && Text[2] == '-' && Text[3] == '-') {
      FirstEnd = Text.Index("-->", 0);
      Text = Text.Replace(FirstBegin, FirstEnd-FirstBegin+3, "");
      continue;
    }

    // Check for specials "<?"
    if (Text[1] == '?') {
      FirstEnd = Text.Index("?>", 0);
      Text = Text.Replace(FirstBegin, FirstEnd-FirstBegin+2, "");
      continue;
    }

    MString Name = Text.GetSubString(FirstBegin+1, FirstEnd-FirstBegin-1);
    Name.Strip();
    
    // Check if it is an empty node
    bool Empty = IsClosed(Text);

    // Make sure to take care of attributes
    size_t FirstAttribute = Name.Index(" ", 0);
    MString Attributes("");
    if (FirstAttribute != MString::npos) {
      Attributes = Name.GetSubString(FirstAttribute+1, Name.Length() - FirstAttribute - 1);
      Name = Name.GetSubString(0, FirstAttribute);
    } 
    if (Name.Length() == 0) {
      merr<<"Xml: Parse error in node \""<<m_Name<<"\" --- empty name"<<endl;
      return false;
    }
    
    // Create a new node and start the game again:
    MXmlNode* Node = new MXmlNode(this, Name);

    // Parse its content if there is any:
    if (Empty == false) {
      // Find the next </Name>, but take care of cases of nested <Name><Name></Name></Name>
      unsigned int NBegins = 1;
      unsigned int NEnds = 0;
      size_t LastBegin = FirstEnd;
      size_t SecondBegin = FirstEnd;
      while (true) {
        SecondBegin = Text.Index(MString("<") + Name + MString(" "), SecondBegin+1);
        if (SecondBegin != MString::npos) {
          if (IsClosed(Text.GetSubString(SecondBegin)) == false) break;
        } else {
          break;
        }
      }
      
      while (NBegins != NEnds && LastBegin != MString::npos) {
        LastBegin = Text.Index(MString("</") + Name + MString(">"), LastBegin+1); 
        if (LastBegin != MString::npos) ++NEnds;
        while (SecondBegin < LastBegin) {
          ++NBegins;
          while (true) {
            SecondBegin = Text.Index(MString("<") + Name + MString(" "), SecondBegin+1);
            if (SecondBegin != MString::npos) {
              if (IsClosed(Text.GetSubString(SecondBegin)) == false) break;
            } else {
              break;
            }
          }
        }
      }
      
      size_t LastSize = (MString("</") + Name + MString(">")).Length();
            
      if (LastBegin == MString::npos) {
        merr<<"Xml: Parse error in node \""<<m_Name<<"\" with "<<Attributes<<" --- cannot find: "<<MString("</") + Name + MString(">")<<endl;
        return false;
      }
    
      // And parse the new node
      Node->Parse(Text.GetSubString(FirstEnd+1, LastBegin-FirstEnd-1));
      Text = Text.Replace(0, LastBegin+LastSize, "");
      
    } else {
      // It's empty (<Name />) thus we only eliminated this tag
      Text = Text.Replace(0, FirstEnd+1, "");
    }
    Text = Text.Strip();


    // Parse its attributes:
    Attributes = Attributes.Strip();
    size_t Equal = MString::npos;
    while ((Equal = Attributes.Index("=")) != MString::npos) {
      MString AttributeName = Attributes.GetSubString(0, Equal);
      AttributeName = AttributeName.Strip();
      
      size_t FirstQuote = Attributes.Index("\"");
      if (FirstQuote == MString::npos) {
        merr<<"Xml: Parse error in node \""<<m_Name<<"\" --- attribute not well formed"<<endl;
        return false;
      }
      size_t SecondQuote = Attributes.Index("\"", FirstQuote+1);
      if (SecondQuote == MString::npos) {
        merr<<"Xml: Parse error in node \""<<m_Name<<"\" --- attribute not well formed"<<endl;
        return false;
      }
      MString AttributeValue = Attributes.GetSubString(FirstQuote+1, SecondQuote-FirstQuote-1);
      
      new MXmlAttribute(Node, AttributeName, AttributeValue);
      
      Attributes = Attributes.GetSubString(SecondQuote+1, Attributes.Length() - SecondQuote-1);
      Attributes = Attributes.Strip();
    }
  } // all nodes loop

  // Now only the pure text content is left - the "value"
  if (Text.Length() > 0) {
    if (m_Nodes.size() > 0) {
      merr<<"Xml: Parse error in node \""<<m_Name<<"\" --- node has text and sub-nodes. Remaining text: "<<Text<<endl;
      return false;      
    } else {
      m_Value = Text;
    }
  }

  return true;
}


// MXmlNode.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
