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
#include <sstream>
#include <iomanip>
#include "SynStruct.h"
#include "yError.h"

using namespace std;

CSynNode::CSynNode(CSynStructPtr* pStruct, std::string const& Tag,
                   char TagSplitChar)
        : m_pStruct(pStruct)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    SetTag(Tag, TagSplitChar);
}

void
CSynNode::SetTag(string const& Tag, char TagSplitChar)
{
    m_Tag = Tag;
    m_TagParts.clear();
    
    if(!Tag.size() || !TagSplitChar) {
        m_TagParts.push_back(Tag);
        return; // done
    }
    
    // split the tag into its component using the given separator character
    string::size_type Begin = 0;
    string::size_type End;

    while(Begin < Tag.size() &&
          (End = Tag.find(TagSplitChar, Begin)) != string::npos) {
        if(End > Begin)
            m_TagParts.push_back(Tag.substr(Begin, End - Begin));
        Begin = End+1;
    }

    if(Begin < Tag.size())
        m_TagParts.push_back(Tag.substr(Begin));
}

CNonTerminal::CNonTerminal(CSynStructPtr* pStruct, std::string const& Tag,
                           char TagSplitChar, vector<int> const& SubNodes,
                           bool bInferHead, int Head)
        : CSynNode(pStruct, Tag, TagSplitChar), m_Dominated(SubNodes),
          m_Head(Head), m_bInferredHead(false)
{
    static char Rname[] = "CNonTerminal::CNonTerminal";
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    if(Head >= (int)SubNodes.size()) {
        yPError(ERR_OUT_OF_RANGE, "head position not one of the nodes");
    }

    if(bInferHead)
        CalcInferredHead();
}

// Calculates the inferred head for this non-terminal.
// A head is inferred if the non-terminal has a single terminal sub-node
// (ignoring empty terminals and punctuation) and at least one non-terminal
// sub-node. In this case the terminal is the inferred head.

void
CNonTerminal::CalcInferredHead()
{
    static char Rname[] = "CNonTerminal::CalcInferredHead";
    
    if(m_Head >= 0)
        return; // head already calculated (inferred or not)

    CSynStruct* pStruct = m_pStruct->Ptr();
    
    if(!pStruct) {
        yPError(ERR_MISSING, "structure pointer missing");
    }
    
    int HeadPos = -1;
    unsigned int Pos = 0;
    bool bAtLeastOneNonTerminal = false;
    
    for(vector<int>::iterator Iter = m_Dominated.begin();
        Iter != m_Dominated.end() ; Iter++, Pos++) {
        if(*Iter >= 0) { // terminal
            if(pStruct->TerminalByNode(*Iter)->Empty() ||
               pStruct->TerminalByNode(*Iter)->Punct())
                continue; // skip this one
            if(HeadPos >= 0) // not first head
                return; // cannot infer head
            else {
                if(pStruct->TerminalByNode(*Iter)->Tag().length() &&
                   pStruct->TerminalByNode(*Iter)->Tag().at(0) != Tag().at(0))
                    return; // tags do not match
                HeadPos = Pos;
            }
        } else {
            // non-terminal
            bAtLeastOneNonTerminal = true;
        }
        
    }

    if(bAtLeastOneNonTerminal && HeadPos >= 0) {
        // inferred head
        m_Head = HeadPos;
        m_bInferredHead = true;
    }
        
}

int
CNonTerminal::GetHead() {
    if(m_Head < 0)
        return CSynStruct::NoNode;
    return m_Dominated[m_Head];
}

CSynNode*
CSynStruct::GetNode(int NodeNum)
{
    if(NodeNum >= 0) {
        if(m_Terminals.size() <= NodeNum)
            return NULL;
        return (CSynNode*)m_Terminals[NodeNum];
    }

    if(m_NonTerminals.size() <= NodeNum2NonTerm(NodeNum))
        return NULL;

    return (CSynNode*)m_NonTerminals[NodeNum2NonTerm(NodeNum)];
}

int
CSynStruct::AddTerminal(string const& Unit, string const& Tag,
                        bool bPunct, bool bEmpty)
{
    CpCTerminal pTerminal =
        new CTerminal(m_pThis, Unit, Tag, m_TerminalTagSplitChar,
                      bPunct, bEmpty);

    m_Terminals.push_back(pTerminal);

    return m_Terminals.size() - 1;
}

int
CSynStruct::AddTerminal(string const& Unit, string const& Tag,
                        unsigned int Pos,
                        bool bPunct, bool bEmpty)
{
    m_Terminals.reserve(Pos+1);
    
    while(m_Terminals.size() < Pos) {
        // append empty terminals
        m_Terminals.push_back(new CTerminal(m_pThis));
    }
    
    if(Pos >= m_Terminals.size())
        m_Terminals.push_back(new CTerminal(m_pThis, Unit, Tag,
                                            m_TerminalTagSplitChar,
                                            bPunct, bEmpty));
    else
        m_Terminals[Pos] = new CTerminal(m_pThis, Unit, Tag,
                                         m_TerminalTagSplitChar,
                                         bPunct, bEmpty);
    
    return Pos;
}


int
CSynStruct::AddNonTerminal(string const& Tag, vector<int>& SubNodes,
                           bool bInferHead, int Head)
{
    m_NonTerminals.push_back(new CNonTerminal(m_pThis, Tag,
                                              m_NonTerminalTagSplitChar,
                                              SubNodes,
                                              bInferHead, Head));
    
    int Pos = NonTerm2NodeNum(m_NonTerminals.size()-1);
    
    for(vector<int>::iterator Iter = SubNodes.begin();
        Iter != SubNodes.end() ; Iter++) {
        if(*Iter < 0)
            // non-terminal
            m_NonTerminals[NodeNum2NonTerm(*Iter)]->AddDominating(Pos);
        else // terminal
            m_Terminals[*Iter]->AddDominating(Pos);
    }

    return Pos;
}

unsigned int
CSynStruct::CountTerminals(bool bNoEmpty, bool bNoPunct)
{
    unsigned int Count = 0;

    if(!bNoEmpty || !bNoPunct)
        return TerminalNum();
    
    for(vector<CpCTerminal>::iterator Iter= m_Terminals.begin() ;
        Iter != m_Terminals.end() ; Iter++) {
        if((!bNoEmpty || !(*Iter)->Empty()) &&
           (!bNoPunct || !(*Iter)->Punct()))
            Count++;
    }

    return Count;
}

unsigned int
CSynStruct::CountContentUnits()
{
    return CountTerminals(true, true);
}

CNonTerminal*
CSynStruct::NonTerminalByNode(int NodeNum) {
    if(NodeNum >= 0 || NodeNum2NonTerm(NodeNum) >= m_NonTerminals.size())
        return NULL;

    return (CNonTerminal*)m_NonTerminals[NodeNum2NonTerm(NodeNum)];
}

void
CSynStruct::CalcInferredHeads()
{
    for(vector<CpCNonTerminal>::iterator Iter = NonTerminalsBegin() ;
        Iter != NonTerminalsEnd() ; Iter++)
        (*Iter)->CalcInferredHead();
}

bool
CSynStruct::SetTerminalTagsFromSuperSequence(CSynStruct* pSuper,
                                             bool bNoEmpty, bool bNoPunct)
{
    if(!pSuper ||
       pSuper->CountTerminals(bNoEmpty, bNoPunct) !=
       CountTerminals(bNoEmpty, bNoPunct))
        return false;

    m_TerminalTagSplitChar = pSuper->m_TerminalTagSplitChar;
    
    // iterate over the terminal lists of both structures
    vector<CpCTerminal>::iterator Iter = TerminalsBegin();
    vector<CpCTerminal>::iterator SupIter = pSuper->TerminalsBegin();

    while(Iter != TerminalsEnd() && SupIter != pSuper->TerminalsEnd()) {
        if((bNoEmpty && (*Iter)->Empty()) || (bNoPunct && (*Iter)->Punct())) {
            Iter++;
            continue;
        }
        if((bNoEmpty && (*SupIter)->Empty()) ||
           (bNoPunct && (*SupIter)->Punct())) {
            SupIter++;
            continue;
        }
        
        (*Iter)->SetTag((*SupIter)->Tag(), m_TerminalTagSplitChar);
        Iter++;
        SupIter++;
    }

    return true;
}

///////////////////////
// Printing routines //
///////////////////////

//
// Simple print (for debugging)
//

void
CSynStruct::SimplePrint(std::ostream& Out)
{
    // determine field widths
    unsigned int MaxUnitLen = 0;
    unsigned int MaxTagLen = 0;

    for(int i = 0 ; i < m_Terminals.size() ; i++) {
        if(m_Terminals[i]->Tag().length() > MaxTagLen)
            MaxTagLen = m_Terminals[i]->Tag().length();
        if(m_Terminals[i]->Unit().length() > MaxUnitLen)
            MaxUnitLen = m_Terminals[i]->Unit().length();
    }

    for(int i = 0 ; i < m_NonTerminals.size() ; i++) {
        if(MaxTagLen < m_NonTerminals[i]->Tag().length())
            MaxTagLen = m_NonTerminals[i]->Tag().length();
    }
    
    // first, the terminals
    
    for(int i = 0 ; i < m_Terminals.size() ; i++) {
        Out << i << ": \t" << setw(MaxTagLen) << left <<
            m_Terminals[i]->Tag() << " " <<
            setw(MaxUnitLen) << left << m_Terminals[i]->Unit() <<
            " dominated by: ";
        for(vector<int>::iterator Iter = m_Terminals[i]->DominatingBegin() ;
            Iter != m_Terminals[i]->DominatingEnd() ; Iter++) {
            Out << *Iter;
            if(Iter+1 != m_Terminals[i]->DominatingEnd())
                Out << ", ";
        }

        if(m_Terminals[i]->Empty())
            Out << " \t[empty]";
        else if(m_Terminals[i]->Punct())
            Out << " \t[punct]";
        
        Out << endl;
    }

    // next, the non-terminals
    
    for(int i = 0 ; i < m_NonTerminals.size() ; i++) {
        Out << NodeNum2NonTerm(i) << ": \t" << setw(MaxTagLen) << left <<
            m_NonTerminals[i]->Tag() << " " << setw(MaxUnitLen) << "" <<
            " dominating: ";
        for(vector<int>::iterator Iter = m_NonTerminals[i]->DominatedBegin() ;
            Iter != m_NonTerminals[i]->DominatedEnd() ; Iter++) {
            Out << *Iter;
            if(Iter+1 != m_NonTerminals[i]->DominatedEnd())
                Out << ", ";
        }

        Out << " \tHead: ";
        
        if(m_NonTerminals[i]->GetHead() != CSynStruct::NoNode) {
            Out << m_NonTerminals[i]->GetHead() << " ";
            if(m_NonTerminals[i]->IsInferredHead())
                Out << "(Inferred) ";
        } else
            Out << "none ";

        Out << " \tdominated by: ";
        
        for(vector<int>::iterator Iter = m_NonTerminals[i]->DominatingBegin() ;
            Iter != m_NonTerminals[i]->DominatingEnd() ; Iter++) {
            Out << *Iter;
            if(Iter+1 != m_NonTerminals[i]->DominatingEnd())
                Out << ", ";
        }

        Out << endl;
    }
}
