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
#include "Token.h"

using namespace std;

void
CToken::SetEndOfToken(std::string const& EoT, bool bWSEoT) {
    m_EoT = EoT;
    if(bWSEoT)
        m_EoT.append(WhiteSpace());
}

bool
CToken::ReadToken(string::size_type& Pos, string const& Input)
{
    if(m_bComplete || !m_bStarted)
        return true; // nothing to add
    
    if(!m_Token.length() && !m_bDelimited) {
        // still looking for the beginning of the token - first skip any
        // white space
        if((Pos = Input.find_first_not_of(WhiteSpace(), Pos)) ==
           string::npos) {
            Pos = Input.length(); // nothing: return first position beyond end
            return false;
        }
        // check for a token symbol
        if(m_TokenSymbols.find(Input[Pos], 0) != string::npos) {
            m_Token.assign(Input, Pos, 1);
            m_bComplete = true;
            m_bTokenSymbol = true;
            Pos++;
            return true;
        }
        // check for a delimiter
        string::size_type DelPos; 
        if((DelPos = m_StartDel.find(Input[Pos], 0)) != string::npos) {
            m_bDelimited = true;
            if(DelPos >= m_EndDel.length())
                m_EndDelimiter = m_StartDel[DelPos];
            else
                m_EndDelimiter = m_EndDel[DelPos];
            if(++Pos >= Input.length())
                return false;
        }
    }

    string::size_type EndPos;
    
    // continue reading the token
    if(m_bDelimited) {
        if((EndPos = Input.find(m_EndDelimiter, Pos)) == string::npos) {
            m_Token.append(Input, Pos, Input.length() - Pos);
            Pos = Input.length(); // return beyond end of input
        } else {
            // copy not including end delimiter
            m_Token.append(Input, Pos, EndPos - Pos);
            m_bComplete = true;
            Pos = EndPos+1;  // beyond end of token (including delimiter)
        }
        return m_bComplete;
    }

    if(!m_EoT.length())
        return (m_bComplete = true); // empty token
    
    if((EndPos = Input.find_first_of(m_EoT, Pos)) == string::npos) {
        m_Token.append(Input, Pos, Input.length() - Pos);
        Pos = Input.length(); // return beyond end of input
    } else {
        // copy not including end-of-token
        m_Token.append(Input, Pos, EndPos - Pos);
        m_bComplete = true;
        m_EndOfToken = Input[EndPos];
        Pos = EndPos;  // beyond end of token (not including EoT) 
    }

    return m_bComplete;
}
