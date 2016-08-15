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

#include "Globals.h"
#include "ListPrint.h"
#include "CCLLexicon.h"

using namespace std;

///////////////////
// Lexical Entry //
///////////////////

CCCLLexEntry::CCCLLexEntry(unsigned int InitialCount) :
        m_Count(InitialCount)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    m_Stats[LEFT] = new CCCLStat();
    m_Stats[RIGHT] = new CCCLStat();
}

CCCLLexEntry::~CCCLLexEntry()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CCCLLexEntry::PrintObj(CRefOStream* pOut, unsigned int Indent,
                       unsigned int SubIndent, eFormat Format,
                       int Parameter)
{
    if(!pOut)
        return;

    CListPrint LexEntryList(pOut, Indent, SubIndent, 1, 0);

    // Increase indentation by 1/3 tab
    LexEntryList.IncIndent(1);

    for(unsigned int Side = LEFT ; Side <= RIGHT ; Side++) {
        CpCCCLStat m_pStat = m_Stats[Side];
        unsigned int Count = 1;
        
        while(m_pStat && !m_pStat->IsEmpty()) {
            LexEntryList << (Side == LEFT ? "Left" : "Right")
                         << " " << Count << ":";
            LexEntryList.PrintNextEntry(m_pStat.Ptr(),0);

            m_pStat = m_pStat->GetNext(false);
            Count++;
        }
    }

    LexEntryList.CloseList();
}

/////////////
// Lexicon //
/////////////

CCCLLexicon::CCCLLexicon()
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    if(g_LexMinPrint) {
        m_PrintBound.first = new CStrKey("");
        m_PrintBound.second = new CCCLLexEntry(g_LexMinPrint);
    }
}

CCCLLexicon::~CCCLLexicon()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

CStrKey*
CCCLLexicon::GetEntryByString(string const& Name, CpCCCLLexEntry& pEntry)
{
    CpCLexEntry pGenericEntry;
    CpCStrKey pKey = CStrLexicon::GetEntryByString(Name, pGenericEntry);

    if(!pKey) {
        pEntry = NULL;
        return NULL;
    }

    pEntry = (CCCLLexEntry*)(pGenericEntry.Ptr());
    return pKey;
}

static bool
LexMoreFreqComp(LexPair const& pA, LexPair const& pB)
{
    if(!pA.second)
        return false;
    
    return (!pB.second || pA.second->Count() > pB.second->Count());
}

tLexComp
CCCLLexicon::PrintComp()
{
    return LexMoreFreqComp;
}


LexPair const&
CCCLLexicon::PrintBound()
{
    return m_PrintBound;
}

CLexEntry*
CCCLLexicon::NewEmptyLexEntry()
{
    return new CCCLLexEntry();
}
