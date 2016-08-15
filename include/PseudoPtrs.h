#ifndef __PSEUDOPTRS_H__
#define __PSEUDOPTRS_H__

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

#include "RefSTL.h"

// To avoid the problem of loops of automatic pointers, we construct a
// vector which stores all pointers (normal pointers, not automatic)
// constructed in a given context. The position of each pointer in this
// vector is then the pseudo vector for this object. A Pseudo pointer
// object then holds an automatic pointer to this vector together with a
// pseudo pointer and can thus replace a real pointer.
//
// Any object which should be stored in the pseudo-pointer vector must
// be derived from 'CThisPtr'. CThisPtr should be initialized
// with the pointer vector and it is responsible for registering the
// object to the vector when it is created and removing it from the
// vecotr when it is destroyed.
// When an object is removed from the vector, its pointer is replaced by
// NULL and all pseudo pointer objects pointing to it will return NULL when
// called.
//
// To make sure that the objects stored in the array are destroyed at the
// right moment, at least one (non-looping) path of automatic pointers
// must be used.

template <class T> class CThisPtr;

template <class T>
class CPseudoPtrs : public CRvector<T*>
{
    friend class CThisPtr<T>;
public:
    CPseudoPtrs() : CRvector<T*>(1,NULL) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    ~CPseudoPtrs() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
private:
    
    // The following two functions can only be called by the CThisPtr<T>
    // class
    
    // assign the pointer a position in the vector
    unsigned int Append(T* pObj) {
        CRvector<T*>::push_back(pObj);
        return CRvector<T*>::size() - 1;
    }
    void Remove(unsigned int Pos) {
        if(Pos < CRvector<T*>::size())
            CRvector<T*>::at(Pos) = NULL;
    }

public:
    void Clear() {
        CRvector<T*>::clear();
        CRvector<T*>::push_back(NULL);
    }

    // Sends the given signal to all pointers stored in the array
    // (the stored object must therefore have a Signal(int) function defined).
    void Signal(int Sig) {
        for(int i = 0 ; i < CRvector<T*>::size() ; i++) {
            if(CRvector<T*>::at(i))
                (CRvector<T*>::at(i))->Signal(Sig);
        }
    }
};

// The following class can be used as a pointer to an object. It
// holds a pointer to the pointer vector defined above together with
// the position of a specific pointer in this vector. It returns a (real)
// pointer to the object it points to (it return a NULL if the object
// has already been destroyed).

template<class T>
class CPseudoPtr
{
    friend class CThisPtr<T>;
private:
    CPtr<CPseudoPtrs<T> > m_pPtrs;
    unsigned int m_Pseudo;
public:
    CPseudoPtr() : m_pPtrs(), m_Pseudo(0) {}
    CPseudoPtr(T* pObj) : m_pPtrs(NULL), m_Pseudo(0) {
        if(!pObj) return;
        m_pPtrs = pObj->This().m_pPtrs;
        m_Pseudo = pObj->This().m_Pseudo;
    }
    CPseudoPtr(T& Obj) {
        m_pPtrs = Obj.This().m_pPtrs;
        m_Pseudo = Obj.This().m_Pseudo;
    }
    CPseudoPtr(CPseudoPtr<T>& Ptr) :
            m_pPtrs(Ptr.m_pPtrs), m_Pseudo(Ptr.m_Pseudo) {}

    operator CPseudoPtrs<T>*() { return m_pPtrs; }
    operator unsigned int() { return m_Pseudo; }
    
    T* Ptr() const {
        return m_pPtrs && m_Pseudo ? (*m_pPtrs)[m_Pseudo] : NULL;
    }
    T* operator->() const { return Ptr(); }
    operator T*() const { return Ptr(); }
    bool NotNull() const { return m_pPtrs && (bool)((*m_pPtrs)[m_Pseudo]); }
    operator bool() const { return NotNull(); }
    // The Initialized() function returns true if the pseudo pointer
    // vector is not NULL, even if the actual pseudo-pointer stored is a
    // NULL pointer.
    bool Initialized() { return m_pPtrs; }
    
    CPseudoPtr<T>& operator=(CPseudoPtr<T> const & Ptr) {
        m_pPtrs = Ptr.m_pPtrs;
        m_Pseudo = Ptr.m_Pseudo;
        return *this;
    }
    
    CPseudoPtr& operator=(T* pObj) {
        if(!pObj) {
            m_pPtrs = NULL;
            m_Pseudo = 0;
            return *this;
        }

        m_pPtrs = pObj->This().m_pPtrs;
        m_Pseudo = pObj->This().m_Pseudo;
        return *this;
    }
    
    CPseudoPtr& operator=(T& Obj) {
        m_pPtrs = Obj.This().m_pPtrs;
        m_Pseudo = Obj.This().m_Pseudo;
        return *this;
    }
    
    bool operator==(CPseudoPtr const & Ptr) const {
        return ((m_pPtrs == Ptr.m_pPtrs) && (m_Pseudo == Ptr.m_Pseudo));
    }
    bool operator==(T* pObj) const {
        return (!pObj && !Ptr()) || (pObj && *this == pObj->This());
    }
};

template<class T>
class CThisPtr : public CPseudoPtr<T>
{
public:
    CThisPtr(CPseudoPtrs<T>* pPtrs, T* pObj) {
        if(!pPtrs) return;
        CPseudoPtr<T>::m_pPtrs = pPtrs;
        CPseudoPtr<T>::m_Pseudo = pPtrs->Append(pObj);
    }

    ~CThisPtr() {
        if(CPseudoPtr<T>::m_pPtrs)
            CPseudoPtr<T>::m_pPtrs->Remove(CPseudoPtr<T>::m_Pseudo);
    }
    
    CPseudoPtr<T>& This() { return *this; }

    // The following function is used to send a signal to all objects
    // stored in a single pseudo-pointer array. It is up to the derived class
    // to specify the behaviour when the signal is received.
    virtual void Signal(int Sig) = 0;
};

#endif /* __PSEUDOPTRS_H__ */
