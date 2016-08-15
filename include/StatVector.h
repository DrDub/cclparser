#ifndef __STATVECTOR_H__
#define __STATVECTOR_H__

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

#include <vector>
#include "Reference.h"
#include "PropConv.h"

// forward declarations

class CStatVectorCopy;

////////////////////////
// Statistics Vectors //
////////////////////////

// The statistics vector is a simple vector of floats with a property
// conversion table. This conversion table translates the property number
// into a position in the vector.

class CStatVector : public CRef
{
    friend class CStatVectorCopy;
    
private:
    std::vector<float> m_Stats; // vector of statistics
public:
    CStatVector() : m_Stats() {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    CStatVector(unsigned int Size) : m_Stats(Size) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    ~CStatVector() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

private:

    // The following routine returns a reference to the property
    // convertor (which must be defined in the derived class)
    virtual CPropConv& GetVecPropConv() = 0;
    
    // The following function converts the absolute property code
    // to the position of the property in the current vector. Returns -1
    // if the requested code is not supported by the vector.
    int GetLocalCode(unsigned int AbsCode) {
        return GetVecPropConv().GetPropCode(AbsCode);
    }
public:
    // operator for returning a reference to the entry at the given position
    // (if no such entry exists, an error is thrown)
    float& operator[](unsigned int AbsCode);
};

typedef CPtr<CStatVector> CpCStatVector;

///////////////////////////////
// Copy of Statistics Vector //
///////////////////////////////

// The following class defines an object which holds a copy of a vector
// statistics. The copy is created at construction and does not change
// even if the original statistics object is modified.
// The copy object uses the same property conversion object as the original
// statistics object.

class CStatVectorCopy : public CRef
{
private:
    std::vector<float> m_Stats; // vector of statistics
    // pointer to original statistics object - for access to the conversion
    // object.
    CpCStatVector m_pVec;
protected:
    // the constructors are protected so as to ensure that only specific
    // derived classes (specific for a specific statistics class) could
    // be used.
    
    // constructor from a vector of statistics
    CStatVectorCopy(CStatVector* pStatVector);

public:
    ~CStatVectorCopy();

    // access to the values
    float operator[](unsigned int AbsCode);
};

#endif /* __STATVECTOR_H__ */
