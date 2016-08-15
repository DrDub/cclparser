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

#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include "ConfArgs.h"

using namespace std;

bool
CConfArgs::AddArg(string ArgName, unsigned int* ArgVar)
{
    m_UnsignedIntArgs[ArgName] = ArgVar;
}

bool
CConfArgs::AddArg(string ArgName, float* ArgVar)
{
    m_FloatArgs[ArgName] = ArgVar;
}

bool
CConfArgs::AddArg(string ArgName, string* ArgVar)
{
    m_StringArgs[ArgName] = ArgVar;
}


// Read all registered arguments from the configuration files

bool
CConfArgs::ReadArgs()
{
    m_ArgFile.Restart();
    string Name;
    float FloatVal;
    unsigned int UnsignedIntVal;
    string StringVal;
    
    while(m_ArgFile.Good()) {
        
        if(!m_ArgFile.GetLine())
            break;
        
        istringstream istr((char const*)m_ArgFile);
        istr >> Name;
        if(istr.fail()) {
            // ignore this line (probably empty)
            continue;
        }

        if(Name.c_str()[0] == '#')
            continue;
        
        if(m_UnsignedIntArgs.count(Name)) {
            istr >> UnsignedIntVal;
            if(istr.fail()) {
                m_Error = true;
                m_ErrorStr = string("Error while reading '") + Name +
                    string("': expected unsigned int value");
                return false;
            }
            *(m_UnsignedIntArgs[Name]) = UnsignedIntVal;
        } else if(m_FloatArgs.count(Name)) {
            istr >> FloatVal;
            if(istr.fail()) {
                m_Error = true;
                m_ErrorStr = string("Error while reading '") + Name +
                    string("': expected float value");
                return false;
            }
            *(m_FloatArgs[Name]) = FloatVal;
        } else if(m_StringArgs.count(Name)) {
            istr >> ws;
            getline(istr, StringVal);
            *(m_StringArgs[Name]) = StringVal;
        } else {
            m_Error = true;
            m_ErrorStr = string("Unknown argument name: '") + Name +
                string("' from: ");
            m_ArgFile.PrintFileList(m_ErrorStr);
            return false;
        }
    }

    if(m_ArgFile.Bad()) {
        m_Error = true;
        m_ErrorStr = "Error while reading arguments from: ";
        m_ArgFile.PrintFileList(m_ErrorStr);
        return false;
    }

    return true;
}

// Read all registered arguments from the specified configuration files

bool
CConfArgs::ReadArgs(std::vector<std::string> const& Patterns)
{
    if(!Patterns.size())
        return true; // nothing to do
    
    if(!m_ArgFile.Restart(Patterns)) {
        m_Error = true;
        m_ErrorStr = "failed to open configuration files: ";
        m_ArgFile.PrintFileList(m_ErrorStr);
        return false;
    }

    return ReadArgs();
}

// Prints the names and current values of all registered variables
// into the given stream.
// 'Prefix' is an optional string to be prefixed to every output line.

void
CConfArgs::PrintArgs(ostream& Ostream, string const& Prefix)
{
    map<string, unsigned int*>::iterator UIIter = m_UnsignedIntArgs.begin();
    map<string, float*>::iterator FIter = m_FloatArgs.begin();
    map<string, string*>::iterator SIter = m_StringArgs.begin();

    while(UIIter != m_UnsignedIntArgs.end()) {
        Ostream << Prefix;
        Ostream << UIIter->first << " = " << *(UIIter->second) << endl;
        UIIter++;
    }

    while(FIter != m_FloatArgs.end()) {
        Ostream << Prefix;
        Ostream << FIter->first << " = " << *(FIter->second) << endl;
        FIter++;
    }

    while(SIter != m_StringArgs.end()) {
        Ostream << Prefix;
        Ostream << SIter->first << " = " << *(SIter->second) << endl;
        SIter++;
    }

    Ostream << endl;
}

// Prints the names and current values of all registered variables
// into the given string.
// 'Prefix' is an optional string to be prefixed to every output line.

void
CConfArgs::PrintArgs(std::string& OutString, string const& Prefix)
{
    ostringstream Ostr(ios::out);

    PrintArgs(Ostr, Prefix);
    OutString = Ostr.str();
}

