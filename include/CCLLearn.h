#ifndef __CCLLEARN_H__
#define __CCLLEARN_H__

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

#include <queue>
#include "CCLUnit.h"

//
// Learning event
//

// A learn object is created for every adjacency position where learning
// should take place. The learning object is then put on a queue. When
// the queue is emptied, all learning events stored on the queue are
// realized (that is, are updated in the lexicon). This (possibly) late
// update is used so that statistics used in parsing remain stable
// throughout the calculation.

class CCCLLearn : public CRef
{
private:
    // The learning unit (on which the statistics are updated)
    CpCSCCLUnit m_pUnit;
    // The adjacency position which is learned (side + position)
    SCCLAdjPos m_AdjPos;
    // the unit which is at the adjacency position. This may be NULL
    // if the adjacency position is beyond the end of the utterance.
    CpCSCCLUnit m_pAdjUnit;

public:

    // The constructor receives the position of the learning unit,
    // the adjacency position at which it learns and the position of
    // the unit at the adjacency position. The position of this unit may
    // be outside the utterance (for example, -1).
    CCCLLearn(CCCLSet* pSet, unsigned int LearnPos, SCCLAdjPos const& AdjPos,
              int AdjUnit);
    ~CCCLLearn();

    // Perform the learning
    void Learn();
};

typedef CPtr<CCCLLearn> CpCCCLLearn;

//
// Learning queue
//

class CCCLLearnQueue : public CRef
{
private:
    std::queue<CpCCCLLearn> m_Queue; // the queue

public:
    // empty queue constructor
    CCCLLearnQueue();
    ~CCCLLearnQueue();

    // push a learning event on the queue
    void Push(CCCLLearn* pLearn);
    // realize all learning events on the queue
    void Realize();
    // clear the queue (without realizing the events stored in it)
    void Clear();
};

typedef CPtr<CCCLLearnQueue> CpCCCLLearnQueue;


#endif /* __CCLLEARN_H__ */
