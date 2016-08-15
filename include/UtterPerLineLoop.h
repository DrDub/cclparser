#ifndef __UTTERPERLINELOOP_H__
#define __UTTERPERLINELOOP_H__

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

#include "PlainTextLoop.h"


class CUtterPerLineLoop : public CPlainTextLoop
{
private:
    // true if the line read is not empty
    bool m_bLineNotEmpty;
public:
    CUtterPerLineLoop(std::vector<std::string> const & InFilePatterns,
                      CCmdArgOpts* pArgs,
                      CpCMessageLine& MsgLine, COutFile* pOutFile);
    
    ~CUtterPerLineLoop();

public:
    bool NextLine(std::string const& Line);
    bool EndLoop();
    // each non-empty line is a separate utterance
    unsigned int IncObjNum() { return m_bLineNotEmpty ? 1 : 0; }
};

typedef CPtr<CUtterPerLineLoop> CpCUtterPerLineLoop;

#endif /* __UTTERPERLINELOOP_H__ */
