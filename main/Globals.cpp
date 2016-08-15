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

#include "Globals.h"
#include "PrintUtils.h"
#include "yError.h"

using namespace std;

//
// Table sizes
//

// maximal number of pre-candidate in a statistic table
unsigned int g_StatisticsTopListMaxLen = 10;
// maximal number of labels (on each side of a labeled object)
unsigned int g_MaxLabels = 10;

//
// Input reading
//

// if the input contains tags (e.g. POS) use these instead of the words
unsigned int g_UseTagsAsWords = 0;
// if the input contains tags (e.g. POS) use these in addition to the
// words to label them.
unsigned int g_UseTagsAsLabels = 0;
// Should currency symbols (e.g. $) be considered punctuation?
unsigned int g_CurrencySymbolIsPunct = 1;
// Should stopping punctuation be used or ignored?
unsigned int g_UseStoppingPunct = 1;
// For backward compatibility with older experiments, terminating
// punctuation marks (such as fullstops, question marks and exclamation
// marks) can be discarded. Except for backward compatibility, these
// should not be discarded and the property should be set to 0
// (which is, of course, the default).
unsigned int g_DiscardTerminatingPunct = 0;
// When reading from Penn treebank style input, reverse the input so that
// the utterance is read from right to left.
unsigned int g_ReversePennObjs = 0;

//
// Parser Specification
//

// Parser which should be used (CCL is the current default)
std::string g_ParserType("CCL");

//
// Evaluation
//

// When precision and recall is calculated, should the top bracket be
// included in the calculation?
unsigned int g_CountTopBracket = 1;

// specific evaluators

// When using the grouped precision and recall evaluator, which function(s)
// should be used for sorting (names separated by white space)
std::string g_EvalGroupedOutputSorting("Expected");
// When using the grouped recall or precision evaluator, how many entries
// should be printed (use 0 for all entries)
unsigned int g_GroupedEvalMaxPrint = 100;

//
// Output printing
//

// Comment line indicator. Typically, but not necessarily, this is a single
// character. Default character is '#'.
std::string g_CommentStr("#");
// minimal count for an entry in the lexicon to be printed
unsigned int g_LexMinPrint = 5;
// Printing mode (see PrintUtils.h and PrintUtils.cpp for possible values).
// These values should appear in a list, separated by whitespace.
std::string g_PrintingMode("");
// Parse trace bits (indicate what type of tracing to perform)
unsigned int g_TraceBits = 0;

//
// Specific parsers
//

// CCL (Common Cover Links)

// Basic parsing function family

// If 'true' the link weight function uses both the eBase and eDerived
// values of the Inbound link property to determine the weight of a link.
// If this is 'false' only the eDerived value is used.
unsigned int g_CCLBasicUseBothInValues = 1; // default is to use both

CGlobals::CGlobals(vector<string> const& List)
{
    InitGlobals(List);
}

CGlobals::CGlobals()
{
    InitGlobals();
}

bool
CGlobals::InitGlobals(vector<string> const& List)
{
    // ----------------- Add all globals here ---------------------------
    AddArg("StatisticsTopListMaxLen", &g_StatisticsTopListMaxLen);
    AddArg("MaxLabels", &g_MaxLabels);
    AddArg("UseTagsAsWords", &g_UseTagsAsWords);
    AddArg("UseTagsAsLabels", &g_UseTagsAsLabels);
    AddArg("CurrencySymbolIsPunct", &g_CurrencySymbolIsPunct);
    AddArg("UseStoppingPunct", &g_UseStoppingPunct);
    AddArg("DiscardTerminatingPunct", &g_DiscardTerminatingPunct);
    AddArg("ReversePennObjs", &g_ReversePennObjs);
    AddArg("ParserType", &g_ParserType);
    AddArg("CountTopBracket", &g_CountTopBracket);
    AddArg("EvalGroupedOutputSorting", &g_EvalGroupedOutputSorting);
    AddArg("GroupedEvalMaxPrint", &g_GroupedEvalMaxPrint);
    AddArg("CommentStr", &g_CommentStr);
    AddArg("LexMinPrint", &g_LexMinPrint);
    AddArg("PrintingMode", &g_PrintingMode);
    AddArg("TraceBits", &g_TraceBits);
    AddArg("CCLBasicUseBothInValues", &g_CCLBasicUseBothInValues);
    // ------------------------------------------------------------------

    return UpdateGlobals(List);
}

// initialization without reading from a file

bool
CGlobals::InitGlobals()
{
    vector<string> Empty;
    
    return InitGlobals(Empty);
}

bool
CGlobals::UpdateGlobals(vector<string> const& List)
{
    if(!ReadArgs(List)) {
        yPWarn(ERR_FILE, GetErrorStr().c_str());
        return false;
    }

    return true;
}
