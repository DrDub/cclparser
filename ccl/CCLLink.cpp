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

#include <algorithm>
#include "CCLLink.h"
#include "yMath.h"
#include "yError.h"

using namespace std;

/////////////////////////
// Single match object //
/////////////////////////

void
CCCLMatch::PrintObj(CRefOStream* pOut, unsigned int Indent,
                       unsigned int SubIndent, eFormat Format,
                       int Parameter)
{
    if(!pOut) {
        yPError(ERR_MISSING, "output stream missing");
    }
    
    ((ostream&)(*pOut)) <<
        string(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');

    ((ostream&)(*pOut)) << "{" << m_Strg;

    for(list<CpCLabel>::iterator Iter = m_Labels.begin() ;
        Iter != m_Labels.end() ; Iter++) {
        string LabelStr;
        if(*Iter) {
            (*Iter)->LabelString(LabelStr);
            ((ostream&)(*pOut)) << " " << LabelStr;
        }
    }

    ((ostream&)(*pOut)) << "}";
}

///////////////////////////
// All best match object //
///////////////////////////

float
CCCLAllMatch::BestMatchStrg()
{
    if(m_BestAdjPos.m_Pos < 0)
        return 0;

    return m_Matches[m_BestAdjPos.m_Side].at(m_BestAdjPos.m_Pos).Strg();
};

bool
CCCLAllMatch::BestMatchClassMatch()
{
    if(m_BestAdjPos.m_Pos < 0)
        return true;

    return m_Matches[m_BestAdjPos.m_Side].at(m_BestAdjPos.m_Pos).ClassMatch();
}

CCCLStatCopy*
CCCLAllMatch::BestMatchStatCopy()
{
    if(m_BestAdjPos.m_Pos < 0)
        return NULL;

    return m_Matches[m_BestAdjPos.m_Side].at(m_BestAdjPos.m_Pos).StatCopy();
}

void
CCCLAllMatch::PrintObj(CRefOStream* pOut, unsigned int Indent,
                       unsigned int SubIndent, eFormat Format,
                       int Parameter)
{
    if(!pOut) {
        yPError(ERR_MISSING, "output stream missing");
    }
    
    ((ostream&)(*pOut)) <<
        string(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');

    // print all the matches and indicate the best match
    ((ostream&)(*pOut)) << " Best: ("
                        << ((m_BestAdjPos.m_Side == LEFT) ? "LEFT," : "RIGHT,")
                        << m_BestAdjPos.m_Pos << ")";

    for(unsigned int Side = LEFT ; Side <= RIGHT ; Side++) {
        if(m_Matches[Side].size()) {
            ((ostream&)(*pOut)) << ((Side == LEFT) ? " LEFT:" : " RIGHT:");
            unsigned int Pos = 0;
            for(vector<CCCLMatch>::iterator Iter = m_Matches[Side].begin() ;
                Iter != m_Matches[Side].end() ; Iter++) {
                ((ostream&)(*pOut)) << " (" << Pos << ") ";
                (*Iter).PrintObj(pOut, 0, 0, Format, Parameter);
                Pos++;
            }
        }
    }
}

/////////////////
// Link Object //
/////////////////

CCCLLink::CCCLLink(CCCLLexicon* pLexicon, CCCLSet* pSet,
                   unsigned int PrefixPos,
                   CCCLDepths const& PrefixDepths,
                   CCCLDepths const& LastDepths) :
        m_pLexicon(pLexicon), m_pSet(pSet)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    
    if(!m_pLexicon) {
        yPError(ERR_MISSING, "Lexicon pointer is missing");
    }
    
    if(!pSet || PrefixPos >= m_pSet->LastNode()) {
        yPError(ERR_MISSING, "no set pointer or position out of prefix");
    }

    m_pUnit[LEFT] = (CSCCLUnit*)(m_pSet->GetUnit(PrefixPos));
    m_pUnit[RIGHT] = (CSCCLUnit*)(m_pSet->GetLastUnit());

    m_AllowedDepths[LEFT] = PrefixDepths;
    m_AllowedDepths[RIGHT] = LastDepths;
    
    CalcLinks();
}

CCCLLink::~CCCLLink()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

///////////////////////////////////
// Best Matching Labels Routines //
///////////////////////////////////

// Given a statistics object (and side) and the labels of a unit, this routine
// calculates the best matching label between the two. Only labels which are
// stronger than 'Block' are considered for matching. The result is
// returned in 'BestMatches'. The routine returns the strength of the
// best match (0 if no match was found).

float
CCCLLink::CalcBestMatch(CCCLStat* pStats, unsigned int Side,
                        CCCLLabelTable* pLabels, CCCLMatch& BestMatches)
{
    if(!pLabels) {
        yPError(ERR_MISSING, "labels missing");
    }
    
    BestMatches.m_Labels.clear();
    BestMatches.m_Strg = 0;

    if(!pStats) // no match
        return 0;
    
    for(CCCLStatIter Iter(pStats, CCCLStat::eSeen) ; Iter ; ++Iter) {
        
        float StatStrg = Iter.QtV(CCCLStat::eLearn);
        
        if(StatStrg < BestMatches.m_Strg ||
           StatStrg <= pStats->QtVV(CCCLStat::eBlock, CCCLStat::eLearn))
            break; // cannot find a better match anymore

        float Match = min(StatStrg, pLabels->Strg(Iter.Data(), OP(Side)));

        if(Match == 0 || Match < BestMatches.m_Strg)
            continue; // better match already found

        if(Match == BestMatches.m_Strg)
            BestMatches.m_Labels.push_back(Iter.Data());
        else {
            BestMatches.m_Strg = Match;
            BestMatches.m_Labels.clear();
            BestMatches.m_Labels.push_back(Iter.Data());
        }
    }

    // Calculate the linking implied by the best matched label
    // (if there is a unique best matching label).
    
    if(BestMatches.m_Labels.size() == 1) {
        // exactly one best match
        BestMatches.m_bClassMatch =
            !(1 & (unsigned int)*(BestMatches.m_Labels.front()));

        string Name = (string const&)*(BestMatches.m_Labels.front());
    
        CpCCCLLexEntry pLEntry;
        m_pLexicon->GetEntryByString(Name, pLEntry);

        BestMatches.m_pStatCopy =
            new CCCLStatCopy(pLEntry->GetCCLStats()[BestMatches.m_bClassMatch ?
                                                    Side : OP(Side)]);
    } else {
        BestMatches.m_pStatCopy = NULL;
    }
    
    return BestMatches.m_Strg;
}

float
CCCLLink::BestMatch(bool bPrefix, SCCLAdjPos const& AdjPos)
{
    unsigned int USide = (bPrefix ? LEFT : RIGHT);
    
    // calculate only if not already calculated
    if(m_Matches[USide].m_Matches[AdjPos.m_Side].size() <= AdjPos.m_Pos ||
       m_Matches[USide].m_Matches[AdjPos.m_Side].at(AdjPos.m_Pos).m_Strg < 0) {
        
        // extend the match array, if necessary
        if(m_Matches[USide].m_Matches[AdjPos.m_Side].size() <= AdjPos.m_Pos)
            m_Matches[USide].m_Matches[AdjPos.m_Side].resize(AdjPos.m_Pos+1);
        
        // Calculate the match
        CalcBestMatch(m_pUnit[USide]->GetStats(AdjPos, false),
                      AdjPos.m_Side, m_pUnit[OP(USide)]->GetLabels(),
                      m_Matches[USide].
                      m_Matches[AdjPos.m_Side].at(AdjPos.m_Pos));
    }
    
    return m_Matches[USide].m_Matches[AdjPos.m_Side].at(AdjPos.m_Pos).m_Strg;
}

float
CCCLLink::BestMatch(bool bPrefix, SCCLAdjPos const& AdjPos,
                    CCCLMatch& Match)
{
    BestMatch(bPrefix, AdjPos);
    
    Match =
        m_Matches[bPrefix ? LEFT : RIGHT].
        m_Matches[AdjPos.m_Side].at(AdjPos.m_Pos);
    return Match.m_Strg;
}

// The following routine calculates all the best matches from the given
// word (LEFT/RIGHT) to the other. It updates the CCCLAllMatch object
// with the result and returns the strength of the best match.
//
// Currently, the match is performed as follows:
// 1. Only adjacency points on the side closer to the other word are used.
// 2. An unused position is considered a better match than
//    a used position.

float
CCCLLink::AllBestMatches(unsigned int USide)
{        
    unsigned int AdjSide = OP(USide);
    int BestPos = -1;
    bool bBestPosUsed = false;
    float Best = 0;
    
    for(unsigned int AdjPos = 0 ; ; AdjPos++) {
        
        if(!m_pUnit[USide]->GetStats(SCCLAdjPos(AdjSide, AdjPos), false))
            break;
        
        bool bUsed = m_pUnit[USide]->AdjUsed(SCCLAdjPos(AdjSide, AdjPos));
        
        if(bUsed && Best > 0 && !bBestPosUsed)
            continue; // already have an unused match.
        
        float Match = BestMatch(USide == LEFT, SCCLAdjPos(AdjSide, AdjPos));

        if(m_Matches[USide].m_Matches[AdjSide].at(AdjPos).StatCopy() &&
           (Match > Best || (Match > 0 && bBestPosUsed && !bUsed))) {
            BestPos = AdjPos;
            Best = Match;
            bBestPosUsed = bUsed;
        }

        if(!bUsed)
            break; // calculate up to the first unused position
    }

    // update the match results
    m_Matches[USide].m_BestAdjPos = SCCLAdjPos(AdjSide, BestPos);

    // Calculate the best match strength (even if there is no matching label)
    m_Matches[USide].m_BestStrg = 0;

    for(unsigned int AdjPos = 0 ;
        AdjPos < m_Matches[USide].m_Matches[AdjSide].size() ;
        AdjPos++) {
        if(m_Matches[USide].m_Matches[AdjSide].at(AdjPos).Strg() >
           m_Matches[USide].m_BestStrg)
            m_Matches[USide].m_BestStrg = 
                m_Matches[USide].m_Matches[AdjSide].at(AdjPos).Strg();
    }
    
    return Best;
}

void
CCCLLink::AllBestMatches()
{
    unsigned int FirstSide = (!m_AllowedDepths[LEFT].Allowed()) ? RIGHT : LEFT;

    float Best = AllBestMatches(FirstSide);

    if(m_AllowedDepths[OP(FirstSide)].Allowed() || Best == 0)
        AllBestMatches(OP(FirstSide));
}

// The following routine calculates the links between the two words.
// It first calculates the best matches between the words and then
// uses these to deduce the links.

void
CCCLLink::CalcLinks()
{
    // calculate the best matches
    AllBestMatches();
    m_Depth = 0;

    // Check which side has a stronger match and use the properties
    // of that match.
    unsigned int StrongSide =
        (m_Matches[LEFT].BestMatchStrg() >
         m_Matches[RIGHT].BestMatchStrg()) ? LEFT : RIGHT;
    
    // if there was no match, set both link strengths to zero
    if(!m_Matches[StrongSide].BestMatchStrg()) {
        m_Link[LEFT] = m_Link[RIGHT] = 0;
        return;
    }
    
    // determine the links based on the best matched label
    CpCCCLStatCopy pStatCopy = m_Matches[StrongSide].BestMatchStatCopy();
    unsigned int MatchSide = m_Matches[StrongSide].BestMatchClassMatch() ?
        StrongSide : OP(StrongSide);

    m_Link[MatchSide] =
        min(pStatCopy->QtVV(CCCLStat::eOut, CCCLStat::eLearn),
            m_Matches[StrongSide].BestMatchStrg());
    
    if(g_CCLBasicUseBothInValues &&
       pStatCopy->Val(CCCLStat::eIn, CCCLStat::eDerived) <= 0 &&
       Abs(pStatCopy->Val(CCCLStat::eIn, CCCLStat::eBase)) >=
       Abs(pStatCopy->Val(CCCLStat::eIn, CCCLStat::eDerived)))
        m_Link[OP(MatchSide)] =
            pStatCopy->QtVV(CCCLStat::eIn, CCCLStat::eBase,
                            CCCLStat::eLearn, CCCLStat::eBase);
    else
        m_Link[OP(MatchSide)] =
            pStatCopy->QtVV(CCCLStat::eIn, CCCLStat::eDerived,
                            CCCLStat::eLearn, CCCLStat::eBase);
    
    m_Link[OP(MatchSide)] = min(m_Link[OP(MatchSide)],
                                m_Matches[StrongSide].BestMatchStrg());

    // if there is no opposite link and the link 'In' base value is
    // negative while the 'In' derived value is positive then the
    // link is of depth 1.
    if(m_Link[MatchSide] <= 0 &&
       pStatCopy->Val(CCCLStat::eIn, CCCLStat::eBase) < 0 &&
       pStatCopy->Val(CCCLStat::eIn, CCCLStat::eDerived) > 0)
        m_Depth = 1;
    
    // links of strength 0 can be replaced by the actual match between
    // the words while links of negative strength can be replaced
    // by the actual match strength in the inbound direction of the
    // best matching label.
    if(m_Link[MatchSide] <= 0 && m_Link[OP(MatchSide)] <= 0) {

        m_Link[OP(MatchSide)] = m_Matches[OP(MatchSide)].BestMatchStrg();
        if(m_Link[MatchSide] == 0)
            m_Link[MatchSide] = m_Matches[MatchSide].BestStrg();
    }

    m_Link[LEFT] = max(m_Link[LEFT], (float)0);
    m_Link[RIGHT] = max(m_Link[RIGHT], (float)0);
    
    // Remove links which conflict with the allowed link depths
    if(!m_AllowedDepths[LEFT][m_Depth])
        m_Link[LEFT] = 0;
    if(!m_AllowedDepths[RIGHT][m_Depth])
        m_Link[RIGHT] = 0;
}

void
CCCLLink::PrintObj(CRefOStream* pOut, unsigned int Indent,
                   unsigned int SubIndent, eFormat Format,
                   int Parameter)
{
    if(!pOut) {
        yPError(ERR_MISSING, "output stream missing");
    }

    // Link ends
    
    ((ostream&)(*pOut)) <<
        string(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');

    ((ostream&)(*pOut)) << PRT_BUTTON_CLOSED << " Link: <"
                        << m_pUnit[LEFT]->GetPos() << ","
                        << m_pUnit[RIGHT]->GetPos() << ">";

    ((ostream&)(*pOut)) << PRT_REGION_START << endl;
    
    SubIndent++;
    
    // Left
    
    ((ostream&)(*pOut)) <<
        string(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');

    ((ostream&)(*pOut)) << "Left: " << m_Link[LEFT]
                        << (m_AllowedDepths[LEFT][0] ? " [0" : " [")
                        << (m_AllowedDepths[LEFT][1] ? "1]" : "]")
                        << "(" << m_Depth << ")";

    m_Matches[LEFT].PrintObj(pOut, 0, 0, Format, Parameter);
    ((ostream&)(*pOut)) << endl;
    
    // Right
    
    ((ostream&)(*pOut)) <<
        string(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');

    ((ostream&)(*pOut)) << "Right: " << m_Link[RIGHT]
                        << (m_AllowedDepths[RIGHT][0] ? " [0" : " [")
                        << (m_AllowedDepths[RIGHT][1] ? "1] " : "] ")
                        << "(" << m_Depth << ")";

    m_Matches[RIGHT].PrintObj(pOut, 0, 0, Format, Parameter);

    ((ostream&)(*pOut)) << PRT_REGION_END;
}
