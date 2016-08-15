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

#include "Parser.h"
#include "Globals.h"

using namespace std;

CParser::CParser() :  m_bLearnCycle(false), m_bParseCycle(false), m_UnitNum(0),
                      m_pTracing(new CTracing())
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    
    m_pInput = new CRdeque<CpCSymbol>();
}

CParser::~CParser()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CParser::SetTracing(COutFile* pOutputFile, unsigned int TraceTypes)
{
    m_pTracing->Initialize(pOutputFile, TraceTypes);
}

// Push the next unit onto the input queue, with the given name and
// labels. This function also assigns each unit its (linear) position
// in the utterance.

void
CParser::PushInputUnit(string const& Name, vector<string>& Labels)
{
    // Create a unit

    CpCUnit pUnit = CreateUnit(Name, Labels);

    // Push it on the queue
    m_UnitNum++;
    m_pInput->push_back(pUnit.Ptr());
}

// Same as above with a single label
void
CParser::PushInputUnit(string const& Name, string const& Label)
{
    vector<string> m_Labels; // empty list of labels

    m_Labels.push_back(Label);
    PushInputUnit(Name, m_Labels);
}

// Same as above, but without having to specify the labels

void
CParser::PushInputUnit(std::string const& Name)
{
    vector<string> m_Labels; // empty list of labels

    PushInputUnit(Name, m_Labels);
}

// Push the next punctuation symbol onto the input queue

void
CParser::PushInputPunct(ePunctType Punct)
{
    CpCPunct pPunct = new CPunct(Punct);
    m_pInput->push_back(pPunct.Ptr());

    // If this punctuation symbol terminates the utterance, processing of the
    // utterance should be completed and then the utterance should be cleared.

    if(IsEoUPunct(Punct))
        UtteranceFullyRead();
}

void
CParser::ClearUtterance()
{
    if(m_pInput)
        m_pInput->clear();
    m_UnitNum = 0;

    ClearDerivedParser();
}

//////////////////////////
// Utterance Processing //
//////////////////////////

void
CParser::DoTracingAtStart()
{
    DO_TRACE {
        if(m_pTracing->IsTraceOn(TB_UTTER)) {
            (*m_pTracing)(TB_UTTER) << PRT_BUTTON_CLOSED;
            for(deque<CpCSymbol>::iterator Iter = m_pInput->begin() ;
                Iter != m_pInput->end() ; Iter++)
                (*m_pTracing)(TB_UTTER) << " " << (CPrintObj*)(*Iter).Ptr();
            (*m_pTracing)(TB_UTTER) << Endl << PRT_REGION_START;
        }
    }
}

void
CParser::UtteranceFullyRead()
{
    DO_TRACE {
        DoTracingAtStart();
    }
    
    Process();
    
    DO_TRACE {
        (*m_pTracing)(TB_UTTER) << PRT_REGION_END;
    }
}

