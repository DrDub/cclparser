#ifndef __OUTFILE_H__
#define __OUTFILE_H__

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

#include <string>
#include <iostream>
#include "Reference.h"
#include "RefStream.h"

class COutFile : public CRef
{
private:
    // The file currently open is <m_BaseName>.<m_Suffix>
    std::string m_BaseName;
    std::string m_Suffix;
    CpCRefOStream m_pOutStream;
public:
    // This version of the constructor only sets the base name of the output
    // file, but does not open any file yet (because there is no suffix).
    // As long as no file is opened, stdout is used.
    COutFile(std::string const& BaseName);
    // Create the object and open the file <BaseName>.<Suffix>.
    COutFile(std::string const& BaseName, std::string const& Suffix);
    ~COutFile();

    // Open the next file with the same base name and the given suffix
    // (this does not check whether the suffix was already used).
    void OpenWithSuffix(std::string const& Suffix);

    // Current base name
    std::string const& BaseName() { return m_BaseName; }
    // Current suffix
    std::string const& Suffix() { return m_Suffix; }
    
    // Stream interface

    // If no file stream was created, open one
    operator std::ostream&();
    operator CRefOStream*();
    
    // Derived streams which have an open/closed state should redefine this
    // function.
    bool IsOpen() { return m_pOutStream ? m_pOutStream->IsOpen() : false; }
};

typedef CPtr<COutFile> CpCOutFile;

#endif /* __OUTFILE_H__ */
