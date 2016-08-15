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

#include "StringUtil.h"
#include "MessageLine.h"

using namespace std;

void
CMessageLine::Clear()
{
    string Blank(m_MessageLength+m_AppendLength, ' ');
    
    cerr << "\r" << Blank << "\r" << flush;
    
    m_MessageLength = 0;
    m_AppendLength = 0;
}
void
CMessageLine::NewMessage(std::string const& Msg)
{
    Clear();
    cerr << Msg << flush;
    m_MessageLength = Msg.length();
}

void CMessageLine::AppendMessage(std::string const& Msg)
{
    // clear the previously appended message (if any)
    if(m_AppendLength) {
        string Blank(m_AppendLength, ' ');
        string Back(m_AppendLength, '\b');
        cerr << Blank << Back << flush;
    }
    
    m_AppendLength = Msg.length();

    string Backspace(m_AppendLength, '\b');
    
    cerr << Msg << Backspace << flush;
}

void
CMessageLine::NewMessageLine()
{
    cerr << EoLDelimiter();

    m_MessageLength = 0;
    m_AppendLength = 0;
}
