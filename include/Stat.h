#ifndef __STAT_H__
#define __STAT_H__

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

#include "StatTable.h"
#include "StatVector.h"
#include "PrintUtils.h"
#include "NameList.h"

////////////////////////////////////
// Statistics Printing Base Class //
////////////////////////////////////

class CStatIterPrintObj;
typedef CPtr<CStatIterPrintObj> CpCStatIterPrintObj;

// This class is used to implement the printing functionality of the statistics
// class. This is not a template class, so its functions are implemented
// in a .cpp file.

class CStatPrintObj : public CPrintObj
{
public:
    // Special printing codes. When one of these is used as the property
    // to be printed, special printing actions are taken. Properties should
    // not be given the same numbers as these codes. For this reason,
    // these codes all have negative values and it is assumed that
    // all properties have non-negative values.
    enum eStatPrintCodes {
        eCR = -1,  // terminate the line and begin printing on the next line
        eNoValue = -2 // next entry is just a group without value
    };
    
    // Main printing function
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
                  unsigned int SubIndent, eFormat Format,
                  int Parameter);
private:

    //
    // Auxiliary functions implemented in the CStat template
    //
    
    // Get the number of entries in the top list of the given table property.
    virtual unsigned int GetTopLength(unsigned int Prop) = 0;
    // Return the value of the given vector statistic.
    virtual float Strg(unsigned int Prop) = 0;

    //
    // Auxiliary functions which should be implemented in the derived classes
    // (they are not implemented in the CStat template)
    // which allow the printing function access to the data which are
    // to be printed and their formating.
    //
    
    // Number of vector entries to print per line (default is set below) 
    virtual unsigned int VecStatFold() { return 3; }
    // Returns a reference to an array of property numbers.
    // These are the numbers of the properties which should be printed.
    virtual std::vector<int>& VecStatsToPrint() = 0;
    // Returns a reference to an object holding the names of the vector
    // properties which should be printed.
    virtual CNameList& VecStatNames() = 0;
    // Returns a reference to an object holding the names of the vector group
    // names which should be printed.
    virtual CNameList& VecGroupNames() = 0;
    // Same as VecStatsToPrint above, only for table statistics
    virtual std::vector<int>& TableStatsToPrint() = 0;
    // Returns a reference to an object holding the names of the table
    // properties which should be printed.
    virtual CNameList& TableStatNames() = 0;
    // Create an iterator object for the given property
    virtual CStatIterPrintObj* GetPrintIter(unsigned int Prop) = 0;
};

// This class implements the printing functionality of the table statistics
// iterator class.

class CStatIterPrintObj : public CPrintObj, public CRef
{
private:
    enum ePrintingStage {
        eInit,
        eDetails
    };
    // Printing stage
    ePrintingStage m_PrintingStage;
public:
    CStatIterPrintObj() : m_PrintingStage(eInit) {}
    
    // Main printing function
    void PrintObj(CRefOStream* pOut, unsigned int Indent,
             unsigned int SubIndent, eFormat Format,
             int Stat);
private:
    // Secondary printing functions
    void PrintInit(CRefOStream* pOut, unsigned int Indent,
                   unsigned int SubIndent, eFormat Format,
                   int Stat);
    void PrintDetails(CRefOStream* pOut, unsigned int Indent,
                      unsigned int SubIndent, eFormat Format,
                      int Stat);

private:

    //
    // Auxiliary functions implemented in the CStatIter template
    //
    
    // Iterator position functions
    
    // advance the iterator - return false if advance beyond the end
    virtual bool operator++() = 0;
    // Is the iterator still pointing at an entry?
    virtual operator bool() = 0;
    
    // Iterator entry value functions
    
    // Returns the strength of the given property for the current entry
    virtual float Strg(unsigned int Prop) = 0;
    // Returns 'true' if the value of the current entry is empty.
    virtual bool ValIsEmpty() = 0; 
    // Call the PrintObj() function on the current value.
    virtual void ValPrintObj(CRefOStream* pOut, unsigned int Indent,
                             unsigned int SubIndent, eFormat Format,
                             int Parameter) = 0;

    //
    // Auxiliary functions which should be implemented in the derived classes
    // (they are not implemented in the CStat template)
    // which allow the printing function access to the data which are
    // to be printed and their formating
    //
    
    // Number of table entries to print per line (default is set below) 
    virtual unsigned int IterStatFold() { return 3; }
    // Returns a reference to a string holding the name of the current
    // entry.
    virtual std::string const& EntryName() = 0;
    // Returns a reference to an array of property numbers.
    // These are the numbers of the additional properties which should be
    // printed for the current entry.
    virtual std::vector<int>& IterStatsAlsoToPrint(unsigned int Stat) = 0;
    // Number of additional entries to print per line (default is set below) 
    virtual unsigned int IterAdditionalStatFold() { return 3; }
    // Returns a reference to an object holding the names of the table
    // properties which should be printed.
    virtual CNameList& TableStatNames() = 0;
};

//////////////////////////////
// Statistics Base Template //
//////////////////////////////

// The following simple template combines a statistics table (for
// statistics per entry) and a statistics vector (for statistics not
// specific for an entry). It adds functionality which allows it to return
// not only raw values out of the table and vector (using the interface of
// those classes), but also functions of these statistics (currently only
// some specific functions are implemented).
//
// Note that any instantiation of this template is an abstract class
// because both CStatTable and CStatVector have pure virtual functions.

template <class K, class V>
class CStat : public CStatTable<K,V>, public CStatVector, public CStatPrintObj
{
public:
    CStat(unsigned int TopNum, unsigned int TopLength,
          unsigned int HashSize, bool bReserve = false,
          unsigned int Size = 0) :
            CStatTable<K,V>(TopNum, TopLength, HashSize, bReserve),
            CStatVector(Size) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    ~CStat() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

protected:
    // Access to the functions of statistics object is only possible at
    // the derived class, so the following functions are all virtual.
    // The defualt behaviour implements no functions at all.
    
    // This function returns true if the given absolute property refers to
    // a function.
    virtual bool IsFunc(unsigned int Prop) { return false; }
    // This function returns the value of the function referred to by the
    // code. If the code does not refer to a function, it returns
    // 0.
    virtual float Func(unsigned int Prop) { return 0; }
    
public:
    //
    // Functions required by the printing base class
    //
    
    // Get the number of entries in the top list of the given table property.
    unsigned int GetTopLength(unsigned int Prop) {
        return CStatTable<K,V>::GetTopLength(Prop);
    }
    
    float Strg(unsigned int Prop) {
        if(IsFunc(Prop))
            return Func(Prop);
        return (*(CStatVector*)this)[Prop];
    }
    
    // The following virtual function indicates whether any statistics were
    // already collected by this object. The function should be defined
    // buy the specific instantiations of this template.

    virtual bool IsEmpty() = 0; 
    
    //
    // Quotient statistics
    //
    // All quotient function have a name QtXX where each of the X's
    // can be either T (for table) or V (for vector). This indicates
    // where each of the two values in the quotient have to come from -
    // the table or the vector.
    // All quotient statistics functions return 0 if the devisor is 0

    // Return the value of the given table statistic for the entry with
    // the given key divided by the given vector statistic.  
    float QtTV(K* pKey, unsigned int TableProp, unsigned int VecProp) {
        float VecStat = (*this)[VecProp];
        return VecStat ? CStatTable<K,V>::GetStrg(pKey, TableProp)/VecStat : 0;
    }
    // Return the value of the given table statistic for the entry with
    // the given value divided by the given vector statistic.  
    float QtTV(V* pVal, unsigned int TableProp, unsigned int VecProp) {
        float VecStat = (*this)[VecProp];
        return VecStat ? CStatTable<K,V>::GetStrg(pVal, TableProp)/VecStat : 0;
    }
    // Return the value of the given table statistic for the cached entry
    // divided by the given vector statistic.  
    float QtTV(unsigned int TableProp, unsigned int VecProp) {
        float VecStat = (*this)[VecProp];
        return VecStat ? CStatTable<K,V>::GetStrg(TableProp)/VecStat : 0;
    }

    // Return the quotient of the two given vector statistics
    float QtVV(unsigned int VecProp1, unsigned int VecProp2) {
        float VecStat2 = (*this)[VecProp2];
        return VecStat2 ? (*this)[VecProp1]/VecStat2 : 0;
    }

    // Not all quotient statistics for two table entries are currently
    // implemented (since there are so many of them possible and most of
    // them are not needed). Here only quotient statistics for two properties
    // of the same entry are implemented.

    // Return the quotient of the given table statistics for the entry with
    // the given key.
    float QtTT(K* pKey, unsigned int TableProp1, unsigned int TableProp2) {
        float TableStat2 = CStatTable<K,V>::GetStrg(pKey, TableProp2);
        // entry is cached - no need to look it up again
        return TableStat2 ?
            CStatTable<K,V>::GetStrg(TableProp1)/TableStat2 : 0;
    }
    // Return the quotient of the given table statistics for the entry with
    // the given value.
    float QtTT(V* pVal, unsigned int TableProp1, unsigned int TableProp2) {
        float TableStat2 = CStatTable<K,V>::GetStrg(pVal, TableProp2);
        // entry is cached, but it is quicker not to use the cache
        return TableStat2 ?
            CStatTable<K,V>::GetStrg(pVal, TableProp1)/TableStat2 : 0;
    }
    // Return the quotient of the given table statistics for the cached
    // entry.
    float QtTT(unsigned int TableProp1, unsigned int TableProp2) {
        float TableStat2 = CStatTable<K,V>::GetStrg(TableProp2);
        return TableStat2 ?
            CStatTable<K,V>::GetStrg(TableProp1)/TableStat2 : 0;
    }
};

//
// Statistics Iterator Base Template
//

// This iterator is based on the standard strength table iterator. The
// added functionality includes the use of the different quotient functions
// defined for the CStat class template

template <class K, class V>
class CStatIter : public CStatIterPrintObj
{
private:
    CPtr<CStat<K, V> > m_pStat;
    CPtr<CTopIter<K, V> > m_pIter;
public:
    CStatIter(CStat<K, V>* pStat, unsigned int Prop) : m_pStat(pStat) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
        if(m_pStat)
            m_pIter = m_pStat->GetIter(Prop);
    }

    ~CStatIter() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

    //
    // Standard iterator interface
    //
    
    // advance the iterator - return false if advance beyond the end
    bool operator++() {
        if(!m_pIter)
            return false;
        ++(*m_pIter);
        return (bool)*m_pIter;
    }
    operator bool() { return m_pIter && *m_pIter; }
    // number of elements remaining in the iterator to the end (including
    // the current element)
    int NumRemaining() { return m_pIter ? m_pIter->NumRemaining() : 0; }
    // restarts the iterator (returns false if the iterator is empty)
    // This should only be used when it is certain that the table has not
    // been changed since the iterator was created.
    bool Restart() { return m_pIter ? m_pIter->Restart() : false; }
    // Strength for the property for which the iterator was created
    float Strg() { return m_pIter ? m_pIter->Strg() : 0; }
    K* Data() { return m_pIter ? m_pIter->Data() : NULL; }
    V* Val() { return m_pIter ? m_pIter->Val() : NULL; }

    // Returns true if there is no value at the current position or if this
    // value is empty.
    bool ValIsEmpty() {
        return !Val() || Val()->IsEmpty(); 
    }
    // Call the PrintObj() function on the current value (if exists)
    void ValPrintObj(CRefOStream* pOut, unsigned int Indent,
                     unsigned int SubIndent, eFormat Format,
                     int Parameter) {
        if(Val())
            Val()->PrintObj(pOut, Indent, SubIndent, Format, Parameter);
    }
    
    //
    // Additional values not available directly from the iterator
    //

    // Return the value of the given vector statistic.
    float operator[](unsigned int Prop) {
        return m_pStat ? (*m_pStat)[Prop] : 0;
    }
    
    // Return the value of the given table statistic for the current iterator
    // position.
    float Strg(unsigned int Prop) {
        return m_pStat ? m_pStat->GetStrg(Val(), Prop) : 0;
    }

    // Returns the quotient of the statistic for which the iterator was
    // created at the current iterator position divided by the given
    // vector statistic.
    float QtV(unsigned int VecProp) {
        float VecStat = m_pStat ? (*m_pStat)[VecProp] : 0;
        return VecStat ? Strg() / VecStat : 0;
    }

    // Returns the quotient of the statistic for which the iterator was
    // created at the current iterator position divided by the given
    // table statistic for the same entry.
    float QtT(unsigned int TableProp) {
        float TableStat = Strg(TableProp);
        return TableStat ? Strg() / TableStat : 0;
    }
    
    // Return quotient of the given table statistic for the current iterator
    // entry divided by the given vector statistic.  
    float QtTV(unsigned int TableProp, unsigned int VecProp) {
        float VecStat = m_pStat ? (*m_pStat)[VecProp] : 0;
        return VecStat ? m_pStat->GetStrg(TableProp)/VecStat : 0;
    }

    // Return quotient of the given table statistics for the current iterator
    // entry.
    float QtTT(unsigned int TableProp1, unsigned int TableProp2) {
        float TableStat2 = Strg(TableProp2);
        return TableStat2 ? Strg(TableProp1)/TableStat2 : 0;
    }
};

#endif /* __STAT_H__ */
