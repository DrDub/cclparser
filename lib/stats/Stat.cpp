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
#include "Stat.h"
#include "NameList.h"
#include "ListPrint.h"
#include "yError.h"

using namespace std;

/////////////////////////
// Statistics Printing //
/////////////////////////

void
CStatPrintObj::PrintObj(CRefOStream* pOut, unsigned int Indent,
                       unsigned int SubIndent, eFormat Format,
                       int Parameter)
{
    // Since there are two sections here, we need to insert a EoL between
    // them. This is true only if both are non-empty. This variable is set to
    // true when the first section is discovered to be non-empty. Every
    // item in the second section checks whether the flag is set. If it is,
    // it prints an EoL and resets the flag. In this way only the first
    // item in the second section sees this flag set and also this only if
    // the first section was not empty.
    bool bNeedEoL = false;
    
    if(!pOut)
        return;

    // First, print the vector statistics (name, value)
    
    CListPrint StatList(pOut, Indent, SubIndent, VecStatFold(), 2, false);

    // Increase indentation by 1/3 tab
    StatList.IncIndent(1);

    bool bNoValue = false;
    
    for(vector<int>::iterator Stat = VecStatsToPrint().begin() ;
        Stat != VecStatsToPrint().end() ; Stat++) {
        
        if(*Stat == CStatPrintObj::eCR) {
            StatList.TerminateLine();
            continue;
        }

        if(*Stat == CStatPrintObj::eNoValue) {
            bNoValue = true;
            continue;
        }

        if(bNoValue) {
            bNoValue = false;
            if(VecGroupNames()[*Stat] == "")
                continue;
            StatList << VecGroupNames()[*Stat] << "  ";
        } else {
            StatList << VecStatNames()[*Stat] << ": "
                     << fixed << setprecision(1) << Strg(*Stat);
        }

        StatList.PrintNextEntry(NULL, 0);
        bNeedEoL = true;
    }
    StatList.CloseList();

    // For each table statistic which does not have an empty iterator,
    // print the statistic name and under it the statistic values
    CListPrint TableStatList(pOut, Indent, SubIndent, 1, 0, true);
    TableStatList.IncIndent(1);
    
    for(vector<int>::iterator Stat = TableStatsToPrint().begin() ;
        Stat != TableStatsToPrint().end() ; Stat++) {
        if(!GetTopLength(*Stat))
            continue; // no entries in this top list
        
        if(bNeedEoL) {
            ((ostream&)*pOut) << endl;
            bNeedEoL = false;
        }
        
        TableStatList << TableStatNames()[*Stat] << ":";
        CpCStatIterPrintObj pIter = GetPrintIter(*Stat);
        TableStatList.PrintNextEntry(pIter.Ptr(), *Stat);
    }
    TableStatList.CloseList();
}

////////////////////////////////
// Iterator printing routines //
////////////////////////////////

// Loop over the iterator, printing at each step the pre-candidate and
// the indicated statistic. Inside the button controlled area,
// additional statistics for the same entry may be printed and the
// value attached to the pre-candidate should be printed.

void
CStatIterPrintObj::PrintObj(CRefOStream* pOut, unsigned int Indent,
                            unsigned int SubIndent, eFormat Format,
                            int Stat)
{
    if(!pOut)
        return;

    // Increase the sub indentation by 1/3 tab
    if((++SubIndent) * PRT_1_3_TAB == PRT_TAB) {
        SubIndent = 0;
        Indent++;
    }

    switch(m_PrintingStage) {
        case eInit:
            PrintInit(pOut, Indent, SubIndent, Format, Stat);
            return;
        case eDetails:
            PrintDetails(pOut, Indent, SubIndent, Format, Stat);
            return;
        default:
            yPError(ERR_OUT_OF_RANGE, "unknown printing stage");
    }
}

void
CStatIterPrintObj::PrintInit(CRefOStream* pOut, unsigned int Indent,
                            unsigned int SubIndent, eFormat Format,
                            int Stat)
{   
    CListPrint IterStatList(pOut, Indent, SubIndent, IterStatFold(), 1, true);

    for( ; *this ; ++(*this)) {
        IterStatList << "\"" << EntryName() << "\" ("
                     << fixed << setprecision(1) << Strg(Stat) << ")";
        // Should additional information from the entry be printed
        // inside the button controlled area ?
        if(IterStatsAlsoToPrint(Stat).size() > 0 || !ValIsEmpty()) {
            m_PrintingStage = eDetails;
            IterStatList.PrintNextEntry(this, Stat);
        } else {
            IterStatList.PrintNextEntry(NULL, 0);
        }
    }

    IterStatList.CloseList();
}

void
CStatIterPrintObj::PrintDetails(CRefOStream* pOut, unsigned int Indent,
                               unsigned int SubIndent, eFormat Format,
                               int Stat)
{
    if(IterStatsAlsoToPrint(Stat).size() > 0) {
        // Print the additional statistics
        CListPrint StatList(pOut, Indent, SubIndent, IterAdditionalStatFold(),
                            1, false);
        for(vector<int>::iterator Also = IterStatsAlsoToPrint(Stat).begin() ;
            Also != IterStatsAlsoToPrint(Stat).end() ; Also++) {
            StatList << TableStatNames()[*Also] << ": "
                     << fixed << setprecision(1) << Strg(*Also);
            StatList.PrintNextEntry(NULL, 0);
        }
        StatList.CloseList();
    }

    if(!ValIsEmpty()) {
        if(IterStatsAlsoToPrint(Stat).size() > 0)
            (ostream&)*pOut << endl;
        ValPrintObj(pOut, Indent, SubIndent, Format, 0);
    }
    
    m_PrintingStage = eInit;
}
