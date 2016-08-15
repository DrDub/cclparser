#ifndef __REFSTL_H__
#define __REFSTL_H__

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
#include <vector>
#include <deque>

// standard types with reference

class CRefUInt : public CRef
{
private:
    unsigned int m_UInt;
public:
    CRefUInt() : m_UInt(0) {}
    CRefUInt(unsigned int UInt) : m_UInt(UInt) {}
    operator unsigned int&() { return m_UInt; }
};

typedef CPtr<CRefUInt> CpCRefUInt;

class CRefInt : public CRef
{
private:
    int m_Int;
public:
    CRefInt() : m_Int(0) {}
    CRefInt(int Int) : m_Int(Int) {}
    operator int&() { return m_Int; }
};

typedef CPtr<CRefInt> CpCRefInt;


class CRefFloat : public CRef
{
private:
    float m_Float;
public:
    CRefFloat() : m_Float(0) {}
    CRefFloat(float Float) : m_Float(Float) {}
    operator float&() { return m_Float; }
};

typedef CPtr<CRefFloat> CpCRefFloat;

// standard STL containers with reference count

template <class T>
class CRvector : public CRef, public std::vector<T>
{
public:
    CRvector() {}
    CRvector(unsigned int size) : std::vector<T>(size) {}
    CRvector(unsigned int size, T const& Val) : std::vector<T>(size, Val) {}
    CRvector(std::vector<T>& Vec) : std::vector<T>(Vec) {}
};

template <class T>
class CRdeque : public CRef, public std::deque<T>
{
public:
    CRdeque() {}
};

// Specific instances of the above containers

typedef CRvector<int> CRIntVector;
typedef CPtr<CRIntVector> CpCRIntVector;

#endif /* __REFSTL_H__ */
