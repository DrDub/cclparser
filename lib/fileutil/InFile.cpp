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

#include "yError.h"
#include "InFile.h"

using namespace std;

//
// CMultiInFile
//

CMultiInFile::CMultiInFile(std::vector<std::string> const& Patterns) :
        m_Files(Patterns)
{
    RestartStream();
}

CMultiInFile::CMultiInFile() : m_Files()
{
    RestartStream();
}

bool
CMultiInFile::OpenNextFile()
{
    static char Rname[] = "CMultiInFile::OpenNextFile()";
    
    if(m_InStream.is_open())
        m_InStream.close();

    m_LastLine = "";
    
    if(!m_Files) {
        m_Eof = true;
        return false;
    }

    m_InStream.clear();
    m_InStream.open(m_Files().c_str());
    ++m_Files;
    
    if(!m_InStream.is_open()) {
        ySWarn(ERR_FILE, "failed to open file");
        m_Bad = true;
        return false;
    }

    return true;
}

bool
CMultiInFile::RestartStream()
{   
    m_Bad = m_Eof = false;
    m_Files.Begin();
    return OpenNextFile();
}

bool
CMultiInFile::Restart()
{
    m_Files.Restart();
    return RestartStream();
}

bool
CMultiInFile::Restart(std::vector<std::string> const& Patterns)
{
    m_Files.ResetPatterns(Patterns);
    return RestartStream();
}

bool
CMultiInFile::GetLine()
{
    if(m_Bad || m_Eof)
        return false;

    if((!m_InStream.is_open() || m_InStream.eof() || m_InStream.fail())
       && !OpenNextFile())
        return false;

    getline(m_InStream, m_LastLine);
        
    if(m_InStream.bad())
        return !(m_Bad = true);

    return true;
}

bool
CMultiInFile::Clear()
{
    if(!m_Bad)
        return true;
    
    if(m_InStream.bad()) {
        m_InStream.clear();
        return !(m_Bad = m_InStream.bad());
    }

    return !(m_Bad = false);
}
