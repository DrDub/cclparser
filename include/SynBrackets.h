#ifndef __SYNBRACKETS_H__
#define __SYNBRACKETS_H__

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

#include <map>
#include <string>
#include <vector>
#include <ostream>
#include "Reference.h"
#include "BitMap.h"
#include "SynStruct.h"
#include "PrintUtils.h"

// The class CSynBrackets holds the bracketing structure of a CSynStruct
// object. In this structure, every non-terminal is reduced to the list
// of terminals it dominates (directly or indirectly). Each non-terminal
// is then represented as a bit-map. Optionally, some terminals may be
// ignored (such as empty terminals or punctuation).
// It may happen that several non-terminals in the original structure
// are reduced to the same bit representation here. This is then considered
// a single node.

class CSynBrackets : public CRef, CPrintObj
{
public:
    enum eLabeling {
        eNone, // no bracket label
        eTag,  // (topmost) non-terminal tag of span
        eShortTag, // same as 'eTag' but using only only first part of tag
        eTagSeq, // sequence of terminal tags of covered terminal nodes
        eShortTagSeq, // same as 'eTagSeq' but using only first part of tag
        eLen,    // one dot per terminal covered
    };
protected:
    // table for looking up by bitmap (the string is a name for the bit span)
    std::map<CpCBitMap, std::string> m_NonTerminals;
    // table for looking up by non-teminal code
    std::vector<CpCBitMap> m_BitMaps;
public:
    CSynBrackets(CSynStruct* pStruct, eLabeling Labeling, bool bNoEmpty = true,
                 bool bNoPunct = true, bool bNoTop = true,
                 bool bNoUnary = true);
    ~CSynBrackets() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
    
    // returns the number of brackets in SynBrackets which also appear
    // in 'this'
    unsigned int Precision(CSynBrackets& SynBrackets);
    // Returns (in 'Matched'/'NotMatched') the labels of those brackets on
    // this SynBracket structure matched/not matched on the given
    // SynBracket structure.
    // The function returns the number of brackets matched.
    unsigned int Match(CSynBrackets& SynBrackets,
                       std::vector<std::string>& Matched,
                       std::vector<std::string>& NotMatched);
    
    // returns the numnber of non-terminals
    unsigned int NonTermNum();
    // Return a pointer to the bitmap of a given non-terminal (by node number).
    CBitMap* NonTermByNodeNum(int NodeNum);
    
    // for debugging
    void PrintBrackets(std::ostream& Out, unsigned int Indent);
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
};

typedef CPtr<CSynBrackets> CpCSynBrackets;

#endif /* __SYNBRACKETS_H__ */
