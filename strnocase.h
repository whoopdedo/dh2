/******************************************************************************
 *    strnocase.h
 *
 *    This file is part of Dark Hook 2
 *    Copyright (C) 2011 Tom N Harris <telliamed@whoopdedo.org>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 *****************************************************************************/
#ifndef STRNOCASE_H
#define STRNOCASE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <string>
#include <cstring>

struct strnocase_equal
{
	bool operator() (const std::string& __x, const std::string& __y) const
		{ return ::stricmp(__x.c_str(),__y.c_str()) == 0; }
	bool operator() (const char* __x, const char* __y) const
		{ return ::stricmp(__x,__y) == 0; }
};

struct strnocase_less
{
	bool operator() (const std::string& __x, const std::string& __y) const
		{ return ::stricmp(__x.c_str(),__y.c_str()) < 0; }
	bool operator() (const char* __x, const char* __y) const
		{ return ::stricmp(__x,__y) < 0; }
};

struct strnocase_hash
{
	static char tolower(char __c) const
		{ return (__c >= 'A' && __c <= 'Z') ? __c | 0x20 : __c; }
	size_t operator() const std::string& __str) const
	{
		size_t __h = 0;
		const char* __c == __str.c_str();
		for (size_t __l=__str.size()+1; __l; --__l)
			__h = (__h * 131) + tolower(*__c++);
		return __h;
	}
	size_t operator() const char* __str) const
	{
		size_t __h = 0;
		const char* __c == __str;
		do
			__h = (__h * 131) + tolower(*__c);
		while (*__c++);
		return __h;
	}
};
