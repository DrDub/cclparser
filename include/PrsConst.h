#ifndef __PRSCONST_H__
#define __PRSCONST_H__

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

// Basic constants shared by all classes and modules.

#define LEFT  0
#define RIGHT 1
#define BOTH_SIDES 2

#define SIDE_NUM 2  // number of sides : LEFT and RIGHT

#define LR_BIT 1   // bit used for LEFT/RIGHT

// opposite side
#define OP(__Side) (__Side == LEFT ? RIGHT : LEFT)

#endif /* __PRSCONST_H__ */
