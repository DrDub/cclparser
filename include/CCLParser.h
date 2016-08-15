#ifndef __CCLPARSER_H__
#define __CCLPARSER_H__

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
#include "Parser.h"
#include "CCLBrackets.h"
#include "CCLLexicon.h"
#include "CCLUnit.h"
#include "CCLLink.h"
#include "CCLLearn.h"

class CCCLParser : public CParser
{
private:
    // Lexicon
    CpCCCLLexicon m_pLexicon;
    // underlying (bracketed) CCL set
    CpCCCLBrackets m_pCCLBrackets;
    // Queue of learing events
    CCCLLearnQueue m_LearnQueue;
    
public:
    CCCLParser(CCCLLexicon* pLexicon);
    ~CCCLParser();
    
private:
    
    //
    // Standard parser interface
    //

    // Create a unit based on the name and labels given
    CUnit* CreateUnit(std::string const& Name,
                      std::vector<std::string>& Labels);
    // clear the utterance
    void ClearDerivedParser();
    
private:

    //
    // Access to units
    //

    // return the unit at the given position (NULL if no such unit exists)
    CSCCLUnit* GetUnit(unsigned int Pos) {
        return (CSCCLUnit*)(m_pCCLBrackets->GetUnit(Pos));
    }
    
    //
    // Processing
    //
    
    // Process the input currently available. In principle, this can be
    // called at any point in time and should be called at least once after
    // the utterance had been fully read.
    void Process();

    void ProcessUnit(CSCCLUnit* pUnit);
    void ProcessPunct(CPunct* pPunct);
    void Terminate();

    // Parsing

    // Main parsing routine. This routine performs all parsing operations
    // needed when the new next word is read from the input.
    void Parse();
    // Given a link object, this routine adds the specified link to the
    // CCL set under the given depth restrictions. It returns false if the
    // corresponding link is not matched by the link structure or if it is not
    // allowed in the set. In this case, the link is not added.
    bool AddLink(CCCLLink& Link, unsigned int BaseSide, CCCLDepths& Depths);
    // This function updates the addable links currently available
    // in the CCL set from the prefix to the last word onto 'PrefixLinks'.
    // If 'Links' is not empty when the routine is called, only those
    // links which have changed are updated. The routine returns true if any
    // links have been added or modified.
    // If 'bNotDirectAdj' is true, a link from the directly adjacent word
    // (one before last) is not included in the list.
    bool UpdatePrefixAddableLinks(std::list<CpCCCLLink>& Links,
                                  bool bNotDirectAdj);
    // This function updates the addable link currently available
    // in the CCL set from the last word to the prefix onto 'LastLink'.
    // If 'LastLink' is not empty when the routine is called, the
    // link is only updated if it has changed.
    // The routine returns true if the link has been added or modified.
    // If 'bNotDirectAdj' is true, a link to the directly adjacent word
    // (one before last) is not allowed (NULL returned).
    bool UpdateLastAddableLink(CpCCCLLink& LastLink, bool bNotDirectAdj);

    //
    // Learning
    //

    // This routine adds learning events for the left side of the last word
    // and for the right side of all those words which have an adjacency
    // at the last word (after all links to the last word have been created).
    void Learn();
    // Create learning events for units which have a right adjacency
    // at the given position (which may be just beyond the end of the
    // utterance). Should only be used with the last unit position or
    // the position beyond it when the unit is terminated.
    void LearnRight(unsigned int AdjUnit);
    
    //
    // Output functions
    //

    // The syntactic structure
    CSynStruct* GetSynStruct();

    // The lexicon
    CLexicon* GetLexicon() { return m_pLexicon; }
    
    //
    // Print function
    //
    
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
};

typedef CPtr<CCCLParser> CpCCCLParser;

#endif /* __CCLPARSER_H__ */
