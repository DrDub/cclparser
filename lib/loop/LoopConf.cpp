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

#include <istream>
#include <sstream>
#include "LoopConf.h"
#include "StringUtil.h"
#include "yError.h"
#include "InFile.h"

using namespace std;

/////////////////////////
// Loop Entry Routines //
/////////////////////////

static struct SInputTypeNames {
    string m_Name;
    CLoopEntry::eInputTypes m_Type;
} InputTypeNames[] = {
    "word", CLoopEntry::eSinglePlain,
    "line", CLoopEntry::eLinePlain, 
    "wsj", CLoopEntry::eWSJPennTB,
    "negra", CLoopEntry::eNegraPennTB,
    "ctb", CLoopEntry::eCTBPennTB,
    "", CLoopEntry::eNoType
};

static struct SActionNames {
    string m_Name;
    CLoopEntry::eActions m_Action;
} ActionNames[] = {
    "filter", CLoopEntry::eFilter,
    "learn", CLoopEntry::eLearn,
    "parse", CLoopEntry::eParse,
    "learn+parse", CLoopEntry::eLearnAndParse,
    "", CLoopEntry::eNoAction
};

CLoopEntry::CLoopEntry(string const& EntryStr, CCmdArgOpts* pGlobalOpts) :
        m_InputType(eNoType), m_Action(eNoAction), m_EntryString(EntryStr),
        m_bError(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    
    if(!EntryStr.size()) {
        yPError(ERR_MISSING, "Cannot create entry from an empty line");
    }

    if(EntryStr.c_str()[0] == '#') {
        yPError(ERR_SHOULDNT, "Cannot create entry from a commented line");
    }
    
    // Create a string stream and read the string
    istringstream istr(EntryStr.c_str());

    istr >> m_InFilePattern;
    if(istr.fail()) {
        SetError("No input file pattern: " + EntryStr);
        return;
    }
    
    string InputType;
    istr >> InputType;
    if(istr.fail()) {
        SetError("No input typed specified: " + EntryStr);
        return;
    }

    for(struct SInputTypeNames* Types = InputTypeNames ;
        Types && Types[0].m_Name.size() ; Types++) {
        if(Types->m_Name == InputType) {
            m_InputType = Types->m_Type;
            break;
        }
    }

    if(m_InputType == eNoType) {
        SetError("Unknown input type: " + InputType);
        return;
    }

    string Action;
    istr >> Action;
    if(istr.fail()) {
        SetError("No action specified: " + EntryStr);
        return;
    }

    for(struct SActionNames* Actions = ActionNames ;
        Actions && Actions[0].m_Name.size() ; Actions++) {
        if(Actions->m_Name == Action) {
            m_Action = Actions->m_Action;
            break;
        }
    }

    if(m_Action == eNoAction) {
        SetError("Unknown action type: " + Action);
        return;
    }

    // Read addition arguments. Need to break them down and store them in
    // a char** array before reading them to the argument object.
    vector<string> Args;
    
    while(1) {
        string Arg;
        istr >> Arg;
        if(istr.fail())
            break;
        Args.push_back(Arg);
    }

    m_pCmdArgOpts = new CCmdArgOpts(Args, pGlobalOpts);

    if(m_pCmdArgOpts->IsError()) {
        SetError(m_pCmdArgOpts->GetErrorStr());
    }
}

CLoopEntry::~CLoopEntry()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

bool
CLoopEntry::ActionRequiresParser()
{
    return (m_Action == eLearn || m_Action == eParse);
}

string const&
CLoopEntry::ProcessingMessage()
{
    static string FilterMsg("Filtering");
    static string LearnMsg("Learning");
    static string ParseMsg("Parsing");
    static string ProcessMsg("Processing");
    
    switch(m_Action) {
        case eFilter:
            return FilterMsg;
        case eLearn:
            return LearnMsg;
        case eParse:
            return ParseMsg;
        default:
            return ProcessMsg;
    }
}

//////////////////////////////
// Loop Configuration Class //
//////////////////////////////

CLoopConf::CLoopConf(vector<string> const& ConfFilePatterns,
                     CCmdArgOpts* pGlobalOpts) : m_bError(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    Init(ConfFilePatterns, pGlobalOpts);
}

CLoopConf::CLoopConf(std::string const& ConfFilePattern,
                     CCmdArgOpts* pGlobalOpts) : m_bError(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    vector<string> Patterns;

    Patterns.push_back(ConfFilePattern);
    Init(Patterns, pGlobalOpts);
}

CLoopConf::~CLoopConf()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CLoopConf::Init(vector<string> const& ConfFilePatterns,
                CCmdArgOpts* pGlobalOpts)
{
    CMultiInFile ConfFiles(ConfFilePatterns);

    while(ConfFiles.Good()) {
        
        if(!ConfFiles.GetLine())
            break;

        string Line = ConfFiles.LastLine();

        if(Line.empty() || Line.c_str()[0] == '#' ||
           Line.find_first_not_of(WhiteSpace()) == string::npos)
            continue; // empty or commented line

        CpCLoopEntry pEntry = new CLoopEntry(ConfFiles.LastLine(),
                                             pGlobalOpts);

        if(pEntry->IsError()) {
            SetError(pEntry->GetErrorStr() + EoLDelimiter());
            continue;
        }
        
        m_LoopEntries.push_back(pEntry);
    }
}
