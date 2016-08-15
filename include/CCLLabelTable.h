#ifndef __CCLLABELTABLE_H__
#define __CCLLABELTABLE_H__

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
#include "Globals.h"
#include "PrsConst.h"
#include "CCLStat.h"

//////////////////////////////////////////
// Label Table Class for the CCL Parser //
//////////////////////////////////////////

// maximal number of labels (on each side)
#define CCLLT_MAX_LABELS (g_MaxLabels)

//
// The following class stores the labels assigned to a unit used by the
// CCL parser. It has two
// properties - LEFT and RIGHT - indicating the side of the unit the labels
// are attached on.
//

class CCCLLabelTable : public CLabelTable
{
public:
    CCCLLabelTable() :
            CLabelTable(SIDE_NUM, CCLLT_MAX_LABELS, CCLLT_MAX_LABELS, true)
        {
#ifdef DETAILED_DEBUG
            IncObjCount();
#endif
        }
    ~CCCLLabelTable() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

    void SetAdjacencyLabels(unsigned int Side, CCCLStat* pStat);
    void SetUnitLabel(CStrKey* pString, float Strg = 1,
                      unsigned int Side = BOTH_SIDES);
};

typedef CPtr<CCCLLabelTable> CpCCCLLabelTable;

#endif /* __CCLLABELTABLE_H__ */
