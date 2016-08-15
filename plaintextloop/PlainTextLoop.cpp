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

#include <map>
#include "Globals.h"
#include "PlainTextLoop.h"
#include "yError.h"

using namespace std;

CPlainTextLoop::CPlainTextLoop(vector<string> const & InFilePatterns,
                               CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                               COutFile* pOutFile) :
        CLoop(InFilePatterns, pArgs, MsgLine, pOutFile)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CPlainTextLoop::~CPlainTextLoop()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CPlainTextLoop::ProcessTerminal(string const& Terminal)
{
    // Determine whether this is a punctuation symbol
    ePunctType PunctType = GetPunctType(Terminal);
    
    // Add the terminal to the syntactic structure
    if(!m_pSynStruct)
        m_pSynStruct = new CSynStruct();
    m_Terminals.push_back(m_pSynStruct->
                          AddTerminal(Terminal, "", (PunctType != eNoPunct)));

    // Send terminal to processing object
    if(PunctType != eNoPunct) {
        ProcessPunct(PunctType);
    } else {
        ProcessUnit(Terminal);
    }
}

void
CPlainTextLoop::ClearObject()
{
    m_pSynStruct = NULL;
    m_Terminals.clear();
    m_ObjSource = "";
}

void
CPlainTextLoop::EndObject()
{
    // Add a non-terminal covering all terminals
    if(m_pSynStruct) {
        m_pSynStruct->AddNonTerminal("", m_Terminals);
    }
    
    // Send the CSynStruct and clear it
    ProcessSynStruct(m_pSynStruct, m_ObjSource);

    if(m_pSynStruct->NonTerminalNum() && ObjectMatches())
        IncObjsProcessed();
    
    ClearObject();
    
    // Signal the end of the utterance
    ProcessPunct(eEoUtterance);
}

//
// Punctuation detection routines
//

// A static map structure is used to store the conversion from punctuation
// symbol as it appears in the file to the ePunctType.

static map<string const, ePunctType> PunctTransTable;

// This function initializes the punctuation translation table

static void
InitPunctTransTable()
{
    if(!PunctTransTable.empty()) {
        yPError(ERR_SHOULDNT, "attempting to re-initialize the table");
    }

    PunctTransTable["."] = eFullStop;
    PunctTransTable["?"] = eQuestion;
    PunctTransTable["!"] = eExclamation;
    PunctTransTable[";"] = eSemiColon;
    PunctTransTable["--"] = eDash;
    PunctTransTable[":"] = eColon;
    PunctTransTable[","] = eComma;
    PunctTransTable["..."] = eEllipsis;
    PunctTransTable["-LCB-"] = eLeftParenthesis;  // Penn TB annotation
    PunctTransTable["-LRB-"] = eLeftParenthesis;  // Penn TB annotation
    PunctTransTable["-RCB-"] = eRightParenthesis; // Penn TB annotation
    PunctTransTable["-RRB-"] = eRightParenthesis; // Penn TB annotation
    PunctTransTable["("] = eLeftParenthesis;
    PunctTransTable[")"] = eRightParenthesis;
    PunctTransTable["``"] = eLeftDoubleQuote;
    PunctTransTable["''"] = eRightDoubleQuote;
    PunctTransTable["`"] = eLeftSingleQuote;
    PunctTransTable["'"] = eRightSingleQuote;
    PunctTransTable["-"] = eHyphen;
    PunctTransTable["$"] = eCurrency;
    PunctTransTable["#"] = eCurrency;             // Penn TB annotation
}

ePunctType
CPlainTextLoop::GetPunctType(string const& Unit)
{
    if(PunctTransTable.empty())
        InitPunctTransTable();
    
    // Get the punctuation symbol
    map<string const, ePunctType>::iterator Entry =
        PunctTransTable.find(Unit);

    if(Entry == PunctTransTable.end()) {
        // not found
        return eNoPunct;
    }

    // should currency symbols be considered punctuation?
    if((*Entry).second == eCurrency && !g_CurrencySymbolIsPunct)
        return eNoPunct;
    
    return (*Entry).second;
}
