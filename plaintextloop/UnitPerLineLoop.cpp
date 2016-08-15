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

#include "UnitPerLineLoop.h"
#include "StringUtil.h"

using namespace std;

CUnitPerLineLoop::CUnitPerLineLoop(vector<string> const & InFilePatterns,
                                   CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                                   COutFile* pOutFile) :
        CPlainTextLoop(InFilePatterns, pArgs, MsgLine, pOutFile),
        m_bNonEmptyObj(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CUnitPerLineLoop::~CUnitPerLineLoop()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CUnitPerLineLoop::EndObject()
{
    // indicate that end of object was reached
    if(m_bNonEmptyObj)
        m_IncObjNum = 1;
    // Add an extra empty line
    m_ObjSource += EoLDelimiter();
    // call the base class 'end of object' handling
    if(m_bNonEmptyObj)
        CPlainTextLoop::EndObject();
    else
        ClearObject();

    m_bNonEmptyObj = false;
}

bool
CUnitPerLineLoop::NextLine(std::string const& Line)
{   
    m_IncObjNum = 0;

    if(IsCommentLineStr(Line))
        return true; // ignore this line
    
    // remove leading and trailing white space
    string CroppedLine = Line;
    RemoveLeadingAndTrailingWS(CroppedLine);

    if(CroppedLine == "") { // end of utterance
        if(CountOnly()) {
            if(m_bNonEmptyObj)
                m_IncObjNum = 1;
            
        } else
            EndObject();

        m_bNonEmptyObj = false; // prepare for next object
        return true;
    }

    // non-empty line
    m_bNonEmptyObj = true;
    
    if(CountOnly())
        return true;

    m_ObjSource += (Line + EoLDelimiter());
    
    ProcessTerminal(CroppedLine);

    return true;
}

bool
CUnitPerLineLoop::EndLoop()
{
    if(CountOnly()) {
        if(m_bNonEmptyObj)
            m_IncObjNum = 1;
    } else
        EndObject();

    m_bNonEmptyObj = false;
    
    return true;
}
