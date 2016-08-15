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

#include "CCLLearn.h"
#include "yError.h"
#include "yMath.h"

using namespace std;

////////////////////
// Learning Event //
////////////////////

CCCLLearn::CCCLLearn(CCCLSet* pSet, unsigned int LearnPos,
                     SCCLAdjPos const& AdjPos, int AdjUnit)
        : m_AdjPos(AdjPos)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    if(!pSet || LearnPos > pSet->LastNode()) {
        yPError(ERR_MISSING, "no set pointer or position out of utterance");
    }

    m_pUnit = (CSCCLUnit*)(pSet->GetUnit(LearnPos));

    if(AdjUnit >= 0 && AdjUnit <= pSet->LastNode())
        m_pAdjUnit = (CSCCLUnit*)(pSet->GetUnit(AdjUnit));
}

CCCLLearn::~CCCLLearn()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

static void
UpdateLabels(CCCLStat::eLabelStat LabelProp, CCCLStat* pUpdateStats,
             CSCCLUnit* pLabelUnit, unsigned int UpdateSide)
{
    for(CpCLabelIter Iter = pLabelUnit->GetLabels()->GetIter(OP(UpdateSide)) ;
        *Iter ; ++*Iter) {
        pUpdateStats->IncStrg(Iter->Data(), LabelProp, Iter->Strg());
    }
}

void
CCCLLearn::Learn()
{
    // Get the statistics object on which learning is performed.
    CpCCCLStat pStat = m_pUnit->GetStats(m_AdjPos, true);
    unsigned int Side = m_AdjPos.m_Side; 
    
    // increase the 'eLearn' value
    (*pStat)[CCCLStat::eLearn] += 1;

    // Is there blocking here?
    if(!m_pAdjUnit || m_pUnit->GetStopPunct(Side) != eNoPunct) {
        (*pStat)[CCCLStat::eBlock] += 1;
    } else {
        // update the labels
        UpdateLabels(CCCLStat::eSeen, pStat, m_pAdjUnit, Side);
        // update global properties

        SCCLAdjPos OpPos(OP(Side), 0);
        SCCLAdjPos OpOpPos(Side, 0);
        
        if(m_AdjPos.m_Pos == 0) {
            // first adjacency position

            CpCCCLStatCopy pOpCopy = m_pAdjUnit->GetStatCopy(OpPos);
            CpCCCLStatCopy pOpOpCopy = m_pAdjUnit->GetStatCopy(OpOpPos);

            if(pOpCopy->TopNum(CCCLStat::eSeen)) {
                if(!pOpCopy->StrongerThanBlockRatio(CCCLStat::eSeen))
                    (*pStat)[CCCLStat::eIn] -= 1;
                else if(!pOpOpCopy->StrongerThanBlockRatio(CCCLStat::eSeen))
                    (*pStat)[CCCLStat::eIn] += 1;
            }
            
            (*pStat)[CCCLStat::eOut] +=
                m_pAdjUnit->GetStatCopy(OpPos)->QtVV(CCCLStat::eIn,
                                                     CCCLStat::eLearn);

            (*pStat)[CCCLStat::
                     PropType2Code(CCCLStat::eIn,CCCLStat::eDerived)] +=
                m_pAdjUnit->GetStatCopy(OpPos)->QtVV(CCCLStat::eOut,
                                                     CCCLStat::eLearn);
        }
    }
}

////////////////////
// Learning Queue //
////////////////////

CCCLLearnQueue::CCCLLearnQueue()
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CCCLLearnQueue::~CCCLLearnQueue()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif

    // before destroying the queue, realize any unrealized learning events.
    Realize();
}

void
CCCLLearnQueue::Push(CCCLLearn* pLearn)
{
    if(pLearn)
        m_Queue.push(pLearn);
}

void
CCCLLearnQueue::Realize()
{
    while(!m_Queue.empty()) {
        m_Queue.front()->Learn();
        m_Queue.pop();
    }
}

void
CCCLLearnQueue::Clear()
{
    while(!m_Queue.empty())
        m_Queue.pop();
}
