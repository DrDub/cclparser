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

#include "PennSynStruct.h"
#include "yError.h"
#include "Globals.h"

using namespace std;

// Appends the segment given by the object's nodes to the current
// syntactic structure. It is assumed that the current syntactic structure
// holds the structure of the prefix preceding this segment.
// The function returns the number of the node at the top of the
// structure (dominating all the rest). Returns CSynStruct::NoNode
// in case of an error.
//
// If bReverse is true, the function reads the nodes from right to left
// thus reversing the structure.

int
CPennSynStruct::AppendSynStruct(CObj* pObj, bool bReverse)
{
    static char Rname[] = "CPennSynStruct::AppendSynStruct";
    
    if(!pObj) {
        yPError(ERR_MISSING, "object missing");
    }

    if(!pObj->SubObjNum()) {
        // should not happen if the object is well formed
        SetError("arrived at leaf node unexpectedly");
        return CSynStruct::NoNode;
    }
    
    if((pObj->SubObjNum() == 1) && !(*(pObj->SubObjBegin()))->SubObjNum()) {
        // This object has a single sub-object which is a bare word.
        // Therefore, this is a terminal.
        return AddTerminal((*(pObj->SubObjBegin()))->Name(), pObj->Name(),
                           IsPunct((*(pObj->SubObjBegin()))->Name(),
                                   pObj->Name()),
                           IsEmpty((*(pObj->SubObjBegin()))->Name(),
                                   pObj->Name()));
    }
    
    // Loop over all sub-nodes and create their syntactic structure.
    // Then, create a node to dominate them all.

    vector<int> SubNodes;
    int Pos;

    if(!bReverse) {
        for(std::list<CpCObj>::iterator Iter = pObj->SubObjBegin() ;
            Iter != pObj->SubObjEnd() ; Iter++) {
            if((Pos = AppendSynStruct((CObj*)*Iter, bReverse)) ==
               CSynStruct::NoNode) {
                return CSynStruct::NoNode;
            }
            SubNodes.push_back(Pos);
        }
    } else {
        for(std::list<CpCObj>::reverse_iterator Iter = pObj->SubObjRBegin() ;
            Iter != pObj->SubObjREnd() ; Iter++) {
            if((Pos = AppendSynStruct((CObj*)*Iter, bReverse)) ==
               CSynStruct::NoNode) {
                return CSynStruct::NoNode;
            }
            SubNodes.push_back(Pos);
        }
    }

    return AddNonTerminal(pObj->Name(), SubNodes);
}

CPennSynStruct::CPennSynStruct(CObj* pObj, bool bReverse)
        : CSynStruct('-','-'), m_bError(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

//
// Derived classes for specific corpora
//

// Wall Street Journal Corpus

CWSJPennSynStruct::CWSJPennSynStruct(CObj* pObj, bool bReverse)
        : CPennSynStruct(pObj, bReverse)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    if(!pObj)
        return;

    AppendSynStruct(pObj, bReverse);
}

CWSJPennSynStruct::~CWSJPennSynStruct()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

// Check whether the given unit is punctuation

bool
CWSJPennSynStruct::IsPunct(string const& Unit, string const& Tag)
{
    if(!Tag.length() || isalnum(Tag.at(0)))
        return false; // not punctuation

    if(Tag == "$" || Tag == "#")
        return g_CurrencySymbolIsPunct;
    
    if(Tag == "-NONE-")
        return false; // empty unit, not punctuation

    if(isalnum(Unit.at(0)))
        return false;
    
    return true;  // all the rest is (stopping) punctuation
}

bool
CWSJPennSynStruct::IsEmpty(std::string const& Unit, string const& Tag)
{
    return (Tag == "-NONE-");
}

// Negra Corpus

CNegraPennSynStruct::CNegraPennSynStruct(CObj* pObj, bool bReverse)
        : CPennSynStruct(pObj, bReverse)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    if(!pObj)
        return;

    AppendSynStruct(pObj, bReverse);
}

CNegraPennSynStruct::~CNegraPennSynStruct()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

// Check whether the given unit is punctuation

bool
CNegraPennSynStruct::IsPunct(string const& Unit, string const& Tag)
{
    return (Tag.length() > 0 && Tag.at(0) == '$');
}

bool
CNegraPennSynStruct::IsEmpty(std::string const& Unit, string const& Tag)
{
    return (Tag.length() >= 2 && Tag.at(0) == '*' && Tag.at(1) == 'T' &&
            Unit == "-");
}

// Chinese Treebank

CCTBPennSynStruct::CCTBPennSynStruct(CObj* pObj, bool bReverse)
        : CPennSynStruct(pObj, bReverse)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    if(!pObj)
        return;

    AppendSynStruct(pObj, bReverse);
}

CCTBPennSynStruct::~CCTBPennSynStruct()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

// Check whether the given unit is punctuation

bool
CCTBPennSynStruct::IsPunct(string const& Unit, string const& Tag)
{
    return (Tag.length() > 0 && Tag == "PU");
}

bool
CCTBPennSynStruct::IsEmpty(std::string const& Unit, string const& Tag)
{
    return (Tag == "-NONE-");
}
