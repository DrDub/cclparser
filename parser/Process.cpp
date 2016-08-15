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

#include "Process.h"
#include "yError.h"

using namespace std;

//
// Auxiliary classes to store symbols in the queue
//

// Base class (stores the labels)

CProcessSymbol::CProcessSymbol()
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CProcessSymbol::CProcessSymbol(vector<string>& Labels) :
        m_Labels(Labels)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CProcessSymbol::~CProcessSymbol()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

// Derived class for storing a unit

CProcessUnit::CProcessUnit(string const& UnitName) :
        CProcessSymbol(), m_UnitName(UnitName)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CProcessUnit::CProcessUnit(string const& UnitName, vector<string>& Labels) :
        CProcessSymbol(Labels), m_UnitName(UnitName)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CProcessUnit::~CProcessUnit()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

// Derived class for storing punctuation

CProcessPunct::CProcessPunct(ePunctType PunctType) :
        CProcessSymbol(), m_PunctType(PunctType)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CProcessPunct::CProcessPunct(ePunctType PunctType, vector<string>& Labels) :
        CProcessSymbol(Labels), m_PunctType(PunctType)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CProcessPunct::~CProcessPunct()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

//
// Process base class
//

CProcessBase::CProcessBase(CParser* pParser, CCmdArgOpts* pArgs,
                           vector<CpCEvaluator>& Evaluators)
        : m_pParser(pParser), m_Evaluators(Evaluators),
          m_NumSymbolsToParser(0), m_bMatches(false)
{
    // Create filter
    if(pArgs)
        m_pFilter = new CSynAndFilter(pArgs);
}

bool
CProcessBase::ShouldStoreSymbolsOnQueue()
{
    // Symbols should be stored on the queue if there are both a
    // filter and a parser defined and the m_pSynStruct object is NULL.

    return(m_pParser && !m_pSynStruct && m_pFilter &&
           !m_pFilter->MatchesAll());
}

void
CProcessBase::ClearSymbolQueue()
{
    while(!m_SymbolQueue.empty())
        m_SymbolQueue.pop();
}

void
CProcessBase::TransferQueueToParser()
{
    if(!m_pParser)
        return;
    
    while(!m_SymbolQueue.empty()) {
        CpCProcessSymbol pSymbol = m_SymbolQueue.front();
        m_SymbolQueue.pop();
        if(!pSymbol) {
            yPError(ERR_MISSING, "missing symbol pointer");
        }
                    
        if(pSymbol->GetType() == eUnit) {
            CpCProcessUnit pUnit = (CProcessUnit*)pSymbol.Ptr();
            m_pParser->PushInputUnit(pUnit->m_UnitName,
                                     pUnit->m_Labels);
        } else if(pSymbol->GetType() == ePunct) {
            CpCProcessPunct pPunct = (CProcessPunct*)pSymbol.Ptr();
            m_pParser->PushInputPunct(pPunct->m_PunctType);
        } else {
            yPError(ERR_OUT_OF_RANGE, "unknown symbol type");
        }

        m_NumSymbolsToParser++;
    }
}

void
CProcessBase::ApplyEvaluators()
{
    if(!m_NumSymbolsToParser)
        return; // nothing parsed

    if(!m_Evaluators.size() || !m_pSynStruct)
        return; // nothing to evaluate
        
    CpCSynStruct pParsedSynStruct;
        
    for(vector<CpCEvaluator>::iterator Iter = m_Evaluators.begin() ;
        Iter != m_Evaluators.end() ; Iter++) {
        if(*Iter) {
            switch((*Iter)->GetType()) {
                case CEvaluator::eSyn:
                    if(!pParsedSynStruct &&
                       !(pParsedSynStruct = m_pParser->GetSynStruct())) {
                        yPError(ERR_SHOULDNT, "failed to get syn struct");
                    }
                    
                    (*Iter)->FromSynStruct(m_pSynStruct, pParsedSynStruct);
                    break;
                default:
                    yPError(ERR_OUT_OF_RANGE, "unknown evaluator type");
            }
        }
    }
}

void
CProcessBase::OutputParse()
{
    if(!m_pParser || !m_pParser->IsParseCycle())
        return; // not a parse
    
    // print the original input (depending on printing mode)
    if(PrintingModeOn(PMODE_SOURCE_TEXT))
        GetOutputStream() << m_ObjSource << endl;

    // Print the parse (always)
    m_pParser->PrintObj(GetOutputStreamObj(), 0, 0, ParsePrintingFormat(), 0);
    GetOutputStream() << endl << endl;

    // Print evaluation
    if(PrintingModeOn(PMODE_EXTRA_EVAL)) {
        for(vector<CpCEvaluator>::iterator Iter = m_Evaluators.begin() ;
            Iter != m_Evaluators.end() ; Iter++) {
            if(*Iter)
                (*Iter)->PrintLastEval(GetOutputStream());
        }
    }
}

void
CProcessBase::ClearProcess()
{
    if(m_pParser)
        m_pParser->ClearUtterance();

    m_pSynStruct = NULL;
    m_ObjSource = "";
    m_NumSymbolsToParser = 0;
    ClearSymbolQueue();
    m_bMatches = false;
}

bool
CProcessBase::ShouldProcessSymbols()
{
    return (m_pParser &&
            (!m_pFilter || m_pFilter->MatchesAll() || m_bMatches ||
             !m_pSynStruct));
}

bool
CProcessBase::ObjectMatches()
{
    return (!m_pFilter || m_pFilter->MatchesAll() || m_bMatches);
}

void
CProcessBase::ProcessSynStruct(CSynStruct* pSynStruct,
                               string const& ObjSource)
{
    // Store the objects
    m_pSynStruct = pSynStruct;
    m_ObjSource = ObjSource;

    m_bMatches = (!m_pFilter || m_pFilter->Match(m_pSynStruct));

    if(m_bMatches) {
        // If the object matches, transfer the symbols from the queue to
        // the parser. If there is no parser, output the object to
        // the output file.
        if(m_pParser) { // Parsing mode
            TransferQueueToParser();
        } else { // Filter mode
            // Print the object string to the output file
            GetOutputStream() << m_ObjSource.c_str();
        }
    } else {
        // if the object does not match, discard any symbols already on
        // the queue
        ClearSymbolQueue();
    }
}

// Process the next punctuation symbol

void
CProcessBase::ProcessPunct(ePunctType PunctType)
{
    // if no parser is defined, discard this symbol
    if(!m_pParser)
        return;

    // For backward compatibility, terminating punctuation symbols may
    // be discarded.
    if(g_DiscardTerminatingPunct && IsTerminatingPunct(PunctType))
        return;
    
    if(PunctType == eEoUtterance) {
        // End of utterance

        // If there are symbols on the queue (which requires there to be
        // both a parser and a filter), check whether the filter matches
        // the current SynStruct (which should be NULL). If yes, transfer
        // the symbols to the parser.
        
        if(m_pFilter && !m_SymbolQueue.empty()) {
            if(m_bMatches = m_pFilter->Match(m_pSynStruct)) {
                TransferQueueToParser();
            } else {
                // Clear the process and parser
                ClearProcess();
                return;
            }
        }

        // If the utterance is being processed, complete procesing it
        if(!m_pFilter || m_bMatches) {
            // Send the EoUtterance symbol to the parser (but don't count it)
            m_pParser->PushInputPunct(PunctType);
        
            // Apply evaluators
            ApplyEvaluators();
        
            // End of utterance output
            OutputParse();
        }
        
        // Clear the utterance and parser
        ClearProcess();

        return;
    }

    // Other punctuation.
    
    if(ShouldStoreSymbolsOnQueue()) {
        // Push punctuation on queue
        m_SymbolQueue.push(new CProcessPunct(PunctType));
        return;
    } else if(!m_pFilter || m_pFilter->MatchesAll() || m_bMatches) {
        // transfer to parser
        if(m_pParser) {
            m_pParser->PushInputPunct(PunctType);
            m_NumSymbolsToParser++;
        }
        return;
    }

    // otherwise, discard (filter does not match)
}

// Process the next unit (word). Optionally, a list of labels
// may be given.
void
CProcessBase::ProcessUnit(string const& Name, vector<string>& Labels)
{
    // Should the unit be discarded, stored on the queue or transferred
    // to the parser.

    if(!m_pParser)
        return; // discard the unit

    if(ShouldStoreSymbolsOnQueue()) {
        // Push unit on queue
        m_SymbolQueue.push(new CProcessUnit(Name, Labels));
        return;
    } else if(!m_pFilter || m_pFilter->MatchesAll() || m_bMatches) {
        // transfer to parser
        if(m_pParser) {
            m_pParser->PushInputUnit(Name, Labels);
            m_NumSymbolsToParser++;
        }
        return;
    } else
        return; // discard, filter does not match
}

// Same as above with a single label
void
CProcessBase::ProcessUnit(string const& Name, string const& Label)
{
    vector<string> Labels(1,Label);

    ProcessUnit(Name, Labels);
}

// Same as above, but without having to specify the labels
void
CProcessBase::ProcessUnit(string const& Name)
{
    vector<string> Labels; // empty sequence of labels

    ProcessUnit(Name, Labels);
}




