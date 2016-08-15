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

#include "SynStruct.h"
#include "SynBrackets.h"
#include "SynFilter.h"
#include "Globals.h"

using namespace std;

//
// Base class
//

void
CSynFilter::SetTracing(COutFile* pOutputFile, unsigned int TraceTypes)
{
    m_pTracing->Initialize(pOutputFile, TraceTypes);
}


//
// Conjunction filter
//

CSynAndFilter::CSynAndFilter(CSynFilter* pFirstFilter)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    
    if(pFirstFilter)
        m_Filters.push_back(pFirstFilter);
}

CSynAndFilter::CSynAndFilter(CCmdArgOpts* pArgs)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    if(!pArgs)
        return; // no filters
    
    // read the filters from the argument list
    
    if(pArgs->GetFilterMaxWordCount())
        m_Filters.
            push_back(new CSynWordCountFilter(pArgs->GetFilterMaxWordCount()));

    if(pArgs->GetOnlyTopTags().size() > 0) {
        m_Filters.
            push_back(new CSynTopTagFilter(pArgs->GetOnlyTopTags(),false));
    }

    if(pArgs->GetNotTopTags().size() > 0) {
        m_Filters.
            push_back(new CSynTopTagFilter(pArgs->GetNotTopTags(),true));
    }

    if(pArgs->GetNonTrivialFilter()) {
        m_Filters.push_back(new CSynNonTrivialFilter());
    }
}

CSynAndFilter::~CSynAndFilter()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CSynAndFilter::AddFilter(CSynFilter* pFilter)
{
    if(pFilter)
        m_Filters.push_back(pFilter);
}

bool
CSynAndFilter::Match(CSynStruct* pSyn)
{
    for(vector<CpCSynFilter>::iterator Iter = m_Filters.begin() ;
        Iter != m_Filters.end() ; Iter++)
        if(!(*Iter)->Match(pSyn))
            return false;

    return true;
}

void
CSynAndFilter::SetTracing(COutFile* pOutputFile, unsigned int TraceTypes)
{
    // set the tracing object on this object as well as all sub-filters

    CSynFilter::SetTracing(pOutputFile, TraceTypes);
    
    for(vector<CpCSynFilter>::iterator Iter = m_Filters.begin() ;
        Iter != m_Filters.end() ; Iter++)
        (*Iter)->SetTracing(pOutputFile, TraceTypes);
}

//
// Word count filter
//

bool
CSynWordCountFilter::Match(CSynStruct* pSyn)
{
    if(!pSyn)
        return false;

    return (!m_WordCount || pSyn->CountContentUnits() <= m_WordCount);
}

//
// Top label filter
//

CSynTopTagFilter::CSynTopTagFilter(vector<string> const& Tags, bool bNeg)
        : m_Tags(Tags), m_bNeg(bNeg)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CSynTopTagFilter::~CSynTopTagFilter()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

bool
CSynTopTagFilter::Match(CSynStruct* pSyn)
{
    if(m_Tags.empty() || !pSyn) {
        return m_bNeg; // tag does not match
    }
    
    CpCSynNode pTopNode = NULL;
    
    if(!pSyn->NonTerminalNum()) {
        // is there a single terminal?
        if(pSyn->TerminalNum() == 1)
            pTopNode = pSyn->TerminalByNode(0);
        else
            return m_bNeg; // tag does not match
    } else {
        // search for the top labeled node
        CpCNonTerminal pNTNode =
            (CNonTerminal*)pSyn->
            GetNode(CSynStruct::NodeNum2NonTerm(pSyn->NonTerminalNum()-1));

        pTopNode = (CNonTerminal*)(pNTNode.Ptr());
        
        // as long as the top node found does not have a tag and dominates
        // only one node, look at the dominated node.
        while(pNTNode &&
              pNTNode->Tag() == "" && pNTNode->NumDominated() == 1) {
            vector<int>::iterator First = pNTNode->DominatedBegin();
            pTopNode = pSyn->GetNode(*First);
            if(*First >= 0)
                break; // the terminal node is the top node

            pNTNode = (CNonTerminal*)(pTopNode.Ptr());
        }
    }

    if(!pTopNode)
        return m_bNeg; // tag does not match

    // check whether the tag matches one of the tags in the list
    for(vector<string>::iterator Iter = m_Tags.begin() ; Iter != m_Tags.end() ;
        Iter++) {
        if(pTopNode->Tag() == (*Iter))
            return !m_bNeg;
    }

    return m_bNeg;
}


//
// Non-trivial structure filter
//

CSynNonTrivialFilter::CSynNonTrivialFilter()
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CSynNonTrivialFilter::~CSynNonTrivialFilter()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

bool
CSynNonTrivialFilter::Match(CSynStruct* pSyn)
{
    if(!pSyn)
        return false;
    
    CpCSynBrackets pSynBrackets = new CSynBrackets(pSyn, CSynBrackets::eNone,
                                                   true, true, false);

    return (pSynBrackets->NonTermNum() > 1);
}
