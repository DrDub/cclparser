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

#include <algorithm>
#include "Lexicon.h"
#include "ListPrint.h"

using namespace std;

/////////////////////////////
// Base String Key Lexicon //
/////////////////////////////

CStrLexicon::CStrLexicon() : CLexHash(LEX_DEFAULT_HASH_SIZE)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CStrLexicon::~CStrLexicon()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

CStrKey*
CStrLexicon::GetKeyByString(string const& Name)
{
    if(Name == "")
        return NULL;
    
    CpCStrKey pName = new CStrKey(Name); 

    // Is the name in the lexicon ?
    
    if(!(*this)[*pName].Found()) {
        // name not in lexicon, create new entry
        Insert(NewEmptyLexEntry());
    }

    // return the key stored in lexicon
    return Key();
}

CStrKey*
CStrLexicon::GetEntryByString(std::string const& Name, CpCLexEntry& pEntry)
{
    if(!GetKeyByString(Name)) {
        pEntry = NULL;
        return NULL;
    } else {
        pEntry = Val();
        // return the key stored in lexicon
        return Key();
    }
}

// sorted printing of the lexicon, with lower bound

bool
CStrLexicon::PrintLexicon(CRefOStream* pOut)
{
    LexPair Array[NumElements()];
    LexPair* Current = Array;
    
    CpCLexIter Iter = Begin();

    // populate the array
    while(*Iter) {
        (*Current).first = Iter->GetKey();
        (*Current).second = Iter->GetVal();
        ++(*Iter);
        ++Current;
    }

    sort(Array, Array+NumElements(), PrintComp());

    CListPrint LexList(pOut, 0, 0, 1, 0);
    
    for(Current = Array ; Current < Array+NumElements() ; Current++) {
        if(PrintComp()(PrintBound(), *Current))
            break;

        LexList << "\"" << ((*Current).first->GetStr()) << "\" ";
        if((*Current).second->Count() >= 0)
            LexList << "(" << ((*Current).second->Count()) << ")";

        LexList.PrintNextEntry((*Current).second.Ptr(),0);
    }

    LexList.CloseList();

    return true;
}

