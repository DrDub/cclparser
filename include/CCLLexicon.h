#ifndef __CCLLEXICON_H__
#define __CCLLEXICON_H__

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
#include "PrsConst.h"
#include "CCLStat.h"
#include "Lexicon.h"

// The lexical entry for the CCL parser

class CCCLLexEntry : public CLexEntry
{
private:
    // A pair of statistics tables (left and right)
    CTwoCCLStats m_Stats;
    int m_Count; // number of times this word was seen
public:
    CCCLLexEntry(unsigned int InitialCount = 0);
    ~CCCLLexEntry();
    void IncCount(unsigned int Inc = 1) { m_Count += Inc; }
    int Count() { return m_Count; }
    CTwoCCLStats const& GetCCLStats() { return m_Stats; }

    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format, int Parameter);
};

typedef CPtr<CCCLLexEntry> CpCCCLLexEntry;

class CCCLLexicon : public CStrLexicon
{
private:
    LexPair m_PrintBound;
public:
    CCCLLexicon();
    ~CCCLLexicon();
private:
    // returns the comparison function for sorted printing
    tLexComp PrintComp();
    // returns the lower bound for printing
    LexPair const& PrintBound();
    // returns a new (empty) lexical entry
    CLexEntry* NewEmptyLexEntry();
public:
    CStrKey* GetEntryByString(std::string const& Name, CpCCCLLexEntry& pEntry);
};

typedef CPtr<CCCLLexicon> CpCCCLLexicon;

#endif /* __CCLLEXICON_H__ */
