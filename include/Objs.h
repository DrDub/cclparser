#ifndef __OBJS_H__
#define __OBJS_H__

// Copyright 2007 Yoav Seginer

// This file is part of CCL-Parser.
// CCL-Parser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// CCL-Parser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with CCL-Parser.  If not, see <http://www.gnu.org/licenses/>.

#include <string>
#include <list>
#include <map>
#include <ostream>
#include "Reference.h"
#include "Token.h"

class CObj;
typedef CPtr<CObj> CpCObj;

// An object has a name, attributes and sub-objects. When an object has no
// attributes and no sub-objects it is equivalent to a string (it's name) 

class CObj : public CRef
{
private:
    std::string m_Name;                             // object name
    std::map<std::string, std::string> m_Attr;      // attribute list
    std::list<CpCObj> m_SubObjs;                  // sub-objects
public:
    CObj() {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }

    CObj(std::string Name) : m_Name(Name) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif  
    }
    
    ~CObj() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

    void SetName(std::string& Name) { m_Name = Name; }
    void SetAttr(std::string& Name, std::string& Val) {
        m_Attr[Name] = Val;
    }
    void AppendObj(CObj* pObj) { m_SubObjs.push_back(pObj); }

    std::string& Name() { return m_Name; }
    unsigned int SubObjNum() { return m_SubObjs.size(); }
    std::list<CpCObj>::iterator SubObjBegin() { return m_SubObjs.begin(); }
    std::list<CpCObj>::iterator SubObjEnd() { return m_SubObjs.end(); }
    std::list<CpCObj>::reverse_iterator SubObjRBegin() {
        return m_SubObjs.rbegin();
    }
    std::list<CpCObj>::reverse_iterator SubObjREnd() {
        return m_SubObjs.rend();
    }

    
    // Prints the object to the stream
    void Print(std::ostream& Out,  unsigned int Indent = 0);
};

// Parsing object

class CObjParse;
typedef CPtr<CObjParse> CpCObjParse;

class CObjParse : public CRef
{
public:
    enum eState {
        // Look for head or foot opening or a bare name (ended by head or
        // foot opening)
        eBegin,

        // Check what of the three posibilities above was matched
        eBeginDecide,

        // if head or foot opening was found and both are the same,
        // determine whether this is a head or a foot based on the following
        // character

        eDecideHeadOrFoot,

        // If a foot was found (at the beginning) this must be the
        // foot of a higher level object. This is no object at all.

        eFootHigherObj,
        
        // if head opening found:

        eName,     // looking for name (terminated by white space,
                   // head opening/closing or foot opening)

        // after name in objects with attributes (and head closing)
        eAttrName, // looking for attribute name or head closing
        eAttrSep,  // looking for separator between attribute name and val
        eAttrVal,  // reading attr. value (head closing also terminator)

        // after name in objects without head attributes or after
        // head closing we read the body of the object (which repeatedly
        // tries to read lower level objects unit a eFootHigherObj is
        // returned)
        eBody,
        
        // if the foot has a name, read it (if the foot does not have
        // a name, then it has already been completely read by the
        // sub-object and the object is complete)

        eFootName,

        // Finally, after the foot name, have to extract the foot close

        eFootClose,

        // General state flags
        eComplete,   // object completed
        eError       // error while reading object
    };
private:

    // parse definition

    std::string m_HeadOpen;  // character opening the head (e.g. '(' or '<')
    std::string m_HeadClose; // character closing the head (e.g. '>' or none)
    std::string m_FootOpen;  // character opening the foot (e.g. '<' or ')')
    std::string m_FootClose; // character closing the foot (e.g. '>' or none)
    std::string m_HeadFootDecide; // if head and foot opening is the same,
                                  // following character should determine
    std::string m_AttrSep;   // separator between attr. name and val (e.g. '=')
    std::string m_StrDelim;  // string delimiters

    // Current state of parsing (from list above)
    eState m_State;

    CToken m_Token; // token currently being read
    CpCObj m_pObj;  // object currently under construction
    std::string m_AttrName; // attribute name read waiting for a value
    CpCObjParse m_pSubObjParse; // currently active sub-object parsing

    std::string m_ErrorMsg;  // error message string
    
public:
    
    CObjParse(std::string const& HeadOpen, std::string const& HeadClose,
              std::string const& FootOpen, std::string const& FootClose,
              std::string const& HeadFootDecide, std::string const& AttrSep,
              std::string const& StrDelim);

    ~CObjParse() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
    
private:    
    // Reads/continues to read the object from the input string (beginning
    // at Pos). Returns in Pos the first position in Input beyond the object.
    // returns false on error.
    bool Parse(std::string::size_type& Pos, std::string const& Input);
public:
    // Parse interface. This routine calls parse. As it always constructs
    // the top level object, it returns an error in case Parse returns
    // in a eFootHigherObj state.
    bool ParseObj(std::string::size_type& Pos, std::string const& Input);

    // resets the object for a new parse
    void NewParse();
    
    bool IsComplete() { return (m_State == eComplete); }
    bool IsStarted() {
        return ((m_State != eBegin) && (m_State != eBeginDecide));
    }
    bool IsError() { return (m_State == eError); }
    eState State() { return m_State; }
    
    std::string& ErrorMsg() { return m_ErrorMsg; }
    CObj* GetObj() { return (CObj*)m_pObj; }
};

// parse objects for specific types of representations

// XML

class CXMLObjParse : public CObjParse
{
public:
    CXMLObjParse();
    
    ~CXMLObjParse() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
};

// Penn treebank representation

class CPennTBObjParse : public CObjParse
{
public:
    CPennTBObjParse();
    
    ~CPennTBObjParse() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
};

#endif /* __OBJS_H__ */
