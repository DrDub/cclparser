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

#include "StatVector.h"
#include "yError.h"

float&
CStatVector::operator[](unsigned int AbsCode)
{
    int LocalCode = GetLocalCode(AbsCode);

    if(LocalCode < 0) {
        yPError(ERR_OUT_OF_RANGE, "property not supported by vector");
    }

    if(m_Stats.size() <= LocalCode)
        m_Stats.resize(LocalCode+1, 0);

    return m_Stats.at(LocalCode);
}

//
// Copy class
//


CStatVectorCopy::CStatVectorCopy(CStatVector* pStatVector) :
        m_pVec(pStatVector)
{
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    
    if(!pStatVector) {
        yPError(ERR_MISSING, "Copy initialized with NULL vector pointer");
    }

    m_Stats = pStatVector->m_Stats;
}

CStatVectorCopy::~CStatVectorCopy()
{
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
}

float
CStatVectorCopy::operator[](unsigned int AbsCode)
{
    int LocalCode = m_pVec->GetLocalCode(AbsCode);

    if(LocalCode < 0) {
        yPError(ERR_OUT_OF_RANGE, "property not supported by vector");
    }

    if(m_Stats.size() <= LocalCode)
        m_Stats.resize(LocalCode+1, 0);

    return m_Stats.at(LocalCode);
}
