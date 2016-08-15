#ifndef __CONFARGS_H__
#define __CONFARGS_H__

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
#include <vector>
#include <map>
#include <ostream>
#include "Reference.h"
#include "InFile.h"

// Sets the values of global variables from a configuration file

class CConfArgs : public CRef
{
private:
    CMultiInFile m_ArgFile;  // multi-file from which arguments are read
    std::map<std::string, unsigned int*> m_UnsignedIntArgs; // list of args
    std::map<std::string, float*> m_FloatArgs; // list of args
    std::map<std::string, std::string*> m_StringArgs; // list of args
    bool m_Error;
    std::string m_ErrorStr;
public:
    CConfArgs() : m_ArgFile(), m_Error(false), m_ErrorStr("") {}
    CConfArgs(std::vector<std::string>& Patterns) :
            m_ArgFile(Patterns), m_Error(false), m_ErrorStr("") {}
    // return false in case of error
    bool AddArg(std::string ArgName, unsigned int* ArgVar);
    bool AddArg(std::string ArgName, float* ArgVar);
    bool AddArg(std::string ArgName, std::string* ArgVar);
    // Read all registered arguments from the configuration files
    bool ReadArgs();
    // Read all registered arguments from the specified configuration files
    bool ReadArgs(std::vector<std::string> const& Patterns);
    // Prints the names and current values of all registered variables
    // into the given stream.
    // 'Prefix' is an optional string to be prefixed to every output line.
    void PrintArgs(std::ostream& Ostream, std::string const& Prefix = "");
    // Prints the names and current values of all registered variables
    // into the given string.
    // 'Prefix' is an optional string to be prefixed to every output line.
    void PrintArgs(std::string& OutString, std::string const& Prefix = "");
    // check error status
    bool IsError() { return m_Error; }
    std::string& GetErrorStr() { return m_ErrorStr; }
};

#endif /* __CONFARGS_H__ */
