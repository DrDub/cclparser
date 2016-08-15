# Copyright 2007 Yoav Seginer

# This file is part of CCL-Parser.
# CCL-Parser is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# CCL-Parser is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with CCL-Parser.  If not, see <http://www.gnu.org/licenses/>.

EXE_TARGET	= $O/cclparser

CCOBJS	= $O/Globals.o $O/Main.o

PRSLIBS =

PRSLIBS	+= $(LIB_CCL)

PRSLIBS += $(LIB_PARSER) $(LIB_LABELS) $(LIB_PLAINTEXTLOOP) \
		   $(LIB_PENNTB) $(LIB_EVALUATION) $(LIB_SYNSTRUCT) \
		   $(LIB_PRSOBJS) $(LIB_LOOP) $(LIB_STATS) $(LIB_ARGUTIL) \
           $(LIB_FILEUTIL) $(LIB_PRINTUTIL) $(LIB_HASH) $(LIB_UTIL)

include $(PRSMK)
