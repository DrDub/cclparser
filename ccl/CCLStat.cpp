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
#include <sstream>
#include <algorithm>
#include "CCLStat.h"
#include "yVector.h"

using namespace std;

///////////////////////
// CCL Parsing Value //
///////////////////////

CCCLVal::CCCLVal()
{
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
}

CCCLVal::CCCLVal(unsigned int Size) :
        CRvector<float>(Size)
{
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
}


CCCLVal::~CCCLVal()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif  
}

void
CCCLVal::PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter)
{
    return; // does nothing
}

////////////////////////////
// CCL Parsing Statistics //
////////////////////////////

static int pCCLStatsTop[] = {
    CCCLStat::eSeen,
    (-1)
};

static int pCCLStatsNoTop[] = {
    (-1)
};

static int pCCLStats[] = {
    CCCLStat::eLearn,
    CCCLStat::eBlock,
    (-1)
};

CPropConv CCCLStat::m_TableConv(pCCLStatsTop, pCCLStatsNoTop);
CPropConv CCCLStat::m_VecConv(NULL, pCCLStats, true);

CCCLStat::CCCLStat() :
        CStat<CLabel, CCCLVal>(m_TableConv.TopListNum(), CCLST_TOP_LENGTH,
                               CCLST_DEFAULT_HASH_SIZE, false, 2)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CCCLStat::~CCCLStat()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

CCCLStat*
CCCLStat::GetNext(bool bCreate)
{
    if(bCreate && !m_pNext)
        m_pNext = new CCCLStat();

    return m_pNext;
}

CCCLStatIter*
CCCLStat::GetIter(unsigned int Stat)
{
    return new CCCLStatIter(this, Stat);
}

//
// Printing auxiliary functions
//

// Tables holding names of statistics

static const SNameList pVecStatNameList[] = {
    CCCLStat::eLearn, "Learned",
    CCCLStat::eBlock, "Blocked",
    CCCLStat::eIn, "In",
    CCCLStat::eOut, "Out",
    0, NULL
};

static CCCLNameList TheVecStatNames(pVecStatNameList, "??");

static const SNameList pVecGroupNameList[] = {
    CCCLStat::eBase, "",
    CCCLStat::eDerived, "Derived",
    0, NULL
};

static CCCLNameList TheVecGroupNames(pVecGroupNameList, "??");

static const SNameList pTableStatNameList[] = {
    CCCLStat::eSeen, "Seen",
    0, NULL
};

static CNameList TheTableStatNames(pTableStatNameList, "??");


CNameList&
CCCLStat::VecStatNames()
{
    return TheVecStatNames;
}

CNameList&
CCCLStat::VecGroupNames()
{
    return TheVecGroupNames;
}

CNameList&
CCCLStat::TableStatNames()
{
    return TheTableStatNames;
}

static int pStatsToPrintInit[] = {
    CCCLStat::eLearn,
    CCCLStat::eBlock,
    (-1)
};

static CVector<int> StatsToPrint(pStatsToPrintInit, -1);
// Number of properties stored in the above vector. This may be smaller
// than the length of the vector since the vector may also hold printing
// control codes.
static int PropNum = 0;

vector<int>&
CCCLStat::VecStatsToPrint()
{
    if(m_VecConv.GetPropNum() != PropNum) {
        // Property list has changed, have to update the list of properties
        // to be printed. All properties are printed, ordered by their
        // property number.
        StatsToPrint.clear();
        PropNum = m_VecConv.GetPropNum();
        
        vector<int> Temp;
        for(unsigned int Code = 0 ; Code < m_VecConv.GetPropNum() ; Code++)
            Temp.push_back(m_VecConv.GetPropByLocalCode(Code));

        sort(Temp.begin(), Temp.end());

        // Copy the sorted properties to the static vector while inserting
        // line breaks and the group name between properties of different
        // groups.
        int Type = -1;
        for(vector<int>::iterator Iter = Temp.begin() ; Iter != Temp.end() ;
            Iter++) {
            if(Type != Code2Type(*Iter)) {
                if(Iter != Temp.begin())
                    StatsToPrint.push_back(CStatPrintObj::eCR);
                StatsToPrint.push_back(CStatPrintObj::eNoValue);
                Type = Code2Type(*Iter);
                StatsToPrint.push_back(Type);
            }

            StatsToPrint.push_back(*Iter);
        }
    }
        
    return StatsToPrint;
}

static int pSortedToPrintInit[] = {
    CCCLStat::eSeen,
    (-1)
};

static CVector<int> SortedToPrint(pSortedToPrintInit, -1);

vector<int>&
CCCLStat::TableStatsToPrint()
{
    return SortedToPrint;
}

CStatIterPrintObj*
CCCLStat::GetPrintIter(unsigned int Stat)
{
    return (CStatIterPrintObj*)GetIter(Stat);
}

///////////////////////////////////
// Vector statistics copy object //
///////////////////////////////////

CCCLStatVectorCopy::CCCLStatVectorCopy(CCCLStat* pStat) :
        CStatVectorCopy((CStatVector*)pStat)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CCCLStatVectorCopy::~CCCLStatVectorCopy()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

// Access to values by property + type

float
CCCLStatVectorCopy::Val(CCCLStat::eGlobalStat Stat, CCCLStat::eStatType Type)
{
    return (*(CStatVectorCopy*)this)[CCCLStat::PropType2Code(Stat, Type)];
}

float
CCCLStatVectorCopy::QtVV(CCCLStat::eGlobalStat Stat1,
                         CCCLStat::eStatType Type1,
                         CCCLStat::eGlobalStat Stat2,
                         CCCLStat::eStatType Type2)
{
    float Val2 = Val(Stat2, Type2);

    return Val2 ? Val(Stat1, Type1) / Val2 : 0;
}

////////////////////////////
// Statistics copy object //
////////////////////////////

//
// Label property statistics
//

CLabelProps::CLabelProps(CCCLStat* pStat, CCCLStat::eLabelStat LabelStat)
{
    Set(pStat, LabelStat);
}

void
CLabelProps::Set(CCCLStat* pStat, CCCLStat::eLabelStat LabelStat)
{
    if(!pStat) {
        yPError(ERR_MISSING, "statistics pointer missing");
    }

    m_TopNum = pStat->GetTopLength(LabelStat);

    float Block = (*pStat)[CCCLStat::eBlock];
    
    m_StrongerThanBlock = 0;

    CCCLStatIter Iter(pStat, LabelStat);

    if(Iter)
        m_Strongest = Iter.Strg();
    
    for( ; Iter ; ++Iter){
        if(Iter.Strg() > Block)
            m_StrongerThanBlock++;
        else
            break;
    }
}

//
// Copy object
//

CCCLStatCopy::CCCLStatCopy(CCCLStat* pStat) : CCCLStatVectorCopy(pStat)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    Set(pStat, CCCLStat::eSeen);
}

CCCLStatCopy::~CCCLStatCopy()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CCCLStatCopy::Set(CCCLStat* pStat, CCCLStat::eLabelStat LabelStat)
{
    if(m_LabelProps.size() <= (unsigned int)LabelStat)
        m_LabelProps.resize(1+(unsigned int)LabelStat);

    m_LabelProps[(unsigned int)LabelStat].Set(pStat, LabelStat);
}

unsigned int
CCCLStatCopy::TopNum(CCCLStat::eLabelStat LabelStat)
{
    if(m_LabelProps.size() <= (unsigned int)LabelStat)
        return 0;

    return m_LabelProps[(unsigned int)LabelStat].TopNum();
}

unsigned int
CCCLStatCopy::StrongerThanBlock(CCCLStat::eLabelStat LabelStat)
{
    if(m_LabelProps.size() <= (unsigned int)LabelStat)
        return 0;

    return m_LabelProps[(unsigned int)LabelStat].StrongerThanBlock();
}

float
CCCLStatCopy::StrongerThanBlockRatio(CCCLStat::eLabelStat LabelStat)
{
    if(m_LabelProps.size() <= (unsigned int)LabelStat ||
       !m_LabelProps[(unsigned int)LabelStat].TopNum())
        return 0;

    return (float)m_LabelProps[(unsigned int)LabelStat].StrongerThanBlock() /
        (float)m_LabelProps[(unsigned int)LabelStat].TopNum();
}

float
CCCLStatCopy::Strongest(CCCLStat::eLabelStat LabelStat)
{
    if(m_LabelProps.size() <= (unsigned int)LabelStat)
        return 0;

    return m_LabelProps[(unsigned int)LabelStat].Strongest();
}

///////////////////////////////////////////
// Iterator printing auxiliary functions //
///////////////////////////////////////////

string const&
CCCLStatIter::EntryName()
{
    if(Data())
        Data()->LabelString(m_EntryName);
    else
        m_EntryName = "<NULL>";

    return m_EntryName;
}

static int pAdditionalToPrintInit[] = {
    (-1)
};

static CVector<int> AdditionalToPrint(pAdditionalToPrintInit, -1);

static int pAdditionalToPrintOpInit[] = {
    (-1)
};

static CVector<int> AdditionalToPrintDefault(NULL, -1);

vector<int>&
CCCLStatIter::IterStatsAlsoToPrint(unsigned int Stat)
{
    if(Stat == CCCLStat::eSeen)
        return AdditionalToPrint;
    
    return AdditionalToPrintDefault;
}

CNameList&
CCCLStatIter::TableStatNames()
{
    return TheTableStatNames;
}


//
// Special flexible name list class extension
//

CCCLNameList::CCCLNameList(SNameList const* pPairs, char const* pDefault) :
        CNameList(pPairs, pDefault)
{
    // no additional initialization
}

string const&
CCCLNameList::operator[](unsigned int const Code)
{
    if(!IsDefined(Code)) {
        ostringstream Ostr(ios::out);
        Ostr << GetName(CCCLStat::Code2Prop(Code));
        // Currently the code type is printed at the beginning of each line,
        // so there is no need to add it here.
//      Ostr << " " << CCCLStat::Code2Type(Code);
        AddName(Code, Ostr.str());
    }

    return GetName(Code);
}
