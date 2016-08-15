#ifndef __MESSAGELINE_H__
#define __MESSAGELINE_H__

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

#include <iostream>
#include <string>
#include "Reference.h"

// Class for printing progress report information

class CMessageLine : public CRef
{
private:
    unsigned int m_MessageLength;
    unsigned int m_AppendLength;
public:
    CMessageLine() : m_MessageLength(0), m_AppendLength(0) {}

    void Clear();
    void NewMessage(std::string const& Msg);
    void AppendMessage(std::string const& Msg);
    void NewMessageLine();
};

typedef CPtr<CMessageLine> CpCMessageLine;


#endif /* __MESSAGELINE_H__ */
