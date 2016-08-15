#ifndef __CCLLINK_H__
#define __CCLLINK_H__

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

#include <list>
#include "CCLUnit.h"
#include "CCLLexicon.h"

class CCCLLink;

//
// A match structure describes the match from the adjacency point of one word
// to another word. One of the words is in the prefix while the other
// is the next word.
//

class CCCLMatch : public CPrintObj
{
    friend class CCCLLink;
    
private:
    // strength of the match (-1) if not yet calculated.
    float m_Strg;
    // Best matching labels (as they appear on the matching side).
    std::list<CpCLabel> m_Labels;

    // If there is exactly one matching label, the next two variables
    // describe the statistics for the match implied by the best match label.

    // Is the best match a class label on the matching side?
    bool m_bClassMatch;
    // A copy of the statistics of the matching label on the side
    // corresponding to this link.
    CpCCCLStatCopy m_pStatCopy;
    
public:
    
    CCCLMatch() : m_Strg(-1), m_bClassMatch(true) {}

    float Strg() { return m_Strg; }
    bool ClassMatch() { return m_bClassMatch; }
    CCCLStatCopy* StatCopy() { return m_pStatCopy; }
    
    CCCLMatch& operator=(CCCLMatch const& Match) {
        m_Strg = Match.m_Strg; m_Labels = Match.m_Labels;
        return (*this);
    }

    // printing function (for debugging)
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
};

//
// The 'all match' class holds the match objects of all adjacency points
// of a word. In addition, it also indicates which of these is the best
// match found.
//

class CCCLAllMatch : public CPrintObj
{
    friend class CCCLLink;

private:
    // vector holding matches for the different adjacency points
    // (side + adjacency position).
    std::vector<CCCLMatch> m_Matches[SIDE_NUM];
    // the best adjacency position matched ((unsigned int)-1 if no match)
    SCCLAdjPos m_BestAdjPos;
    // Best match strength (even if no best label could be determined)
    float m_BestStrg;
    
public:
    CCCLAllMatch() : m_BestAdjPos(LEFT, -1), m_BestStrg(0) {}

    // return the values of the best match
    float BestMatchStrg();
    SCCLAdjPos BestAdjPos() { return m_BestAdjPos; }
    bool BestMatchClassMatch();
    CCCLStatCopy* BestMatchStatCopy();
    float BestStrg() { return m_BestStrg; }
    
    // printing function (for debugging)
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
};

//
// A link object is used to calculate the possible linking between a word
// in the prefix and the next word.
//

class CCCLLink : public CRef, public CPrintObj
{
private:
    CpCCCLLexicon m_pLexicon; // the lexicon
    CpCCCLSet m_pSet;      // the CCL set on which calculations are made
    // the units between which the links are calculated. LEFT is the
    // prefix unit and RIGHT is the last unit.
    CpCSCCLUnit m_pUnit[SIDE_NUM];  

    // Requirements imposed by the set structure. For each unit
    // (LEFT - prefix unit, RIGHT - last unit) there is a depth vector
    // describing the allowed depths for the link in that direction.
    CCCLDepths m_AllowedDepths[SIDE_NUM];
    
    // Matching values
    
    // Matching values from the prefix word (LEFT) and from the
    // last word (RIGHT).
    CCCLAllMatch m_Matches[SIDE_NUM];
    
    // Calculated links

    // strength of links. LEFT: from prefix to last word. RIGHT: from
    // last word to prefix.
    float m_Link[SIDE_NUM];
    unsigned int m_Depth; // the depth of the links (must be equal)
    
public:
    // Constructor receives the prefix unit position and the pointer to the
    // CCL set. 'PrefixDepths' and 'LastDepths' describe the depths
    // allowed for a link from the prefix and from the last word
    // (respectively).
    CCCLLink(CCCLLexicon* pLexicon, CCCLSet* pSet, unsigned int PrefixPos,
             CCCLDepths const& PrefixDepths, CCCLDepths const& LastDepths);
    ~CCCLLink();

private:
    // function to actually perform the matching calculation

    // Given a statistics object (and side) and the labels of a unit,
    // this routine calculates the best matching label between the two.
    // Only labels which are stronger than 'Block' are considered for
    // matching. The result is returned in 'BestMatches'. The routine
    // returns the strength of the best match (0 if no match was found).
    float CalcBestMatch(CCCLStat* pStats, unsigned int Side,
                        CCCLLabelTable* pLabels, CCCLMatch& BestMatches);

public:

    // Functions to read the matching values.
    
    // Calculate the match between a given adjacency position on one word
    // unit and the other word. 'bPrefix' indicates whether the matching
    // word is the prefix word or the last word.
    // The return value is the strength of the best match.
    float BestMatch(bool bPrefix, SCCLAdjPos const& AdjPos);
    // Same as BestMatch() above only also returns the best match in
    // 'Match'.
    float BestMatch(bool bPrefix, SCCLAdjPos const& AdjPos,
                    CCCLMatch& Match);

private:
    // Calculate all matches between the given word (LEFT/RIGHT) and the
    // other word. The routine updates the appropriate CCCLAllMatch object.
    float AllBestMatches(unsigned int USide);
    // Calculate all best matches on both sides. If one of the link
    // directions is not incrementally addable, it is only calculated if
    // the opposite link direction resulted in no match.
    void AllBestMatches();

    // Calculate the links between the two words
    void CalcLinks();

public:
    
    //
    // Properties of the links calculated between the two words.
    //

    // Depth of the link (0 or 1)
    unsigned int LinkDepth() { return m_Depth; }
    // Strength of the link from the prefix word to the last word
    float Link(unsigned int BaseSide) { return m_Link[BaseSide]; }
    // The adjacency position used to make the best match on the
    // unit on the given side (LEFT - prefix unit, RIGHT - last unit).
    // The adjacency position is -1 if no match was found.
    SCCLAdjPos BestAdjPos(unsigned int UnitSide) {
        return m_Matches[UnitSide].BestAdjPos();
    }
    // returns true if the best adjacency position has already been used
    bool BestIsUsed(unsigned int UnitSide) {
        m_pUnit[UnitSide]->AdjUsed(BestAdjPos(UnitSide));
    }
    // return a pointer to the unit on the given side
    // (LEFT - prefix unit, RIGHT - last unit).
    CSCCLUnit* GetUnit(unsigned int UnitSide) { return m_pUnit[UnitSide]; }
    // get the position of the unit on the given side
    // (LEFT - prefix unit, RIGHT - last unit).
    unsigned int GetPos(unsigned int UnitSide) {
        return m_pUnit[UnitSide]->GetPos();
    }
    CCCLDepths& GetDepths(unsigned int UnitSide) {
        return m_AllowedDepths[UnitSide];
    }

    //
    // Printing routine (for debugging)
    //

    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
};

typedef CPtr<CCCLLink> CpCCCLLink;

#endif /* __CCLLINK_H__ */
