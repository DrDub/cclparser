#ifndef __MAIN_H__
#define __MAIN_H__

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
#include "time.h"
#include "Reference.h"
#include "OutFile.h"
#include "Globals.h"
#include "CmdArgs.h"
#include "Loop.h"
#include "LoopConf.h"
#include "MessageLine.h"
#include "RefSTL.h"
#include "Parser.h"
#include "EvaluatorTable.h"

class CMain : public CRef
{
protected:
    CpCCmdArgs m_pArgs;
    CpCMessageLine m_pMsgLine;
    CpCOutFile m_pOutputFile;
    CpCGlobals m_pGlobals;
    
    CpCLoopConf m_pLoopConf; // list of loops to be executed
    std::string m_ParserType;
    CpCParser m_pParser;     // parser used by current loop
    // Table accessing the correct evaluator
    CEvaluatorTable m_EvaluatorTable;
    CpCLoop m_pLoop; // current execution loop
    
    // error messages
    bool m_Error;
    std::string m_ErrorStr;
public:
    CMain(int ac, char** av);
    ~CMain() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
public:
    // execute the program (returns false on error)
    bool Run();
private:
    // Auxiliary functions

    // Reset the values of the global variables based on the arguments
    // in pArgs.
    void SetGlobals(CCmdArgOpts* pArgs);
    // Determine what type of parser to use, create it and store it on
    // m_pParser.
    void SelectParser();
    // Determine which loop has to be executed based on the given loop
    // configuration entry. Returns false on error.
    bool SelectLoop(CLoopEntry* pEntry);
    // Perform any actions (such as printing) which belong at the end of
    // the loop.
    void PostLoopActions(CLoopEntry* pEntry, time_t& StartTime,
                         time_t& EndTime);
public:
    // error messages
    bool IsError() { return m_Error; }
    std::string& GetErrorStr() { return m_ErrorStr; }
protected:
    void SetError(std::string const& ErrorMsg) {
        m_ErrorStr = ErrorMsg;
        m_Error = true;
    }
};

typedef CPtr<CMain> CpCMain;

#endif /* __MAIN_H__ */
