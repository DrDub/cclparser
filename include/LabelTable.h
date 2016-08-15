#ifndef __LABELTABLE_H__
#define __LABELTABLE_H__

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
#include "Strength.h"
#include "Label.h"

/////////////////
// Label Table //
/////////////////

//
// value class for the label table
//

class CLabelVal : public CRvector<float>
{
public:
    CLabelVal() : CRvector<float>() {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }

    CLabelVal(unsigned int Size) : CRvector<float>(Size) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    
    ~CLabelVal() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
};

typedef CPtr<CLabelVal> CpCLabelVal;

//
// label table class
//

typedef CTopIter<CLabel, CLabelVal> CLabelIter;
typedef CPtr<CLabelIter> CpCLabelIter;

class CLabelTable : public CStrengths<CLabel, CLabelVal>, public CRef
{
public:
    CLabelTable(unsigned int TopListNum, unsigned int MaxTopListLength,
                unsigned int HashSize, bool bReserve) :
            CStrengths<CLabel, CLabelVal>(TopListNum, MaxTopListLength,
                                          HashSize, bReserve) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    
    ~CLabelTable() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
    // Adds the given label to the table for the given property. If the same
    // label already exists in the table, the maximum of the given strength
    // and the strength stored in the table becomes the new strength of the
    // label. Returns the value object of the label.
    CLabelVal* AddLabel(CLabel* pLabel, unsigned int Prop, float Strg);
    // Same as above, only with the label value already given.
    CLabelVal* AddLabel(CLabelVal* pVal, unsigned int Prop, float Strg);
    // Flips the given label and then adds the flipped label as in the
    // routines above.
    // If 'bOp' is set then the label is an opposite side label. Flipping
    // such a label consists of recording this fact on the label type.
    CLabelVal* FlipAndAddLabel(CLabel* pLabel, unsigned int Prop,
                               bool bOp, float Strg);
    
    // Copy the labels for the given property of pLabels to this label table
    // (with the same strength). If a label already exists in the table,
    // only increment its strength up to the strength in pLabels (if this
    // strength is greater than that already in the table)
    void CopyLabels(CLabelTable* pLabels, unsigned int Prop);
    // returns the strength of the given label.
    float Strg(CLabel* pLabel, unsigned int Prop);
};

typedef CPtr<CLabelTable> CpCLabelTable;

#endif /* __LABELTABLE_H__ */
