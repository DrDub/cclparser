#ifndef __CCLSTAT_H__
#define __CCLSTAT_H__

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

#include "PrsConst.h"
#include "Stat.h"
#include "Label.h"
#include "Globals.h"

/////////////////
// Value Class //
/////////////////

//
// simple CCL parser label value class
//

class CCCLVal : public CRvector<float>, public CPrintObj
{
public:
    CCCLVal();
    CCCLVal(unsigned int Size);
    ~CCCLVal();
    bool IsEmpty() { return true; }

    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
};

///////////////////////////
// CCL Parser Statistics //
///////////////////////////

// Number of entries to store in a top list
#define CCLST_TOP_LENGTH (g_StatisticsTopListMaxLen)
// Default hash table size for statistics tables
#define CCLST_DEFAULT_HASH_SIZE (16) // usually not many entries

// Forward declarations

class CCCLStat;
typedef CPtr<CCCLStat> CpCCCLStat;
class CCCLStatIter;
typedef CPtr<CCCLStatIter> CpCCCLStatIter;

class CCCLStat : public CStat<CLabel, CCCLVal>
{
public:
    //
    // Statistics which apply to each label separately
    //

    enum eLabelStat {
        // The strength at which the given label was seen on adjacent words.
        eSeen = 0,
    };

    //
    // Global statistics
    //

    enum eGlobalStat {
        // Number of times learning was performed
        eLearn,
        // basic blocking statistic - strength at which the beginning/end
        // was seen.
        eBlock,
        // The following statistics are combined with the type
        // to define their code.
        eIn, 
        eOut,
        eEoL,        // not a statistic, only marks the end of the list
    };

    //
    // types of properties
    //

    enum eStatType {
        eBase = 0,    // Base statistics (blocking, learning count, etc.)
        eDerived = 1, // Derived from the base statistics
    };
    
    // The following function converts (property,type) pairs to a single
    // property code.
    static unsigned int PropType2Code(unsigned int Prop, unsigned int Type){
        return Type * eEoL + Prop;
    }
    // This function returns the property coded by the given property code
    static unsigned int Code2Prop(unsigned int Code) {
        return Code % eEoL;
    }
    // This function returns the type of the property coded by the
    // given code
    static unsigned int Code2Type(unsigned int Code) {
        return Code / eEoL;
    }
    
private:
    static CPropConv m_TableConv;
    static CPropConv m_VecConv;

    CpCCCLStat m_pNext; // next attachment statistics
    
public:
    CCCLStat();
    ~CCCLStat();
    bool IsEmpty() { return !(bool)(*this)[eLearn]; }
    // Returns the next statistics object. If bCreate is set and no such
    // object exists, it is created.
    CCCLStat* GetNext(bool bCreate);
private:
    CPropConv& GetTablePropConv() { return m_TableConv; }
    CPropConv& GetVecPropConv() { return m_VecConv; }
public:
    //
    // Access to the properties stored on this object
    //
    // Get the number of properties stored in the vector part
    unsigned int GetVecPropNum() { return m_VecConv.GetPropNum(); }
    // Returns the property which has this local code
    int GetVecPropByCode(int LocalCode) {
        return m_VecConv.GetPropByLocalCode(LocalCode);
    }
    
    CCCLStatIter* GetIter(unsigned int Stat);
private:
    // Printing auxiliary functions
    std::vector<int>& VecStatsToPrint();
    CNameList& VecStatNames();
    unsigned int VecStatFold() { return 6; } // change default vector folding
    CNameList& VecGroupNames();
    std::vector<int>& TableStatsToPrint();
    CNameList& TableStatNames();
    CStatIterPrintObj* GetPrintIter(unsigned int Stat);
};

//
// Vector statistics copy object
//

class CCCLStatVectorCopy : public CStatVectorCopy
{
public:
    CCCLStatVectorCopy(CCCLStat* pStat);
    ~CCCLStatVectorCopy();

    // Access to values by property + type
    float Val(CCCLStat::eGlobalStat Stat, CCCLStat::eStatType Type);
    // return the quotient of the two values given
    float QtVV(CCCLStat::eGlobalStat Stat1, CCCLStat::eStatType Type1,
               CCCLStat::eGlobalStat Stat2, CCCLStat::eStatType Type2);

    // same functions as above, but for the 'eBase' type
    float Val(CCCLStat::eGlobalStat Stat) {
        return Val(Stat, CCCLStat::eBase);
    }
    float QtVV(CCCLStat::eGlobalStat Stat1, CCCLStat::eGlobalStat Stat2) {
        return QtVV(Stat1, CCCLStat::eBase, Stat2, CCCLStat::eBase);
    }

    
};

//
// The following classes create a copy of the statistics object which is
// not changed when the statistics are later modified. This includes
// a static copy of the vector properties and several additional properties
// based on the labels, but does not include a full copy of the labels.
//

// This class provides statistics for a single label property

class CLabelProps
{
private:
    // number of top labels for this property
    unsigned int m_TopNum;
    // number of top properties which are stronger than 'Block'
    unsigned int m_StrongerThanBlock;
    // normalized strength of strongest label
    float m_Strongest;

public:
    // empty constructor
    CLabelProps() : m_TopNum(0), m_StrongerThanBlock(0), m_Strongest(0) {}
    // copy constructor
    CLabelProps(CLabelProps const& LabelProps) :
            m_TopNum(LabelProps.m_TopNum),
            m_StrongerThanBlock(LabelProps.m_StrongerThanBlock) {}
    // constructor based on a specific statistics object and property
    CLabelProps(CCCLStat* pStat, CCCLStat::eLabelStat LabelStat);

    CLabelProps& operator=(CLabelProps const& LabelProps) {
        m_TopNum = LabelProps.m_TopNum;
        m_StrongerThanBlock = LabelProps.m_StrongerThanBlock;
        return (*this);
    }
    
    // set the values based on the given statistics object
    void Set(CCCLStat* pStat, CCCLStat::eLabelStat LabelStat);

    // access to the values
    unsigned int TopNum() { return m_TopNum; }
    unsigned int StrongerThanBlock() { return m_StrongerThanBlock; }
    float Strongest() { return m_Strongest; }
};

class CCCLStatCopy : public CCCLStatVectorCopy
{
private:
    // The following vector records statistics from different label properties.
    // Each vector position is for the corresponding label property.
    std::vector<CLabelProps> m_LabelProps;
    
public:
    CCCLStatCopy(CCCLStat* pStat);
    ~CCCLStatCopy();

    // Add the statistics of the given label property of the given statistics
    // object.
    void Set(CCCLStat* pStat, CCCLStat::eLabelStat LabelStat);

    // access to label properties
    
    // Returns the number of top label for the given property. Returns 0
    // if no statistics are available for this property.
    unsigned int TopNum(CCCLStat::eLabelStat LabelStat);
    // Returns the number of labels of the given property stronger than
    // 'block'. Returns 0 if no statistics are available for this property.
    unsigned int StrongerThanBlock(CCCLStat::eLabelStat LabelStat);
    // returns the ratio between the number of labels stronger than 'block'
    // and the total number of top labels (returns zero if there are
    // no top labels).
    float StrongerThanBlockRatio(CCCLStat::eLabelStat LabelStat);
    // returns the strength of the strongest label
    float Strongest(CCCLStat::eLabelStat LabelStat);
};

typedef CPtr<CCCLStatCopy> CpCCCLStatCopy;

//
// Iterator class
//

class CCCLStatIter : public CStatIter<CLabel, CCCLVal>
{
private:
    // buffer for storing the current entry name
    std::string m_EntryName;
public:
    CCCLStatIter(CCCLStat* pStatTable, unsigned int Prop) :
            CStatIter<CLabel, CCCLVal>(pStatTable, Prop) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    ~CCCLStatIter() {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
private:
    //
    // Printing auxiliary functions
    //

    std::string const& EntryName();
    std::vector<int>& IterStatsAlsoToPrint(unsigned int Stat);
    CNameList& TableStatNames();
};

//
// A pair of statistics collection objects (left and right)
//

typedef CpCCCLStat CTwoCCLStats[SIDE_NUM];

//
// Name list class extension
//

// The following class extends the CNameList class so that it can output
// not only names it has stored in its lists but also names with the
// name+{type number} structure for an arbitrary type number.
// Currently, only the name is printed (without the type number).

class CCCLNameList : public CNameList
{
private:
    // properties of the entries in the list. Currently this only indicates
    // whether the entry already exists or not.
    std::vector<unsigned int> m_Props;
public:
    CCCLNameList(SNameList const* pPairs, char const* pDefault);
    // Operator for retrieving the string for a given code.
    std::string const& operator[](unsigned int Code);
};

#endif /* __CCLSTAT_H__ */
