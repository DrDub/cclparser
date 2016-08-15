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

#include <ostream>
#include <sstream>
#include "Loop.h"
#include "StringUtil.h"
#include "yError.h"

using namespace std;

CLoop::CLoop(vector<string> const & InFilePatterns, CCmdArgOpts* pArgs,
             CpCMessageLine& MsgLine, COutFile* pOutFile) :
        m_Args(pArgs), m_MsgLine(MsgLine),
        m_InFiles(InFilePatterns), m_pOutFile(pOutFile),
        m_Error(false), m_ErrorStr(""), m_ObjNum(0), m_bCountOnly(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CLoop::CLoop(vector<string> const & InFilePatterns, CCmdArgOpts* pArgs,
             COutFile* pOutFile) :
        m_Args(pArgs), m_MsgLine(NULL), m_InFiles(InFilePatterns),
        m_pOutFile(pOutFile), m_Error(false), m_ErrorStr(""),
        m_ObjNum(0), m_bCountOnly(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CLoop::~CLoop()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

bool
CLoop::ResetLoop()
{
    m_ObjNum = 0;
    m_ObjsProcessed = 0;
    return m_InFiles.Restart();
}

bool
CLoop::DoLoop()
{
    unsigned int LineNum = 0;
    
    m_ObjNum = 1;
    
    if(!m_InFiles.Good()) {
        m_ErrorStr = m_InFiles.Eof() ? 
            "At end of file (no files or must be restarted?)" :
            m_ErrorStr = "Problem while opening/reading from file";
        return !(m_Error = true);
    }

    while(m_InFiles.Good()) {
        
        if(!m_InFiles.GetLine())
            break;

        LineNum++;
        
        if(!NextLine(m_InFiles.LastLine())) {
            
            ostringstream Ostr(ios::out);
            if(m_ErrorStr.length())
                Ostr << endl << "In object " << m_ObjNum << "(line "
                     << LineNum << ")";
            else
                Ostr << "In object " << m_ObjNum << "(line " << LineNum << ")"
                     << ": error while processing";
            m_ErrorStr.append(Ostr.str());
            return !(m_Error = true);
        }
        
        if(IncObjNum() > 0) {

            if(m_Args->GetLastObjToProcess() &&
               m_ObjNum >= m_Args->GetLastObjToProcess()) {
                m_ObjNum += IncObjNum();
                break;
            }
            
            if(m_InFiles.Good()) {
                m_ObjNum += IncObjNum();
                if(m_ObjNum >= m_Args->GetFirstObjToProcess() &&
                   m_Args->GetProgressReport() &&
                   !(m_ObjNum % m_Args->GetProgressReport())) {
                    ostringstream Ostr(ios::out);
                    Ostr << m_ObjNum;
                    if(m_Args->GetLastObjToProcess())
                        Ostr << " / " << m_Args->GetLastObjToProcess();
                    if(m_MsgLine)
                        m_MsgLine->AppendMessage(Ostr.str());
                }
            }
        }
    }

    // loop terminated properly ?

    if(m_InFiles.Bad()) {
        m_ErrorStr = "Error while reading from files";
        return !(m_Error = true);
    }

    if(!EndLoop()) {
        if(m_ErrorStr.length())
            m_ErrorStr.insert(0, string("Error while ending loop: ") +
                              EoLDelimiter());
        else
            m_ErrorStr = "Error while ending loop";
        return !(m_Error = true);
    }

    // If no error occurred, the "current" object number is of an object
    // beyond the end of the file.
    m_ObjNum--;
    
    return true;
}


////////////////////////////
// Loop printing routines //
////////////////////////////

// Opens the output stream on the loop according to the path given
// by the argument structure and the given suffix.

bool
CLoop::OpenOutFileFromArgs(string const& Suffix)
{
    if(!GetArgs())
        return false;

    m_pOutFile = new COutFile(GetArgs()->GetOutFile(), Suffix);

    if(!m_pOutFile->IsOpen()) {
        ySWarn(ERR_FILE, "Failed to open output file");
        return false;
    }

    return true;
}

ostream&
CLoop::GetOutputStream()
{
    if((!m_pOutFile || !m_pOutFile->IsOpen()) &&
       !OpenOutFileFromArgs(GetArgs()->GetOutFileSuffix())) {
        ySWarn(ERR_FILE, "Failed to open output file");
        return cout;
    }

    return (ostream&)*m_pOutFile;
}

CRefOStream*
CLoop::GetOutputStreamObj()
{
    if((!m_pOutFile || !m_pOutFile->IsOpen()) &&
       !OpenOutFileFromArgs(GetArgs()->GetOutFileSuffix())) {
        ySWarn(ERR_FILE, "Failed to open output file");
    }

    return *m_pOutFile;
}
