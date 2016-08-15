#ifndef __SYMBOL_H__
#define __SYMBOL_H__

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

#include "Reference.h"
#include "PrintUtils.h"
#include <ostream>

// The symbol class is a base class for both unit classes
// which describes content units of the utterance and the punctuation class
// which describes punctuation which appears along the utterance.

enum eSymbolType {
    eUnit,
    ePunct
};

class CSymbol : public CRef, public CPrintObj
{
public:
    CSymbol() {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }

    ~CSymbol() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

    // function to determine the type of this symbol
    virtual eSymbolType SymbolType() = 0;

    // Printing routines

    // Generic printing routine (from CPrintObj). Here we ignore the
    // indentation, format and parameter.
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter) {
        if(pOut)
            PrintSymbol((std::ostream&)*pOut);
    }
    
    // Prints the symbol to the given stream (for debugging)
    virtual void PrintSymbol(std::ostream& Out) = 0;
};

typedef CPtr<CSymbol> CpCSymbol;

#endif /* __SYMBOL_H__ */
