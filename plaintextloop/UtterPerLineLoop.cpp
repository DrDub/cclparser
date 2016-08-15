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

#include <sstream>
#include "StringUtil.h"
#include "UtterPerLineLoop.h"

using namespace std;

CUtterPerLineLoop::CUtterPerLineLoop(vector<string> const & InFilePatterns,
                                     CCmdArgOpts* pArgs,
                                     CpCMessageLine& MsgLine,
                                     COutFile* pOutFile) :
        CPlainTextLoop(InFilePatterns, pArgs, MsgLine, pOutFile),
        m_bLineNotEmpty(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CUtterPerLineLoop::~CUtterPerLineLoop()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

bool
CUtterPerLineLoop::NextLine(std::string const& Line)
{
    m_bLineNotEmpty = false;
    
    // should this line be ignored?
    if(IsCommentLineStr(Line) || IsWhiteSpace(Line)) {
        ClearObject();
        return true;
    }

    m_bLineNotEmpty = true;
    
    m_ObjSource = Line + EoLDelimiter();
    
    if(CountOnly()) {
        // only have to indicate that the line is not empty
        return true;
    }

    // read the word tokens from the line
    istringstream IStr(Line);
    while(!IStr.fail()) {

        string Token;
        IStr >> Token;
        if(IStr.fail())
            break;
        
        ProcessTerminal(Token);
    }

    EndObject();
    
    return true;
}

bool
CUtterPerLineLoop::EndLoop()
{
    // nothng to do
    return true;
}
