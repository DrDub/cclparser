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

#include <ostream>
#include <sstream>
#include <algorithm>
#include "SynStruct.h"
#include "SynBrackets.h"
#include "yError.h"

using namespace std;

//////////////////////////
// Bracketing Structure //
//////////////////////////

static unsigned int
SetTerminalBits(CNonTerminal* pNonTerminal, CBitMap& BitMap,
                vector<int>& DominatedTerminals,
                CSynStruct* pStruct, vector<int>& Conversion)
{
    unsigned int Count = 0; // number of bits set

    if(!pNonTerminal || !pStruct) {
        yPError(ERR_MISSING, "pNonTerminal or pStruct missing");
    }
        
    // loop over the node dominated by the non-terminal
    for(vector<int>::iterator Iter = pNonTerminal->DominatedBegin() ;
        Iter != pNonTerminal->DominatedEnd() ; Iter++) {
        if(*Iter < 0) {
            // non terminal
            Count += SetTerminalBits(pStruct->NonTerminalByNode(*Iter),
                                     BitMap, DominatedTerminals, pStruct,
                                     Conversion);
        } else {
            // terminal
            if(Conversion[*Iter] >= 0) {
                if(BitMap.SetBit(Conversion[*Iter])) {
                    Count++;
                    DominatedTerminals.push_back(*Iter);
                }
            }
        }
    }

    return Count;
}

CSynBrackets::CSynBrackets(CSynStruct* pStruct, eLabeling Labeling,
                           bool bNoEmpty, bool bNoPunct, bool bNoTop,
                           bool bNoUnary)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    
    if(!pStruct)
        return;
    
    // determine terminal number conversion
    vector<int> Conversion;
    vector<string> TerminalLabels;
    int Count = 0;
    
    for(vector<CpCTerminal>::iterator Iter = pStruct->TerminalsBegin() ;
        Iter != pStruct->TerminalsEnd() ; Iter++) {

         // store the terminal tag
        if(Labeling == eShortTagSeq && (*Iter)->TagParts().size())
            TerminalLabels.push_back((*Iter)->TagParts().front());
        else
            TerminalLabels.push_back((*Iter)->Tag());
        
        if((bNoEmpty && (*Iter)->Empty()) || (bNoPunct && (*Iter)->Punct()))
            // skip this one
            Conversion.push_back(-1);
        else
            Conversion.push_back(Count++);
    }

    // loop over the non-terminals and store each of them both in the BitMap
    // map structure an in the BitMap vector.
    
    m_BitMaps.resize(pStruct->NonTerminalNum()); // initialize to NULL pointers
    
    for(vector<CpCNonTerminal>::iterator Iter = pStruct->NonTerminalsBegin() ;
        Iter != pStruct->NonTerminalsEnd() ; Iter++) {

        CpCBitMap pBitMap(new CBitMap());
        
        vector<int> DomTerms; // list of dominated terminals
        
        unsigned int Count =
            SetTerminalBits((CNonTerminal*)*Iter, *pBitMap, DomTerms,
                            pStruct, Conversion);

        m_BitMaps[Iter - pStruct->NonTerminalsBegin()] = pBitMap;

        // construct label (if necessary)
        string Label;

        switch(Labeling) {
            case eTag:
                Label = (*Iter)->Tag();
                break;
            case eShortTag: // only first part of the tag
                Label = (*Iter)->TagParts().size() ?
                    (*Iter)->TagParts().front() : (*Iter)->Tag();
                break;
            case eTagSeq:
            case eShortTagSeq:
                sort(DomTerms.begin(), DomTerms.end());
                for(vector<int>::iterator DomIter = DomTerms.begin() ;
                    DomIter != DomTerms.end() ; DomIter++) {
                    if(DomIter != DomTerms.begin())
                        Label += " ";
                    Label += TerminalLabels[*DomIter];
                }
                break;
            case eLen:
            {
                ostringstream OStr(ios::out);
                OStr << Count;
                Label = OStr.str();
                break;
            }
            case eNone:
            default:
                Label = "";
        }
        
        if(!bNoUnary || Count > 1)
            m_NonTerminals[pBitMap] = Label;
    }

    if(bNoTop) {
        // remove the top bracket (the one covering all terminals) from the
        // bitmap lookup table.
        CpCBitMap pBitMap = new CBitMap();
        if(Count > 0)
            pBitMap->SetBitsUpTo(Count-1);
        
        m_NonTerminals.erase(pBitMap);
    }
}

// returns the number of brackets in SynBrackets which are also in 'this'

unsigned int
CSynBrackets::Precision(CSynBrackets& SynBrackets)
{
    unsigned int Precision = 0;
    
    for(map<CpCBitMap, string>::iterator Iter =
            SynBrackets.m_NonTerminals.begin() ;
        Iter != SynBrackets.m_NonTerminals.end() ; Iter++) {
        if(Iter->first && m_NonTerminals.count(Iter->first))
            Precision++;
    }

    return Precision;
}

unsigned int
CSynBrackets::Match(CSynBrackets& SynBrackets,
                    vector<string>& Matched, vector<string>& NotMatched)
{
    Matched.clear();
    NotMatched.clear();

    unsigned int Count = 0;
    
    for(map<CpCBitMap, string>::iterator Iter = m_NonTerminals.begin() ;
        Iter != m_NonTerminals.end() ; Iter++) {
        if(!Iter->first)
            continue;

        map<CpCBitMap, string>::iterator Match = 
            SynBrackets.m_NonTerminals.find(Iter->first);

        if(Match == SynBrackets.m_NonTerminals.end())
            NotMatched.push_back(Iter->second);
        else {
            Matched.push_back(Iter->second);
            Count++;
        }
    }

    return Count;
}

unsigned int
CSynBrackets::NonTermNum()
{
    return m_NonTerminals.size();
}

CBitMap*
CSynBrackets::NonTermByNodeNum(int NodeNum)
{
    int Pos = CSynStruct::NodeNum2NonTerm(NodeNum);

    if(Pos < 0 || Pos >= m_BitMaps.size())
        return NULL;

    return m_BitMaps[Pos];
}

void
CSynBrackets::PrintBrackets(ostream& Out, unsigned int Indent)
{
    string Tab(Indent, ' ');
    
    for(map<CpCBitMap, string>::iterator Iter = m_NonTerminals.begin() ;
        Iter != m_NonTerminals.end() ; Iter++) {
        if(!Iter->first)
            continue;
        Iter->first->Print(Out);
        if(Iter->second.size() > 0)
            Out << " " << Iter->second;
        Out << endl;
    }
}

void
CSynBrackets::PrintObj(CRefOStream* pOut, unsigned int Indent,
                       unsigned int SubIndent, eFormat Format,
                       int Parameter)
{
    if(!pOut)
        return;

    PrintBrackets((ostream&)*pOut,
                  Indent * PRT_TAB + SubIndent * PRT_1_3_TAB);
}
