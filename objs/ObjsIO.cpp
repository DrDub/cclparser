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

#include "Objs.h"
#include "ObjsIO.h"
#include "StringUtil.h"
#include "yError.h"

using namespace std;

CObjsIO::CObjsIO(CObjParse* pParser,
                 vector<string> const & InFilePatterns,
                 CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                 COutFile* pOutFile) :
        m_pParser(pParser),
        CLoop(InFilePatterns, pArgs, MsgLine, pOutFile),
        m_IncObjNum(0)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

bool
CObjsIO::NextLine(std::string const& Line)
{
    m_IncObjNum = 0;
    
    if(!m_pParser) { yPError(ERR_MISSING, "parse object pointer missing"); }

    string::size_type Pos = 0;
    string::size_type PrevPos = 0;
    
    if(IsCommentLine(Line))
        return true; // skip comment lines
    
    while(m_pParser->ParseObj(Pos, Line)) {

        // Add the input read to 'm_ObjSource'
        m_ObjSource.append(Line, PrevPos, Pos - PrevPos);
        PrevPos = Pos;
        if(Pos >= Line.length())
            m_ObjSource += EoLDelimiter();
        
        if(m_pParser->IsComplete()) {
            // increment the number of objects completed in this line 
            m_IncObjNum++;
            m_ObjSource += EoLDelimiter();
            if(!CountOnly()) {
                // call the routines which process this object
                if(!ProcessObject(m_pParser->GetObj(), m_ObjSource))
                    return false; // error occurred
            }

            m_ObjSource = "";
            m_pParser->NewParse();
            if(Pos < Line.length())
                continue;
        }

        // otherwise, wait for more data
        return true;
    }

    // an error occurred
    SetErrorStr(m_pParser->ErrorMsg());
    return false;
}

bool
CObjsIO::EndLoop()
{
    if(m_pParser && m_pParser->IsStarted() && !m_pParser->IsComplete()) {
        SetErrorStr("End of input reached before end of object:" +
                    EoLDelimiter() + m_ObjSource);
        return false;
    }

    return true;
}

////////////////////////////////
// specific object IO classes //
////////////////////////////////

CXMLObjsIO::CXMLObjsIO(vector<string> const & InFilePatterns,
                       CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                       COutFile* pOutFile) :
        CObjsIO(new CXMLObjParse(), InFilePatterns, pArgs, MsgLine, pOutFile)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CPennTBObjsIO::CPennTBObjsIO(vector<string> const & InFilePatterns,
                             CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                             COutFile* pOutFile) :
        CObjsIO(new CPennTBObjParse(), InFilePatterns,
                pArgs, MsgLine, pOutFile)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}
