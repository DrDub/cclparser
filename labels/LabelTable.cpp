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

#include "LabelTable.h"
#include "yError.h"

using namespace std;

/////////////////
// Label Table //
/////////////////

// Adds the given label to the table for the given property. If the same label
// already exists in the table, the maximum of the given strength and the
// strength stored in the table becomes the new strength of the label.
// Returns the value object of the label.

CLabelVal*
CLabelTable::AddLabel(CLabel* pLabel, unsigned int Prop, float Strg)
{
    if(!pLabel)
        return NULL;

    CpCLabelVal pVal = GetVal(*pLabel);

    if(!pVal)
        return IncStrength(*pLabel, Prop, Strg);
    
    return AddLabel(pVal, Prop, Strg);
}

CLabelVal*
CLabelTable::AddLabel(CLabelVal* pVal, unsigned int Prop, float Strg)
{
    if(!pVal)
        return NULL;

    float TableStrg = GetStrengthFromVec(pVal, Prop);
    
    if(TableStrg < Strg)
        return IncStrength(pVal, Prop, Strg - TableStrg);
    else
        return pVal;
}

CLabelVal*
CLabelTable::FlipAndAddLabel(CLabel* pLabel, unsigned int Prop,
                             bool bOp, float Strg)
{
    if(!pLabel)
        return NULL;

    // flip the label
    CpCLabel pFlipped;
    
    // if the label is an 'opposite side' label, flipping the label
    // also consists of inserting the 'opposite side bits'.
    if(bOp) {
        unsigned int Type = (unsigned int)*pLabel;

        // label,1 -> label,0,1,0
        // label,0 -> label,1,1,1

        Type ^= 1; // flip the bit
        Type << 2;
        Type |= 2;
        Type |= ((Type & 4) >> 2);

        pFlipped = new CLabel(Type, *pLabel);
    } else
        // flips the first bit of the label
        pFlipped = new CLabel(*pLabel);
    
    // add the label
    return AddLabel(pFlipped, Prop, Strg);
}

void
CLabelTable::CopyLabels(CLabelTable* pLabels, unsigned int Prop)
{
    if(!pLabels)
        return;

    for(CpCLabelIter pLabelIter = pLabels->GetIter(Prop) ;
        pLabelIter && *pLabelIter ; ++(*pLabelIter)) {
        AddLabel(pLabelIter->Data(), Prop, pLabelIter->Strg());
    }
}

float
CLabelTable::Strg(CLabel* pLabel, unsigned int Prop)
{
    if(!pLabel)
        return 0;

    return GetStrength(*pLabel, Prop);
}
