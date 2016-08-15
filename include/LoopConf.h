#ifndef __LOOPCONF_H__
#define __LOOPCONF_H__

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

#include <vector>
#include <string>
#include "Reference.h"
#include "CmdArgs.h"

// The CLoopConf class is initialized from a configuration file. This
// file has the following columns:
// 1. File specification (regular expression). This specifies the files
//    which should be processed in the loop described by this entry.
// 2. Input type (format of the input files for this entry).
//    Currently supported: single, penn (single word per line and Penn TB)
// 3. Action.
//    Currently supported: filter, learn, parse (extract the sentences
//    which match the filter, learn from input, parse).
//
// After these three columns, addition arguments may appear in command
// line format. These argument override the values specified
// on the command line for the loop entry for which they are given.
//
// The CLoopConf class then supplies an iterator over the entries read. Each
// such entry is a CLoopEntry object, which has one field for every
// column in the configuration file.

class CLoopEntry : public CRef
{
public:
    // Input file types
    enum eInputTypes {
        eNoType, // no type specified (this is an error)
        eSinglePlain, // single token (word/punctuation) per line
        eLinePlain, // single utterance per line. No annotation, but
                    // tokens are separated by white space (also between
                    // words and punctuation)
        eWSJPennTB, // Wall Street Journal Penn TB format
        eNegraPennTB, // Negra Penn TB format
        eCTBPennTB, // Chinese Treebank format
    };
    // Action types
    enum eActions {
        eNoAction = 0x0,    // No action to be taken (this is an error)
        eFilter = 0x1,      // extract the sentences which match the filter
        eLearn = 0x2,       // learn the lexicon from the input file
        eParse = 0x4,       // parse the utterances in the input file
        eLearnAndParse = (eLearn|eParse),
    };
    
private:
     // the configuration file line from which this entry was created
    // (this is mainly useful for error messages)
    std::string m_EntryString;
    std::string m_InFilePattern;
    eInputTypes m_InputType;
    eActions m_Action;
    // Command arguments read for this entry
    CpCCmdArgOpts m_pCmdArgOpts;

    bool m_bError;
    std::string m_ErrorStr;
public:
    CLoopEntry(std::string const& EntryStr, CCmdArgOpts* pGlobalOpts = NULL);
    ~CLoopEntry();

    std::string const& GetEntryString() { return m_EntryString; }
    std::string const& GetInFilePattern() { return m_InFilePattern; }
    eInputTypes GetInputType() { return m_InputType; }
    eActions GetAction() { return m_Action; }
    bool ActionRequiresParser();
    CCmdArgOpts* GetCmdArgOpts() { return m_pCmdArgOpts; }

    // Different functions based on the values store in the entry

    // message to be printed we processing such an entry
    std::string const& ProcessingMessage();
    
    // Error handling
    // check error status
    bool IsError() { return m_bError; }
    std::string& GetErrorStr() { return m_ErrorStr; }
private:
    void SetError(std::string const& ErrorMsg) {
        m_ErrorStr = "In '" + m_EntryString + "': " + ErrorMsg;
        m_bError = true;
    }
};

typedef CPtr<CLoopEntry> CpCLoopEntry;

class CLoopConf : public CRef
{
private:
    // files from which the configuration should be read
    std::vector<std::string> m_ConfFiles;
    std::vector<CpCLoopEntry> m_LoopEntries;

    bool m_bError;
    std::string m_ErrorStr;
public:
    // initialization by multiple configuration files and with the global
    // arguments. If given, these are defaults for values not specified in
    // the loop configuration entries.
    CLoopConf(std::vector<std::string> const& ConfFilePatterns,
              CCmdArgOpts* pGlobalOpts = NULL);
    // initialization by a single configuration file
    CLoopConf(std::string const& ConfFilePattern,
              CCmdArgOpts* pGlobalOpts = NULL);
    ~CLoopConf();
private:
    // Actually performs the initialization (called by the constructors)
    void Init(std::vector<std::string> const& ConfFilePatterns,
              CCmdArgOpts* pGlobalOpts);
public:
    // Iterator over loop entries
    std::vector<CpCLoopEntry>::iterator Begin() {
        return m_LoopEntries.begin();
    }
    std::vector<CpCLoopEntry>::iterator End() {
        return m_LoopEntries.end();
    }

    // Error handling
    // check error status
    bool IsError() { return m_bError; }
    std::string& GetErrorStr() { return m_ErrorStr; }
private:
    void SetError(std::string const& ErrorMsg) {
        m_ErrorStr += ErrorMsg;
        m_bError = true;
    }
};

typedef CPtr<CLoopConf> CpCLoopConf;

#endif /* __LOOPCONF_H__ */
