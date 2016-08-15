#ifndef __INFILE_H__
#define __INFILE_H__

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

#include "FileUtil.h"

// The CMultiInFile class allows to read information from a list of files
// (as if they were concatenated to each other). The class is initialized
// with a vector of patterns. The files matched by the patterns are read.
// The order of the patterns determines the order of the files, that is,
// files matched by the first pattern appear before those matched by
// the second, etc. The order of files matched by each pattern is sorted
// (alphabetically).
// The last line read is stored in a string internal to the object.
// This string is always guaranteed to contain the complete line
// (there is no limit on the length of the line as long as there is
// sufficient memory).
// The line delimiter is removed.

class CMultiInFile : CRef
{
private:
    CFilesFromPatterns m_Files; // iterator over files
    std::string m_LastLine;     // (complete) last line read
    std::ifstream m_InStream;   // stream based on current input file

    // state bits
    bool m_Bad;                 // an error has occurred
    bool m_Eof;                 // EOF reached
public:
    CMultiInFile();
    CMultiInFile(std::vector<std::string> const & Patterns);
private:
    // Opens the next file
    bool OpenNextFile();
    // Restarts the stream with the current list of files
    bool RestartStream();
public:
    bool Restart();  // resets to the beginning of the same patterns
    // restart with new patterns.
    bool Restart(std::vector<std::string> const& Patterns);
    bool GetLine(); // reads the next line
    bool Bad() { return m_Bad; }
    bool Eof() { return m_Eof; }
    bool Fail() { return m_Bad || m_Eof; }
    bool Good() { return !m_Bad && !m_Eof; }
    // clears the m_Bad flag if the error is not fatal returns true upon
    // success
    bool Clear();
    operator char const*() { return m_LastLine.c_str(); }
    std::string& LastLine() { return m_LastLine; }
    // Appends the list of files to a string
    void PrintFileList(std::string& OutString) {
        m_Files.PrintFileList(OutString);
    }
    // Appends the list of patterns to a string
    void PrintPatternList(std::string& OutString) {
        m_Files.PrintPatternList(OutString);
    }
};

#endif /* __INFILE_H__ */
