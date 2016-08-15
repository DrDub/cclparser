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

#include <sstream>
#include <iomanip>
#include "Globals.h"
#include "StringUtil.h"

using namespace std;

// converts the given string to lower case

void
ToLower(string& Str)
{
    for(string::iterator Iter = Str.begin() ; Iter != Str.end() ; Iter++) {
        *Iter = tolower(*Iter);
    }
}

//
// White space routines
//

// string defining the white space characters
static const string WhiteSpaceChars(" \f\n\r\t\v");

// Remove leading white space
void
RemoveLeadingWS(std::string& Str)
{
    string::size_type Begin = Str.find_first_not_of(WhiteSpaceChars);

    if(Begin != string::npos)
        Str = Str.substr(Begin);
}

// Remove trailing white space
void
RemoveTrailingWS(std::string& Str)
{
    string::size_type End = Str.find_last_not_of(WhiteSpaceChars);

    if(End != string::npos)
        Str.erase(End+1);
}

// Remove leading and trailing white space
void
RemoveLeadingAndTrailingWS(std::string& Str)
{
    RemoveLeadingWS(Str);
    RemoveTrailingWS(Str);
}

// return true if the string only contains white space
bool
IsWhiteSpace(std::string const& Str)
{
    return (Str.find_first_not_of(WhiteSpaceChars) == string::npos);
}

// Return the current eol character(s)
string const&
EoLDelimiter()
{
    static string EoL;

    if(!EoL.size()) { // initialize the string
        ostringstream Ostr(ios::out);
        Ostr << endl;
        EoL = Ostr.str();
    }

    return EoL;
}

// Return a string containing all white space characters
string const&
WhiteSpace()
{
    return WhiteSpaceChars;
}

// Determine whether an input line is comment line
bool
IsCommentLineStr(string const& Line)
{
    return (Line.length() > 0 && g_CommentStr.length() > 0 &&
            !Line.compare(0,g_CommentStr.length(),g_CommentStr));
}
