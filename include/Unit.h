#ifndef __UNIT_H__
#define __UNIT_H__

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

#include "RefSTL.h"
#include "Symbol.h"
#include "HashKey.h"
#include "PrintUtils.h"

class CUnit : public CSymbol
{
private:
    CpCStrKey m_pName;         // the name (currently, word) of this unit
public:
    // A unit can be created based on a string or a string key. If a
    // string key is given, there is no need to duplicate the key
    // which means that if the word is used to create entries in the lexicon,
    // all these entries will use the same key (thus saving memory).
    CUnit(std::string const& Name);
    CUnit(CStrKey* pName);
    ~CUnit();

    eSymbolType SymbolType() { return eUnit; }
    
    // return the name of th unit
    operator char const*() { return m_pName ? (char const*)*m_pName : NULL; }
    CStrKey* GetName() { return m_pName; }
    
    // Prints the symbol to the given stream
    void PrintSymbol(std::ostream& Out);
    // Print the object
    virtual void PrintObj(CRefOStream* pOut, unsigned int Indent,
                          unsigned int SubIndent, eFormat Format,
                          int Parameter);
};

typedef CPtr<CUnit> CpCUnit;

// vector of units
typedef CRvector<CpCUnit> CUnitVector;
typedef CPtr<CUnitVector> CpCUnitVector;

#endif /* __UNIT_H__ */
