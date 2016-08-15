// Copyright 2007,2008 Yoav Seginer

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

#include <iomanip>
#include <string>
#include <ostream>
#include <sstream>
#include "Main.h"
#include "Parser.h"
#include "CCLParser.h"
#include "UnitPerLineLoop.h"
#include "UtterPerLineLoop.h"
#include "PennParse.h"
#include "Process.h"
#include "yError.h"
#include "StringUtil.h"

using namespace std;

CMain::CMain(int ac, char** av) : m_Error(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    m_pMsgLine = new CMessageLine();
    m_pMsgLine->NewMessageLine();
    m_pMsgLine->NewMessage(
        "CCL-Parser version 1.0.0.2 Copyright (C) 2007,2008 Yoav Seginer.");
    m_pMsgLine->NewMessageLine();
    m_pMsgLine->NewMessageLine();
    
    m_pArgs = new CCmdArgs(ac, av);
 
    if(!m_pArgs) {
        SetError("arguments not initialized");
        return;
    } else if(m_pArgs->IsError()) {
        SetError("Error while reading args: " + m_pArgs->GetErrorStr());
        return;
    }

    SetGlobals(m_pArgs);
    if(IsError())
        return;

    // Open the output file
    m_pOutputFile =
        new COutFile(m_pArgs->GetOutFile().c_str());

    m_pLoopConf = new CLoopConf(m_pArgs->GetInFiles(), m_pArgs.Ptr());

    if(m_pLoopConf->IsError())
        SetError("Error in reading execution sequence:" + EoLDelimiter() +
                 EoLDelimiter() + m_pLoopConf->GetErrorStr());
}

// The architecture allows for different parsers to be used, though in
// the current version only one parser is implemented.

void
CMain::SelectParser()
{
    // If the required parser is the same as the existing parser type,
    // there is nothing to do.
    if(m_ParserType == g_ParserType)
        return; // nothing to do

    m_ParserType = g_ParserType;

    // Currently only the CCL parser is supported, so this is the default.
    // The 'NoParser' parser setting can be used to re-initialize the
    // parser by forcing the next loop iteration to create a new parser.
    if(g_ParserType == "NoParser")
        m_pParser = NULL;
    else
        m_pParser = new CCCLParser(NULL);
}

bool
CMain::Run()
{
    if(IsError())
        return false;

    // map to store the size of each input pattern so that it does not
    // have to be calculated again.
    map<string, unsigned int> InFileSizes;
    
    // loop over the loop configuration list and execute every loop which
    // appears there.

    for(vector<CpCLoopEntry>::iterator Iter = m_pLoopConf->Begin() ;
        Iter != m_pLoopConf->End() ; Iter++) {
        
        if(!*Iter) {
            yPError(ERR_SHOULDNT, "missing loop entry pointer");
        }

        CpCCmdArgOpts pArgs = (*Iter)->GetCmdArgOpts();
        
        // Update the globals
        SetGlobals(pArgs);
        if(IsError())
            return false;

        if((*Iter)->GetAction() == CLoopEntry::eNoAction)
            continue;

        // Select the parser. This will keep the existing parser if
        // the parser type specified is not changed. If the parser
        // specification has changed, the previous parser is discarded
        // and all information stored in its lexicon is lost.
        SelectParser();

        // If a parser is required but none was created, return an error
        if(!m_pParser && (*Iter)->ActionRequiresParser()) {
            SetError("Failed to create parser of type '" +
                     g_ParserType + "'");
            return false;
        }
        
        if(m_pParser) {
            m_pParser->
                SetLearnCycle((*Iter)->GetAction() & CLoopEntry::eLearn);
            m_pParser->
                SetParseCycle((*Iter)->GetAction() & CLoopEntry::eParse);
        }
        
        // Set the current evaluators
        if(!m_EvaluatorTable.
           SetCurrentEvaluators(pArgs->GetEvaluators())) {
            SetError("Error when setting evaluators: " +
                     m_EvaluatorTable.GetErrorStr());
            return false;
        }

        // If the output file base name or suffix have changed, need to
        // create a new output file.
        if(pArgs->GetOutFile() != "") {
           if(!m_pOutputFile ||
              pArgs->GetOutFile() != m_pOutputFile->BaseName())
               m_pOutputFile = new COutFile(pArgs->GetOutFile());
           if(!m_pOutputFile->IsOpen() ||
              pArgs->GetOutFileSuffix() != m_pOutputFile->Suffix()) {
               m_pOutputFile->OpenWithSuffix(pArgs->GetOutFileSuffix());
               if(PrintingModeOn(PMODE_CONFIG))
                   m_pGlobals->PrintArgs(*m_pOutputFile, g_CommentStr+" ");
           }
        }
        
        if(!SelectLoop(*Iter))
            return false;

        // Should the size of the loop be determined before running
        // the loop? If the size of the input file is already known,
        // there is no need to loop over the file.

        map<string, unsigned int>::iterator Found =
            InFileSizes.find((*Iter)->GetInFilePattern());

        if(Found != InFileSizes.end()) {
            // found, use the previously calculated size
            if(!m_pLoop->GetArgs()->GetLastObjToProcess() ||
               (*Found).second < m_pLoop->GetArgs()->GetLastObjToProcess()) {
                m_pLoop->GetArgs()->SetLastObjToProcess((*Found).second);
            }
        } else if(!m_pLoop->GetArgs()->GetLastObjToProcess()) {
            // Not found and there is no explicit limit on the number of
            // sentences to process, so determine the size and store it.
            m_pLoop->SetCountOnly(true);
            m_pMsgLine->NewMessage("Determining input length ... ");

            if(!m_pLoop->DoLoop()) {
                SetError(m_pLoop->GetErrorStr());
                return false;
            }

            // This is the new maximal number of lines to be processed
            m_pLoop->GetArgs()->SetLastObjToProcess(m_pLoop->GetObjNum());
            InFileSizes[(*Iter)->GetInFilePattern()] = m_pLoop->GetObjNum();
        }
        
        // Perform the loop
        m_pLoop->SetCountOnly(false);
        m_pMsgLine->NewMessage((*Iter)->ProcessingMessage()+": ");
        m_pLoop->ResetLoop();

        // record loop start time
        time_t StartTime = time(NULL);
        
        if(!m_pLoop->DoLoop()) {
            SetError(m_pLoop->GetErrorStr());
            return false;
        }

        // record loop end time
        time_t EndTime = time(NULL);
        
        // Post-loop actions (mostly, printing)
        PostLoopActions(*Iter, StartTime, EndTime);

        ostringstream Ostr(ios::out);
        Ostr << "done: " << m_pLoop->GetObjsProcessedNum() << " objects";
        m_pMsgLine->AppendMessage(Ostr.str());
        m_pMsgLine->NewMessageLine();
    }

    return true;
}

void
CMain::SetGlobals(CCmdArgOpts* pArgs)
{
    if(!pArgs) {
        yPError(ERR_MISSING, "Argument object missing");
    }
    
    if(!m_pGlobals)
        m_pGlobals = new CGlobals(pArgs->GetConfFiles());
    else
        m_pGlobals->UpdateGlobals(pArgs->GetConfFiles());
    
    if(m_pGlobals->IsError()) {
        SetError("Error while reading configuration: " +
                 m_pGlobals->GetErrorStr());
        return;
    }
    
    // some reinitilization may be required as a result of this update
    ResetPrintingMode();
}

bool
CMain::SelectLoop(CLoopEntry* pEntry)
{
    if(!pEntry) {
        yPError(ERR_MISSING, "Loop entry missing");
    }

    if(pEntry->GetAction() !=
       (pEntry->GetAction() &
        (CLoopEntry::eFilter|CLoopEntry::eLearn|CLoopEntry::eParse))) {
        SetError("Action not supported for this input type: " +
                 pEntry->GetEntryString());
        return false;
    }
    
    vector<string> InFilePatterns;
    InFilePatterns.push_back(pEntry->GetInFilePattern());

    CpCParser pParser;
    
    // If the action is parsing or learning, use the current parser, otherwise
    // (in case of filtering) no parser should be used.
    if(pEntry->GetAction() & (CLoopEntry::eLearn|CLoopEntry::eParse))
        pParser = m_pParser;
    
    // Use the action and input type to determine the loop object
    // for this entry.

    switch(pEntry->GetInputType()) {
        case CLoopEntry::eSinglePlain:
            m_pLoop =
                new CProcess<CUnitPerLineLoop>(pParser, InFilePatterns,
                                               pEntry->GetCmdArgOpts(),
                                               m_pMsgLine, m_pOutputFile,
                                               m_EvaluatorTable.Evaluators());
            return true;
        case CLoopEntry::eLinePlain:
            m_pLoop =
                new CProcess<CUtterPerLineLoop>(pParser, InFilePatterns,
                                                pEntry->GetCmdArgOpts(),
                                                m_pMsgLine, m_pOutputFile,
                                                m_EvaluatorTable.Evaluators());
            return true;
        case CLoopEntry::eWSJPennTB:
            m_pLoop =
                new CProcess<CWSJPennParse>(pParser, InFilePatterns,
                                            pEntry->GetCmdArgOpts(),
                                            m_pMsgLine, m_pOutputFile,
                                            m_EvaluatorTable.Evaluators());
            return true;
        case CLoopEntry::eNegraPennTB:
            m_pLoop =
                new CProcess<CNegraPennParse>(pParser, InFilePatterns,
                                              pEntry->GetCmdArgOpts(),
                                              m_pMsgLine, m_pOutputFile,
                                              m_EvaluatorTable.Evaluators());
            return true;
        case CLoopEntry::eCTBPennTB:
            m_pLoop =
                new CProcess<CCTBPennParse>(pParser, InFilePatterns,
                                            pEntry->GetCmdArgOpts(),
                                            m_pMsgLine, m_pOutputFile,
                                            m_EvaluatorTable.Evaluators());
            return true;
        default:
            SetError("Unsupported input type: " + pEntry->GetEntryString());
        return false;
    }

    return true;
}

void
CMain::PostLoopActions(CLoopEntry* pEntry, time_t& StartTime, time_t& EndTime)
{
    if(!pEntry)
        return;

    // If required by printing mode, output the loop timing
    if(PrintingModeOn(PMODE_TIMING)) {
        (m_pLoop->GetOutputStream())
            << endl << g_CommentStr << " Loop started: "
            << ctime(&StartTime);
        (m_pLoop->GetOutputStream())
            << g_CommentStr << " Loop ended:   " << ctime(&EndTime)
            << g_CommentStr << " Total time:   " << (EndTime - StartTime)
            << " sec" << " (" << ((EndTime - StartTime) / 60) << ":"
            << setw(2) << setfill('0') << ((EndTime - StartTime) % 60)
            << ")" << endl << endl;
    }

    if(pEntry->GetAction() & CLoopEntry::eParse) {
        // Print evaluation results (if any)
        for(vector<CpCEvaluator>::iterator Iter = m_EvaluatorTable.Begin();
            Iter != m_EvaluatorTable.End() ; Iter++) {
            ((ostream&)*m_pOutputFile) << endl; 
            if(*Iter && m_pOutputFile && m_pOutputFile->IsOpen())
                (*Iter)->PrintTotalEval(*m_pOutputFile);
        }
    }
    
    if(PrintingModeOn(PMODE_OBJ_COUNT) &&
       m_pOutputFile && m_pOutputFile->IsOpen()) {
        // Print the total number of sentences parsed
        ((ostream&)(*m_pOutputFile)) << g_CommentStr
                                     << " Total number of sentences: "
                                     << m_pLoop->GetObjsProcessedNum()
                                     << endl;
    }

    if(m_pParser && m_pParser->GetLexicon() &&
       pEntry->GetCmdArgOpts()->PrintLexicon() && m_pOutputFile) {

        // print the lexicon
        
        string Suffix =
            pEntry->GetCmdArgOpts()->GetOutFileSuffix().empty() ?
            string("lexicon") :
            pEntry->GetCmdArgOpts()->GetOutFileSuffix() + ".lexicon";
        
        m_pOutputFile->OpenWithSuffix(Suffix);
        
        if(PrintingModeOn(PMODE_CONFIG))
            m_pGlobals->PrintArgs(*m_pOutputFile, g_CommentStr+" ");
        
        if(m_pOutputFile->IsOpen()) {
            m_pMsgLine->AppendMessage("Printing lexicon ... ");
            m_pParser->GetLexicon()->PrintLexicon(*m_pOutputFile);
        }
    }
}

int
main(int ac, char** av)
{
#ifdef DEBUG    
    cerr << "Objects at start: " << CRef::TotalObjects() << endl;
#endif

    CpCMain pMain = new CMain(ac, av);

    if(pMain->IsError()) {
        cerr << endl << "Error in intialization:" << endl;
        cerr << pMain->GetErrorStr() << endl;
        return -1;
    }

    if(!pMain->Run()) {
        cerr << endl << pMain->GetErrorStr() << endl;
        return -1;
    }
    
#ifdef DEBUG    
    cerr << "Objects at end: " << CRef::TotalObjects() << endl;
#endif
#ifdef DETAILED_DEBUG
    CRef::PrintObjectTable((ostream&)cerr);
#endif    
    return 0;
}
