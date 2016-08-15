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

#include <utility>
#include "Globals.h"
#include "PennParse.h"
#include "StringUtil.h"
#include "yError.h"

using namespace std;

/////////////////////
// IO loop objects //
/////////////////////

//
// Penn TB parsing object
//

CPennParse::CPennParse(vector<string> const & InFilePatterns,
                       CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                       COutFile* pOutFile) :
          CPennTBObjsIO(InFilePatterns, pArgs, MsgLine, pOutFile)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

bool
CPennParse::ProcessObject(CObj* pObj, std::string const& ObjInput)
{    
    CpCPennSynStruct pSynStruct = GetSynStruct(pObj, g_ReversePennObjs);

    if(pSynStruct->IsError()) {
        SetErrorStr("While reading object:" + EoLDelimiter()
                    + pSynStruct->GetErrorStr());
        return false;
    }

    // First, transfer the synstruct object
    ProcessSynStruct(pSynStruct, ObjInput);

    // Next, if the object was matched and there is a parser defined,
    // transfer the units one by one
    if(ShouldProcessSymbols()) {
    
        for(vector<CpCTerminal>::iterator Iter = pSynStruct->TerminalsBegin() ;
            Iter != pSynStruct->TerminalsEnd() ; Iter++) {
        
            if((*Iter)->Empty())
                continue;

            if((*Iter)->Punct()) {
                ePunctType Type = GetPunct(Iter);
                ProcessPunct(Type);
            }
            else if(g_UseTagsAsWords)
                ProcessUnit((*Iter)->Tag());
            else if(g_UseTagsAsLabels)
                ProcessUnit((*Iter)->Unit(), (*Iter)->Tag());
            else
                ProcessUnit((*Iter)->Unit());
        }
    }

    // if the object was matched, increment the count of objects processed
    if(ObjectMatches())
        IncObjsProcessed();
    
    // Signal the end of the utterance
    ProcessPunct(eEoUtterance);
    
    return true;
}

/////////////////////
// Derived Classes //
/////////////////////

//
// Wall Street Journal Corpus
//

CWSJPennParse::CWSJPennParse(std::vector<std::string> const & InFilePatterns,
                             CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                             COutFile* pOutFile) :
        CPennParse(InFilePatterns, pArgs, MsgLine, pOutFile)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif    
}

CWSJPennParse::~CWSJPennParse()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

bool
CWSJPennParse::IsCommentLine(string const& Line)
{
    // Use the standard comment line (line beginning with g_CommentStr)
    return IsCommentLineStr(Line);
}

CPennSynStruct*
CWSJPennParse::GetSynStruct(CObj* pObj, bool bReverse)
{
    return new CWSJPennSynStruct(pObj, bReverse);
}

// Wall Street Journal Corpus Punctuation

//
// The following functions translate the punctuation marks in the corpus into
// the standard (ePunctType) punctuation symbols. The following are the
// punctuation symbols found in the Wall Street Journal Corpus:
//
// Symbol    Tag     Remark
//
//  ','      ','
//  '.'      '.'
//  '``'     '``'
//  ''''     ''''
//  '--'     ':'     This is a dash
//  ';'      ':'
//  ':'      ':'
//  '...'    ':'
// '-LCB-' '-LRB-'
// '-RCB-' '-RRB-'
// '-LRB-' '-LRB-'
// '-RRB-' '-RRB-'
//  '?'      '.'
//  '`'      '``'
//  '''      ''''
//  '-'      ':'     This is usually a hyphen (though sometimes a dash)
//  '!'      '.'

// A static map structure is used to store the conversion from punctuation
// symbol as it appears in the WSJ corpus to the ePunctType.

static map<string const, ePunctType> WSJPunctTransTable;

// This function initializes the punctuation translation table

static void
WSJInitPunctTransTable()
{
    if(!WSJPunctTransTable.empty()) {
        yPError(ERR_SHOULDNT, "attempting to re-initialize the table");
    }

    WSJPunctTransTable["."] = eFullStop;
    WSJPunctTransTable["?"] = eQuestion;
    WSJPunctTransTable["!"] = eExclamation;
    WSJPunctTransTable[";"] = eSemiColon;
    WSJPunctTransTable["--"] = eDash;
    WSJPunctTransTable[":"] = eColon;
    WSJPunctTransTable[","] = eComma;
    WSJPunctTransTable["..."] = eEllipsis;
    WSJPunctTransTable["-LCB-"] = eLeftParenthesis;
    WSJPunctTransTable["-LRB-"] = eLeftParenthesis;
    WSJPunctTransTable["-RCB-"] = eRightParenthesis;
    WSJPunctTransTable["-RRB-"] = eRightParenthesis;
    WSJPunctTransTable["``"] = eLeftDoubleQuote;
    WSJPunctTransTable["''"] = eRightDoubleQuote;
    WSJPunctTransTable["`"] = eLeftSingleQuote;
    WSJPunctTransTable["'"] = eRightSingleQuote;
    WSJPunctTransTable["-"] = eHyphen;
    WSJPunctTransTable["$"] = eCurrency;
    WSJPunctTransTable["#"] = eCurrency;
}

ePunctType
CWSJPennParse::GetPunct(vector<CpCTerminal>::iterator const& Punct)
{
    if(WSJPunctTransTable.empty())
        WSJInitPunctTransTable();
    // Get the punctuation symbol
    
    map<string const, ePunctType>::iterator Entry =
        WSJPunctTransTable.find((*Punct)->Unit());

    if(Entry == WSJPunctTransTable.end()) {
        Entry = WSJPunctTransTable.find((*Punct)->Tag());
        if(Entry == WSJPunctTransTable.end()) {
            // not found
            yPError(ERR_OUT_OF_RANGE, "Failed to translate punctuation");
        }
    }

    return (*Entry).second;
}

//
// Negra Corpus
//

CNegraPennParse::CNegraPennParse(vector<string> const & InFilePatterns,
                                 CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                                 COutFile* pOutFile) :
        CPennParse(InFilePatterns, pArgs, MsgLine, pOutFile)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif    
}

CNegraPennParse::~CNegraPennParse()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

bool
CNegraPennParse::IsCommentLine(string const& Line)
{
    // A line beginning with '%' is a comment line. This is the comment
    // of the annotation.
    // The system standard comment (a line beginning with g_CommentStr)
    // is also considered a comment.
    return (Line.length() > 0 &&
            (Line.at(0) == '%' || IsCommentLineStr(Line)));
}

CPennSynStruct*
CNegraPennParse::GetSynStruct(CObj* pObj, bool bReverse)
{
    return new CNegraPennSynStruct(pObj, bReverse);
}

// Negra Corpus Punctuation

//
// The following functions translate the punctuation marks in the corpus into
// the standard (ePunctType) punctuation symbols. The following are the
// punctuation symbols found in the Wall Street Journal Corpus:
//
// Symbol    Tag          Frequency   Remark
//
//  ','      '$,'         17884
//  ','      '$,-NMC'     2
//  '.'      '$.'         16382
//  0xb7     '$.'         8
//  '.'      '$.-UC'      3
//  '.'      '$.-CJ'      1
//  '.'      '$.-NK'      1
//  '"'      '$*LRB*'     9371        both left and right double quote
//  ':'      '$.'         1952
//  ':'      '$.-CD'      18
//  ':'      '$.-NMC'     2
//  ':'      '$.-UC'      1
//  '*LRB*'  '$*LRB*'     1745
//  '*RRB*'  '$*LRB*'     1745
//  '-'      '$*LRB*'     1283        dash
//  '/'      '$*LRB*'     389         probably indicates the end of headline
//  '/'      '$*LRB*-NMC' 2
//  '/'      '$*LRB*-UC'  2
//  '?'      '$.'         304
//  ';'      '$.'         269
//  '!'      '$.'         112
//  '!'      '$.-PNC'     3
//  '''      '$*LRB*'     92          both left and right quote
//  '''      '$*LRB*-PNC' 2
//  '...'    '$*LRB*'     45
// 

// A static map structure is used to store the conversion from punctuation
// symbol as it appears in the Negra corpus to the ePunctType.

static map<string const, ePunctType> NegraPunctTransTable;

// This function initializes the punctuation translation table

static void
NegraInitPunctTransTable()
{
    if(!NegraPunctTransTable.empty()) {
        yPError(ERR_SHOULDNT, "attempting to re-initialize the table");
    }

    NegraPunctTransTable["."] = eFullStop;
    NegraPunctTransTable[string(1,0xb7)] = eFullStop;
    NegraPunctTransTable["?"] = eQuestion;
    NegraPunctTransTable["!"] = eExclamation;
    NegraPunctTransTable[";"] = eSemiColon;
    NegraPunctTransTable["-"] = eDash;
    NegraPunctTransTable[":"] = eColon;
    NegraPunctTransTable[","] = eComma;
    NegraPunctTransTable["..."] = eEllipsis;
    NegraPunctTransTable["\""] = eDoubleQuote;
    NegraPunctTransTable["'"] = eSingleQuote;
    NegraPunctTransTable["/"] = eHeadlineStop;
    NegraPunctTransTable["*LRB*"] = eLeftParenthesis;
    NegraPunctTransTable["*RRB*"] = eRightParenthesis;
}

ePunctType
CNegraPennParse::GetPunct(vector<CpCTerminal>::iterator const& Punct)
{
    if(NegraPunctTransTable.empty())
        NegraInitPunctTransTable();
    // Get the punctuation symbol
    
    map<string const, ePunctType>::iterator Entry =
        NegraPunctTransTable.find((*Punct)->Unit());

    if(Entry == NegraPunctTransTable.end()) {
        // not found
        yPError(ERR_OUT_OF_RANGE, "Failed to translate punctuation");
    }

    return (*Entry).second;
}

//
// Chinese Treebank Corpus
//

CCTBPennParse::CCTBPennParse(vector<string> const & InFilePatterns,
                             CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                             COutFile* pOutFile) :
        CPennParse(InFilePatterns, pArgs, MsgLine, pOutFile)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif    
}

CCTBPennParse::~CCTBPennParse()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

bool
CCTBPennParse::IsCommentLine(string const& Line)
{
    // Is this a standard comment line (line beginning with g_CommentStr)
    if(IsCommentLineStr(Line))
        return true;
    
    // lines with XML-style tags should be removed
    string::size_type FirstChar = Line.find_first_not_of(WhiteSpace());
    return (FirstChar != string::npos && Line.at(FirstChar) == '<');
}

CPennSynStruct*
CCTBPennParse::GetSynStruct(CObj* pObj, bool bReverse)
{
    return new CCTBPennSynStruct(pObj, bReverse);
}

// Chinese Treebank Punctuation

//
// The following functions translate the punctuation marks in the corpus into
// the standard (ePunctType) punctuation symbols.
// The table below gives the hex representation of the punctuation codes in
// the Chinese treebank together with the associated punctuation type.
// All punctuation marks appear under a single tag: PU.
//
// UTF-8              Punctuation        Frequency    Remark
// 0x7477             eNoPunct                1       'tw' unknown
// 0xa1b6             eLeftSingleQuote      771       '<<' before book name
// 0xa1aa             eDash                  18       long dash
// 0x2d2d2d2d         eDash                   1       long dash ?
// 0xa3ad x 2         eDash                  63       long dash ?
// 0x2f2f             eNoPunct                4       '//' unknown
// 0xa1af             eRightSingleQuote      25
// 0xa1b1             eRightDoubleQuote    1102
// 0xa1a1             eNoPunct                4       unknown
// 0x2c               eComma                  5       ','
// 0xa1aa x 2         eDash                 260       long dash
// 0xa3af             eNoPunct               59       unknown
// 0xa3a9             eRightParenthesis    2053       )
// 0xa3ae             eComma                198       check with Fenrong
// 0xa1aa x 3         eDash                  10
// 0xa9a5 x 2         eNoPunct                4       unknown
// 0x3a               eColon                  4
// 0xa3ba             eColon                866
// 0xa1ad             eEllipsis             212
// 0xa3a1             eExclamation          293
// 0xa3a7             eRightSingleQuote       1
// 0xa3a2             eRightDoubleQuote       4
// 0xa1ad x 2         eEllipsis               3       Chinese ellipsis
// 0xa1a3             eFullStop           14467
// 0xa1a4             eFullStop               6
// 0xa3bf             eQuestion             721
// 0x3333             eNoPunct                1       '33' annotation error
// 0xa1b9             eRightSingleQuote    3542       Chinese single quote
// 0xa3ac             eComma              35502       dou hao (comma)
// 0xa3be             eNoPunct               16       unknown
// 0xa3ad             eDash                  17       
// 0xa1bf             eRightSingleQuote       6       Old Chinese
// 0xa1ba             eLeftSingleQuote      134       Chinese single quote
// 0x2d               eDash                   6       '-'
// 0xa3a8             eLeftParenthesis     2065
// 0xa1a12a           eNoPunct                1       unknown
// 0x2f               eNoPunct               95       '/' unknown
// 0x21               eExclamation            2       '!'
// 0x2a               eNoPunct                2       '*' unknown
// 0xa1ab             eNoPunct                1       unknown
// 0x7e               eNoPunct                1       unknown
// 0xa1b0             eLeftDoubleQuote     1102
// 0xa9a5             eNoPunct                1       unknown
// 0xa3bc             eNoPunct                1       unknown
// 0xa9a4 x 2         eNoPunct                8       unknown
// 0xa1b5             eRightSingleQuote      41       >
// 0xa1bb             eRightSingleQuote     134       Chinses single quote
// 0xb0ebd1a8cabd     eNoPunct                1       Word, not punctuation
// 0xa1b4             eLeftSingleQuote       41       <
// 0xa1c3             eColon                 26
// 0x2e               eFullStop              43       '.'
// 0xa3aa             eNoPunct                8       unknown
// 0xa1b8             eLeftSingleQuote     3543       Chinese single quote
// 0xa1aaa3ad         eNoPunct                1       unknown
// 0x3f               eQuestion               1       '?'
// 0xa1b7             eRightSingleQuote     772       '>>' after book name
// 0xa1be             eLeftSingleQuote        6       Old Chinese
// 0xa1ae             eLeftSingleQuote       21
// 0xa3a6             eNoPunct                1       unknown
// 0x60               eLeftSingleQuote        1       '`'
// 0xa1a2             eComma               8515       dun hao
// 0xa3bb             eSemiColon            979

// A static map structure is used to store the conversion from punctuation
// symbol as it appears in the Negra corpus to the ePunctType.

static map<string const, ePunctType> CTBPunctTransTable;

// This function initializes the punctuation translation table

static void
CTBInitPunctTransTable()
{
    if(!CTBPunctTransTable.empty()) {
        yPError(ERR_SHOULDNT, "attempting to re-initialize the table");
    }

    { static char s[] = {0x74,0x77,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa1,0xb6,0};
        CTBPunctTransTable[s] = eLeftSingleQuote; }
    { static char s[] = {0xa1,0xaa,0};
        CTBPunctTransTable[s] = eDash; }
    { static char s[] = {0x2d,0x2d,0x2d,0x2d,0};
        CTBPunctTransTable[s] = eDash; }
    { static char s[] = {0xa3,0xad,0xa3,0xad,0};
        CTBPunctTransTable[s] = eDash; }
    { static char s[] = {0x2f,0x2f,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa1,0xaf,0};
        CTBPunctTransTable[s] = eRightSingleQuote; }
    { static char s[] = {0xa1,0xb1,0};
        CTBPunctTransTable[s] = eRightDoubleQuote; }
    { static char s[] = {0xa1,0xa1,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0x2c,0};
        CTBPunctTransTable[s] = eComma; }
    { static char s[] = {0xa1,0xaa,0xa1,0xaa,0};
        CTBPunctTransTable[s] = eDash; }
    { static char s[] = {0xa3,0xaf,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa3,0xa9,0};
        CTBPunctTransTable[s] = eRightParenthesis; }
    { static char s[] = {0xa3,0xae,0};
        CTBPunctTransTable[s] = eComma; }
    { static char s[] = {0xa1,0xaa,0xa1,0xaa,0xa1,0xaa,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa9,0xa5,0xa9,0xa5,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0x3a,0};
        CTBPunctTransTable[s] = eColon; }
    { static char s[] = {0xa3,0xba,0};
        CTBPunctTransTable[s] = eColon; }
    { static char s[] = {0xa1,0xad,0};
        CTBPunctTransTable[s] = eEllipsis; }
    { static char s[] = {0xa3,0xa1,0};
        CTBPunctTransTable[s] = eExclamation; }
    { static char s[] = {0xa3,0xa7,0};
        CTBPunctTransTable[s] = eRightSingleQuote; }
    { static char s[] = {0xa3,0xa2,0};
        CTBPunctTransTable[s] = eRightDoubleQuote; }
    { static char s[] = {0xa1,0xad,0xa1,0xad,0};
        CTBPunctTransTable[s] = eEllipsis; }
    { static char s[] = {0xa1,0xa3,0};
        CTBPunctTransTable[s] = eFullStop; }
    { static char s[] = {0xa1,0xa4,0};
        CTBPunctTransTable[s] = eFullStop; }
    { static char s[] = {0xa3,0xbf,0};
        CTBPunctTransTable[s] = eQuestion; }
    { static char s[] = {0x33,0x33,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa1,0xb9,0};
        CTBPunctTransTable[s] = eRightSingleQuote; }
    { static char s[] = {0xa3,0xac,0};
        CTBPunctTransTable[s] = eComma; }
    { static char s[] = {0xa3,0xbe,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa3,0xad,0};
        CTBPunctTransTable[s] = eDash; }
    { static char s[] = {0xa1,0xbf,0};
        CTBPunctTransTable[s] = eRightSingleQuote; }
    { static char s[] = {0xa1,0xba,0};
        CTBPunctTransTable[s] = eLeftSingleQuote; }
    { static char s[] = {0x2d,0};
        CTBPunctTransTable[s] = eDash; }
    { static char s[] = {0xa3,0xa8,0};
        CTBPunctTransTable[s] = eLeftParenthesis; }
    { static char s[] = {0xa1,0xa1,0x2a,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0x2f,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0x21,0};
        CTBPunctTransTable[s] = eExclamation; }
    { static char s[] = {0x2a,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa1,0xab,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0x7e,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa1,0xb0,0};
        CTBPunctTransTable[s] = eLeftDoubleQuote; }
    { static char s[] = {0xa9,0xa5,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa3,0xbc,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa9,0xa4,0xa9,0xa4,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa1,0xb5,0};
        CTBPunctTransTable[s] = eRightSingleQuote; }
    { static char s[] = {0xa1,0xbb,0};
        CTBPunctTransTable[s] = eRightSingleQuote; }
    { static char s[] = {0xb0,0xeb,0xd1,0xa8,0xca,0xbd,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa1,0xb4,0};
        CTBPunctTransTable[s] = eLeftSingleQuote; }
    { static char s[] = {0xa1,0xc3,0};
        CTBPunctTransTable[s] = eColon; }
    { static char s[] = {0x2e,0};
        CTBPunctTransTable[s] = eFullStop; }
    { static char s[] = {0xa3,0xaa,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0xa1,0xb8,0};
        CTBPunctTransTable[s] = eLeftSingleQuote; }
    { static char s[] = {0xa1,0xaa,0xa3,0xad,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0x3f,0};
        CTBPunctTransTable[s] = eQuestion; }
    { static char s[] = {0xa1,0xb7,0};
        CTBPunctTransTable[s] = eRightSingleQuote; }
    { static char s[] = {0xa1,0xbe,0};
        CTBPunctTransTable[s] = eLeftSingleQuote; }
    { static char s[] = {0xa1,0xae,0};
        CTBPunctTransTable[s] = eLeftSingleQuote; }
    { static char s[] = {0xa3,0xa6,0};
        CTBPunctTransTable[s] = eNoPunct; }
    { static char s[] = {0x60,0};
        CTBPunctTransTable[s] = eLeftSingleQuote; }
    { static char s[] = {0xa1,0xa2,0};
        CTBPunctTransTable[s] = eComma; }
    { static char s[] = {0xa3,0xbb,0};
        CTBPunctTransTable[s] = eSemiColon; }   
}

ePunctType
CCTBPennParse::GetPunct(vector<CpCTerminal>::iterator const& Punct)
{
    if(CTBPunctTransTable.empty())
        CTBInitPunctTransTable();
    
    // Get the punctuation symbol by the first 4 bytes of the code
    map<string const, ePunctType>::iterator Entry =
        CTBPunctTransTable.find((*Punct)->Unit());

    if(Entry == CTBPunctTransTable.end()) {
        // not found
        yPError(ERR_OUT_OF_RANGE, "Failed to translate punctuation");
    }

    return (*Entry).second;
}

