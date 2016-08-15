#ifndef __STRENGTH_H__
#define __STRENGTH_H__

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
#include "Hash.h"
#include "RefSTL.h"

// this constant allow the creation of a strength table with an unbounded
// number of properties.
#define PROPS_UNBOUNDED ((unsigned int)-1)

class CTopBase;
template <class K, class V> class CStrengths;
template <class K, class V> class CTopIter;

// A single entry in the top strength list

class CTopEntry
{
    friend class CTopBase;
private:
    float m_Strg;      // Strength of the current entry
    CpCRef m_Data;     // the value to which the properties are assigned
    CPtr<CRvector<float> > m_Props; // property vector of the data
public:
    // default constructor
    CTopEntry() : m_Strg(0), m_Data(), m_Props() {}
    // copy constructor
    CTopEntry(const CTopEntry& Ent) :
            m_Strg(Ent.m_Strg), m_Data(Ent.m_Data), m_Props(Ent.m_Props) {} 
    CTopEntry& operator=(CTopEntry const& Ent) {
        m_Strg = Ent.m_Strg;
        m_Data = Ent.m_Data;
        m_Props = Ent.m_Props;
    }
    CRef* GetData() { return (CRef*)m_Data; }
    float GetStrg() { return m_Strg; }
    CRvector<float>* GetVal() { return (CRvector<float>*)m_Props; }
};

// the class CTopBase Maintains a list of entries, with strengths. The list
// has a bounded length. This is a base class for CTop<> templates
//
// Two operations are supported - incrementing the strength of an entry
// and adding a new entry. After each of these operations, the entries
// in the list are reordered so that they always remain sorted by strength.
// In case of insertion, if the bound on the size of the list is exceeded,
// the weakest strength elements are removed.
//
// When an entry is in the top strength list, its position in the list is
// recorded into the corresponding property strength entry in its property
// vector, as the negative number -(pos+1). As property strengths are never
// negative, after looking an entry up through the hash table in
// CStrengths, it is immediately possible to tell whether an entry
// is in the top strength property list and if yes - at which position.
// If one is interested in the strength of that entry, this strength must
// be read from the entry in the top strength list.
//
// Equal strength units are ordered by recency (most recent is at top).
// Therefore, when the list is full and a new entry is added with a
// strength equal to that of the weakest entry in the list, the old entry
// is replaced by the new one.

class CTopBase : public CRef
{
public:
    std::vector<CTopEntry> m_Entries; // vector of top entries
    unsigned int m_MaxEntries;  // maximal number of entries to be stored
    unsigned int m_NextEntry; // index beyond the last entry in the list
public:
    std::vector<CTopEntry>& GetEntries() { return m_Entries; }
    unsigned int const GetNextEntry() { return m_NextEntry; }
    CTopBase() : m_MaxEntries(0), m_NextEntry(0), m_Entries(0) {}
    // constructor
    CTopBase(unsigned int MaxEntries, bool bReserve = false) :
            m_MaxEntries(MaxEntries), m_NextEntry(0), m_Entries(0)
        {
            if(bReserve)
                m_Entries.reserve(m_MaxEntries);
        }
    // copy constructor
    CTopBase(CTopBase const& Top) :
            m_MaxEntries(Top.m_MaxEntries), m_NextEntry(Top.m_NextEntry),
            m_Entries(Top.m_Entries) {}
    // change the maximal number of entries
    void ResetMaxEntries(unsigned int MaxEntries, bool bReserve = false) {
        m_MaxEntries = MaxEntries;
        if(m_NextEntry > m_MaxEntries)
            m_NextEntry = m_MaxEntries;
        if(bReserve)
            m_Entries.reserve(m_MaxEntries);
    }
protected:
    // Add the given entry to the list (returns position, -1 if none)
    int Add(float Strg, CRef* pData, CRvector<float>* pProps,
            unsigned int PropNum);
public:
    // Increment the strength of entry in position Pos by strength Strg
    // (returns new position)
    int Inc(unsigned int Pos, float Strg, unsigned int PropNum);
    // Clear the table
    void Clear();
private:
    // Push the entry at position Pos up until only entries with strictly
    // higher strengths are above it in the list
    int PushUp(unsigned int Pos, unsigned int PropNum);
    // conversion functions from position to position coded as strength
    // (see explanation above)
public:
    static float Pos2Strg(unsigned int Pos) { return -((float)Pos+1); }
    static int Strg2Pos(float Strg) { return -(int)(Strg+1); }
    float GetStrg(float StrgPos) {
        return m_Entries[Strg2Pos(StrgPos)].m_Strg;
    }
    bool IsEmpty() { return !m_NextEntry; }
    float GetTopStrg() {
        return m_NextEntry ? GetStrg(-1) : 0;
    }
    float GetLastTopStrg() {
        return m_NextEntry ? GetStrg(-m_NextEntry) : 0;
    }
protected:
    CRef* GetTopData() {
        return m_NextEntry ? m_Entries.front().GetData() : NULL;
    }
    CRvector<float>* GetTopVec() {
        return m_NextEntry ? m_Entries.front().GetVal() : NULL;
    }
    CRef* GetLastTopData() {
        return m_NextEntry ? m_Entries.back().GetData() : NULL;
    }
    CRvector<float>* GetLastTopVec() {
        return m_NextEntry ? m_Entries.back().GetVal() : NULL;
    }
};

template <class K, class V>
class CTop : public CTopBase
{
public:
    CTop(unsigned int MaxEntries, bool bReserve = false) :
            CTopBase(MaxEntries, bReserve) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    // copy constructor
    CTop(CTop const& Top) : CTopBase((CTopBase const&)Top) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    ~CTop() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
    // Add the given entry to the list (returns position, -1 if none)
    int Add(float Strg, K* pData, V* pProps, unsigned int PropNum) {
        return CTopBase::Add(Strg, (CRef*)pData, (CRvector<float>*)pProps,
                             PropNum);
    }

    K* GetTopData() { return (K*)CTopBase::GetTopData(); }
    V* GetTopVal() { return (V*)CTopBase::GetTopVec(); }
    K* GetLastTopData() { return (K*)CTopBase::GetLastTopData(); }
    V* GetLastTopVal() { return (V*)CTopBase::GetLastTopVec(); }
};

// Iterator through a CTop list.

template <class K, class V>
class CTopIter : public CRef
{
    std::vector<CTopEntry>::iterator m_Iter;
    std::vector<CTopEntry>::iterator m_Begin;
    std::vector<CTopEntry>::iterator m_End;
public:
    CTopIter(CStrengths<K, V>* Tbl, unsigned int Prop) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
        if(!(bool)Tbl || Tbl->m_TopLists.size() <= Prop) return;
        m_Begin = m_Iter = Tbl->m_TopLists[Prop].GetEntries().begin();
        m_End = m_Iter + Tbl->m_TopLists[Prop].GetNextEntry();
    }
    CTopIter(CTop<K,V>& Top) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
        m_Begin = m_Iter = Top.GetEntries().begin();
        m_End = m_Iter + Top.GetNextEntry();
    }
    ~CTopIter() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
    
    // advance the iterator - return false if advance beyond the end
    bool operator++() {
        ++m_Iter;
        return (m_Iter < m_End);
    }
    operator bool() { return (m_Iter < m_End); }
    // number of elements remaining in the iterator to the end (including
    // the current element)
    int NumRemaining() { return (int)(m_End - m_Iter); }
    // restarts the iterator (returns false if the iterator is empty)
    // This should only be used when it is certain that the table has not
    // been changed since the iterator was created.
    bool Restart() {
        m_Iter = m_Begin;
        return (m_Iter < m_End);
    }
    float Strg() { return (m_Iter < m_End) ? m_Iter->GetStrg() : 0; }
    K* Data() { return (m_Iter < m_End) ? (K*)(m_Iter->GetData()) : NULL; }
    V* Val() { return (m_Iter < m_End) ? (V*)(m_Iter->GetVal()) : NULL; }
};

//////////////////////////
// Strength Table Class //
//////////////////////////

//
// Auxiliary Functions
//

// Return the position of the entry with the given property vector in the
// top list (0 = first position) for property 'prop' . Returns -1
// if no top list exists for 'prop' or the entry is not in the top list.
// Note that because of the way the position is coded, this function
// does not need to have access to the tables themselves, but only to
// the property vector of the entry.
extern int GetTopListPosFromVec(CRvector<float>* pVec, unsigned int Prop);

// Is the entry with the given property vector in the top list for the
// given property ?
// Note that because of the way the position is coded, this function
// does not need to have access to the tables themselves, but only to
// the property vector of the entry.
extern bool IsInTheTopList(CRvector<float>* pVec, unsigned int Prop);

// The following class consists of a hash table with values which are
// (fixed length) arrays of property strengths.
// For the specified number of properties at the beginning of the
// property list, a list of highest strength keys is maintained.
// The class K may be any class derived from CKey.
// The class V should be derived from CRvector<float>.

// No reference count is defined for this class (so that it could be
// used in combination with other classes which have a reference count).

template <class K, class V>
class CStrengths
{
    typedef CRvector<float> CRVecFl;
    friend class CTopIter<K, V>;
private:
    CPtr<CHash<K, V> > m_pHash;
    
    // number of properties (at the beginning of the list) for which
    // a top strength list is maintained
    unsigned int m_TopNum;
    unsigned int m_MaxTopLength; // max. num. of entries in top strength lists
    // when a top list is constructed, should the vectors be reserved ?
    bool m_bReserve;
    
    std::vector<CTop<K,V> > m_TopLists; // Vector of the top lists

public:
    CStrengths(unsigned int TopNum, unsigned int MaxTopLength,
               bool bReserve) :
            m_pHash(new CHash<K,V>()),
            m_TopNum(TopNum), m_MaxTopLength(MaxTopLength),
            m_bReserve(bReserve),
            m_TopLists((bReserve && (TopNum != PROPS_UNBOUNDED)) ? TopNum : 0,
                       CTop<K,V>(MaxTopLength, m_bReserve)) {
    }

    CStrengths(unsigned int TopNum, unsigned int MaxTopLength,
               unsigned int HashSize, bool bReserve) :
            m_pHash(new CHash<K,V>(HashSize)), m_TopNum(TopNum),
            m_MaxTopLength(MaxTopLength), m_bReserve(bReserve),
            m_TopLists((bReserve && (TopNum != PROPS_UNBOUNDED)) ? TopNum : 0,
                       CTop<K,V>(MaxTopLength, m_bReserve)) {
    }

    ~CStrengths() {}
    
    // Given a property, returns an iterator to retrieve all entries
    // from the top strength list for this property (returns NULL
    // if there is no top strength list for the given property)

    CTopIter<K, V>* GetIter(unsigned int Prop) {
        return (m_TopLists.size() <= Prop) ?
            NULL : new CTopIter<K, V>(this, Prop);
    }

    CHashIter<K, V>* GetFullIter() {
        return m_pHash->Begin();
    }
    
    // Increments the strength for the given property.
    // If bAddToTopList is false, the entry is not added into the top list
    // (but if it is already in the top list, it will stay there).
    // Returns a pointer to the value of the entry.

    V* IncStrength(V* pVal, unsigned int Prop, float Strg,
                   bool bAddToTopList = true) {
        if(Strg < 0)
            return NULL;

        if((*(CRVecFl*)pVal).size() <= Prop)
            (*(CRVecFl*)pVal).resize(Prop+1, 0);

        if(Prop >= m_TopNum) {
            ((*(CRVecFl*)pVal)[Prop] += Strg);
        } else if((*(CRVecFl*)pVal)[Prop] < 0) {
            m_TopLists[Prop].
                Inc(CTopBase::Strg2Pos((*(CRVecFl*)pVal)[Prop]), Strg ,Prop);
        } else if(!bAddToTopList) {
            ((*(CRVecFl*)pVal)[Prop] += Strg);
        } else {
            (*(CRVecFl*)pVal)[Prop] += Strg;
            if(m_TopLists.size() <= Prop)
                m_TopLists.resize(Prop+1,
                                  CTop<K,V>(m_MaxTopLength, m_bReserve));
            m_TopLists[Prop].Add((*(CRVecFl*)pVal)[Prop], m_pHash->Key(),
                                 pVal, Prop);
        }

        return pVal;
    }
    
    // Increments the strength on the given key entry for the given property.
    // If bAddToTopList is false, the entry is not added into the top list
    // (but if it is already in the top list, it will stay there).
    // Returns a pointer to the value of the entry

    V* IncStrength(K& Key, unsigned int Prop, float Strg,
                   bool bAddToTopList = true) {
        
        if(Strg < 0)
            return NULL;

        V* pVec;
        
        if(!(*m_pHash)[Key].Found()) {
            *m_pHash = pVec = new V(Prop+1);
        } else {
            pVec = m_pHash->Val();
            if(pVec->size() <= Prop)
                pVec->resize(Prop+1, 0);
        }

        if(Prop >= m_TopNum) {
            ((*pVec)[Prop] += Strg);
        } else if((*pVec)[Prop] < 0) {
            m_TopLists[Prop].Inc(CTopBase::Strg2Pos((*pVec)[Prop]), Strg,
                                 Prop);
        } else if(!bAddToTopList) {
            ((*(CRVecFl*)pVec)[Prop] += Strg);
        } else {
            (*pVec)[Prop] += Strg;
            if(m_TopLists.size() <= Prop)
                m_TopLists.resize(Prop+1,
                                  CTop<K,V>(m_MaxTopLength, m_bReserve));
            m_TopLists[Prop].Add((*pVec)[Prop], m_pHash->Key(), pVec, Prop);
        }

        return (V*)pVec;
    }

    // Get the last key used to access the table. This key is cached on the
    // hash table object

    K* GetKey() {
        return m_pHash->Key();
    }
    
    // Get the value object 

    V* GetVal(K& Key) {
        return m_pHash->Val(Key);
    }
    
    // Get the complete strength vector (to be used when the strength of
    // several properties for the same entry have to be looked up).

    CRvector<float>* GetStrengthVector(K& Key) {
        return (CRvector<float>*)GetVal(Key);
    }

    // Retrieve the position in top list (0 - first position) for the
    // given key and property. Returns -1 if no top list exists for the
    // property or the entry is not in the top list.
    
    int GetTopListPos(K& Key, unsigned int Prop) {
        return GetTopListPosFromVec(GetStrengthVector(Key), Prop);
    }
    
    // Is the entry in the top list for the given property ?
    
    bool IsInTopList(K& Key, unsigned int Prop) {
        return (GetTopListPos(Key, Prop) >= 0);
    }
    
    // Retrieve the strength of a given property from a property vector
    
    float GetStrengthFromVec(CRvector<float>* pVec, unsigned int Prop) {
        if(!pVec || Prop >= pVec->size())
            return 0;
        if((*pVec)[Prop] < 0) {
            return m_TopLists[Prop].GetStrg((*pVec)[Prop]);
        }
        return (*pVec)[Prop];
    }

    // Get the strength of the given property for the given key

    float GetStrength(K& Key, unsigned int Prop) {
        return GetStrengthFromVec(GetStrengthVector(Key), Prop);
    }
    
     // Get the number of properties which have a top list

    unsigned int GetTopNum() { return m_TopLists.size(); }
    
    // Get the number of entries in the top list

    unsigned int GetTopLength(unsigned int Prop) {
        if(Prop >= m_TopLists.size())
            return 0;
        return m_TopLists[Prop].GetNextEntry();
    }

    V* GetTopVal(unsigned int Prop) {
        if(Prop >= m_TopLists.size())
            return NULL;
        return m_TopLists[Prop].GetTopVal();
    }

    float GetTopStrg(unsigned int Prop) {
        if(Prop >= m_TopLists.size())
            return 0;
        return m_TopLists[Prop].GetTopStrg();
    }

    V* GetLastTopVal(unsigned int Prop) {
        if(Prop >= m_TopLists.size())
            return NULL;
        return m_TopLists[Prop].GetLastTopVal();
    }

    float GetLastTopStrg(unsigned int Prop) {
        if(Prop >= m_TopLists.size())
            return 0;
        return m_TopLists[Prop].GetLastTopStrg();
    }
    
    // Set to zero the strength for the given properties on all entries
    void Clear(unsigned int Prop) {
        if(Prop < m_TopLists.size() && GetTopLength(Prop) <= m_MaxTopLength) {
            for(CPtr<CTopIter<K, V> > pIter = GetIter(Prop) ; *pIter ;
                ++(*pIter))
                pIter->Val()->at(Prop) = 0;
        } else {
            for(CPtr<CHashIter<K, V> > pIter = m_pHash->Begin() ; *pIter ;
                ++(*pIter))
                if(pIter->GetVal()->size() > Prop)
                    pIter->GetVal()->at(Prop) = 0;
        }
        if(Prop < m_TopLists.size())
            m_TopLists[Prop].Clear();
    }

    // Completely clears the table (clears all top lists and the hash table)
    void Clear() {
        for(unsigned int Prop = 0 ; Prop < m_TopLists.size() ; Prop++)
            m_TopLists[Prop].Clear();
        m_pHash->Clear();
    }

    // Returns true if there are no entries in the table
    bool IsEmpty() {
        return !m_pHash->NumElements();
    }
};

#endif /* __STRENGTH_H__ */
