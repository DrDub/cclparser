#ifndef __PENNPARSE_H__
#define __PENNPARSE_H__

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
#include <map>
#include "Reference.h"
#include "ObjsIO.h"
#include "SynStruct.h"
#include "PennSynStruct.h"
#include "Punct.h"

//
// IO loop object to read Penn treebank objects and parse them
//

// Parsing

class CPennParse : public CPennTBObjsIO
{
public:
    CPennParse(std::vector<std::string> const & InFilePatterns,
               CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
               COutFile* pOutFile);
    
    virtual ~CPennParse() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

public:    
    bool ProcessObject(CObj* pObj, std::string const& ObjInput);
private:
    // Return the syntactic structure based on the input object
    // This conversion may depend on the exact corpus used.
    virtual
    CPennSynStruct* GetSynStruct(CObj* pObj, bool bReverse = false) = 0;
    // return the punctuation code for the given punctuation terminal.
    // This conversion is defined in the derived class because it
    // differs between corpora.
    virtual
    ePunctType GetPunct(std::vector<CpCTerminal>::iterator const& Punct) = 0;

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

typedef CPtr<CPennParse> CpCPennParse;

//
// Specific derived classes to handle variations between different corpora
//

// Wall Street Journal Corpus

class CWSJPennParse : public CPennParse
{
public:
    CWSJPennParse(std::vector<std::string> const & InFilePatterns,
                  CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                  COutFile* pOutFile);
    ~CWSJPennParse();
private:
    // Function to determine whether a line is a comment line
    bool IsCommentLine(std::string const& Line);
    // Return the syntactic structure based on the input object
    // This conversion may depend on the exact corpus used.
    CPennSynStruct* GetSynStruct(CObj* pObj, bool bReverse);
    // return the punctuation code for the given punctuation terminal
    ePunctType GetPunct(std::vector<CpCTerminal>::iterator const& Punct);
};

// Negra Corpus

class CNegraPennParse : public CPennParse
{
public:
    CNegraPennParse(std::vector<std::string> const & InFilePatterns,
                    CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                    COutFile* pOutFile);
    ~CNegraPennParse();
private:
    // Function to determine whether a line is a comment line
    bool IsCommentLine(std::string const& Line);
    // Return the syntactic structure based on the input object
    // This conversion may depend on the exact corpus used.
    CPennSynStruct* GetSynStruct(CObj* pObj, bool bReverse);
    // return the punctuation code for the given punctuation terminal
    ePunctType GetPunct(std::vector<CpCTerminal>::iterator const& Punct);
};

// Chinese Treebank Corpus

class CCTBPennParse : public CPennParse
{
public:
    CCTBPennParse(std::vector<std::string> const & InFilePatterns,
                  CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                  COutFile* pOutFile);
    ~CCTBPennParse();
private:
    // Function to determine whether a line is a comment line
    bool IsCommentLine(std::string const& Line);
    // Return the syntactic structure based on the input object
    // This conversion may depend on the exact corpus used.
    CPennSynStruct* GetSynStruct(CObj* pObj, bool bReverse);
    // return the punctuation code for the given punctuation terminal
    ePunctType GetPunct(std::vector<CpCTerminal>::iterator const& Punct);
};

#endif /* __PENNPARSE_H__ */
