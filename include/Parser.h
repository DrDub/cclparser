#ifndef __PARSER_H__
#define __PARSER_H__

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

#include <string>
#include <vector>
#include "Reference.h"
#include "RefSTL.h"
#include "Symbol.h"
#include "Punct.h"
#include "SynStruct.h"
#include "RefStream.h"
#include "Lexicon.h"
#include "Tracing.h"
#include "Unit.h"
#include "PrintUtils.h"

// The following class defines the interface to various parsing modules.
// It also implements some of the basic functionality, such as the input
// queue, basic tracing and matching. 
// The class receives either units (words) or punctuation symbols
// (one by one). After a full stop has been received by the class, the
// utterance should be fully parsed and the structure of the resulting
// utterance may be retrieved. Not all parsers must be able to return
// all types of parse structures (SynStruct/Undirected dependencies, etc.).

class CParser : public CRef, public CPrintObj
{
protected:
    // Is this a learning cycle?
    bool m_bLearnCycle;
    // Is this a parsing cycle?
    bool m_bParseCycle;
    
    //
    // Input queue
    //
    
    // Input queue of new symbols waiting to be processed.
    CPtr<CRdeque<CpCSymbol> > m_pInput;
    // Current number of units in the queue
    unsigned int m_UnitNum;

    //
    // Tracing
    //
    
    // Tracing object
    CpCTracing m_pTracing;

public:
    CParser();
    virtual ~CParser();

    bool IsLearnCycle() { return m_bLearnCycle; }
    bool IsParseCycle() { return m_bParseCycle; }
    
    // (Re-)set the tracing object with the given parameters
    void SetTracing(COutFile* pOutputFile, unsigned int TraceTypes);

    // Push the next unit onto the input queue, with the given name and
    // labels. This function also assigns each unit its (linear) position
    // in the utterance.
    void PushInputUnit(std::string const& Name,
                       std::vector<std::string>& Labels);
    // Same as above with a single label
    void PushInputUnit(std::string const& Name, std::string const& Label);
    // Same as above, but without having to specify the labels
    void PushInputUnit(std::string const& Name);
    // Push the next punctuation symbol onto the input queue
    void PushInputPunct(ePunctType Punct);
    // Clear all data of the current utterance being processed.
    void ClearUtterance();
    // returns true if the input queue is empty
    bool InputQueueEmpty() { return !m_pInput || m_pInput->empty(); }

    // returns the number of units (minus 1) already read into the queue
    unsigned int GetUnitNum() { return m_UnitNum; }

    //
    // Utterance processing
    //

    // Perform the tracing which takes place just before the start of
    // processing.
    void DoTracingAtStart();
    // Called when the utterance was fully read. Processing should be
    // completed and the utterance is then cleared.
    void UtteranceFullyRead();
    
    //
    // Parser interface functions to be implemented by the derived classes
    //

private:
    // Create a unit based on the name and labels given
    virtual CUnit* CreateUnit(std::string const& Name,
                              std::vector<std::string>& Labels) = 0;
    // Clear all data of the current utterance which is stored on the derived
    // class.
    virtual void ClearDerivedParser() = 0;

    // Process the input currently available. In principle, this can be
    // called at any point in time and should be called at least once after
    // the utterance had been fully read.
    virtual void Process() = 0;
public:
    // Determine whether this is a learning cycle
    void SetLearnCycle(bool bLearn) { m_bLearnCycle = bLearn; }
    // Determine whether this is a parsing cycle    
    void SetParseCycle(bool bParse) { m_bParseCycle = bParse; }
    
    // Return the syntactic structure of the current utterance being processed.
    // May return NULL if processing has not been completed yet or if this
    // representation is not supported.
    virtual CSynStruct* GetSynStruct() = 0;

    // Get the current lexicon object (if exists) as currently stored in
    // the parser object.
    virtual CLexicon* GetLexicon() = 0;
};

typedef CPtr<CParser> CpCParser;

#endif /* __PARSER_H__ */
