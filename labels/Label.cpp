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

#include "Label.h"

using namespace std;

void
CLabel::LabelString(string& Output)
{
    string Prefix;
    string Suffix;
    
    unsigned int Type = m_Type;

    if(Type & 1)
        Prefix += '.';
    
    while(Type & 2) {
        Prefix += '{';
        Suffix += '}';
        if(Type & 4)
            Prefix += '.';

        Type = Type >> 2;
    }

    Output = Prefix + (string const&)*m_StrKey + Suffix;
}
