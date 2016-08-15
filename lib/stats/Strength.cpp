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

#include "yError.h"
#include "Strength.h"

// pushes the entry at position Pos up until only entries with strictly
// higher strengths are above it in the list

int
CTopBase::PushUp(unsigned int Pos, unsigned int PropNum)
{
    static char Rname[] = "CTopBase::PushUp";
    
    if(Pos >= m_NextEntry) {
        // should not happen
        yPError(ERR_OUT_OF_RANGE, "position not in table");
    }
    
    if(!Pos || m_Entries[Pos].m_Strg < m_Entries[Pos-1].m_Strg)
        return Pos; // nothing to do
    
    unsigned int NewPos = Pos;
    CTopEntry PosEntry(m_Entries[Pos]);

    do {
        // move the entry down
        m_Entries[NewPos] = m_Entries[NewPos-1];
        if(m_Entries[NewPos].m_Props)
            (*(m_Entries[NewPos].m_Props))[PropNum] = Pos2Strg(NewPos);
        NewPos--;
    } while(NewPos > 0 && m_Entries[NewPos-1].m_Strg <= PosEntry.m_Strg);

    if(PosEntry.m_Props)
        (*(PosEntry.m_Props))[PropNum] = Pos2Strg(NewPos);
    m_Entries[NewPos] = PosEntry;
    return NewPos;
}

// Add the given entry to the top list.

int
CTopBase::Add(float Strg, CRef* pData, CRvector<float>* pProps,
              unsigned int PropNum)
{
    if(m_MaxEntries > m_NextEntry) {
        m_NextEntry++;
        if(m_Entries.size() < m_NextEntry)
            m_Entries.resize(m_NextEntry);
    }

    if(!m_NextEntry)
        return -1; // top list length is 0
    
    CTopEntry& Last = m_Entries[m_NextEntry-1];
    
    if(Strg < Last.m_Strg)
        return -1; // list is full
    
    // If an entry with a property vector is being replaced, update its
    // property vector
    if(Last.m_Props)
        (*(Last.m_Props))[PropNum] = Last.m_Strg;

    // replace (or initialize) the last entry

    Last.m_Strg = Strg;
    Last.m_Data = pData;
    Last.m_Props = pProps;
    if(pProps)
        (*(Last.m_Props))[PropNum] = Pos2Strg(m_NextEntry-1);
    
    // Reorder the entries, if needed.

    return PushUp(m_NextEntry-1, PropNum);
}

int
CTopBase::Inc(unsigned int Pos, float Strg, unsigned int PropNum)
{
    static char Rname[] = "CTopBase::Inc(unsigned int Pos, float Strg)";
    
    if(Pos >= m_NextEntry) {
        // should not happen
        yPError(ERR_OUT_OF_RANGE, "position requested is out of range");
    }

    if(Strg < 0) {
        yPError(ERR_OUT_OF_RANGE, "negative strength increment");
    }
    
    m_Entries[Pos].m_Strg += Strg;

    // Reorder the entries, if needed
    return PushUp(Pos, PropNum);
}

void
CTopBase::Clear()
{
    m_Entries.clear();
    m_NextEntry = 0;
}

/////////////////////////
// Auxiliary Functions //
/////////////////////////

int
GetTopListPosFromVec(CRvector<float>* pVec, unsigned int Prop)
{
    if(!pVec || Prop >= pVec->size())
        return -1;
    if((*pVec)[Prop] < 0)
        return CTopBase::Strg2Pos((*pVec)[Prop]);
    return -1;
}

// Is the entry in the top list for the given property ?

bool
IsInTheTopList(CRvector<float>* pVec, unsigned int Prop) {
    return (GetTopListPosFromVec(pVec, Prop) >= 0);
}

