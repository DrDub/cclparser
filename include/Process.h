#ifndef __PROCESS_H__
#define __PROCESS_H__

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
#include <queue>
#include "Reference.h"
#include "SynFilter.h"
#include "SynStruct.h"
#include "Evaluator.h"
#include "Parser.h"
#include "Loop.h"
#include "Globals.h"

// This class implements the abstract interface between the loop object
// (which reads the input data) and the specific processing class
// (parser/filter) which is applied to the data.
// This interface defines two channels by which the loop should send the
// data it has read from the files to the processing class:
// 1. A CSynStruct object describing a full object (utterance) in the
//    input file.
// 2. An incremental symbol by symbol interface which sends each symbol
//    as it becomes available. For each symbol it must indicate whether
//    it is a unit (word) or a punctuation symbol.
//
// In addition, a verbatim copy of the object (as it appears in the
// input files) should be transferred (for printing purposes).
//
// In principle, to make processing truely incremental, the CSynStruct object
// should be sent only when all symbols have been already transferred.
// However, the filter is usually applied to the CSynStruct object and may
// result in the decision to ignore the object (utterance). Therefore,
// the underlying loop object may transfer the CSynStruct at any time
// during the sending of the symbols and the CProcess object defined here
// must buffer the symbols, if necessary.
//
// When a eEoUtterance is received (or when the process has just been
// initialized) all subsequent symbols are stored on the queue until a
// CSynStruct object is received and the filter can be applied. If there
// is no filter, the symbols are transferred directly to the parser.
// When the process receives a CSynStruct object, it applies the
// filter (if it exists) to it. If the filter matches, all symbols on the
// symbol queue and any subsequent symbols received (until the first
// eEoUtterance) are transferred to the parser (if defined).
// If the filter does not match, all symbols on the queue are discarded
// and no further symbols are stored on the queue or transferred to the
// parser until a eEoUtterance is received.
// If no CSynStruct object is received before the next eEoUtterance
// and a filter is defined, the filter is applied to the NULL CSynStruct
// object (which will probably result in no match, but this is to
// be defined by the filter).
// 
// If evaluators (and a parser) are defined, the evaluators are applied
// when the eEoUtterance is received (and is used with the last
// CSynStruct object available). 
//
// If no parser is defined (but a filter is defined) the process
// works in filter mode and the filter is applied to each
// structure received. Symbols transferred to the process are discarded.

//
// Auxiliary classes
//

//
// These auxiliary classes can be used to store a punctuation symbol
// or a unit name together with an optional sequence of labels.
// The base class stores the labels while the derived classes
// store either the unit name or the punctuation code.
//
// All members of these classes are public since they are only used
// by the 'CProcessBase' class, which shoudl have direct access to their
// content.

class CProcessSymbol : public CRef {
public:
    std::vector<std::string> m_Labels; // sequence of labels

    CProcessSymbol();
    CProcessSymbol(std::vector<std::string>& Labels);

    virtual ~CProcessSymbol();

    virtual eSymbolType GetType() = 0;
};

typedef CPtr<CProcessSymbol> CpCProcessSymbol;

class CProcessUnit : public CProcessSymbol {
public:
    std::string m_UnitName;

    CProcessUnit(std::string const& UnitName);
    CProcessUnit(std::string const& UnitName,
                 std::vector<std::string>& Labels);

    ~CProcessUnit();

    eSymbolType GetType() { return eUnit; }
};

typedef CPtr<CProcessUnit> CpCProcessUnit;

class CProcessPunct : public CProcessSymbol {
public:
    ePunctType m_PunctType;

    CProcessPunct(ePunctType PunctType);
    CProcessPunct(ePunctType PunctType, std::vector<std::string>& Labels);

    ~CProcessPunct();

    eSymbolType GetType() { return ePunct; }
};

typedef CPtr<CProcessPunct> CpCProcessPunct;

//
// The following base class implements the functionality of the processing
// class. It should only be used through the CProcess template.
//

class CProcessBase {
protected:

    // Process defining objects (access to these is allowed from the
    // derived class)
    
    // A process must define either a filter or a parser. It may also
    // specify both.
    CpCSynFilter m_pFilter;
    CpCParser m_pParser;
    // When a parser is defined, evaluators may be applied to the parse
    // result.
    std::vector<CpCEvaluator> m_Evaluators;

private:
    // Current process state

    // Current syntactic structure
    CpCSynStruct m_pSynStruct;
    // Representation of object in input file (for printing)
    std::string m_ObjSource;
    // Number of symbols in this utterance sent to the parser
    // (not including the terminating eEoUtterance symbol)
    unsigned int m_NumSymbolsToParser;
    // Queue of symbols not yet passed to the parser. A set of labels may
    // be attached to each symbol.
    std::queue<CpCProcessSymbol> m_SymbolQueue;
    // If there is a m_pSynStruct object, did it match the filter (if defined)
    bool m_bMatches;
    
public:
    // Constructor. The filter is defined by 'pArgs'. 'pParser' is allowed
    // to be NULL if a filter is defined. 
    CProcessBase(CParser* pParser, CCmdArgOpts* pArgs,
                 std::vector<CpCEvaluator>& Evaluators);

    virtual ~CProcessBase() {}
private:

    // Returns true if any symbol received should be stored in the queue.
    bool ShouldStoreSymbolsOnQueue();

    // Clear the symbol queue
    void ClearSymbolQueue();
    // Send all symbols on the queue to the parser
    void TransferQueueToParser();
    // Apply the evaluators to the current parse
    void ApplyEvaluators();
    // Output the current parser to the output file
    void OutputParse();
    // Clear the parser and the utterance
    void ClearProcess();
    
protected:
    
    //
    // Processing functions
    //

    // Return 'true' if there is a parser and the utterance has not
    // been rejected by the filter.
    bool ShouldProcessSymbols();
    // Return true if the object curently being processed was matched.
    // 'false' is returned if this cannot yet be determined.
    bool ObjectMatches();
    // Returns the number of symbols sent to the parser (not including
    // the final eEoUtterance).
    unsigned int GetNumSymbolsToParser() { return m_NumSymbolsToParser; }
    
    // Process the full object. 'ObjSource' should be a copy of the
    // representation of the object in the input file.
    void ProcessSynStruct(CSynStruct* pSynStruct,
                          std::string const& ObjSource);
    // Process the next punctuation symbol
    void ProcessPunct(ePunctType PunctType);
    // Process the next unit (word). Optionally, a list of labels
    // may be given.
    void ProcessUnit(std::string const& Name,
                     std::vector<std::string>& Labels);
    // Same as above with a single label
    void ProcessUnit(std::string const& Name, std::string const& Label);
    // Same as above, but without having to specify the labels
    void ProcessUnit(std::string const& Name);

    // Interfaces to services of derived class
    virtual std::ostream& GetOutputStream() = 0;
    virtual CRefOStream* GetOutputStreamObj() = 0;
};

// The following template allows different loop classes to be associated
// with the processing class CProcessBase, which actually implements
// the functionality.

template <class LoopClass>
class CProcess : public CProcessBase, public LoopClass
{
public:

    // Constructor (the filter is deterimed from the pArg structure)
    CProcess(CParser* pParser,
             std::vector<std::string> const & InFilePatterns,
             CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
             COutFile* pOutFile,
             std::vector<CpCEvaluator>& Evaluators) :
            CProcessBase(pParser, pArgs, Evaluators),
            LoopClass(InFilePatterns, pArgs, MsgLine, pOutFile)
        {
#ifdef DETAILED_DEBUG
            IncObjCount();
#endif
            // Set tracing on the parser and evaluation objects
            if(pParser)
                pParser->SetTracing(LoopClass::GetOutFile(), g_TraceBits);
            for(std::vector<CpCEvaluator>::iterator
                    Iter = m_Evaluators.begin() ;
                Iter != m_Evaluators.end() ; Iter++) {
                if(*Iter)
                    (*Iter)->SetTracing(LoopClass::GetOutFile(), g_TraceBits);
            }
        }
    
    ~CProcess() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
    
private:
    // Return 'true' if there is a parser and the utterance has not
    // been rejected by the filter.
    bool ShouldProcessSymbols() {
        return CProcessBase::ShouldProcessSymbols();
    }    
    // Return true if the object curently being processed was matched.
    // 'false' is returned if this cannot yet be determined.
    bool ObjectMatches() {
        return CProcessBase::ObjectMatches();
    }
    // Returns the number of symbols sent to the parser (not including
    // the final eEoUtterance).
    unsigned int GetNumSymbolsToParser() {
        return CProcessBase::GetNumSymbolsToParser();
    }
    
    // Process the full object. 'ObjSource' should be a copy of the
    // representation of the object in the input file.
    void ProcessSynStruct(CSynStruct* pSynStruct,
                          std::string const& ObjSource) {
        CProcessBase::ProcessSynStruct(pSynStruct, ObjSource);
    }
    // Process the next punctuation symbol
    void ProcessPunct(ePunctType PunctType) {
        CProcessBase::ProcessPunct(PunctType);
    }
    // Process the next unit (word). Optionally, a list of labels
    // may be given.
    void ProcessUnit(std::string const& Name,
                     std::vector<std::string>& Labels) {
        CProcessBase::ProcessUnit(Name, Labels);
    }
    // Same as above with a single label
    void ProcessUnit(std::string const& Name, std::string const& Label) {
        CProcessBase::ProcessUnit(Name, Label);
    }
    // Same as above, but without having to specify the labels
    void ProcessUnit(std::string const& Name) {
        CProcessBase::ProcessUnit(Name);
    }

    // Interfaces to loop class services
    std::ostream& GetOutputStream() {
        return LoopClass::GetOutputStream();
    }
    CRefOStream* GetOutputStreamObj() {
        return LoopClass::GetOutputStreamObj();
    }
};





#endif /* __PROCESS_H__ */
