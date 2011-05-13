###############################################################################
##  Makefile-b32
##
##  This file is part of Dark Hook 2
##  Copyright (C) 2005-2011 Tom N Harris <telliamed@whoopdedo.org>
##
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program. If not, see <http://www.gnu.org/licenses/>.
##
###############################################################################

.autodepend
.cacheautodepend

.suffixes:
.suffixes: .o .cpp .rc .res

BCCROOT = D:\BCC

IMPLIB = $(BCCROOT)\bin\Implib
ILINK32 = $(BCCROOT)\bin\ILink32
TLIB = $(BCCROOT)\bin\TLib
BRC32 = $(BCCROOT)\bin\Brc32
TASM32 = $(BCCROOT)\bin\Tasm32
CC = $(BCCROOT)\bin\BCC32

srcdir = .
LGDIR = ..\lg

DEFINES = -DWINVER=0x0400 -D_WIN32_WINNT=0x0400 -DWIN32_LEAN_AND_MEAN
INCLUDES = -I. -I$(LGDIR)
LIBS = lg.lib uuid.lib
LDFLAGS = -q -aa -Tpd -c -b:0x12200000
LIBDIRS = -L.;$(LGDIR);$(BCCROOT)\lib;$(BCCROOT)\lib\psdk
CXXFLAGS =  -q -P -tWD -tWM
RCFLAGS = -r

BCC32STARTUP = c0d32.obj
BCC32RTLIB = cw32mt.lib
BCCJUNK = *.il* *.csm *.tds *.map

ALL = dh2.osl dh2.lib

DLLSRCS = dh2.cpp dh2dll.cpp dh2lib.cpp
DLLOBJS = dh2.obj dh2dll.obj
DLLRES = dh2.res

LIBSRCS = dh2lib.cpp
LIBOBJS = dh2lib.obj

.c.obj:
	$(CC) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $<

.cpp.obj:
	$(CC) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $<

.rc.res:
	$(BRC32) -r $<

all:	$(ALL)

clean:
	del /q $(ALL) $(DLLOBJS) $(DLLRES) $(LIBOBJS) $(BCCJUNK)

dh2.osl: $(DLLOBJS) $(DLLRES)
	$(ILINK32) $(LDFLAGS) $(LIBDIRS) $(BCC32STARTUP) $(DLLOBJS), $@ ,,import32.lib $(LIBS) $(BCC32RTLIB) , , $(DLLRES)

dh2.lib: $(LIBOBJS)
	del /q $@
	$(TLIB) /a $@ $?


dh2.obj: dh2.cpp dh2.h darkhook.h objprop.h

dh2dll.obj: dh2dll.cpp dh2.h darkhook.h

dh2lib.obj: dh2lib.cpp darkhook.h

