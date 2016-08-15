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

#include <iomanip>
#include <list>
#include "StringUtil.h"
#include "CCLParser.h"
#include "CCLUnit.h"
#include "CCLLink.h"
#include "yError.h"

using namespace std;

CCCLParser::CCCLParser(CCCLLexicon* pLexicon) :
        m_pLexicon(pLexicon)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    
    if(!m_pLexicon) // if no external lexicon is given, create one (empty)
        m_pLexicon = new CCCLLexicon();

    m_pCCLBrackets = new CCCLBrackets(m_pTracing);
}

CCCLParser::~CCCLParser()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

CUnit*
CCCLParser::CreateUnit(string const& Name, vector<string>& Labels)
{
    // convert to lower-case
    string LCName = Name;
    ToLower(LCName);

    // Get the name from the lexicon

    CpCCCLLexEntry pLEntry;
    CpCStrKey pName = m_pLexicon->GetEntryByString(LCName, pLEntry);

    if(m_bLearnCycle)
        pLEntry->IncCount();
    
    // Create the list of labels
    vector<CpCStrKey> UnitLabels;

    // The name of the unit itself is always a label of itself
    UnitLabels.push_back(pName);

    for(vector<string>::iterator Iter = Labels.begin() ;
        Iter != Labels.end() ; Iter++) {
        // convert to lower case
        string LCLabel = *Iter;
        ToLower(LCLabel);

        if(LCLabel == "" || LCLabel == LCName)
            continue;

        // To conserve memory, we store also the labels in the lexicon
        // (though they are not necessarily lexical items)
        CpCStrKey pLabel = m_pLexicon->GetKeyByString(LCLabel);
        UnitLabels.push_back(pLabel);
    }

    // Create a unit and return it

    return new CSCCLUnit(pName, UnitLabels, pLEntry->GetCCLStats());
}

void
CCCLParser::ClearDerivedParser()
{
    if(m_pCCLBrackets)
        m_pCCLBrackets->Clear();
}

//////////////////////////////
// Main Processing Routines //
//////////////////////////////

void
CCCLParser::Process()
{
    while(m_pInput && !m_pInput->empty()) {

        CpCSymbol pSymbol = m_pInput->front();
        m_pInput->pop_front();

        if(!pSymbol) {
            yPError(ERR_SHOULDNT, "Symbol pointer missing");
        }
        
        // Check whether the input is a punctuation mark or a unit
        // and call the appropriate processing routine

        switch(pSymbol->SymbolType()) {
            case ePunct:
                ProcessPunct((CPunct*)pSymbol.Ptr());
                break;
            case eUnit:
                ProcessUnit((CSCCLUnit*)pSymbol.Ptr());
                break;
            default:
                yPError(ERR_OUT_OF_RANGE, "Unknown symbol type");
        }
    }
}

void
CCCLParser::ProcessUnit(CSCCLUnit* pUnit)
{
    if(!pUnit) {
        yPError(ERR_MISSING, "Unit pointer missing");
    }

    // Set stopping punctuation on the left side of the unit.
    if(CpCCCLUnit pPrevUnit = m_pCCLBrackets->GetLastUnit()) {
        pUnit->SetStopPunct(LEFT, pPrevUnit->GetStopPunct(RIGHT));
    }
    
    // Add the new unit at the end of the utterance
    if(!m_pCCLBrackets->IncNodeNum(pUnit)) {
        m_pCCLBrackets->TraceSet();
        yPError(ERR_SHOULDNT, "attempting to add unit when violation exists");
    }

    // parsing can now take place
    Parse();
}

void
CCCLParser::ProcessPunct(CPunct* pPunct)
{
     if(!pPunct) {
         yPError(ERR_MISSING, "Punctuation pointer missing")
     }

     // If this is stopping punctuation, store it on the preceding unit
     // (if there is no such unit, the punctuation may be discarded)
     if(g_UseStoppingPunct && IsStoppingPunct(pPunct->Type())) {
         CpCCCLUnit pUnit = m_pCCLBrackets->GetLastUnit();

         if(pUnit)
             pUnit->SetStopPunct(RIGHT, pPunct->Type());
     }

     // update the bracketing
     m_pCCLBrackets->CalcBrackets();
     
     // If this is the end of the utterance, terminate this utterance
     if(IsEoUPunct(pPunct->Type())) {
         Terminate();
     }
}

void
CCCLParser::Terminate()
{
    // calculate the bracketing (this should have already been calculated,
    // but calling this function twice should do no harm).
    m_pCCLBrackets->CalcBrackets();
    if(m_bLearnCycle) {
        // add learning events for all units with adjacency at the end of
        // the utterance
        LearnRight(m_pCCLBrackets->LastNode()+1);
        m_LearnQueue.Realize();
    } else
        m_LearnQueue.Clear();
}

//////////////////////
// Parsing routines //
//////////////////////

// Given a list of links (from the prefix to the last word) and a link
// from the last word to the prefix, this routine finds
// the link with the strongest match. It returns a pointer to the
// strongest match or NULL if no match is found. If there are several
// links with the same maximal strength, the shortest of them is returned
// and if there are two of the same length, the one from the prefix to
// the last word is returned.

static CCCLLink*
GetStrongestMatch(list<CpCCCLLink>& Links, CCCLLink* pLastLink)
{
    bool bBestIsUsed = true;
    float BestPrefix = 0;
    CpCCCLLink pBest;
        
    for(list<CpCCCLLink>::iterator Iter = Links.begin() ;
        Iter != Links.end() ; Iter++) {
        if((*Iter)->Link(LEFT) > BestPrefix ||
           (bBestIsUsed && (*Iter)->Link(LEFT) > 0 &&
            !(*Iter)->BestIsUsed(LEFT))) {
            BestPrefix = (*Iter)->Link(LEFT);
            pBest = *Iter;
            if(bBestIsUsed)
                bBestIsUsed = (*Iter)->BestIsUsed(LEFT);
        }
    }

    if(pLastLink && pLastLink->Link(RIGHT) > 0 &&
       ((!pLastLink->BestIsUsed(RIGHT) && bBestIsUsed) ||
        (pLastLink->BestIsUsed(RIGHT) == bBestIsUsed &&
         pLastLink->Link(RIGHT) >= BestPrefix &&
         (pLastLink->Link(RIGHT) > BestPrefix ||
          pLastLink->GetPos(LEFT) > pBest->GetPos(LEFT)))))
        pBest = pLastLink;
    
    return pBest;
}

// Same as above, but based on the potential rather than the actual match.
// The potential match is calculated based on the number of labels
// stronger than 'Block' (of the first adjacency point). If two words
// have the same number of such labels, the decision is made based on the
// strength of the strongest label.
// If this still does not determine the link uniquely, the shortest link
// is taken and among the shortest links (of equal strength) the one
// from the prefix is preferred.

static CCCLLink*
GetStrongestPotentialMatch(list<CpCCCLLink>& Links, CCCLLink* pLastLink)
{
    unsigned int BestStrong = 0;
    float BestMax = 0;
    CpCCCLLink pBest;
    SCCLAdjPos AdjPos(RIGHT,0);
        
    for(list<CpCCCLLink>::iterator Iter = Links.begin() ;
        Iter != Links.end() ; Iter++) {
        CpCCCLStatCopy pStatCopy = (*Iter)->GetUnit(LEFT)->GetStatCopy(AdjPos);

        if(!pBest ||
            (pStatCopy->StrongerThanBlock(CCCLStat::eSeen) >= BestStrong &&
              (pStatCopy->StrongerThanBlock(CCCLStat::eSeen) > BestStrong ||
               pStatCopy->Strongest(CCCLStat::eSeen) > BestMax))) {
                BestStrong = pStatCopy->StrongerThanBlock(CCCLStat::eSeen);
                BestMax = pStatCopy->Strongest(CCCLStat::eSeen);
                pBest = *Iter;
        }
    }

    if(pLastLink) {
        CpCCCLStatCopy pStatCopy =
            pLastLink->GetUnit(RIGHT)->GetStatCopy(SCCLAdjPos(LEFT,0));

        if(!pBest ||
           (pStatCopy->StrongerThanBlock(CCCLStat::eSeen) >= BestStrong &&
            (pStatCopy->StrongerThanBlock(CCCLStat::eSeen) > BestStrong ||
             pStatCopy->Strongest(CCCLStat::eSeen) > BestMax ||
             (pStatCopy->Strongest(CCCLStat::eSeen) == BestMax &&
              pLastLink->GetPos(LEFT) > pBest->GetPos(LEFT)))))
            pBest = pLastLink;
    }
    
    return pBest;
}

// The following routine performs all the parsing operations needed when
// the new next word is read from the input.
// The routine first constructs a link object for the two links between
// the last word and the word immediately preceeding it.
// After the links for these two words are added, the routine continues
// to add links as long as there is an addable link which has a match.
// It also continues to ad links as long as there is a resolution
// violation.
//
// After the links between the last word and the preceeding word are
// calculated, the following situations are possible:
//
// If only a link from left to right has been added then there
// are no more addable links.
//
// If no links were added, compare all possible addable links
// (all are from left to right) and select the best matching of these
// (if there is any match). At most one link can be added in this case.
//
// If both left to right and right to left links have been added,
// it may still be possible to add links from right to left
// (but only of depth 1). Continue to add right to left links until
// there is no match anymore.
//
// If only a right to left link was added, links can still be added
// in both directions and there may also be a resolution violation.
// Continue to search for the strongest addable link and add it.
// Stop when there are no matches anymore.

void
CCCLParser::Parse()
{
    int Last = m_pCCLBrackets->LastNode();

    if(m_bParseCycle) {
        int a = 0; // for setting a breakpoint
    }
    
    if(Last <= 0) {
        Learn();
        return; // no parsing to perform
    }

    if(m_pCCLBrackets->GetLastUnit()->GetStopPunct(LEFT) != eNoPunct) {
        Learn();
        return;
    }
    
    // first calculate the links connecting the last two nodes.
    CCCLDepths Depths(0);
    
    CCCLLink Link(m_pLexicon, m_pCCLBrackets, Last-1, Depths, Depths);
    
    // debugging
    (*m_pTracing)(TB_PARSER) << (CPrintObj&)Link << Endl;
    
    if(Link.Link(LEFT))
        AddLink(Link, LEFT, Depths);
    if(Link.Link(RIGHT))
        AddLink(Link, RIGHT, Depths);

    // Next, compare all addable links and add the strongest of them.
    // Continue doing so until there are no more matches. If a resolution
    // violation still exists, an addable link must still
    // be added, so the matching criterion is relaxed and attachment is
    // calculated based on the attachment potential of each word.

    list<CpCCCLLink> Links; // links calculated based at the prefix
    CpCCCLLink pLastLink;   // the link from the last word to the prefix
    
    while(1) {
        // calculate the matches for the addable links
        UpdatePrefixAddableLinks(Links, true);
        UpdateLastAddableLink(pLastLink, true);

        // is there a best match?
        CpCCCLLink pBestLink = GetStrongestMatch(Links, pLastLink);

        // debug printing
        if(m_pTracing->IsTraceOn(TB_PARSER) && (Links.size() || pLastLink)) {
            (*m_pTracing)(TB_PARSER) << PRT_BUTTON_CLOSED
                                     << "Secondary links: " << Endl
                                     << PRT_REGION_START;
            if(pLastLink)
                (*m_pTracing)(TB_PARSER) << Indent
                                         << (CPrintObj*)pLastLink << Endl;
            for(list<CpCCCLLink>::iterator Iter = Links.begin() ;
                Iter != Links.end() ; Iter++)
                (*m_pTracing)(TB_PARSER) << Indent
                                         << (CPrintObj*)((*Iter).Ptr())
                                         << Endl;
            (*m_pTracing)(TB_PARSER) << PRT_REGION_END;
        }
        
        // if there is no best match but there is a resolution violation,
        // must select the best link.
        if(!pBestLink && m_pCCLBrackets->HasRV())
            pBestLink = GetStrongestPotentialMatch(Links, pLastLink);
        
        unsigned int Side = (pBestLink == pLastLink) ? RIGHT : LEFT;
        
        if(pBestLink) {
            // add the link
            if(!AddLink(*pBestLink, Side, pBestLink->GetDepths(Side))) {
                yPError(ERR_SHOULDNT, "cannot add the link");
            }
            if(Side == LEFT)
                // empty the link list
                Links.empty();
        } else
            break; // no more links to add
    }

    Learn();
}

bool
CCCLParser::UpdatePrefixAddableLinks(list<CpCCCLLink>& Links,
                                     bool bNoDirectAdj)
{
    bool bNew = false; // were there any new links added/modified
    
    // find the incrementally addable links and add them to a list.
    list<CpCCCLLink>::iterator LIter = Links.begin();
    for(CCCLPrefixIncAddableIter Iter(m_pCCLBrackets) ;
        !Iter.End() ; ++Iter) {
        
        if(bNoDirectAdj && Iter.Base() == m_pCCLBrackets->LastNode() - 1)
            continue;
        
        // erase any links which appear before the link which should
        // currently be added.
        while(LIter != Links.end() && (*LIter)->GetPos(LEFT) > Iter.Base())
            LIter = Links.erase(LIter);
        
        if(LIter == Links.end() || (*LIter)->GetPos(LEFT) != Iter.Base()
           || !((*LIter)->GetDepths(LEFT) == Iter.Depths())) {
            Links.insert(LIter, new CCCLLink(m_pLexicon, m_pCCLBrackets,
                                             Iter.Base(), Iter.Depths(),
                                             CCCLDepths(-1)));
            bNew = true;
        } else {
            // Otherwise, the link has already been calculated
            ++LIter;
        }
    }
    // remove all links which were not used at the end of the list
    while(LIter != Links.end())
        LIter = Links.erase(LIter);
    
    return bNew;
}

bool
CCCLParser::UpdateLastAddableLink(CpCCCLLink& LastLink, bool bNotDirectAdj)
{
    bool bNew = false; // were there any new links added/modified
    
    // calculate the addable link from the last word to the prefix.
    CCCLDepths LastDepths;
    int LastAddable = m_pCCLBrackets->LastAddableLink(LastDepths);
    
    if(!LastDepths.Allowed() || LastAddable < 0 ||
       (bNotDirectAdj && LastAddable == m_pCCLBrackets->LastNode() - 1))
        LastLink = NULL;
    else if(!LastLink || LastAddable != LastLink->GetPos(LEFT) ||
            !(LastLink->GetDepths(RIGHT) == LastDepths)) {
        LastLink = new CCCLLink(m_pLexicon, m_pCCLBrackets, LastAddable,
                                CCCLDepths(-1), LastDepths);
        bNew = true;
    }
    
    // otherwise, the link has already been calculated

    return bNew;
}

bool
CCCLParser::AddLink(CCCLLink& Link, unsigned int BaseSide, CCCLDepths& Depths)
{
    // Was there a match?
    bool bMatched = Link.Link(BaseSide);

    // detemine the depth
    
    unsigned int Depth;
    // check that the depth of the link is allowed
    if(bMatched) {
        if(!Depths[Link.LinkDepth()])
            return false;
        Depth = Link.LinkDepth();
    } else {
        // if there was no match, take the smallest allowed depth
        Depth = Depths[0] ? 0 : 1;
    }
    
    // Add the link
    if(!m_pCCLBrackets->AddLink(Link.GetUnit(BaseSide)->GetPos(),
                                Link.GetUnit(OP(BaseSide))->GetPos(),
                                Depth)) {
        yPError(ERR_SHOULDNT, "failed to add the link");
        return false;
    }

    if(bMatched)
        // If there is a match on the base side of the link, mark that
        // adjacency position as used.
        Link.GetUnit(BaseSide)->SetAdjUsed(Link.BestAdjPos(BaseSide));

    return true;
}

///////////////////////
// Learning Routines //
///////////////////////

void
CCCLParser::Learn()
{
    if(!m_bLearnCycle)
        return; // no learning

    SCCLAdjPos AdjPos(LEFT, 0);
    int LastNode = m_pCCLBrackets->LastNode();
    CpCSCCLUnit pUnit = (CSCCLUnit*)(m_pCCLBrackets->GetLastUnit());
    
    if(!pUnit)
        return;

    // Add learning events for the left side of the last word
    
    if(pUnit->GetPos() == 0 || pUnit->GetStopPunct(LEFT) != eNoPunct) {
        // Add a 'block' learning event
        m_LearnQueue.Push(new CCCLLearn(m_pCCLBrackets, pUnit->GetPos(),
                                        AdjPos, -1));
    } else {
        bool bAdjPosCanLearn = true;
        // First loop over all outbound links and add a learning
        // event for each one.
        for(CCCLOutboundIter Iter(m_pCCLBrackets, pUnit->GetPos(), LEFT) ;
            Iter ; ++Iter) {
            m_LearnQueue.Push(new CCCLLearn(m_pCCLBrackets,
                                            pUnit->GetPos(),
                                            AdjPos, ((SLinkPair)Iter).m_End));
            if(pUnit->AdjUsed(AdjPos))
                AdjPos.m_Pos++;
            else { // cannot learn anymore on this side
                bAdjPosCanLearn = false;
                break;
            }
        }
        // if the adjacency position can still be learned, learn the
        // unused adjacency
        SLinkPair UnusedAdj(m_pCCLBrackets->
                            UnusedAdj(LastNode, LEFT));
        if(bAdjPosCanLearn) {
            if(UnusedAdj.m_Depth <= 1)
                m_LearnQueue.Push(new CCCLLearn(m_pCCLBrackets,
                                                pUnit->GetPos(),
                                                AdjPos, UnusedAdj.m_End));
            else
                m_LearnQueue.Push(new CCCLLearn(m_pCCLBrackets,
                                                pUnit->GetPos(),
                                                AdjPos, -1));
        }
    }

    LearnRight(LastNode);
}

// Create learning events for units which have a right adjacency
// at the given position (which may be just beyond the end of the utterance).

void
CCCLParser::LearnRight(unsigned int AdjUnit)
{
    // Loop from the word before 'AdjUnit' backwards along inbound links
    // until a word attached to 'AdjUnit' is found. Add learning
    // events for these words.
    int Pos = AdjUnit - 1;
    int AdjToLearn =
        (AdjUnit <= m_pCCLBrackets->LastNode()) ? (int)AdjUnit : -1;
    SCCLAdjPos AdjPos(RIGHT, 0);
    
    while(Pos >= 0) {

        bool bLinkToAdjUnit =
            (m_pCCLBrackets->GetLastOutbound(Pos, RIGHT).m_End == AdjUnit);
        
        CpCSCCLUnit pUnit = (CSCCLUnit*)(m_pCCLBrackets->GetUnit(Pos));
        AdjPos.m_Pos = m_pCCLBrackets->GetOutboundNum(Pos, RIGHT);
        if(bLinkToAdjUnit)
            AdjPos.m_Pos--;
        
        // check whether the unit still has adjacency position to be
        // learned.
        if(AdjPos.m_Pos == 0 ||
           pUnit->AdjUsed(SCCLAdjPos(AdjPos.m_Side, AdjPos.m_Pos - 1))) {
            // if this is direct adjacency and there is stopping punctuation,
            // create a 'block' learning event.
            if(Pos == AdjToLearn - 1 &&
               m_pCCLBrackets->GetUnit(AdjToLearn)->GetStopPunct(LEFT) !=
               eNoPunct)
                m_LearnQueue.Push(new CCCLLearn(m_pCCLBrackets, Pos,
                                                AdjPos, -1));
            else
                m_LearnQueue.Push(new CCCLLearn(m_pCCLBrackets, Pos,
                                                AdjPos, AdjToLearn));
        }
        
        if(bLinkToAdjUnit)
            break;
        
        int NextPos = m_pCCLBrackets->GetInbound(Pos, LEFT).m_End;
        if(NextPos == Pos)
            break;

        Pos = NextPos;
    }
}

/////////////////////
// Output Routines //
/////////////////////

CSynStruct*
CCCLParser::GetSynStruct()
{
    return m_pCCLBrackets ? m_pCCLBrackets->GetSynStruct() : NULL;
}

///////////////////////
// Printing Routines //
///////////////////////

void
CCCLParser::PrintObj(CRefOStream* pOut, unsigned int Indent,
                     unsigned int SubIndent, eFormat Format,
                     int Parameter)
{
    if(m_pCCLBrackets)
        m_pCCLBrackets->PrintObj(pOut, Indent, SubIndent, Format, Parameter);
}
