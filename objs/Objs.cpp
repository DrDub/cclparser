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

#include "Objs.h"
#include "Token.h"
#include "StringUtil.h"
#include "yError.h"

using namespace std;

/////////////////////
// Object routines //
/////////////////////

void
CObj::Print(ostream& Out, unsigned int Indent)
{
    string Tabs(Indent, '\t');
    
    Out << Tabs << ":" << m_Name;

    if(m_Attr.size() || m_SubObjs.size())
        Out << " (" << endl;
    else {
        Out << endl;
        return;
    }

    for(map<string, string>::iterator Iter = m_Attr.begin() ;
        Iter != m_Attr.end() ; Iter++) {
        Out << Tabs << "\t" << Iter->first << " = " << Iter->second << endl;
    }

    for(list<CpCObj>::iterator Iter = m_SubObjs.begin() ;
        Iter != m_SubObjs.end() ; Iter++) {
        
        if(!(*Iter))
            continue;
        
        (*Iter)->Print(Out, Indent+1);
    }

    if(m_Attr.size() || m_SubObjs.size())
        Out << Tabs << ")" << endl;
}


////////////////////
// Object Parsing //
////////////////////

CObjParse::CObjParse(string const& HeadOpen, string const& HeadClose,
                     string const& FootOpen, string const& FootClose,
                     string const& HeadFootDecide, string const& AttrSep,
                     string const& StrDelim) :
        m_HeadOpen(HeadOpen), m_HeadClose(HeadClose),
        m_FootOpen(FootOpen), m_FootClose(FootClose),
        m_HeadFootDecide(HeadFootDecide), m_AttrSep(AttrSep),
        m_StrDelim(StrDelim), m_State(eBegin)
{
    static char Rname[] = "CObjParse::CObjParse";
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    if(!m_HeadOpen.size() || !m_FootOpen.size() ||
       (m_HeadClose.size() && !m_AttrSep.size())) {
        yPError(ERR_INIT, "incorrect initialization of parse object");
    }
}

bool
CObjParse::Parse(string::size_type& Pos, std::string const& Input)
{
    while(1) {
        
        string::size_type InPos = Pos;
    
        // if a sub-object is being parsed, call it's parse function
        if(m_pSubObjParse && !m_pSubObjParse->IsComplete()) {
        
            m_pSubObjParse->Parse(Pos, Input);
            
            if(m_pSubObjParse->IsError()) {
                m_ErrorMsg = m_pSubObjParse->ErrorMsg();
                m_State = eError;
                return false;
            }

            if(m_pSubObjParse->State() == eFootHigherObj) {
                // should not have called the sub-object, this is actually
                // the foot of this object
                m_pSubObjParse = NULL;
                // if there is a foot close, then read the foot name,
                // otherwise, this object is complete
                if(m_FootClose.size()) {
                    m_Token.SetAndStart("", m_StrDelim, m_StrDelim,
                                        m_FootClose);
                    m_State = eFootName;
                    continue;
                } else {
                    m_State = eComplete;
                    return true;
                }
            } else {
            
                if(!m_pSubObjParse->IsComplete())
                    return true;

                // completed reading the sub-object. The state remains the
                // same (we continue to read sub objects). Append the
                // sub-object to the current object

                if(!m_pObj) {
                    yPError(ERR_SHOULDNT,
                            "constructed sub-object, but have no object");
                }

                m_pObj->AppendObj(m_pSubObjParse->GetObj());
                m_pSubObjParse = NULL;
            }
            
        } else if(m_Token.Started() && !m_Token.Complete()) {
            // a token is being read - continue reading it
            m_Token.ReadToken(Pos, Input);
            
            if(!m_Token.Complete())
                return true;
        }

        // deterine which token/sub-object needs to be read now
    
        switch(m_State) {
            case eBegin:
                m_Token.SetAndStart(m_HeadOpen+m_FootOpen, m_StrDelim,
                                    m_StrDelim, m_HeadOpen+m_FootOpen, false);
                m_State = eBeginDecide;
                break;
            case eBeginDecide:
                if(m_Token.TokenSymbol()) {
                    
                    bool bHead =
                        (m_HeadOpen.find(m_Token.GetToken(), 0) !=
                         string::npos);
                    bool bFoot =
                        (m_FootOpen.find(m_Token.GetToken(), 0) !=
                         string::npos);
                    
                    if(bHead && bFoot) {
                        m_State = eDecideHeadOrFoot;
                        m_Token.SetAndStart(m_HeadFootDecide, "", "",
                                            "", false);
                    } else if(bHead) {
                        // this is a head, look for the name
                        m_State = eName;
                        m_Token.SetAndStart("", m_StrDelim, m_StrDelim,
                                            m_HeadClose+m_FootOpen+m_HeadOpen);
                    } else if(bFoot) {
                        // foot of higher level object
                        m_State = eFootHigherObj;
                        return true;
                    } else {
                        // shouldn't happen - a token symbol was matched,
                        // but it's not the symbol we were looking for
                        yPError(ERR_SHOULDNT,
                                "token found is not token searched for");
                    }
                    break;
                }
                // otherwise, it's a bare name object
                m_pObj = new CObj(m_Token.GetToken());
                m_State = eComplete;
                return true;
            case eDecideHeadOrFoot:
                if(m_Token.TokenSymbol()) {
                    // this is a foot (of higher level object)
                    m_State = eFootHigherObj;
                    return true;
                }
                // otherwise, it's the head, read its name
                m_Token.SetAndStart("", m_StrDelim, m_StrDelim,
                                    m_HeadOpen+m_HeadClose+m_FootOpen);
                m_State = eName;
                break;
            case eName:
                m_pObj = new CObj(m_Token.GetToken());
                // if there is a head close, read attributes (or head close)
                if(m_HeadClose.length()) {
                    m_Token.SetAndStart(m_HeadClose, m_StrDelim,
                                        m_StrDelim, m_AttrSep);
                    m_State = eAttrName;
                } else {
                    m_pSubObjParse = new CObjParse(m_HeadOpen, m_HeadClose,
                                                   m_FootOpen, m_FootClose,
                                                   m_HeadFootDecide,
                                                   m_AttrSep, m_StrDelim);
                    m_State = eBody;
                }
                break;
            case eAttrName:
                if(m_Token.TokenSymbol()) {
                    // head close, continue to read body
                    m_pSubObjParse = new CObjParse(m_HeadOpen, m_HeadClose,
                                                   m_FootOpen, m_FootClose,
                                                   m_HeadFootDecide,
                                                   m_AttrSep, m_StrDelim);
                    m_State = eBody;
                    break;
                }
                // store the attribute name and check that it is indeed
                // followed by the attribute separator
                m_AttrName = m_Token.GetToken();
                m_Token.SetAndStart(m_AttrSep, "", "", "", false);
                m_State = eAttrSep;
                break;
            case eAttrSep:
                if(!m_Token.TokenSymbol()) {
                    // separator not found, this is an error
                    m_State = eError;
                    m_ErrorMsg = "no attribute separator after attribute name";
                    return false;
                }
                // read the attribute value
                m_Token.SetAndStart("", m_StrDelim,
                                    m_StrDelim, m_HeadClose);
                m_State = eAttrVal;
                break;
            case eAttrVal:
                // store the head/attribute pair
                m_pObj->SetAttr(m_AttrName, m_Token.GetToken());
                // read the next attribute (or head close)
                m_Token.SetAndStart(m_HeadClose, m_StrDelim,
                                    m_StrDelim, m_AttrSep);
                m_State = eAttrName;
                break;
            case eBody:
                // try to read teh next sub-object
                m_pSubObjParse = new CObjParse(m_HeadOpen, m_HeadClose,
                                               m_FootOpen, m_FootClose,
                                               m_HeadFootDecide,
                                               m_AttrSep, m_StrDelim);
                m_State = eBody;
                break;
            case eFootName:
                if(m_pObj->Name() != m_Token.GetToken()) {
                    // foot name does not match head name
                    m_State = eError;
                    m_ErrorMsg = "foot name '" +  m_Token.GetToken() +
                        "' does not match head name '" + m_pObj->Name() + "'";
                    return false;
                }
                // look for foot close
                m_Token.SetAndStart(m_FootClose, "", "", "", false);
                m_State = eFootClose;
                break;
            case eFootClose:
                if(!m_Token.TokenSymbol()) {
                    // foot close not found, this is an error
                    m_State = eError;
                    m_ErrorMsg = "foot close missing";
                    return false;
                }
                
                // object is complete
                m_State = eComplete;
                return true;
            case eComplete:
                return true;
            case eError:
                return false;
            default:
                yPError(ERR_OUT_OF_RANGE, "unknown state");
        }
    }
}

bool
CObjParse::ParseObj(string::size_type& Pos, std::string const& Input)
{
    if(!Parse(Pos, Input))
        return false;

    if(m_State == eFootHigherObj) {
        // this is an error at the top level
        m_State = eError;
        m_ErrorMsg = "object begins with foot";
        return false;
    }

    return true;
}

void
CObjParse::NewParse()
{
    m_State = eBegin;
    m_Token.Stop();
    m_pObj = NULL;
    m_AttrName = "";
    m_pSubObjParse = NULL;
    m_ErrorMsg = "";
}

////////////////////////////
// Specific parse objects //
////////////////////////////

// XML objects

CXMLObjParse::CXMLObjParse() :
        CObjParse("<", ">", "<", ">", "/", "=", "\"")
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

// Penn treebank

CPennTBObjParse::CPennTBObjParse() :
        CObjParse("(", "", ")", "", "", "", "")
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}
