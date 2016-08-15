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

#include <string.h>
#include <istream>
#include <sstream>
#include "CmdArgs.h"

using namespace std;

CCmdArgOpts::CCmdArgOpts(int ac, char** av, CCmdArgOpts* pGlobalOpts)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    SetDefaults(pGlobalOpts);
    ReadArgs(ac, av);

    if(ac) {
        // extra arguments
        SetError("unknown arguments");
        return;
    }
}

CCmdArgOpts::CCmdArgOpts(vector<string>& ArgVec, CCmdArgOpts* pGlobalOpts)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    int ac = ArgVec.size();
    char** av = NULL;

    if(ac) {
        av = new char*[ac+1];
        int i = 0;
        for(vector<string>::iterator Iter = ArgVec.begin() ;
            Iter != ArgVec.end() ; Iter++, i++) {
            av[i] = new char[(*Iter).size()+1];
            strcpy(av[i], (*Iter).c_str());
        }
        av[ac] = NULL;
    }

    SetDefaults(pGlobalOpts);
    // since the pointer transferred to ReadArgs() is moved, we pass it a
    // copy of av.
    char** av2 = av;
    ReadArgs(ac, av2);

    if(av) {
        for(char** p = av ; p && *p ; p++)
            delete[] *p;
        delete[] av;
    }
    
    if(ac) {
        // extra arguments
        SetError("unknown arguments");
        return;
    }
}

CCmdArgOpts::CCmdArgOpts()
{
    SetDefaults();
}

// read the next option into target. This option is assumed to have an
// unsigned int value. Returns false if an error occurs (but also sets
// m_Error)

bool
CCmdArgOpts::ReadArg(int& ac, char**& av, unsigned int& target)
{
    ac--; av++;
    
    if(!ac || av[0][0] == '-') {
        m_Error = true;
        m_ErrorStr = string("missing value for option '") +
            string((*(av-1))+1) + string("'");
        return false;
    }
    
    istringstream istr(av[0]);
    istr >> target;
    if(istr.fail()) {
        m_Error = true;
        m_ErrorStr = string("option '") + string((*(av-1))+1)
            + string("': number value expected");
        return false;
    }
    ac--; av++;
    return true;
}

// read the next option into target. This option is assumed to have an
// string value. Returns false if an error occurred (but also sets m_Error)

bool
CCmdArgOpts::ReadArg(int& ac, char**& av, string& target)
{
    ac--; av++;
    
    if(!ac || av[0][0] == '-') {
        m_Error = true;
        m_ErrorStr = string("missing value for option '") +
            string((*(av-1))+1) + string("'");
    } else {
        target = av[0];
        ac--; av++;
    }
    
    return true;
}

// read the next option into target. This option is assumed to have a
// multiple string value. Returns false if an error occurred (but also sets
// m_Error)

bool
CCmdArgOpts::ReadArg(int& ac, char**& av, vector<string>& target)
{
    target.clear();
    
    ac--; av++;
    
    if(!ac || av[0][0] == '-') {
        m_Error = true;
        m_ErrorStr = string("missing value for option '") +
            string((*(av-1))+1) + string("'");
    } else {
        while(ac && av[0][0] != '-') {
            target.push_back(av[0]);
            ac--; av++;
        }
    }
    
    return true;
}

void
CCmdArgOpts::SetDefaults(CCmdArgOpts* pGlobalOpts)
{
    m_Error = false;
    m_ErrorStr = "";
    
    // set default argument values

    // configuration file(s). No configuration file by default
    if(pGlobalOpts)
        m_ConfFiles = pGlobalOpts->m_ConfFiles;
    else
        m_ConfFiles.clear();
    // standard output
    m_OutputFile = pGlobalOpts ? pGlobalOpts->m_OutputFile : "";
    // standard output suffix
    m_OutputFileSuffix = pGlobalOpts ? pGlobalOpts->m_OutputFileSuffix : "";
    // no progress report
    m_ProgressReport = pGlobalOpts ? pGlobalOpts->m_ProgressReport : 0;
    // from the beginning
    m_FirstObjToProcess = pGlobalOpts ? pGlobalOpts-> m_FirstObjToProcess : 0;
    // to the end
    m_LastObjToProcess = pGlobalOpts ? pGlobalOpts->m_LastObjToProcess : 0;
    // no filtering by default
    m_FilterMaxWordCount = pGlobalOpts ? pGlobalOpts->m_FilterMaxWordCount : 0;
    // No filtering by default
    if(pGlobalOpts)
        m_OnlyTopTags = pGlobalOpts->m_OnlyTopTags;
    else
        m_OnlyTopTags.clear();
    // No filtering by default
    if(pGlobalOpts)
        m_NotTopTags = pGlobalOpts->m_NotTopTags;
    else
        m_NotTopTags.clear();
    // No filtering by default
    m_bNonTrivialFilter =
        pGlobalOpts ? pGlobalOpts->m_bNonTrivialFilter : false;
    // No evaluators by default
    if(pGlobalOpts)
        m_Evaluators = pGlobalOpts->m_Evaluators;
    else
        m_Evaluators.clear();
    // No lexicon printing by default
    m_bPrintLexicon = pGlobalOpts ? pGlobalOpts->m_bPrintLexicon : false;
}

bool
CCmdArgOpts::ArgSwitch(int& ac, char**& av)
{
    switch(av[0][1]) {
        case 'G':
            ReadArg(ac, av, m_ConfFiles);
            break;
        case 'o':
            ReadArg(ac, av, m_OutputFile);
            break;
        case 's':
            ReadArg(ac, av, m_OutputFileSuffix);
            break;
        case 'R':
            ReadArg(ac, av, m_ProgressReport);
            break;
        case 'L':
            ReadArg(ac, av, m_LastObjToProcess);
            break;
        case 'B':
            ReadArg(ac, av, m_FirstObjToProcess);
            break;
        case 'c':
            ReadArg(ac, av, m_FilterMaxWordCount);
            break;
        case 't':
            ReadArg(ac, av, m_OnlyTopTags);
            break;
        case 'n':
            ReadArg(ac, av, m_NotTopTags);
            break;
        case 'C':
            m_bNonTrivialFilter = true;
            ac--; av++;
            break;
        case 'e':
            ReadArg(ac, av, m_Evaluators);
            break;
        case 'p':
            m_bPrintLexicon = true;
            ac--; av++;
            break;
        case '-':
            // Just a separator (end of multi-value argument).
            ac--; av++;
            break;
        default:
            return false; // not matched
    }

    return true;
}

bool
CCmdArgOpts::ReadArgs(int& ac, char**& av)
{
    // read program arguments

    while(ac > 0) {
        if(av[0][0] != '-')
            break;

        // try to read arguments
        if(!ArgSwitch(ac, av)) {
            // argument not matched
            SetError(string("unknown option: '")+string(av[0]+1)+string("'"));
            return false;
        }
            
        // argument matched, but still have to check whether an error
        // occurred
        if(m_Error)
            return false;
    }
 
    return true;
}

CCmdArgs::CCmdArgs(int ac, char** av) : CCmdArgOpts()
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    // Read program name
    if(!ac) {
        SetError("program name missing");
        return;
    }
    
    m_ProgramName = av[0];
    ac--;
    av++;

    SetDefaults();
    
    if(!ReadArgs(ac, av)) // modifies ac and av when reading arguments
        return;
    
    if(!ac) {
        // no file list
        SetError("missing input files");
        return;
    }

    for( ; ac > 0 ; ac--, av++)
        m_InFiles.push_back(av[0]);
}
