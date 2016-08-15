#ifndef __UNITDECL_H__
#define __UNITDECL_H__

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

#include "Reference.h"
#include "PseudoPtrs.h"

class CUnit;
typedef CPtr<CUnit> CpCUnit;
typedef CPseudoPtrs<CUnit> CUnitPseudoPtrs;
typedef CPtr<CUnitPseudoPtrs> CpCUnitPseudoPtrs;
typedef CPseudoPtr<CUnit> CpsCUnit;
class CAtomicUnit;
typedef CPtr<CAtomicUnit> CpCAtomicUnit;
class CCompUnit;
typedef CPtr<CCompUnit> CpCCompUnit;


#endif /* __UNITDECL_H__ */
