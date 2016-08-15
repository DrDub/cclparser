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

#include "time.h"
#include "StringUtil.h"
#include "OutFile.h"

using namespace std;


COutFile::COutFile(std::string const& BaseName) : m_BaseName(BaseName)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

COutFile::COutFile(std::string const& BaseName, std::string const& Suffix)
        : m_BaseName(BaseName), m_Suffix(Suffix)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    OpenWithSuffix(m_Suffix);
}

COutFile::~COutFile()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

// Open the next file with the same base name and the given suffix
// (this does not check whether the suffix was already used).
void
COutFile::OpenWithSuffix(std::string const& Suffix)
{
    m_Suffix = Suffix;
    
    string FileName;

    if(m_BaseName.empty())
        FileName = "Unnamed";
    else
        FileName = m_BaseName;
    
    if(m_Suffix.empty()) {
        time_t CurrentTime = time(NULL);
        string TimeStr(ctime(&CurrentTime));
        // Remove trailing white space, including EoL
        RemoveTrailingWS(TimeStr);
        FileName = FileName + "." + TimeStr;
    } else
        FileName = FileName + "." + Suffix;
        
    
    m_pOutStream = new CRefOFStream(FileName.c_str());
}

// If no file stream was created, open one
COutFile::operator std::ostream&()
{
    if(!m_pOutStream)
        OpenWithSuffix("");

    return m_pOutStream ?
        (std::ostream&)*m_pOutStream : (std::ostream&)std::cout;
}

COutFile::operator CRefOStream*()
{
    if(!m_pOutStream)
        OpenWithSuffix("");
    
    return m_pOutStream;
}
