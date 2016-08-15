#ifndef __PLAINTEXTLOOP_H__
#define __PLAINTEXTLOOP_H__

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

#include "Loop.h"
#include "SynStruct.h"
#include "Punct.h"


class CPlainTextLoop : public CLoop
{
private:
    // Syntactic structure describing the current utterance
    CpCSynStruct m_pSynStruct;
    // Vector of all terminal node numbers
    std::vector<int> m_Terminals;
protected:
    // Description of utterance object in input file
    std::string m_ObjSource;
public:
    CPlainTextLoop(std::vector<std::string> const & InFilePatterns,
                   CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                   COutFile* pOutFile);
    
    ~CPlainTextLoop();

protected:
    // Process the next terminal.
    void ProcessTerminal(std::string const& Terminal);
    // Clear information about the object collected so far.
    void ClearObject();
    // To be called by derived class when the end of the object is reached.
    // This should only be called if any data was read for the object.
    void EndObject();
    
protected:
    // Function to return the punctuation type of a punctuation symbol.
    // If the given string is not a punctuation string,
    // eNoPunct is returned.
    ePunctType GetPunctType(std::string const& Unit);

    //
    // Interface to process object
    //
    
    // Return 'true' if there is a parser and the utterance has not
    // been rejected by the filter.
    virtual bool ShouldProcessSymbols() = 0;
    // Return true if the object curently being processed was matched.
    // 'false' is returned if this cannot yet be determined.
    virtual bool ObjectMatches() = 0;
    // Returns the number of symbols sent to the parser (not including
    // the final eEoUtterance).
    virtual unsigned int GetNumSymbolsToParser() = 0;
    
    // Process the full object. 'ObjSource' should be a copy of the
    // representation of the object in the input file.
    virtual void ProcessSynStruct(CSynStruct* pSynStruct,
                                  std::string const& ObjSource) = 0;
    // Process the next punctuation symbol
    virtual void ProcessPunct(ePunctType PunctType) = 0;
    // Process the next unit (word). Optionally, a list of labels
    // may be given.
    virtual void ProcessUnit(std::string const& Name,
                             std::vector<std::string>& Labels) = 0;
    // Same as above with a single label
    virtual void ProcessUnit(std::string const& Name,
                             std::string const& Label) = 0;
    // Same as above, but without having to specify the labels
    virtual void ProcessUnit(std::string const& Name) = 0;
};

typedef CPtr<CPlainTextLoop> CpCPlainTextLoop;

#endif /* __PLAINTEXTLOOP_H__ */
