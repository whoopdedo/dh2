###############################################################################
##  Makefile
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

.SUFFIXES:
.SUFFIXES: .o .c .cpp .rc

srcdir = .
LGDIR = ../lg

CC = g++
AR = ar
LD = g++
DLLTOOL = dlltool
RC = windres

DEFINES = -DWINVER=0x0400 -D_WIN32_WINNT=0x0400 -DWIN32_LEAN_AND_MEAN
INCLUDES = -I. -I$(LGDIR)
LIBS = -luuid
ARFLAGS = rc
LDFLAGS = -mwindows -mdll -static-libgcc -static-libstdc++ -L$(LGDIR)
CFLAGS = -W -Wall -masm=intel -std=gnu++0x
DLLFLAGS = --add-underscore
ifdef DEBUG
CDEBUG = -g -O0
LDDEBUG = -g
LGLIB = -llg-d
else
CDEBUG = -O2
LDDEBUG =
LGLIB = -llg
endif

ALL = dh2.osl libdh2.a

DLLSRCS = dh2.cpp dh2dll.cpp dh2lib.cpp
DLLOBJS = dh2_exp.o dh2.o dh2dll.o dh2_res.o

.INTERMEDIATE:

LIBSRCS = dh2lib.cpp
LIBOBJS = dh2lib.o

%.o: %.c
	$(CC) $(CFLAGS) $(CDEBUG) $(DEFINES) $(INCLUDES) -o $@ -c $<

%.o: %.cpp
	$(CC) $(CFLAGS) $(CDEBUG) $(DEFINES) $(INCLUDES) -o $@ -c $<

all:	$(ALL)

clean:
	$(RM) $(ALL) $(DLLOBJS) $(LIBOBJS)

dh2_res.o: dh2.rc
	$(RC) $(DEFINES) -o $@ -i $<

dh2_exp.o: dh2dll.o
	$(DLLTOOL) $(DLLFLAGS) --dllname dh2.osl --output-exp $@ $^

dh2.osl: $(DLLOBJS)
	$(LD) $(LDFLAGS) $(LDDEBUG) -Wl,--image-base=0x12200000 -o $@ $^ $(LGLIB) $(LIBS)

libdh2.a: $(LIBOBJS)
	$(AR) $(ARFLAGS) $@ $?

paramres.o: params.rc
	$(RC) $(DEFINES) -o $@ -i $<

paramexp.o: paramdll.o
	$(DLLTOOL) $(DLLFLAGS) --output-exp $@ $^

params.osl: paramexp.o params.o paramdll.o paramres.o
	$(LD) $(LDFLAGS) -Wl,--image-base=0x12300000 -o $@ $^ $(LIBS)

libscriptparam.a: paramlib.o
	$(AR) $(ARFLAGS) $@ $?


dh2.o: dh2.cpp dh2.h darkhook.h objprop.h

dh2dll.o: dh2dll.cpp dh2.h darkhook.h

dh2lib.o: dh2lib.cpp darkhook.h

params.o: params.cpp params.h scriptparam.h strless_nocase.h

paramdll.o: paramdll.cpp params.h scriptparam.h

paramlib.o: paramlib.cpp scriptparam.h

