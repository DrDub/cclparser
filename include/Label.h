#ifndef __LABEL_H__
#define __LABEL_H__

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

#include <typeinfo>
#include <string>
#include "Reference.h"
#include "HashKey.h"

///////////////////
// Label classes //
///////////////////

// We distinguish beween labels which originated on "this side"
// of the adjacency and those originating on the "other side" of the
// adjacency. A bit (the side bit) in the label class indicates whether
// the label is "this side" or "other side".

// Label side bit
#define LB_OTHER_SIDE 0x01  // label originates as an adjacent word

// Currently, there is only one type of label object. This 
// consists of a string and a side bit. The copy
// constructor of the label key flips the side bit.
// The CStrKey part of the CLabel may be shared among several
// CLabel objects.

class CLabel : public CKey
{
private:
    unsigned int m_Type;
    CpCStrKey m_StrKey;
public:
    // default is other side empty adjacency label
    CLabel() : m_StrKey(new CStrKey("")), m_Type(LB_OTHER_SIDE) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    CLabel(unsigned int Type, std::string const& s) :
            m_StrKey(new CStrKey(s)), m_Type(Type) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    CLabel(unsigned int Type, char const* s) :
            m_StrKey(new CStrKey(s)), m_Type(Type) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    CLabel(unsigned int Type, CStrKey* pKey) :
            m_StrKey(pKey), m_Type(Type) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    CLabel(unsigned int Type, CLabel& Key) :
            m_StrKey(Key.m_StrKey), m_Type(Type) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    // creates an adjacency label based on a label (flips the side bit)
    CLabel(CLabel& Key) :
            m_StrKey(Key.m_StrKey), m_Type(Key.m_Type ^ 1) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }

    ~CLabel() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
    
    unsigned int HashFunc() {
        return (m_Type ^ m_StrKey->HashFunc());
    }
    bool HashEqual(CKey* pKey) {
        return pKey && (typeid(*pKey) == typeid(*this)) &&
            (m_Type == ((CLabel*)pKey)->m_Type) &&
            (m_StrKey == ((CLabel*)pKey)->m_StrKey ||
             m_StrKey->HashEqual(((CLabel*)pKey)->m_StrKey));
    }
    
    operator CpCStrKey const&() { return m_StrKey; }
    operator char const*() { return m_StrKey->GetStr(); }
    operator std::string const&() { return (std::string const&)(*m_StrKey); }
    operator unsigned int() { return m_Type; }
    bool operator==(CLabel& Label) {
        return (m_Type == Label.m_Type) &&
            ((m_StrKey == Label.m_StrKey) ||
             (m_StrKey && Label.m_StrKey && (*m_StrKey == *(Label.m_StrKey))));
    }

    // Prints the label into the given output string
    void LabelString(std::string& Output);
};

typedef CPtr<CLabel> CpCLabel;

#endif /* __LABEL_H__ */
