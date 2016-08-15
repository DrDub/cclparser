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

#include "PropConv.h"

/////////////////////////////////////
// Property Code Conversion Vector //
/////////////////////////////////////

CPropConv::CPropConv(int* pTop, int* pNoTop, bool bExtendable) :
        m_PropNum(0), m_TopListNum(0), m_bExtendable(bExtendable)
{
    int Round = 0;

    while(Round < 2) {
        
        for(int* pProp = (Round == 0 ? pTop : pNoTop) ; pProp && *pProp >= 0 ;
            pProp++) {
            
            if(*pProp >= m_Conv.size())
                m_Conv.resize((*pProp)+1, -1);

            if(m_Conv[*pProp] >= 0)
                continue; // property already assigned

            m_Conv[*pProp] = m_PropNum;
            m_OpConv.push_back(*pProp);
            m_PropNum++;
            if(Round == 0)
                m_TopListNum++;
        }

        Round++;
    }
}

bool
CPropConv::AddNonTopProp(int Prop)
{
    if(Prop >= m_Conv.size())
        m_Conv.resize(Prop+1, -1);
    
    if(m_Conv[Prop] >= 0)
        return false; // property already assigned

    m_Conv[Prop] = m_PropNum;
    m_OpConv.push_back(Prop);
    m_PropNum++;

    return true;
}

int
CPropConv::GetPropCode(unsigned int Prop)
{
    if(Prop >= m_Conv.size() || m_Conv[Prop] < 0) {
        if(m_bExtendable)
            AddNonTopProp(Prop);
        else
            return -1;
    }

    return m_Conv[Prop];
}
