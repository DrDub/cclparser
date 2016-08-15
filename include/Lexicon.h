#ifndef __LEXICON_H__
#define __LEXICON_H__

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

#include <utility>
#include "RefStream.h"
#include "PrintUtils.h"
#include "Hash.h"
#include "HashKey.h"

#define LEX_DEFAULT_HASH_SIZE 4096 // lots of words

//
// Generic lexicon interface
//

class CLexicon
{
public:
    CLexicon() {}
    // lexicon printing routine
    virtual bool PrintLexicon(CRefOStream* pOut) = 0;
};

//
// Generic lexical entry
//

class CLexEntry : public CRef, public CPrintObj
{
public:
    // Returns the count of this entry. If this is not defined, returns -1. 
    virtual int Count() = 0;
};

typedef CPtr<CLexEntry> CpCLexEntry;

//
// Base class for lexicons with string keys. This class implements all
// functions which do not need to know the contents of the value stored
// for each key.
// This class is a abstrct class.
//

// type definitions for hash table
typedef CHash<CStrKey, CLexEntry> CLexHash;
typedef CHashIter<CStrKey, CLexEntry> CLexIter;
typedef CPtr<CLexIter> CpCLexIter;

// type definitions for sorting
typedef std::pair<CpCStrKey, CpCLexEntry> LexPair; // key/value pairs
typedef bool(*tLexComp)(LexPair const&, LexPair const&);

class CStrLexicon : public CLexicon, public CLexHash 
{
public:
    CStrLexicon();
    virtual ~CStrLexicon();
    // Get the key of the entry matching the given string. If no entry is
    // found, an empty entry is created. This ensures that only one copy
    // of the key is created.
    CStrKey* GetKeyByString(std::string const& Name);
protected:
    // For use by derived classes only
    CStrKey* GetEntryByString(std::string const& Name, CpCLexEntry& pEntry);
    // Sorted printing of the lexicon. Which entries are printed and in
    // which order is determined by the derived class.
    bool PrintLexicon(CRefOStream* pOut);
    
private:
    
    //
    // Functions which need to be implemented by the derived classes
    //

    // Returns pointer to entry sorting function (defined in derived class) 
    virtual tLexComp PrintComp() = 0;
    // Returns a lexical pair which defines the minimal entry to print
    // (defined in the derived class)
    virtual LexPair const& PrintBound() = 0;
    
    // returns a new (empty) lexical entry
    virtual CLexEntry* NewEmptyLexEntry() = 0;
};

#endif /* __LEXICON_H__ */
