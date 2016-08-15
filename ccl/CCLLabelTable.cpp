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

#include "CCLLabelTable.h"
#include "yError.h"

using namespace std;

/////////////////////////////
// CCL Parser Label Tables //
/////////////////////////////

// Store the labels with the highest eSeen values
// as labels. The strength of a label is the eSeen value of the
// label divided by the eLearn value of the attachment point.
// The labels are flipped before being stored, to make matching with
// opposite labels faster.

void
CCCLLabelTable::SetAdjacencyLabels(unsigned int Side, CCCLStat* pStat)
{
    if(Side >= SIDE_NUM) {
        yPError(ERR_OUT_OF_RANGE, "Side does not have a label list");
    }

    if(!pStat || pStat->IsEmpty())
        return;

    for(CCCLStatIter Iter(pStat, CCCLStat::eSeen) ; Iter ; ++Iter)
        FlipAndAddLabel(Iter.Data(), Side, false,
                        Iter.QtV(CCCLStat::eLearn));
}

void
CCCLLabelTable::SetUnitLabel(CStrKey* pString, float Strg,
                             unsigned int Side)
{
    if(!pString || !Strg)
        return;
    
    if(Side > BOTH_SIDES)
        yPError(ERR_OUT_OF_RANGE, "invalid side");

    // flip the label before adding it
    CpCLabel pLabel = new CLabel(LB_OTHER_SIDE, pString);

    CpCLabelVal pVal;
    
    if(Side == BOTH_SIDES) {
        pVal = AddLabel(pLabel, LEFT, Strg);
        AddLabel(pVal, RIGHT, Strg);
    } else {
        AddLabel(pLabel, Side, Strg);
    }
}
