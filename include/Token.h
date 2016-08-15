#ifndef __TOKEN_H__
#define __TOKEN_H__

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
#include "Reference.h"

// Leading white space is always skipped before a token.
// Beyond the leading white space, a token is defined by a set of
// end-of-token symbols, a set of delimiter pairs and a set
// of token symbols.
// If the first character is matched by any of the token symbols,
// it is the token.
// If any of the start delimiters is matched at the beginning of the
// token, its corresponding end delimiter is the only character which
// can terminate the token (the delimiters are not part of the token
// but are read off the input). Otherwise, the first end-of-token
// symbol matched terminates the token. In this case, the end-of-token
// symbol remains on the input (for the subsequent token). 

class CToken;
typedef CPtr<CToken> CpCToken;

class CToken : public CRef
{
private:
    std::string m_Token;      // Token read
    bool m_bStarted;          // set if started reading token
    bool m_bComplete;         // set if the whole token was read
    bool m_bTokenSymbol;      // token is a single token symbol
    bool m_bDelimited;        // set if a start delimiter was matched
    char m_EndDelimiter;      // set to the required end delimiter if a start
                              // delimiter was seen.
    char m_EndOfToken;        // actual end of token symbol encountered
    
    std::string m_TokenSymbols; // Symbol which are full tokens
    std::string m_StartDel;   // string with start delimiters
    std::string m_EndDel;     // string with end delimiters (in same order)
    std::string m_EoT;        // string with end of token symbols
public:
    CToken() : m_bStarted(false), m_bComplete(false), m_bTokenSymbol(false),
               m_bDelimited(false) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }

    ~CToken() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

    void SetTokenSymbols(std::string const& TokenSymbols) {
        m_TokenSymbols = TokenSymbols;
    }
    // if Start is longer than End, then all delimiters in Start with no
    // corresponding character in End are considered both start and end
    // delimiters
    void SetDelimiters(std::string const& Start, std::string const& End) {
        m_StartDel = Start;
        m_EndDel = End;
    }
    void SetEndOfToken(std::string const& EoT, bool bWSEoT = true);

    // Clear token and start reading a new token (using the existing
    // format)
    void StartToken() {
        m_Token = "";
        m_bStarted = true;
        m_bComplete = false;
        m_bTokenSymbol = false;
        m_bDelimited = false;
    }

    // stop token processing
    void Stop() {
        m_bStarted = false;
    }
    
    // combines all of the above in one
    void SetAndStart(std::string const& TokenSymbols, std::string const& Start,
                     std::string const& End, std::string const& EoT,
                     bool bWSEoT = true) {
        SetTokenSymbols(TokenSymbols);
        SetDelimiters(Start, End);
        SetEndOfToken(EoT, bWSEoT);
        StartToken();
    }
    
    // reads/continues to read/ the token from the given position in the
    // input string.
    // Returns true if the complete token was read, false otherwise.
    // Returns in Pos the first position after the last position read into
    // the token.
    bool ReadToken(std::string::size_type& Pos, std::string const& Input);
    
    // return the token read
    std::string& GetToken() { return m_Token; }
    // has the token been started ?
    bool Started() { return m_bStarted; }
    // is the token complete ?
    bool Complete() { return m_bComplete; }
    // is the token complete assuming eol was reached ? If no start delimiter
    // was read, then the eol terminates the token (even without an
    // end-of-token character). If a start delimiter was seen then an
    // end delimiter is required.
    bool EolComplete() { return !m_bDelimited || m_bComplete; }

    bool TokenSymbol() { return m_bTokenSymbol && m_bComplete; }
};

#endif /* __TOKEN_H__ */
