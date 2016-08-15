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

##############################################################################
#                                                                            #
#                        D E F I N I T I O N S                               #
#                                                                            #
##############################################################################

#
# Architecture macros
#

ifndef OSTYPE
OSTYPE = UnknownOS
endif

ifndef ARCH
ARCH =	$(OSTYPE)
endif

ifndef O
O					:=	$(ARCH)
endif #! O

#
# Utilities
#

MKDIR				=	mkdir
AR					=	ar rv
LD					=	ld

CDMAKE				=	$(MAKE) MROOT='$(MROOT)/..' \
								PRSMK='../$(PRSMK)' -C

#
# Compilers
#

# c++ compiler 

CC					=	g++

#
# Debugging - the debug flag is set unless COPT is defined (it may still be
# empty)
#

ifndef COPT
COPT = 
CDEBUG				= -g -DTRACING
# For additional debugging, uncomment the following line
#CDEBUG				+= -DDEBUG -DDETAILED_DEBUG
endif

#
# Flags and libraries
#
INCDIRS				+=	$(MROOT)/include

INCLUDES			=	$(INCDIRS:%=-I%)

CPPFLAGS			=	$(COPT) $(CDEBUG) $(INCLUDES)

LINKER_FLAGS		=	-lc


CREATE_DIRECTORIES	+=	$O

#
# Sources, Objects
#
OBJS				=	$(CCOBJS)
LIB_OBJS			=	$(LIB_CCOBJS)
CCSRCS				=	$(CCOBJS:$O/%.o=%.cpp) $(LIB_CCOBJS:$O/%.o=%.cpp)
SRCS				=	$(CCSRCS)

#
# Direcotries
#

#
# Libraries
#

# general purpose libraries
LIB_HASH			=	$(MROOT)/lib/hash/$O/libhash.a
LIB_UTIL			=	$(MROOT)/lib/util/$O/libutil.a
LIB_FILEUTIL		=	$(MROOT)/lib/fileutil/$O/libfileutil.a
LIB_PRINTUTIL		=	$(MROOT)/lib/printutil/$O/libprintutil.a
LIB_ARGUTIL			=	$(MROOT)/lib/argutil/$O/libargutil.a
LIB_STATS			=	$(MROOT)/lib/stats/$O/libstats.a
LIB_LOOP			=	$(MROOT)/lib/loop/$O/libloop.a
LIB_PRSOBJS			=	$(MROOT)/objs/$O/libprsobjs.a
LIB_PLAINTEXTLOOP	=	$(MROOT)/plaintextloop/$O/libplaintextloop.a
# genral parsing libraries 
LIB_PENNTB			=	$(MROOT)/penntb/$O/libpenntb.a
LIB_SYNSTRUCT		=	$(MROOT)/synstruct/$O/libsynstruct.a
LIB_EVALUATION		=	$(MROOT)/evaluation/$O/libevaluation.a
LIB_LABELS			=	$(MROOT)/labels/$O/liblabels.a
LIB_PARSER			=	$(MROOT)/parser/$O/libparser.a

# specific parser libraries

# common cover link parser
LIB_CCL				=	$(MROOT)/ccl/$O/libccl.a

#
# Targets
#

TARGETS				=	$(EXE_TARGET) $(LIB_TARGET)


SUBDIRSclean		= $(SUBDIRS:%=__%_clean)

##############################################################################
#                                                                            #
#                               R U L E S                                    #
#                                                                            #
##############################################################################

.PRECIOUS: $(SRCS)

#
# Standard makes
#

default subdir all: $(CREATE_DIRECTORIES) $(SUBDIRS) $(TARGETS)

#
# directory creation
#

$(CREATE_DIRECTORIES):
	-@ $(MKDIR) $@

#
# executable rule
#

$(EXE_TARGET): $(PRSLIBS) $(OBJS)
	$(CC) -o $@ $(OBJS) $(PRSLIBS) $(LINKER_FLAGS)

#
# library rule
#

$(LIB_TARGET): $(LIB_OBJS)
	$(AR) $@ $?

#
# object rule
#

$(O)/%.o: %.cpp
	$(CC) $(CPPFLAGS) -I. -c -o $@ $?

#
# clean
#
RMOBJS	=	$(O)/*.o $(O)/lib*.a

clean: $(SUBDIRSclean)
	-$(RM) $(RMOBJS) $(EXE_TARGET) $(OBJ_TARGET) $(OBJS) 

#
# Subdirectories
#
$(SUBDIRS): FRC
	$(CDMAKE) $@

$(SUBDIRSclean): FRC
	$(CDMAKE) $(patsubst __%_clean,%,$@) clean

#
# FRC rule
#
FRC:
