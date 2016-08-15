#ifndef __PUNCT_H__
#define __PUNCT_H__

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
#include "Symbol.h"

enum ePunctType {
    eNoPunct = 0x0,
    eEoUtterance = 0x1, // indicator of end of utterance (not real punct.) 
    eFullStop = 0x2,
    eQuestion = 0x4,
    eExclamation = 0x8,
    eSemiColon = 0x10,
    eDash = 0x20,
    eColon = 0x40,
    eComma = 0x80,
    eEllipsis = 0x100,
    eParenthesis = 0x200,  // left or right
    eLeftParenthesis = 0x400,
    eRightParenthesis = 0x800,
    eDoubleQuote = 0x1000,  // left or right
    eLeftDoubleQuote = 0x2000,
    eRightDoubleQuote = 0x4000,
    eSingleQuote = 0x8000, // left or right
    eLeftSingleQuote = 0x10000,
    eRightSingleQuote = 0x20000,
    eHyphen = 0x40000,
    eHeadlineStop = 0x80000, // probably end of newspaper headline / title
    eCurrency = 0x100000
};

class CPunct : public CSymbol
{
private:
    ePunctType m_Type;

public:
    CPunct(ePunctType PunctType) : m_Type(PunctType) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }

    ~CPunct() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

    // function to determine the type of this symbol
    eSymbolType SymbolType() {return ePunct; }
    ePunctType Type() { return m_Type; }
    // Prints the symbol to the given stream
    void PrintSymbol(std::ostream& Out);
};

typedef CPtr<CPunct> CpCPunct;

// Returns true if the punctuation symbol terminates the utterance.
// Currently, only the eEoUtterance special punctuation indicates the
// end of the utterance.
extern bool IsEoUPunct(ePunctType Punct);

// Returns true if the punctuation is terminating punctuation
// (full stop, question mark, exclamation)
extern bool IsTerminatingPunct(ePunctType Punct);

// Returns true if the given punctuation symbol is a 'stopping' punctuation
extern bool IsStoppingPunct(ePunctType Punct);

#endif /* __PUNCT_H__ */
