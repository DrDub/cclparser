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

#include "CCLBrackets.h"
#include "yError.h"
#include "CCLUnit.h"

using namespace std;

////////////////////////
// Single CCL Bracket //
////////////////////////

CCCLBracket::CCCLBracket(CCCLUnitVector* pUnits, unsigned int LeftEnd) :
        m_pUnits(pUnits), m_Escapes(-1)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    m_End[LEFT] = LeftEnd;
    m_End[RIGHT] = -1; // covers the last node
}

CCCLBracket::~CCCLBracket()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

int
CCCLBracket::AddToSynStruct(CSynStruct* pSynStruct, unsigned int LastNode)
{
    if(!pSynStruct) {
        yPError(ERR_MISSING, "syntactic structure pointer missing");
    }

    // loop over the positions covered by this bracket. If the position
    // is covered by a bracket, add the bracket. Otherwise, add the terminals.
    
    unsigned int Pos = m_End[LEFT];
    unsigned int RightEnd = (m_End[RIGHT] < 0) ? LastNode : m_End[RIGHT];
    deque<CpCCCLBracket>::iterator DomIter = m_Dominated.begin();
    vector<int> SubNodes;
    
    while(Pos <= RightEnd) {
        if(DomIter != m_Dominated.end() && Pos >= (*DomIter)->m_End[LEFT]) {
            // Add the dominated bracket
            SubNodes.push_back((*DomIter)->
                               AddToSynStruct(pSynStruct, LastNode));
            Pos = (((*DomIter)->m_End[RIGHT] < 0) ?
                   LastNode : (*DomIter)->m_End[RIGHT]) + 1;
            DomIter++;
        } else {
            // add the terminal

            if(m_pUnits && m_pUnits->size() > Pos && (*m_pUnits)[Pos])
                SubNodes.push_back(pSynStruct->AddTerminal(
                                       (char const*)*((*m_pUnits)[Pos]), ""));
            else
                SubNodes.push_back(pSynStruct->AddTerminal("??",""));
                                   
            Pos++;
        }
    }

    return pSynStruct->AddNonTerminal("", SubNodes);
}

void
CCCLBracket::PrintObj(CRefOStream* pOut, unsigned int Indent,
                      unsigned int SubIndent, eFormat Format,
                      int Parameter)
{
    if(!pOut) {
        yPError(ERR_MISSING, "stream pointer missing");
    }
    
    // indentation string
    string IndentStr(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');
    
    ((ostream&)(*pOut)) << IndentStr << "("; // open the bracket

    SubIndent++;
    IndentStr.assign(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');
    
    // Detemrine the last position in the utterance. This is needed because
    // brackets which cover the last node have -1 as their right end.
    int LastNode = m_pUnits->size() - 1;
    
    // loop over the positions covered by this bracket. If the position
    // is covered by a bracket, print the bracket. Otherwise, print the
    // word at that position.
    int Pos = m_End[LEFT];
    unsigned int RightEnd = (m_End[RIGHT] < 0) ? LastNode : m_End[RIGHT];
    deque<CpCCCLBracket>::iterator DomIter = m_Dominated.begin();
    bool bNewLine = false;
    
    while(Pos <= RightEnd) {
        if(DomIter != m_Dominated.end() && Pos >= (*DomIter)->m_End[LEFT]) {
            // print the dominated bracket
            bNewLine = true;
            ((ostream&)(*pOut)) << endl;
            (*DomIter)->PrintObj(pOut, Indent, SubIndent, Format, Parameter);
            Pos = (((*DomIter)->m_End[RIGHT] < 0) ?
                   LastNode : (*DomIter)->m_End[RIGHT]) + 1;
            DomIter++;
        } else {
            // print the word at the given position
            if(bNewLine) {
                ((ostream&)(*pOut)) << endl;
                ((ostream&)(*pOut)) << IndentStr << " ";
            }
            if(m_pUnits && m_pUnits->size() > Pos && (*m_pUnits)[Pos]) {
                // add a space between consecutive words
                if(!bNewLine && Pos != m_End[LEFT])
                    ((ostream&)(*pOut)) << " ";
                (*m_pUnits)[Pos]->PrintObj(pOut, 0, 0, Format, Parameter);
            } else
                ((ostream&)(*pOut)) << Pos;
            Pos++;
        }
    }

    // close the bracket
    ((ostream&)(*pOut)) << ")";
}

//////////////////////
// All CCL Brackets //
//////////////////////

CCCLBrackets::CCCLBrackets(CTracing* pTracing) : CCCLSet(pTracing),
                                                 m_BracketingUpTo(-1)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    // initialization without brackets
}

CCCLBrackets::~CCCLBrackets()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CCCLBrackets::Clear()
{
    ClearSet(); // clear the underlying set
    m_CoverLast.clear();
    m_MaxNotEnd.clear();
    m_B1x.clear();
    m_B2x.clear();
    m_BracketingUpTo = -1;
}

CRvector<CpCCCLBracket>*
CCCLBrackets::GetMaxBrackets()
{
    CRvector<CpCCCLBracket>* pMaxBrackets =
        new CRvector<CpCCCLBracket>(m_MaxNotEnd);

    if(m_CoverLast.size()) {
        while(pMaxBrackets->size() &&
              pMaxBrackets->back()->m_End[LEFT] >=
              m_CoverLast.front()->m_End[LEFT]) {
            pMaxBrackets->pop_back();
        }
        pMaxBrackets->push_back(m_CoverLast.front());
    }

    return pMaxBrackets;
}

void
CCCLBrackets::UpdateBrackets()
{
    if(m_BracketingUpTo == LastNode())
        return; // bracketing already calculated
    m_BracketingUpTo = LastNode();
    
    if(LastNode() < 0)
        return;
    
    // Fix the right end of all brackets which do not need to be extended

    // A bracket needs to be extended only if there is a link from inside
    // that bracket to the last word.

    SLinkPair Inbound(GetInbound(LastNode(),LEFT));
    CpCCCLBracket pMaxNotExtended; // maximal bracket not extended
    
    // Because every bracket is removed here at most once, the total
    // running time is linear in the number of brackets (which is
    // linear in the number of nodes).
    while(m_CoverLast.size()) {
        if(Inbound.m_Depth > 1 // no inbound link
           || m_CoverLast.back()->m_End[LEFT] > Inbound.m_End // doesnt cover
           || (Inbound.m_Depth == 1 && // B_1(x) not extended by depth 1
               m_CoverLast.back() == m_B1x[Inbound.m_End])) {
            
            // do not extend the bracket
            m_CoverLast.back()->m_End[RIGHT] = LastNode() - 1;
            pMaxNotExtended = m_CoverLast.back();
            m_CoverLast.pop_back();
        } else
            break;
    }

    // Add the maximal bracket which was not extended to the list of
    // maximal brackets not covering the last node.

    if(pMaxNotExtended) {
        while(m_MaxNotEnd.size()) {
            if(m_MaxNotEnd.back()->m_End[LEFT] < pMaxNotExtended->m_End[LEFT])
                break;
            m_MaxNotEnd.pop_back();
        }
        m_MaxNotEnd.push_back(pMaxNotExtended);
    }
    
    // Check whether a depth 1 link from the prefix to the last node
    // requires a new bracket to be created
    if(Inbound.m_Depth == 1 && m_B1x[Inbound.m_End]->m_Escapes < 0) {
        // now there is an escaping node
        m_B1x[Inbound.m_End]->m_Escapes = LastNode();
        // create a bracket and add it as the smallest bracket covering
        // the last word.
        CpCCCLBracket pNew =
            new CCCLBracket(GetUnits(), m_B1x[Inbound.m_End]->m_End[LEFT]);
        
        m_B2x[Inbound.m_End] = pNew;
        // this bracket can only dominate B_1(x)
        if(m_CoverLast.size()) {
            m_CoverLast.back()->m_Dominated.pop_back(); // pop B_1(x)
            m_CoverLast.back()->m_Dominated.push_back(pNew);
        }
        pNew->m_Dominated.push_back(m_B1x[Inbound.m_End]);
        
        m_CoverLast.push_back(pNew);
    }

    // Add brackets which are generated by the last word

    bool bB1Created = false;
    bool bB2Created = false;

    // B_1(x_k)
    
    unsigned int LastOutbound0 = GetLastOutbound0(LastNode(), LEFT);
    
    if(LastOutbound0 < LastNode() && m_B1x[LastOutbound0]->m_End[RIGHT] < 0) {
        // the node has already been created.
        m_B1x.push_back(m_B1x[LastOutbound0]);
    } else {
        m_B1x.push_back(new CCCLBracket(GetUnits(),
                                        GetLongestPath(LastNode(), LEFT, 0)));
        // Set the brackets dominated by this bracket
        for(vector<CpCCCLBracket>::reverse_iterator
                Iter = m_MaxNotEnd.rbegin() ;
            Iter != m_MaxNotEnd.rend() ; Iter++) {
            if((*Iter)->m_End[LEFT] < m_B1x.back()->m_End[LEFT])
                break;
            m_B1x.back()->m_Dominated.push_front(*Iter);
        }
        bB1Created = true;
    }
    
    // B_2(x_k)

    // find the last link of depth 1
    SLinkPair LastLink(GetLastOutbound(LastNode(),LEFT));

    // Should a bracket be created?
    if(LastLink.m_Depth != 1) {
        m_B2x.push_back(NULL);
    } else {
        unsigned int B2XkLeftEnd = GetLongestPath(LastNode(), LEFT);
        // check whether this bracket was already created. If it was created,
        // its position in the brackets which cover the last node depends
        // on whether B_1(x_k) was already added.
        unsigned int B2InList = bB1Created ? 1 : 2; 

        if(m_CoverLast.size() < B2InList ||
           m_CoverLast[m_CoverLast.size()-B2InList]->m_End[LEFT] !=
           B2XkLeftEnd) {
            // create the bracket
            m_B2x.push_back(new CCCLBracket(GetUnits(), B2XkLeftEnd));
            m_B1x.back()->m_Escapes = LastNode();

            // Set the brackets dominated by this bracket
            for(vector<CpCCCLBracket>::reverse_iterator
                    Iter = m_MaxNotEnd.rbegin() ;
                Iter != m_MaxNotEnd.rend() ; Iter++) {
                if((*Iter)->m_End[LEFT] < m_B2x.back()->m_End[LEFT])
                    break;
                if((*Iter)->m_End[LEFT] < m_B1x.back()->m_End[LEFT])
                    m_B2x.back()->m_Dominated.push_front(*Iter);
            }
            
            bB2Created = true;
        } else {
            // the bracket already exists, record it
            m_B2x.push_back(m_CoverLast[m_CoverLast.size()-B2InList]);
        }
    }

    if(bB2Created) {
        if(!bB1Created)
            m_CoverLast.pop_back(); // remove B_1(x_k)
        
        // update the bracket dominated by the directly dominating bracket
        if(m_CoverLast.size()) {
            while(m_CoverLast.back()->m_Dominated.size()) {
                if(m_CoverLast.back()->m_Dominated.back()->m_End[LEFT] <
                   m_B2x.back()->m_End[LEFT])
                    break;
                m_CoverLast.back()->m_Dominated.pop_back();
            }
            m_CoverLast.back()->m_Dominated.push_back(m_B2x.back());
        }

        m_CoverLast.push_back(m_B2x.back());
    }

    if(bB1Created || bB2Created) {

        if(m_CoverLast.size()) {
            // update the bracket dominated by the directly dominating bracket
            while(m_CoverLast.back()->m_Dominated.size()) {
                if(m_CoverLast.back()->m_Dominated.back()->m_End[LEFT] <
                   m_B1x.back()->m_End[LEFT])
                    break;
                m_CoverLast.back()->m_Dominated.pop_back();
            }
            m_CoverLast.back()->m_Dominated.push_back(m_B1x.back());
        }

        m_CoverLast.push_back(m_B1x.back());
    }
}

bool
CCCLBrackets::IncNodeNum(CCCLUnit* pNextUnit)
{
    // first, call the base class to see whether the node number can be
    // increased.
    if(!CCCLSet::CanIncNodeNum())
        return false;

    // Update the brackets
    UpdateBrackets();
    
    // Increment the node number
    return CCCLSet::IncNodeNum(pNextUnit);
}

bool
CCCLBrackets::CalcBrackets()
{
    // indicate that no more links may be added between the last word
    // and the prefix (returns false if there is a resolution violation).
    if(!CloseSet())
        return false;
    
    // Update the brackets
    UpdateBrackets();

    return true;
}

CSynStruct*
CCCLBrackets::GetSynStruct()
{
    CSynStruct* pSynStruct(new CSynStruct());
    vector<int> SubNodes;
    
    // First, create a list of maximal brackets    
    CPtr<CRvector<CpCCCLBracket> > pMaxBrackets = GetMaxBrackets();
    
    // Add the maximal brackets (and dominated brackets and nodes)
    // to the syntactic structure.
    for(vector<CpCCCLBracket>::iterator Iter = pMaxBrackets->begin() ;
        Iter != pMaxBrackets->end() ; Iter++) {
        SubNodes.push_back((*Iter)->AddToSynStruct(pSynStruct, LastNode()));
    }

    // if there is more than one top node, add a top bracket
    if(pMaxBrackets->size() > 1)
        pSynStruct->AddNonTerminal("", SubNodes);

    return pSynStruct;
}

void
CCCLBrackets::PrintObj(CRefOStream* pOut, unsigned int Indent,
                       unsigned int SubIndent, eFormat Format,
                       int Parameter)
{
    if(!pOut) {
        yPError(ERR_MISSING, "stream pointer missing");
    }
    
    // First, create a list of maximal brackets
    
    CPtr<CRvector<CpCCCLBracket> > pMaxBrackets = GetMaxBrackets();

    // indentation string
    string IndentStr(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');
    
    ((ostream&)(*pOut)) << IndentStr << "(" << endl; // open top bracket

    SubIndent++;
    
    // Loop over the brackets and print each one of them.
    for(vector<CpCCCLBracket>::iterator Iter = pMaxBrackets->begin() ;
        Iter != pMaxBrackets->end() ; Iter++) {
        if(Iter != pMaxBrackets->begin())
            ((ostream&)(*pOut)) << endl;
        (*Iter)->PrintObj(pOut, Indent, SubIndent, Format, Parameter);
    }

    // close top bracket
    ((ostream&)(*pOut)) << ")";
}
