#ifndef __HASH_H__
#define __HASH_H__

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

#include "HashDef.h"
#include "HashKey.h"

#define HASH_DEFAULT_SIZE (1 << 10)  // 1024
// log_2 of the maximal ratio allowed between the number of elements and
// number of slots in the table before the table is resized.
#define HASH_FULL_DEPTH_FACTOR 1
// log_2 of the factor by which the size of the table is increased when
// it is resized
#define HASH_DEFAULT_RESIZE_FACTOR 2

//
// Hash entry structure (for internal use only)
//

struct CHashEnt
{
    friend class CHashBase;
    friend class CHashIterBase;
public:
    CHashEnt() {}
    ~CHashEnt() {}
private:
    CpCKey m_Key;     // generic key
    CpCRef m_Val;     // generic value
    CHashEnt* pNext;  // no reference count needed - each hash entry used once 
};

//////////////////
// Base Classes //
//////////////////

//
// base hash table class
//

class CHashBase : public CRef {

    friend class CHashIterBase;
    
private:

    unsigned int m_HashMask; // mask to be used to convert hash function
                             // value to a table slot (this mask is the size
                             // of the table minus 1)

    CHashEnt** m_pSlots;
    
    unsigned int m_NElements; // Number of elements in the hash

private:
    CHashEnt** m_pLastLookup; // result of last lookup operation
    CpCKey m_pLastKey;        // last key used for lookup

    // Construction, destruction, clearing
protected:    
    CHashBase(unsigned int Size  = HASH_DEFAULT_SIZE);
public:
    virtual ~CHashBase(); // deletes the entry array
    void Clear(); // clear all entries from the table

private:

    unsigned int Resize(unsigned int FacExp2); // resizes the table
    
    // Lookup, insertion, deletion (internal and external versions)
    
protected:
    CHashEnt** Lookup(CKey& Key);
private:
    int DeleteAt(CHashEnt** ppEnt);
protected:
    int Delete(CKey& Key);
    int Delete();
    // Inserts an entry at the specified location. If ppEnt is not NULL,
    // the entry it points to is overwritten.
    // delete last key looked up
private:    
    int InsertAt(CKey& Key, CRef* pVal, CHashEnt** ppEnt);
protected:    
    int Insert(CKey& Key, CRef* pVal);
    int Insert(CRef* pVal);
public:
    // indicates whether the last lookup succeeded
    bool Found() { return m_pLastLookup && *m_pLastLookup; }
    // Returns the value found by the last lookup (NULL if none)
    CRef* Val() {
        return m_pLastLookup && *m_pLastLookup ?
            (CRef*)((*m_pLastLookup)->m_Val) : NULL;
    }
    // The following function performs a 'passive' lookup. This means that
    // the lookup key is not stored in the table's last lookup cache and
    // therefore cannot later be used for inserting a value.
    CRef* Val(CKey& Key);
protected:
    // returns the key of the last lookup, if found (NULL if not found)
    CKey* Key() {
        return m_pLastLookup && *m_pLastLookup ?
            (CKey*)((*m_pLastLookup)->m_Key) : NULL;
    }
public:
    unsigned int NumElements() { return m_NElements; }
};

typedef CPtr<CHashBase> CpCHash;

//
// Base iterator class
//

class CHashIterBase : public CRef {
    
    friend class CHashBase;
    
private:
    CpCHash m_pTable;    // hash table from which the iterator was created
    unsigned int m_Slot; // Current slot number
    CHashEnt** m_Ent;    // Current entry
protected:
    CHashIterBase(CHashBase* pTable) :
            m_Ent(NULL), m_Slot(0), m_pTable(pTable) {}
    virtual ~CHashIterBase() {}
public:    
    bool First();     // reset to the beginning of the hash table. Returns 
                      // false if the table is empty, true otherwise
protected:
    CKey* FirstKey(); // reset to the beginning and return the key
    CRef* FirstVal(); // reset to the beginning and return the value
public:
    bool Next();      // advances to next entry. Returns false if no such
                      // entry exists, true otherwise
    bool operator++() { return Next(); } // the same
protected:
    CKey* NextKey();  // advances to the next entry and returns the key
    CRef* NextVal();  // advances to the next entry and returns the value

    CKey* GetKey();   // get current key (NULL if none)
    CRef* GetVal();   // get current value (NULL if none)
public:
    // Has the iterator reached the end ?
    bool IsEnd() { return (!m_pTable || (m_pTable->m_HashMask < m_Slot)); }
    operator bool() { return !IsEnd(); }
};

///////////////////////
// Derived Templates //
///////////////////////

// For every specific type of key and value, the templates below should
// be used to create the appropriate hash class.

//
// Specific key and value hash iterator (template)
//

template <class K, class V>
class CHashIter : public CHashIterBase
{
public:
    CHashIter(CHashBase* pTable) : CHashIterBase(pTable) {}
    ~CHashIter() {}

    // reset to the beginning and return the key/value
    K* FirstKey() { return (K*)CHashIterBase::FirstKey();  } 
    V* FirstVal() { return (V*)CHashIterBase::FirstVal();  }

    // advances to the next entry and returns the key/value
    K* NextKey() { return (K*)CHashIterBase::NextKey(); }
    V* NextVal() { return (V*)CHashIterBase::NextVal(); }

    // Get current key/val
    K* GetKey() { return (K*)CHashIterBase::GetKey(); }
    V* GetVal() { return (V*)CHashIterBase::GetVal(); }
};

//
// Specific key and value hash tables types (template)
//

template <class K, class V>
class CHash : public CHashBase
{
public:

    // Construction, destruction, clearing
    CHash() : CHashBase() {}
    CHash(unsigned int Size) : CHashBase(Size) {}
    ~CHash() {}
    
    // Lookup, insertion, deletion (external versions)
    
    // key lookup operator
    CHash& operator[](K& Key) { CHashBase::Lookup((CKey&)Key); return *this; }
    int operator=(V* pVal) { return CHashBase::Insert((CRef*)pVal); }
    operator V*() { return (V*)CHashBase::Val(); }
    V* Val() { return (V*)CHashBase::Val(); }
    V* Val(K& Key) { return (V*)CHashBase::Val((CKey&)Key); }
    K* Key() { return (K*)CHashBase::Key(); }
    // delete from hash
    int Delete(K& Key) { return CHashBase::Delete((CKey&)Key); }
    // delete last key looked up
    int Delete() { return CHashBase::Delete(); }
    // insert into hash
    int Insert(K& Key, V* pVal) {
        return CHashBase::Insert((CKey&)Key, (CRef*)pVal);
    }
    int Insert(V* pVal) { return CHashBase::Insert((CRef*)pVal); }
    // returns an iterator to the first entry
    CHashIter<K,V>* Begin() {
        CHashIter<K,V>* pIter = new CHashIter<K,V>(this);
        pIter->First();
        return pIter;
    }
};

///////////////////////
// Dummy value class //
///////////////////////

// to be used in tables where we only want to know whether a certain key
// is in the table or not.

class CHashDummyVal : public CRef
{
    // empty class
};

#endif /* __HASH_H__ */
