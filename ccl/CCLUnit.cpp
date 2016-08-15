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

#include "CCLUnit.h"
#include "yError.h"

using namespace std;

////////////////////
// Basic CCL Unit //
////////////////////

CCCLUnit::CCCLUnit(string const& Name) : CUnit(Name)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    m_StopPunct[LEFT] = m_StopPunct[RIGHT] = eNoPunct;
}

CCCLUnit::CCCLUnit(CStrKey* pName) : CUnit(pName)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    m_StopPunct[LEFT] = m_StopPunct[RIGHT] = eNoPunct;
}

CCCLUnit::~CCCLUnit()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

int
CCCLUnit::GetPos()
{
    return m_pNode ? (int)(m_pNode->GetPos()) : -1;
}

void
CCCLUnit::PrintObj(CRefOStream* pOut, unsigned int Indent,
                   unsigned int SubIndent, eFormat Format,
                   int Parameter)
{
    
    if(!pOut) {
        yPError(ERR_MISSING, "output stream missing");
    }
    
    ((ostream&)(*pOut)) <<
        string(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');
    
    if(IS_DEBUG_FORMAT(Format) && m_pNode) {
        // print the links on the left side of the unit
        if(Format == eLongDebug) {
            SLinkPair Inbound = m_pNode->GetInbound(LEFT);
            if(Inbound.m_Depth <= 1) {
                ((ostream&)(*pOut)) << Inbound.m_End
                                    << ((Inbound.m_Depth == 0) ? "> " : ">> ");
            }
        }

        for(vector<SLinkPair>::reverse_iterator
                Iter = m_pNode->OutboundRBegin(LEFT) ;
            Iter != m_pNode->OutboundREnd(LEFT) ; Iter++) {
            ((ostream&)(*pOut)) << (*Iter).m_End
                                << (((*Iter).m_Depth == 0) ? "< " : "<< ");
        }
    }
    
    PrintSymbol(*pOut);

    if(IS_DEBUG_FORMAT(Format) && m_pNode) {

        // print the node number and the links on the right side of the unit
        
        ((ostream&)(*pOut)) << " (" << m_pNode->GetPos() << ") ";
        
        for(vector<SLinkPair>::iterator Iter = m_pNode->OutboundBegin(RIGHT) ;
            Iter != m_pNode->OutboundEnd(RIGHT) ; Iter++) {
            ((ostream&)(*pOut)) << (((*Iter).m_Depth == 0) ? ">" : ">>")
                                << (*Iter).m_End << " ";
        }

        if(Format == eLongDebug) {
            SLinkPair Inbound = m_pNode->GetInbound(RIGHT);
            if(Inbound.m_Depth <= 1) {
                ((ostream&)(*pOut)) << ((Inbound.m_Depth == 0) ? "<" : "<<")
                                    << Inbound.m_End << " ";
            }
        }
    }
}

//////////////////////////////
// CCL Unit with Statistics //
//////////////////////////////

CSCCLUnit::CSCCLUnit(CStrKey* pName, vector<CpCStrKey>& UnitLabels,
                     CTwoCCLStats const CCLStats)
        : CCCLUnit(pName)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    
    // Store the statistics
    m_Stats[LEFT] = CCLStats[LEFT];
    m_Stats[RIGHT] = CCLStats[RIGHT];
    // create a copy of the first statistics
    m_StatCopy[LEFT] = new CCCLStatCopy(m_Stats[LEFT]);
    m_StatCopy[RIGHT] = new CCCLStatCopy(m_Stats[RIGHT]);
    // mark all adjacencies as unused
    m_AdjUsed[LEFT] = m_AdjUsed[RIGHT] = 0;
    
    // Set labels
    
    CpCCCLLabelTable pLabels = GetLabels();
    
    // Set unit labels
    for(vector<CpCStrKey>::iterator Iter = UnitLabels.begin() ;
        Iter != UnitLabels.end() ; Iter++)
        pLabels->SetUnitLabel(*Iter);
    
    // Add adjacency labels (derived from statistics) to the label list
    GetLabels()->SetAdjacencyLabels(LEFT, CCLStats[LEFT]);
    GetLabels()->SetAdjacencyLabels(RIGHT, CCLStats[RIGHT]);
}

CSCCLUnit::~CSCCLUnit()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

CCCLLabelTable*
CSCCLUnit::GetLabels()
{
    if(!m_pLabels)
        m_pLabels = new CCCLLabelTable();

    return m_pLabels;
}

CCCLStat*
CSCCLUnit::GetStats(SCCLAdjPos const& AdjPos, bool bCreate)
{
    if(AdjPos.m_Pos < 0)
        return NULL;
    
    CpCCCLStat pStats = m_Stats[AdjPos.m_Side];
        
    for(int Pos = AdjPos.m_Pos ; Pos > 0 ; Pos--) {
        if(!(pStats = pStats->GetNext(bCreate)))
            return NULL;
    }

    return pStats;
}

CCCLStatCopy*
CSCCLUnit::GetStatCopy(SCCLAdjPos const& AdjPos)
{
    if(AdjPos.m_Pos != 0) {
        yPError(ERR_NOT_YET,
                "statistics copy of non-first adjacency point not supported");
    }

    return m_StatCopy[AdjPos.m_Side];
}

void
CSCCLUnit::SetAdjUsed(SCCLAdjPos const& AdjPos)
{
    if(AdjPos.m_Pos < 0)
        return;

    if(AdjPos.m_Pos >= 8 * sizeof(unsigned int)) {
        yPError(ERR_OUT_OF_RANGE, "adjacency position out of range");
    }

    m_AdjUsed[AdjPos.m_Side] |= (1 << AdjPos.m_Pos);
}

bool
CSCCLUnit::AdjUsed(SCCLAdjPos const& AdjPos)
{
    if(AdjPos.m_Pos >= 8 * sizeof(unsigned int) || AdjPos.m_Pos < 0)
        return false;

    return (m_AdjUsed[AdjPos.m_Side] & (1 << AdjPos.m_Pos));
}
