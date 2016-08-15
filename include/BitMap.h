#ifndef __BITMAP_H__
#define __BITMAP_H__

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

#include <ostream>
#include "Reference.h"
#include "PrintUtils.h"

class CBitMap : public CRef, public CPrintObj
{
private:
    unsigned int* m_pBits; // array to hold the bits
    unsigned int m_Len; // number of integers allocated for the bitmap
public:
    CBitMap();
    CBitMap(CBitMap const& BitMap);
    
    ~CBitMap() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
        if(m_pBits)
            delete [] m_pBits;
    }

    void Resize(unsigned int Size);
    bool SetBit(unsigned int BitNum);
    void Zero() {
        for(unsigned int i = 0 ; i < m_Len ; i++)
            m_pBits[i] = 0;
    }
    // sets all bits up to and including BitNum
    void SetBitsUpTo(unsigned int BitNum);

    // returns true if the number of bits set is zero
    bool IsZero();
    // returns the number of bits set in the bit map
    unsigned int BitCount();
    
    bool operator==(CBitMap const& BitMap) const;
    bool operator<(CBitMap const& BitMap) const;

    // for debugging
    void Print(std::ostream& Out) const;
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
};

// The pointer class for the bit map allows comparison of the bitmaps
// being pointed to.

class CpCBitMap : public CPtr<CBitMap>
{
public:
    CpCBitMap() {} // default constructor does nothing
    CpCBitMap(CBitMap* pBitMap) : CPtr<CBitMap>(pBitMap) {}
    CpCBitMap(CpCBitMap const & BitMap) :
            CPtr<CBitMap>((CPtr<CBitMap>&)BitMap) {}
    
    CpCBitMap& operator=(CpCBitMap const & BitMap) {
        Set(BitMap.Ptr());
        return *this;
    }

    CpCBitMap& operator=(CBitMap* pBitMap) {
        Set(pBitMap); 
        return *this;
    }

    CpCBitMap& operator=(CBitMap& BitMap) {
        Set(&BitMap);
        return *this;
    }
    
    bool operator==(CpCBitMap const& pBitMap) const;
    bool operator==(CBitMap* pBitMap) const;
    // a Null bitmap is considered smaller than all other bitmaps.
    bool operator<(CpCBitMap const& pBitMap) const;
};




#endif /* __BITMAP_H__ */
