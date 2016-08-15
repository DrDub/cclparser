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
#include "Hash.h"

CHashBase::CHashBase(unsigned int Size) : m_NElements(0)
{
    unsigned int HashSize;

    m_pLastLookup = NULL;
    
    if(!Size)
        Size = HASH_DEFAULT_SIZE;
    else
        // round to the nearest (larger) power of 2
        for(HashSize = 1 ; HashSize < Size ; HashSize = (HashSize << 1));

    m_pSlots = new CHashEnt*[HashSize];

    for(int i = 0 ; i < HashSize ; i++)
        m_pSlots[i] = NULL;
    
    m_HashMask = HashSize - 1;
}

CHashBase::~CHashBase()
{
    Clear();
    delete[] m_pSlots; 
}

void
CHashBase::Clear()
{
    m_pLastLookup = NULL;
    m_pLastKey = NULL;

    if(!m_NElements)
        return; // nothing to clear
    
    for(int Slot = 0 ; Slot <= m_HashMask ; Slot++) {
        CHashEnt* pS = *(m_pSlots+Slot); 
        while(pS) {
            CHashEnt* pT = pS;
            pS = pS->pNext;
            delete pT;
        }
        *(m_pSlots+Slot) = 0;
    }

    m_NElements = 0;
}

// Resizes the table (and redistributes all entries accordingly)
// The table size is increased by a factor of 2 ^ Log2Fac. 
// Upon success, the function returns the new table size. Returns 0
// upon failure. 

unsigned int
CHashBase::Resize(unsigned int Log2Fac)
{
    unsigned int NewMask;
    CHashEnt** ppNewSlots;
    
    if(!Log2Fac)
        return m_HashMask + 1; // nothing to do
    
    NewMask = (m_HashMask + 1) * (1 << Log2Fac) - 1;
    
    // allocate the new table
    ppNewSlots = new CHashEnt* [NewMask+1];

    for(int i = 0 ; i <= NewMask ; i++)
        ppNewSlots[i] = NULL;

    // loop over all entries and re-distribute the entries

    for(int Slot = 0 ; Slot <= m_HashMask ; Slot++) {
        CHashEnt* pS = *(m_pSlots+Slot); 
        while(pS) {
            unsigned int Slot = (NewMask & pS->m_Key->HashFunc());
            CHashEnt* pT = pS->pNext;
            pS->pNext = ppNewSlots[Slot];
            ppNewSlots[Slot] = pS;
            pS = pT;
        }
    }

    delete[] m_pSlots;
    m_pSlots = ppNewSlots;
    m_HashMask = NewMask;

    // position of last lookup has changed 

    if(m_pLastLookup)
        Lookup((CKey&)m_pLastKey);
    
    return m_HashMask+1;
}

CHashEnt**
CHashBase::Lookup(CKey& Key)
{
    CHashEnt** ppEnt;

    for(ppEnt = m_pSlots+(m_HashMask & Key.HashFunc()) ; *ppEnt ;
        ppEnt = &((*ppEnt)->pNext)) {
        if(Key.HashEqual((CKey*)((*ppEnt)->m_Key)))
            break; // found matching entry
    }

    m_pLastLookup = ppEnt;
    m_pLastKey = Key;
    
    return ppEnt;
}

int
CHashBase::DeleteAt(CHashEnt** ppEnt)
{
    CHashEnt* pDEnt;
    
    if(!*ppEnt)
        return m_NElements;
    
    pDEnt = *ppEnt;
    *ppEnt = pDEnt->pNext;
    delete pDEnt;
    
    return --m_NElements;
}

int
CHashBase::Delete(CKey& Key)
{
    return DeleteAt(Lookup(Key));
}

int
CHashBase::Delete()
{
    if(!m_pLastLookup)
        return m_NElements;

    return DeleteAt(m_pLastLookup);
}

int
CHashBase::InsertAt(CKey& Key, CRef* pVal, CHashEnt** ppEnt)
{
    if(!*ppEnt) {
        *ppEnt = new CHashEnt;
        (*ppEnt)->pNext = NULL;
        m_NElements++;
    }

    (*ppEnt)->m_Key = Key;
    (*ppEnt)->m_Val = pVal;

    // Check whether the table needs to be resized
    if(((m_HashMask + 1) << HASH_FULL_DEPTH_FACTOR) < m_NElements)
        Resize(HASH_DEFAULT_RESIZE_FACTOR);
    
    return m_NElements;
}

int
CHashBase::Insert(CKey& Key, CRef* pVal)
{
    return InsertAt(Key, pVal, Lookup(Key));
}

int
CHashBase::Insert(CRef* pVal)
{
    static char Rname[] = "CHashBase::Insert(CRef* pVal)";
    
    if(!m_pLastLookup) {
        derror("insertion without key, but without previous lookup");
        return m_NElements;  // doesn't do anything
    }

    return InsertAt((CKey&)m_pLastKey, pVal, m_pLastLookup);
}

// Passive lookup function

CRef*
CHashBase::Val(CKey& Key)
{
    CHashEnt** pEnt = Lookup(Key);

    // remove the lookup from the cache
    m_pLastLookup = NULL;
    m_pLastKey = NULL;

    return (pEnt && *pEnt) ? (CRef*)((*pEnt)->m_Val) : NULL;
}

// Base iterator class

// Resets the iterator to the beginning of the hash table
// Returns false if the table is empty and true otherwise

bool
CHashIterBase::First()
{
    if(m_pTable.IsNull())
        return false;

    m_Slot = 0;
    m_Ent = m_pTable->m_pSlots;

    if(!m_Ent || !*m_Ent)
        return Next();

    return true;
}

CKey*
CHashIterBase::FirstKey()
{
    if(!First() || !m_Ent || !(*m_Ent))
        return NULL;

    return (CKey*)((*m_Ent)->m_Key);
}

CRef*
CHashIterBase::FirstVal()
{
    if(!First() || !m_Ent || !(*m_Ent))
        return NULL;

    return (CRef*)((*m_Ent)->m_Val);
}

bool
CHashIterBase::Next()
{
    if(m_pTable.IsNull()) {
        return false;
    }
    
    if(m_Ent && *m_Ent)
        m_Ent = &((*m_Ent)->pNext);
    
    if(!m_Ent || !*m_Ent) {
        if(++m_Slot > m_pTable->m_HashMask)
            return false;
        m_Ent = m_pTable->m_pSlots + m_Slot;
        while(!(*m_Ent)) {
            m_Slot++;
            m_Ent++;
            if(m_Slot > m_pTable->m_HashMask)
                return false;
        }
    }

    return true;
}

CKey*
CHashIterBase::NextKey()
{
    if(!Next())
        return NULL;

    return (CKey*)((*m_Ent)->m_Key);
}

CRef*
CHashIterBase::NextVal()
{
    if(!Next())
        return NULL;

    return (CRef*)((*m_Ent)->m_Val);
}

CKey*
CHashIterBase::GetKey()
{
    if(m_Ent && *m_Ent)
        return (CKey*)((*m_Ent)->m_Key);
    return NULL;
}

CRef*
CHashIterBase::GetVal()
{
    if(m_Ent && *m_Ent)
        return (CRef*)((*m_Ent)->m_Val);
    return NULL;
}
