#ifndef __GLOBALS_H__
#define __GLOBALS_H__

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

#include <string>
#include <vector>
#include "Reference.h"
#include "ConfArgs.h"

// header for global variables

//
// Table sizes
//

// maximal number of top entries in a statistics table
extern unsigned int g_StatisticsTopListMaxLen;
// maximal number of labels (on each side of a labeled object)
extern unsigned int g_MaxLabels;

//
// Input reading
//

// if the input contains tags (e.g. POS) use these instead of the words
extern unsigned int g_UseTagsAsWords;
// if the input contains tags (e.g. POS) use these in addition to the
// words to label them.
extern unsigned int g_UseTagsAsLabels;
// Should currency symbols (e.g. $) be considered punctuation?
extern unsigned int g_CurrencySymbolIsPunct;
// Should stopping punctuation be used or ignored?
extern unsigned int g_UseStoppingPunct;
// For backward compatibility with older experiments, terminating
// punctuation marks (such as fullstops, question marks and exclamation
// marks) can be discarded. Except for backward compatibility, these
// should not be discarded and the property should be set to 0
// (which is, of course, the default).
extern unsigned int g_DiscardTerminatingPunct;
// When reading from Penn treebank style input, reverse the input so that
// the utterance is read from right to left.
extern unsigned int g_ReversePennObjs;

//
// Parser Specification
//

// Parser which should be used
extern std::string g_ParserType;

//
// Evaluation
//

// When precision and recall is calculated, should the top bracket be
// included in the calculation?
extern unsigned int g_CountTopBracket;

// Specific evaluators

// When using the grouped precision and recall evaluator, which function(s)
// should be used for sorting (names separated by white space)
extern std::string g_EvalGroupedOutputSorting;
// When using the grouped precision and recall evaluator, how many entries
// should be printed (use 0 for all entries)
extern unsigned int g_GroupedEvalMaxPrint;

//
// Output printing
//

// Comment line indicator. Typically, but not necessarily, this is a single
// character.
extern std::string g_CommentStr;
// minimal count for an entry in the lexicon to be printed
extern unsigned int g_LexMinPrint;
// Printing mode (see PrintUtils.h and PrintUtils.cpp for possible values).
// These values should appear in a list, separated by whitespace.
extern std::string g_PrintingMode;
// Parse trace bits (indicate what type of tracing to perform) 
extern unsigned int g_TraceBits;

//
// Specific parsers
//

// CCL (Common Cover Links)

// Basic parsing function family

// If 'true' the link weight function uses both the eBase and eDerived
// values of the Inbound link property to determine the weight of a link.
// If this is 'false' only the eDerived value is used.
extern unsigned int g_CCLBasicUseBothInValues;

// global initialization and printing class

class CGlobals : public CConfArgs
{
public:
    CGlobals();
    CGlobals(std::vector<std::string> const& List);
    bool InitGlobals(std::vector<std::string> const& List);
    bool InitGlobals();
    // update the values of the globals based on the values in the given
    // configuration files.
    bool UpdateGlobals(std::vector<std::string> const& List);
};

typedef CPtr<CGlobals> CpCGlobals;

#endif /* __GLOBALS_H__ */
