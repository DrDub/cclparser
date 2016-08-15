#ifndef __CCLUNIT_H__
#define __CCLUNIT_H__

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

#include "Unit.h"
#include "CCLUnitDecl.h"
#include "CCLSet.h"
#include "CCLLabelTable.h"
#include "CCLStat.h"
#include "Punct.h"

//
// This file defines the unit used by the CCL parser. The basic unit
// simply has a link to the CCL set node which describes the links
// associated with this unit.
// The unit also records any stopping punctuation which appears to its
// left and right.
//

class CCCLUnit : public CUnit
{
private:
    CpCCCLNode m_pNode;
    // If there is a stopping punctuation on the given side then this is
    // set to the type of that punctuation.
    // If no such punctuation exists, this is set to eNoPunct.
    ePunctType m_StopPunct[SIDE_NUM];
    
public:
    CCCLUnit(std::string const& Name);
    CCCLUnit(CStrKey* pName);
    ~CCCLUnit();

    // returns the position of the unit in the utterance. Returns -1 if
    // this position has not (yet) been determined.
    int GetPos();
    
    void SetNode(CCCLNode* pNode) { m_pNode = pNode; }

    void SetStopPunct(unsigned int Side, ePunctType Punct) {
        m_StopPunct[Side] = Punct;
    }

    ePunctType GetStopPunct(unsigned int Side) {
        return m_StopPunct[Side];
    }
    
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
};

typedef CPtr<CCCLUnit> CpCCCLUnit;

//
// Adjacency position structure
//

// An adjacncy position is given by two values: the side of the adjacency
// and the position of the adjacency (0,1,2,...)

struct SCCLAdjPos
{
public:
    unsigned int m_Side; // the side of the adjacency
    int m_Pos;           // position of the adjacency (-1 if no position)

    SCCLAdjPos(unsigned int Side, int Pos) :
            m_Side(Side), m_Pos(Pos) {}

    SCCLAdjPos& operator=(SCCLAdjPos const& AdjPos) {
        m_Side = AdjPos.m_Side;
        m_Pos = AdjPos.m_Pos;
        return (*this);
    }
};

//
// A derived class of the basic unit stores the statistics and labels
// used in determining links (and their properties) between pairs of words.
//

class CSCCLUnit : public CCCLUnit
{
private:
    // Labels on this unit (left and right)
    CpCCCLLabelTable m_pLabels;
    // left and right statistics (collected)
    CpCCCLStat m_Stats[SIDE_NUM];
    // read-only copy of the statistics for each adjacency position
    CpCCCLStatCopy m_StatCopy[SIDE_NUM]; 
    // indicator which adjacency positions were already used for attachment
    // (a single bit for each position). The number of adjacency positions
    // is restricted to 32, but this seems to be no restriction in practice.
    unsigned int m_AdjUsed[SIDE_NUM];
public:

    CSCCLUnit(CStrKey* pName, std::vector<CpCStrKey>& UnitLabels,
              CTwoCCLStats const CCLStats);

    ~CSCCLUnit();

    CCCLLabelTable* GetLabels();

    // return the statistics object for the given adjacency position
    // If 'bCreate' is true, the object is created if it does
    // not yet exist (otherwise, NULL is returned).
    CCCLStat* GetStats(SCCLAdjPos const& AdjPos, bool bCreate);
    // Returns the static copy of the statistics on the given
    // adjacency position. Returns NULL if such a copy does not exist.
    // A NULL return value does not necessarily mean that the original
    // statistics object does not exist but only that no copy of it
    // was created when the unit was constructed. Currently, only a copy
    // of the first adjacency position is created.
    CCCLStatCopy* GetStatCopy(SCCLAdjPos const& AdjPos);

    // Mark the given adjacency position as used
    void SetAdjUsed(SCCLAdjPos const& AdjPos);
    
    // Returns true if the given adjacency has already been used for
    // attachment to this unit.
    bool AdjUsed(SCCLAdjPos const& AdjPos);
};

typedef CPtr<CSCCLUnit> CpCSCCLUnit;

#endif /* __CCLUNIT_H__ */
