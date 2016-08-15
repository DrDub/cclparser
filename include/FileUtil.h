#ifndef __FILEUTIL_H__
#define __FILEUTIL_H__

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

#include "Reference.h"
#include <vector>
#include <string>
#include <istream>
#include <ostream>
#include <fstream>

#define FILE_SLASH '/'   // slash use to separate directories

// Given a list of file patterns, this class is an iterator over the matching
// files names

class CFilesFromPatterns : CRef
{
    std::vector<std::string> m_Patterns; // the patterns 
    std::vector<std::string> m_Files;  // the file names matching the patterns
    std::vector<std::string>::iterator m_Iter; // the file iterator
public:
    CFilesFromPatterns() : m_Patterns(0), m_Files(0) {
        m_Iter = m_Files.begin();
    }
    // In the following functions, if FilesOnly is set, paths which are
    // directories are skipped.
    CFilesFromPatterns(std::vector<std::string> const& Patterns,
                       bool FilesOnly = true);
    bool ResetPatterns(std::vector<std::string> const& Patterns,
                       bool FilesOnly = true); // restarts iterator
    // Re-evaluates the patterns and restarts the iterator. 
    bool Restart(bool FilesOnly = true);
    // sets the iterator back to the beginning (without recalculating paths)
    void Begin() { m_Iter = m_Files.begin(); }
    operator bool() { return m_Iter != m_Files.end(); }
    bool operator ++() {
        if(m_Iter != m_Files.end())
            m_Iter++;
        return m_Iter != m_Files.end();
    }
    std::string& operator()() { return *m_Iter; }

    // Appends the file list to the given string
    void PrintFileList(std::string& OutString);
    // Appends the pattern list to the given string
    void PrintPatternList(std::string& OutString);
};

#endif /* __FILEUTIL_H__ */
