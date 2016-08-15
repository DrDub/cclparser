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

#include <algorithm>
#include <iomanip>
#include <istream>
#include <sstream>
#include "Evaluator.h"
#include "SynBrackets.h"
#include "Globals.h"
#include "yError.h"

using namespace std;

void
CEvaluator::SetTracing(COutFile* pOutputFile, unsigned int TraceTypes)
{
    m_Tracing.Initialize(pOutputFile, TraceTypes);
}

CPrecisionAndRecall::CPrecisionAndRecall(bool bActive) :
        CEvaluator(CEvaluator::eSyn),
        m_bActive(bActive),
        m_Precision(0), m_PrecisionMax(0), m_Recall(0), m_RecallMax(0),
        m_LastPrecision(0), m_LastPrecisionMax(0), m_LastRecall(0),
        m_LastRecallMax(0)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CPrecisionAndRecall::~CPrecisionAndRecall()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CPrecisionAndRecall::FromSynStruct(CSynStruct* pStandard, CSynStruct* pParsed)
{
    if(!pStandard || !pParsed || !m_bActive)
        return; // nothing to do here

    CpCSynBrackets pStandardBrackets =
        new CSynBrackets(pStandard, CSynBrackets::eNone, true, true,
                         !g_CountTopBracket);
    CpCSynBrackets pParsedBrackets =
        new CSynBrackets(pParsed, CSynBrackets::eNone, true, true,
                         !g_CountTopBracket);

    DO_TRACE {       
        m_Tracing(TB_EVAL) << "Standard brackets:" << Endl
                           << (CPrintObj*)pStandardBrackets.Ptr();
        m_Tracing(TB_EVAL) << "Parsed brackets:" << Endl
                           << (CPrintObj*)pParsedBrackets.Ptr();
    }
    
    m_Recall +=
        (m_LastRecall = pStandardBrackets->Precision(*pParsedBrackets));
    m_RecallMax +=
        (m_LastRecallMax = pStandardBrackets->NonTermNum());
    m_Precision +=
        (m_LastPrecision = pParsedBrackets->Precision(*pStandardBrackets));
    m_PrecisionMax += (m_LastPrecisionMax = pParsedBrackets->NonTermNum());
}

float
CPrecisionAndRecall::Precision()
{
    return m_PrecisionMax ? m_Precision / m_PrecisionMax : 0;
}

float
CPrecisionAndRecall::Recall()
{
    return m_RecallMax ? m_Recall / m_RecallMax : 0;
}

float
CPrecisionAndRecall::LastPrecision()
{
    return m_LastPrecisionMax ? m_LastPrecision / m_LastPrecisionMax : 0;
}

float
CPrecisionAndRecall::LastRecall()
{
    return m_LastRecallMax ? m_LastRecall / m_LastRecallMax : 0;
}


void
CPrecisionAndRecall::PrintLastEval(ostream& Out)
{
    // Set the printing precision
    Out << fixed << setprecision(4);
    
    Out << g_CommentStr
        <<" Precision" << (g_CountTopBracket ? " (with top)" : " (no top)")
        << ": " << (int)m_LastPrecision << "/"
        << (int)m_LastPrecisionMax << " (" << LastPrecision() << ")" << endl;
    Out << g_CommentStr
        << " Recall" << (g_CountTopBracket ? " (with top)" : " (no top)")
        << ": " << (int)m_LastRecall << "/" << (int)m_LastRecallMax
        << " (" << LastRecall() << ")" << endl;
    Out << endl;
}

// Print evaluation for all structures processed until now.
void
CPrecisionAndRecall::PrintTotalEval(ostream& Out)
{
    // Set the printing precision
    Out << fixed << setprecision(4);
    
    Out << g_CommentStr << " Total precision"
        << (g_CountTopBracket ? " (with top)" : " (no top)") << ": "
        << (int)m_Precision << "/" << (int)m_PrecisionMax
        << " (" << Precision() << ")" << endl;
    Out << g_CommentStr << " Total recall"
        << (g_CountTopBracket ? " (with top)" : " (no top)") << ": "
        << (int)m_Recall << "/" << (int)m_RecallMax
        << " (" << Recall() << ")" << endl;

    Out << g_CommentStr << " F1: ";
    
    if(Recall() == 0 || Precision() == 0)
        Out << "0";
    else
        Out << (2 * Recall() * Precision()) / (Recall() + Precision());

    Out << endl;
}

//
// Grouped recall evaluator
//

CEvalGroupedPnR::CEvalGroupedPnR(bool bActive, eGrouping GroupingType) :
        CEvaluator(CEvaluator::eSyn), m_bActive(bActive),
        m_GroupingType(GroupingType)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    m_pHash = new CEvalGroupedHash();
}

CEvalGroupedPnR::~CEvalGroupedPnR()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

CEvalGroupedVal*
CEvalGroupedPnR::GetHashVal(string const& Key)
{
    CpCStrKey StrKey = new CStrKey(Key);
        
    if(!(*m_pHash)[StrKey].Found())
        (*m_pHash) = new CEvalGroupedVal();
    return m_pHash->Val();
}

void
CEvalGroupedPnR::FromSynStruct(CSynStruct* pStandard, CSynStruct* pParsed)
{
    m_Expected.clear();
    m_Observed.clear();
    m_Matched.clear();

    if(!pStandard || !pParsed || !m_bActive)
        return; // nothing to do here

    CSynBrackets::eLabeling Type;
    
    switch(m_GroupingType) {
        case eTag:
            Type = CSynBrackets::eTag;
            break;
        case eShortTag:
            Type = CSynBrackets::eShortTag;
            break;
        case eTagSeq:
            Type = CSynBrackets::eTagSeq;
            break;
        case eShortTagSeq:
            Type = CSynBrackets::eShortTagSeq;
            break;
        case eLen:
            Type = CSynBrackets::eLen;
            break;
        default:
            yPError(ERR_OUT_OF_RANGE, "unknown grouping type");
    }

    // set labels
    pParsed->SetTerminalTagsFromSuperSequence(pStandard, true, true);
    
    CpCSynBrackets pStandardBrackets =
        new CSynBrackets(pStandard, Type, true, true, !g_CountTopBracket);
    CpCSynBrackets pParsedBrackets =
        new CSynBrackets(pParsed, Type, true, true, !g_CountTopBracket);

    DO_TRACE {
        m_Tracing(TB_EVAL) << "Standard brackets:" << Endl
                           << (CPrintObj*)pStandardBrackets.Ptr();
        m_Tracing(TB_EVAL) << "Parsed brackets:" << Endl
                           << (CPrintObj*)pParsedBrackets.Ptr();
    }

    // precision
    pParsedBrackets->Match(*pStandardBrackets, m_Matched, m_Observed);

    // matched labels are stored below, so only need to store unmatched
    // observed labels
    for(vector<string>::iterator Iter = m_Observed.begin() ;
        Iter != m_Observed.end() ; Iter++)
        GetHashVal(*Iter)->m_Observed++;
    
    // recall
    pStandardBrackets->Match(*pParsedBrackets, m_Matched, m_Expected);
    
    // store matched and unmatched labels in hash table
    for(vector<string>::iterator Iter = m_Matched.begin() ;
        Iter != m_Matched.end() ; Iter++) {
        GetHashVal(*Iter)->m_Matched++;
        GetHashVal(*Iter)->m_Expected++;
        GetHashVal(*Iter)->m_Observed++;
    }
    for(vector<string>::iterator Iter = m_Expected.begin() ;
        Iter != m_Expected.end() ; Iter++)
        GetHashVal(*Iter)->m_Expected++;
}

void
CEvalGroupedPnR::PrintLastEval(std::ostream& Out)
{
    Out << g_CommentStr << " Expected (not matched): ";

    for(vector<string>::iterator Iter = m_Expected.begin() ;
        Iter != m_Expected.end() ; Iter++) {
        if(Iter != m_Expected.begin())
            Out << ", ";
        Out << "-" << *Iter << "-"; // enclosed in - - to simplify finding
    }

    Out << endl << g_CommentStr << " Observed (not matched): ";

    for(vector<string>::iterator Iter = m_Observed.begin() ;
        Iter != m_Observed.end() ; Iter++) {
        if(Iter != m_Observed.begin())
            Out << ", ";
        Out << "." << *Iter << "."; // enclosed in . . to simplify finding
    }
    
    Out << endl << g_CommentStr << " Matched: ";

    for(vector<string>::iterator Iter = m_Matched.begin() ;
        Iter != m_Matched.end() ; Iter++) {
        if(Iter != m_Matched.begin())
            Out << ", ";
        Out << ":" << *Iter << ":"; // enclosed in : : to simplify finding
    }

    Out << endl << endl;
}

typedef std::pair<CpCStrKey, CpCEvalGroupedVal> EvalPair; // key/value pairs

typedef CHashIter<CStrKey, CEvalGroupedVal> CEvalIter;
typedef CPtr<CEvalIter> CpCEvalIter;

// different comparison functions to determine the order in which
// entries are to be printed

// comparison function type definition
typedef bool(*tEvalComp)(EvalPair const&, EvalPair const&);

static bool
EvalExpectedComp(EvalPair const& pA, EvalPair const& pB)
{
    return (pA.second->m_Expected > pB.second->m_Expected);
}

static bool
EvalObservedComp(EvalPair const& pA, EvalPair const& pB)
{
    return (pA.second->m_Observed > pB.second->m_Observed);
}

static bool
EvalOverObservedComp(EvalPair const& pA, EvalPair const& pB)
{
    return ((pA.second->m_Observed - pA.second->m_Matched) >
            (pB.second->m_Observed - pB.second->m_Matched));
}

static bool
EvalUnderObservedComp(EvalPair const& pA, EvalPair const& pB)
{
    return ((pA.second->m_Expected - pA.second->m_Matched) >
            (pB.second->m_Expected - pB.second->m_Matched));
}

void
CEvalGroupedPnR::PrintTotalEval(std::ostream& Out)
{
    EvalPair Array[m_pHash->NumElements()];
    EvalPair* Current = Array;
    
    CpCEvalIter Iter = m_pHash->Begin();

    // populate the array
    while(*Iter) {
        (*Current).first = Iter->GetKey();
        (*Current).second = Iter->GetVal();
        ++(*Iter);
        ++Current;
    }

    // global variable holds list of sorting functions to be used, separated
    // by whitespace
    vector<string> m_Sorters;
    istringstream IStr(g_EvalGroupedOutputSorting);
    while(!IStr.fail()) {
        string Name;
        IStr >> Name;
        if(!IStr.fail())
            m_Sorters.push_back(Name);
    }

    // print evaluation based on each sorter
    for(vector<string>::iterator Iter = m_Sorters.begin() ;
        Iter != m_Sorters.end() ; Iter++) {

        tEvalComp pCompFunc = NULL;
        
        if(*Iter == "Expected") {
            pCompFunc = EvalExpectedComp;
            Out << endl << g_CommentStr << " Most expected: " << endl << endl;
        } else if(*Iter == "Observed") {
            pCompFunc = EvalObservedComp;
            Out << endl << g_CommentStr << " Most observed: " << endl << endl;
        } else if(*Iter == "OverObserved") {
            pCompFunc = EvalOverObservedComp;
            Out << endl << g_CommentStr
                << " Most over observed: " << endl << endl;
        } else if(*Iter == "UnderObserved") {
            pCompFunc = EvalUnderObservedComp;
            Out << endl << g_CommentStr
                << " Most under observed: " << endl << endl;
        } else {
            yPError(ERR_OUT_OF_RANGE, "unknown sorting function");
        }

        sort(Array, Array+m_pHash->NumElements(), pCompFunc);

        // print the top entries
        unsigned int MaxPrint = g_GroupedEvalMaxPrint ?
            min(m_pHash->NumElements(), g_GroupedEvalMaxPrint) :
            m_pHash->NumElements(); 
    
        for(EvalPair* pPos = Array ; (pPos - Array) < MaxPrint ; pPos++) {
            Out << g_CommentStr << " " << pPos->first->GetStr()
                << ": expected: " << pPos->second->m_Expected
                << ": observed: " << pPos->second->m_Observed
                << ": matched: " << pPos->second->m_Matched
                << " precision: ";
            if(!pPos->second->m_Observed)
                Out << "-";
            else
                Out << (float)(pPos->second->m_Matched) /
                    (float)(pPos->second->m_Observed);
            Out << " recall: ";
            if(!pPos->second->m_Expected)
                Out << "-";
            else
                Out << (float)(pPos->second->m_Matched) /
                    (float)(pPos->second->m_Expected);
            if(pPos->second->m_Expected && pPos->second->m_Observed) {
                Out << " F1: "
                    << (float)(2 * pPos->second->m_Matched) /
                    ((float)(pPos->second->m_Expected) +
                     (float)(pPos->second->m_Observed));
            }
            Out << endl;
        }
    }

    Out << endl;
}
