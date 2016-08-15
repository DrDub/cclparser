#ifndef __CCLBRACKETS_H__
#define __CCLBRACKETS_H__

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

#include <deque>
#include "CCLSet.h"
#include "PrintUtils.h"
#include "SynStruct.h"

class CCCLBracket;
typedef CPtr<CCCLBracket> CpCCCLBracket;
class CCCLBrackets;

// The classes in this file implement an extension of the CCL set which
// also calculates the bracketing implied by the links. 

//
// Single bracket object
//

class CCCLBracket : public CRef, public CPrintObj
{
    friend class CCCLBrackets;
private:
    // List of all units in the utterance
    CpCCCLUnitVector m_pUnits;
    
    // left and right end of the bracket (the right end is -1 if the bracket
    // covers the last node).
    int m_End[SIDE_NUM];
    // The leftmost node at which a link escapes this bracket to the left
    // (-1) if there is no such node.
    int m_Escapes;
    // List of brackets which are directly dominated by this bracket
    // (sorted from left to right).
    std::deque<CpCCCLBracket> m_Dominated; 
public:
    // Every bracket created covers the last node when it is created.
    // Therefore, only the left end is given.
    CCCLBracket(CCCLUnitVector* pUnits, unsigned int LeftEnd);
    ~CCCLBracket();

    int GetEnd(unsigned int Side) { return m_End[Side]; }

    // Adds the bracket to the given syntactic structure. Returns the
    // node number assigned to this bracket.
    // Because brackets which cover the last node do not specify
    // its position (but only -1) the position of the last node must
    // also be given to the function.
    int AddToSynStruct(CSynStruct* pSynStruct, unsigned int LastNode);
    
    // Print function
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
};

class CCCLBrackets : public CCCLSet, public CPrintObj
{
private:
    // Vector of brackets which cover the last node. The vectors are
    // ordered from largest to smallest.
    std::vector<CpCCCLBracket> m_CoverLast;
    // List of maximal brackets which do not cover the last node
    // (sorted from left to right).
    std::vector<CpCCCLBracket> m_MaxNotEnd;
    // For each node in the utterance, this vector holds a pointer to the
    // minimal bracket covering it.
    std::vector<CpCCCLBracket> m_B1x;
    // For each node which generates a bracket of height 2 above the node,
    // this vector holds a pointer to that bracket (otherwise, a NULL pointer).
    std::vector<CpCCCLBracket> m_B2x;
    // Indicates the last node for which the bracketing was calculated
    // (the bracketing should not be updated twice for the same last node).
    // This is set to -1 when the CCL set has no nodes.
    int m_BracketingUpTo;
public:
    // Initialization is with no brackets
    CCCLBrackets(CTracing* pTracing);
    ~CCCLBrackets();

    // Clear the brackets and the CCL set
    void Clear();
    
private:

    // Create a vector holding the maximal brackets in the bracketing
    // (ordered from left to right)
    CRvector<CpCCCLBracket>* GetMaxBrackets();
    
    // Update the bracketing based on the links of the last node
    void UpdateBrackets();

public:
    // the bracketing is updated when the next word is read.
    bool IncNodeNum(CCCLUnit* pNextUnit);
    // the following function should be called when the processing of the
    // current last word is completed but no next word is read
    // (for example, when punctuation is read or the end of the utterance
    // is reached). This updates the brackets for the CCL set.
    // Calling this function blocks any additional links being added
    // before the next word is read. Therefore, it returns false if
    // processing for the last word is not yet complete (resolution violation).
    bool CalcBrackets();

    // Create the CSynStruct object describing this bracketing
    CSynStruct* GetSynStruct();
    
    // Print function
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
};

typedef CPtr<CCCLBrackets> CpCCCLBrackets;

#endif /* __CCLBRACKETS_H__ */
