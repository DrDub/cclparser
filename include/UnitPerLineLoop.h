#ifndef __UNITPERLINELOOP_H__
#define __UNITPERLINELOOP_H__

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


class CUnitPerLineLoop : public CPlainTextLoop
{
private:
    // number of objects terminated in the last line passed to NextLine().
    // Here, this is 1 if the line was empty and 0 otherwise. This is also
    // 0 if there were no non-empty lines read before.
    unsigned int m_IncObjNum;
    // Set to 'true' when a non-empty line is read and set to 'false'
    // when an utterance is terminated
    bool m_bNonEmptyObj;
public:
    CUnitPerLineLoop(std::vector<std::string> const & InFilePatterns,
                     CCmdArgOpts* pArgs,
                     CpCMessageLine& MsgLine, COutFile* pOutFile);
    
    ~CUnitPerLineLoop();

private:
    // Carry out the operations required when the end of the object
    // (utterance) is reached.
    void EndObject();

public:
    bool NextLine(std::string const& Line);
    bool EndLoop();
    // line (sentence, actually) is terminated by an empty input line
    unsigned int IncObjNum() { return m_IncObjNum; }
};

typedef CPtr<CUnitPerLineLoop> CpCUnitPerLineLoop;

#endif /* __UNITPERLINELOOP_H__ */
