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

#include "Punct.h"

using namespace std;

// Prints the symbol to the given stream (for debugging)
void
CPunct::PrintSymbol(ostream& Out)
{
    switch(m_Type) {
        case eNoPunct:
            Out << "<no punct>";
            break;
        case eEoUtterance:
            Out << "<EoU>";
            break;
        case eFullStop:
            Out << ".";
            break;
        case eQuestion:
            Out << "?";
            break;
        case eExclamation:
            Out << "!";
            break;
        case eSemiColon:
            Out << ";";
            break;
        case eDash:
            Out << "--";
            break;
        case eColon:
            Out << ":";
            break;
        case eComma:
            Out << ",";
            break;
        case eEllipsis:
            Out << "...";
            break;
        case eParenthesis:
            Out << ")/(";
            break;
        case eLeftParenthesis:
            Out << "(";
            break;
        case eRightParenthesis:
            Out << ")";
            break;
        case eDoubleQuote:
            Out << "\"";
            break;
        case eLeftDoubleQuote:
            Out << "``";
            break;
        case eRightDoubleQuote:
            Out << "''";
            break;
        case eSingleQuote:
            Out << "'";
            break;
        case eLeftSingleQuote:
            Out << "`";
            break;
        case eRightSingleQuote:
            Out << "'";
            break;
        case eHyphen:
            Out << "-";
            break;
        case eHeadlineStop:
            Out << "-";
            break;
        case eCurrency:
            Out << "$";
            break;
        default:
            Out << "<punct?>";
            break;
    }
}


// Returns true if the punctuation symbol terminates the utterance

bool
IsEoUPunct(ePunctType Punct)
{
    static unsigned int EoUPunctTypes =
//        (eFullStop|eQuestion|eExclamation|eHeadlineStop|eEoUtterance);
        eEoUtterance;

    return (Punct & EoUPunctTypes);
}

// Returns true if the punctuation is terminating punctuation
// (full stop, question mark, exclamation)
bool
IsTerminatingPunct(ePunctType Punct)
{
    static unsigned int TerminatingPunctTypes =
        (eFullStop|eQuestion|eExclamation|eHeadlineStop);

    return (Punct & TerminatingPunctTypes);
}

// Returns true if the given punctuation symbol is a 'stopping' punctuation

bool
IsStoppingPunct(ePunctType Punct)
{
    static unsigned int StoppingPunctTypes =
        (eFullStop|eQuestion|eExclamation|eHeadlineStop|
         eSemiColon|eComma|eDash);
    
    return (Punct & StoppingPunctTypes);
}
