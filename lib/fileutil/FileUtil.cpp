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

#include <glob.h>
#include <string.h>
#include "yError.h"
#include "FileUtil.h"

using namespace std;

CFilesFromPatterns::CFilesFromPatterns(std::vector<string> const& Patterns,
                                       bool FilesOnly) : m_Patterns(Patterns)
{
    Restart(FilesOnly);
}

bool
CFilesFromPatterns::ResetPatterns(std::vector<string> const& Patterns,
                                  bool FilesOnly)
{
    m_Patterns = Patterns;
    return Restart(FilesOnly);
}

bool
CFilesFromPatterns::Restart(bool FilesOnly)
{
    static char Rname[] = "CFilesFromPatterns::Restart()";
    glob_t GlobBuf;
    int flags = GLOB_MARK;
    int rc;
    
    m_Files.clear();
    
    GlobBuf.gl_offs = 0;
    GlobBuf.gl_pathc = 0;
    
    // iterate over the patterns

    for(vector<string>::iterator Iter = m_Patterns.begin();
        Iter != m_Patterns.end();
        Iter++) {

        if(rc = glob((*Iter).c_str(), flags, NULL, &GlobBuf)) {

            if(rc == GLOB_NOSPACE) {
                yMemError("in call to glob()");
            } else if(rc == GLOB_ABORTED) {
                ySWarn(ERR_SYS, "glob aborted (read error?) continuing ... ");
                continue;
            } else if(rc == GLOB_NOMATCH) {
                continue;
            }
        }
    
        flags |= GLOB_APPEND;
    }

    if(GlobBuf.gl_pathc)
        for(char** pS = GlobBuf.gl_pathv ; pS && *pS ; pS++) {
            if(!FilesOnly || (*pS)[strlen(*pS)-1] != FILE_SLASH)
                m_Files.push_back(*pS);
        }

    if(GlobBuf.gl_pathc)
        globfree(&GlobBuf);
                
    // restart iterator
    m_Iter = m_Files.begin();
    return true;
}

void
CFilesFromPatterns::PrintFileList(std::string& OutString)
{
    vector<string>::iterator Iter = m_Files.begin();
    
    while(Iter != m_Files.end()) {
        OutString += *Iter;
        Iter++;
        if(Iter != m_Files.end())
            OutString += ", ";
    }
}

void
CFilesFromPatterns::PrintPatternList(std::string& OutString)
{
    vector<string>::iterator Iter = m_Patterns.begin();
    
    while(Iter != m_Patterns.end()) {
        OutString += *Iter;
        Iter++;
        if(Iter != m_Patterns.end())
            OutString += ", ";
    }
}
