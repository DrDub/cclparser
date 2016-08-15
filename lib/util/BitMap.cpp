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

#include <algorithm>
#include "BitMap.h"
#include "yError.h"

using namespace std;

CBitMap::CBitMap() : m_pBits(NULL), m_Len(0)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CBitMap::CBitMap(CBitMap const& BitMap) {
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    m_Len = BitMap.m_Len;
    if(m_Len) {
        m_pBits = new unsigned int[m_Len];

        for(unsigned int i = 0 ; i < m_Len ; i++)
            m_pBits[i] = BitMap.m_pBits[i];
    } else
        m_pBits = NULL;
}

// resize the bit array (if needed)

void
CBitMap::Resize(unsigned int Size)
{
    if(m_Len >= Size)
        return;

    unsigned int* pBuf = new unsigned int[Size];

    for(unsigned int i = 0 ; i < Size ; i++)
        pBuf[i] = (i < m_Len) ? m_pBits[i] : 0;

    if(m_pBits)
        delete [] m_pBits;

    m_pBits = pBuf;
    m_Len = Size;
}

// sets the bit whose number is given by BitNum. Returns true if the bit
// was not previously set.

bool
CBitMap::SetBit(unsigned int BitNum)
{
    // cell in array where the bit has to be placed
    unsigned int Cell = BitNum / (8 * sizeof(unsigned int));

    if(Cell >= m_Len)
        Resize(Cell+1);

    unsigned int PrevValue = m_pBits[Cell]; 
    m_pBits[Cell] |= (1 << (BitNum % (8 * sizeof(unsigned int))));

    return (PrevValue != m_pBits[Cell]);
}

// sets all the bits whose number is <= BitNum

void
CBitMap::SetBitsUpTo(unsigned int BitNum)
{
    // cell in array where the last bit has to be placed
    unsigned int Cell = BitNum / (8 * sizeof(unsigned int));

    if(Cell >= m_Len)
        Resize(Cell+1);

    for(int i = 0 ; i < Cell ; i++) {
        m_pBits[i] = (unsigned int)-1;
    }

    for(int i = 0 ; i <= BitNum - Cell * 8 * sizeof(unsigned int) ; i++)
        m_pBits[Cell] |= (1 << i);
}

bool
CBitMap::IsZero()
{
    for(int i = 0 ; i < m_Len ; i++)
        if(m_pBits[i])
            return false;
    
    return true;
}

unsigned int
CBitMap::BitCount()
{
    unsigned int Count = 0;
    
    for(int i = 0 ; i < m_Len ; i++) {
        if(!m_pBits[i])
            continue;
        // simply check for a bit in every position
        unsigned int BitsChecked = 0;
        for(int c = 0 ; c < 8 * sizeof(unsigned int) ; c++) {
            if(m_pBits[i] & (1 << c))
                Count++;
            BitsChecked |= (1 << c);
            if((m_pBits[i] & BitsChecked) == m_pBits[i])
                break;
        }
    }

    return Count;
}

bool
CBitMap::operator==(CBitMap const& BitMap) const
{
    for(unsigned int i = 0 ; i < min(m_Len, BitMap.m_Len) ; i++)
        if(m_pBits[i] != BitMap.m_pBits[i])
            return false;

    if(m_Len < BitMap.m_Len) {
        for(unsigned int i = m_Len ; i < BitMap.m_Len ; i++)
            if(BitMap.m_pBits[i])
                return false;
    } else if(BitMap.m_Len < m_Len) {
        for(unsigned int i = BitMap.m_Len ; i < m_Len ; i++)
            if(m_pBits[i])
                return false;
    }
    
    return true;
}

// lexicographic comparison

bool
CBitMap::operator<(CBitMap const& BitMap) const
{
    for(unsigned int i = 0 ; i < min(m_Len, BitMap.m_Len) ; i++) {
        if(m_pBits[i] < BitMap.m_pBits[i])
            return true;
        if(BitMap.m_pBits[i] < m_pBits[i])
            return false;
    }

    if(m_Len < BitMap.m_Len) {
        for(unsigned int i = m_Len ; i < BitMap.m_Len ; i++)
            if(BitMap.m_pBits[i])
                return true;
    } else if(BitMap.m_Len < m_Len) {
        for(unsigned int i = BitMap.m_Len ; i < m_Len ; i++)
            if(m_pBits[i])
                return false;
    }

    // equality
    return false;
}

void
CBitMap::Print(ostream& Out) const
{
    for(unsigned int i = 0 ; i < m_Len ; i++) {
        for(unsigned int j = 0 ; j < sizeof(int) * 8 ; j++) {
            if(m_pBits[i] & (1 << j))
                Out << "1";
            else
                Out << "0";
        }
    }
}

void
CBitMap::PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter)
{
    if(!pOut) {
        yPError(ERR_MISSING, "output stream missing");
    }
    
    ((ostream&)(*pOut)) <<
        string(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');

    Print((ostream&)(*pOut));
}

//
// Automatic pointer with comparison operators
//

bool
CpCBitMap::operator==(CpCBitMap const& pBitMap) const
{
    if(!this->Ptr())
        return !pBitMap.Ptr();

    if(!pBitMap.Ptr())
        return false;

    return (*(this->Ptr()) == *(pBitMap.Ptr()));
}

bool
CpCBitMap::operator==(CBitMap* pBitMap) const
{
    if(!this->Ptr())
        return !pBitMap;

    if(!pBitMap)
        return false;

    return (*(this->Ptr()) == *(pBitMap));
}

bool
CpCBitMap::operator<(CpCBitMap const& pBitMap) const
{
    if(!this->Ptr())
        return pBitMap.Ptr();

    if(!pBitMap.Ptr())
        return false;

    return (*(this->Ptr()) < *(pBitMap.Ptr()));
}
