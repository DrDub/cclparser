#ifndef __CMDARGS_H__
#define __CMDARGS_H__

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

// The CCmdArgOpts reads the options (of the form -x y) from a command line
// argument list. The class receives an argument option vector, parses it,
// and can then be used to retrieve the value of the different arguments
// options (those which are not given receive a default value).

class CCmdArgOpts : public CRef
{
private:
    std::vector<std::string> m_ConfFiles; // configuration files
    std::string m_OutputFile;
    std::string m_OutputFileSuffix;
    unsigned int m_ProgressReport;
    unsigned int m_FirstObjToProcess; // 0 or 1 - from the beginning
    unsigned int m_LastObjToProcess;  // 0 - no limit
    // Filter properties
    // For a word counting filter - maximal number of words in a sentence.
    // A value of 0 means no filtering.
    unsigned int m_FilterMaxWordCount;
    // Top tag filters
    // Only sentences with one of these tags should be matched by a filter 
    std::vector<std::string> m_OnlyTopTags;
    // Sentences with these top tags should not be matched
    std::vector<std::string> m_NotTopTags;
    // Should trivial syntactic structures be filtered out?
    bool m_bNonTrivialFilter;
    // The evaluators to be used (format: "name:type")
    std::vector<std::string> m_Evaluators;
    // Should the lexicon be printed at the end of this loop (if relevant)
    bool m_bPrintLexicon;
    
    bool m_Error;
    std::string m_ErrorStr;
public:
    // Constructor to be called when the object is constructed independently
    // (that is, not as the base class of some derived class).
    // If pGlobalOpts is given, the values in this object are used as
    // defaults.
    CCmdArgOpts(int ac, char** av, CCmdArgOpts* pGlobalOpts = NULL);
    // Same as above, but from a vector of strings
    CCmdArgOpts(std::vector<std::string>& ArgVec,
                CCmdArgOpts* pGlobalOpts = NULL);
protected:
    // Constructor to be called by derived classes (sets all values to default)
    CCmdArgOpts();
public:
    ~CCmdArgOpts() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
protected:
    bool ReadArgs(int& ac, char**& av);
    // set default value (and reset any error). If pGlobalOpts is given,
    // the values in this object are used as defaults.
    virtual void SetDefaults(CCmdArgOpts* pGlobalOpts = NULL);
    // read a single argument with string value into target
    bool ReadArg(int& ac, char**& av, std::string& target);
    // read a single argument with unsigned int value into target
    bool ReadArg(int& ac, char**& av, unsigned int& target);
    // read a single argument with multiple string values into target
    bool ReadArg(int& ac, char**& av, std::vector<std::string>& target);
    // try to read the next argument (the '-' was already read).
    // If no argument was matched, should call the same function of the base
    // class.
    // Returns true if the argument type was matched (whether successfully
    // read or not). If not read successfully, the error flag and message
    // are set. Returns false only if the option flag was not matched.
    virtual bool ArgSwitch(int& ac, char**& av);
public:
    // get argument values
    std::vector<std::string> const& GetConfFiles() { return m_ConfFiles; }
    std::string const& GetOutFile() { return m_OutputFile; }
    std::string const& GetOutFileSuffix() { return m_OutputFileSuffix; }
    unsigned int GetProgressReport() { return m_ProgressReport; }
    unsigned int GetFirstObjToProcess() { return m_FirstObjToProcess; }
    unsigned int GetLastObjToProcess() { return m_LastObjToProcess; }
    unsigned int GetFilterMaxWordCount() { return m_FilterMaxWordCount; }
    std::vector<std::string>& GetOnlyTopTags() { return m_OnlyTopTags; }
    std::vector<std::string>& GetNotTopTags() { return m_NotTopTags; }
    bool GetNonTrivialFilter() { return m_bNonTrivialFilter; }
    std::vector<std::string>& GetEvaluators() { return m_Evaluators; }
    bool PrintLexicon() { return m_bPrintLexicon; }
    // set argument values
    void SetLastObjToProcess(unsigned int Last) {
        m_LastObjToProcess = Last;
    }
    // check error status
    bool IsError() { return m_Error; }
    std::string& GetErrorStr() { return m_ErrorStr; }
protected:
    void SetError(std::string const& ErrorMsg) {
        m_ErrorStr = ErrorMsg;
        m_Error = true;
    }
};

typedef CPtr<CCmdArgOpts> CpCCmdArgOpts;

// The class CCmdArgs  

class CCmdArgs : public CCmdArgOpts
{
private:
    std::string m_ProgramName;
    std::vector<std::string> m_InFiles;
public:
    CCmdArgs(int ac, char** av);
    ~CCmdArgs() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
    // get argument values
    std::string const& GetProgramName() { return m_ProgramName; }
    std::vector<std::string> const& GetInFiles() { return m_InFiles; }
};

typedef CPtr<CCmdArgs> CpCCmdArgs;

#endif /* __CMDARGS_H__ */
