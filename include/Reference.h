#ifndef __REFERENCE_H__
#define __REFERENCE_H__

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

#ifndef NULL
#define NULL (0)
#endif

#ifdef DETAILED_DEBUG
#include <string>
#include <map>
#include <typeinfo>
#include <ostream>
#endif // DETAILED_DEBUG

//
// Simple Reference count
// 

// The following class may be inherited by classes which require reference
// counting.

class CRef {
private:
    int m_Count;
#ifdef DEBUG
public:
    static int m_ObjCount;
#endif // DEBUG
#ifdef DETAILED_DEBUG
    static std::map<std::string, int> m_ObjCountTable;
#endif // DETAILED_DEBUG
public:
    CRef() : m_Count(0) {
#ifdef DEBUG
        m_ObjCount++;
#endif        
    }
    virtual ~CRef() {
#ifdef DEBUG
        m_ObjCount--;
#endif
    }
    int Ref() { m_Count++; return m_Count; }
    int UnRef() { m_Count--; return m_Count; }
    int RefCount() { return m_Count; }
#ifdef DETAILED_DEBUG
    void IncObjCount() { m_ObjCountTable[typeid(*this).name()]++; }
    void DecObjCount() { m_ObjCountTable[typeid(*this).name()]--; }
    static void PrintObjectTable(std::ostream& Out);
#endif // DETAILED_DEBUG
#ifdef DEBUG
    static int TotalObjects() { return m_ObjCount; }
#endif    
};

// The following template can be used to created a pointer to class T

template <class T>
class CPtr
{
private:
    T* m_Node;
public:
    // Constructors and destructors
    CPtr() : m_Node(NULL) {}
    CPtr(T *p) : m_Node(p) { if (p) p->Ref(); }
    CPtr(CPtr const& r) : m_Node(r.m_Node) {
        if (m_Node) m_Node->Ref();
    }
    ~CPtr() { if (m_Node && (m_Node->UnRef() <= 0)) delete m_Node; }

    // Operators
    T *operator->() const { return m_Node; }
    T &operator()() { return *m_Node; }
    T *Ptr() const { return m_Node; }
    operator T*() const { return m_Node; }
    operator T&() { return *m_Node; }
    operator bool() const { return (bool)m_Node; }
    bool IsNull() const { return !m_Node; }
    
    CPtr& operator=(CPtr const & r) {
        if(r.m_Node) r.m_Node->Ref();
        if(m_Node && (m_Node->UnRef() <= 0)) delete m_Node;
        m_Node = r.m_Node;
        return *this;
    }

    CPtr& operator=(T* p) {
        if(p) p->Ref();
        if(m_Node && (m_Node->UnRef() <= 0)) delete m_Node;
        m_Node = p;
        return *this;
    }

    CPtr& operator=(T& p) {
        p.Ref();
        if(m_Node && (m_Node->UnRef() <= 0)) delete m_Node;
        m_Node = &p;
        return *this;
    }

    // The set function is identical to the assignment operator
    
    CPtr& Set(CPtr const & r) {
        if(r.m_Node) r.m_Node->Ref();
        if(m_Node && (m_Node->UnRef() <= 0)) delete m_Node;
        m_Node = r.m_Node;
        return *this;
    }

    CPtr& Set(T* p) {
        if(p) p->Ref();
        if(m_Node && (m_Node->UnRef() <= 0)) delete m_Node;
        m_Node = p;
        return *this;
    }

    CPtr& Set(T& p) {
        p.Ref();
        if(m_Node && (m_Node->UnRef() <= 0)) delete m_Node;
        m_Node = &p;
        return *this;
    }
    
    bool operator==(CPtr const & r) const { return m_Node == r.m_Node; }
    bool operator==(T* p) const { return m_Node == p; }
    // Generic comparison operator. A NULL pointer is always smaller
    // than a non-NULL pointer. If neither of the pointers is NULL,
    // the comparison function for the objects stored is called.
    bool operator<(T* p) const {
        if(!m_Node && p) return true;
        if(!p) return false;
        return *m_Node < *p;
    }
    // Same as above, only the automatic pointer, rather than the standard
    // one, is used on the right side.
    bool operator<(CPtr const & r) const {
        return *this < r.Ptr();
    }
};
  
typedef CPtr<CRef> CpCRef;

#endif /* __REFERENCE_H__ */
