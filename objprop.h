/******************************************************************************
 *  objprop.h
 *
 *  This file is part of Dark Hook 2
 *  Copyright (C) 2005-2011 Tom N Harris <telliamed@whoopdedo.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/
#ifndef OBJPROP_H
#define OBJPROP_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <string>
#include <cstring>

class objprop
{
	char m_propname[32];
	int m_objid;

	friend struct objprop_hash;

public:
	~objprop()
		{ }
	objprop()
		{ m_propname[0] = '\0'; m_objid = 0; }
	objprop(const char * nm, int id)
		{ if (nm) ::strncpy(m_propname,nm,32); else m_propname[0] = '\0'; m_objid = id; }
	objprop(const std::string& nm, int id)
		{ nm.copy(m_propname,31); m_objid = id; }
	objprop& operator = (const objprop& cpy)
		{ ::memcpy(m_propname,cpy.m_propname,32); m_objid = cpy.m_objid; return *this; }

	bool operator == (const objprop& rhs) const
		{ return (m_objid == rhs.m_objid) && (::stricmp(m_propname,rhs.m_propname) == 0); }
	bool operator != (const objprop& rhs) const
		{ return (m_objid != rhs.m_objid) || (::stricmp(m_propname,rhs.m_propname) != 0); }
	bool operator < (const objprop& rhs) const
		{
			int __cmp = ::stricmp(m_propname,rhs.m_propname);
			if (__cmp != 0) return __cmp < 0;
			return m_objid < rhs.m_objid;
		}
	bool operator <= (const objprop& rhs) const
		{
			int __cmp = ::stricmp(m_propname,rhs.m_propname);
			if (__cmp != 0) return __cmp < 0;
			return m_objid <= rhs.m_objid;
		}
	bool operator > (const objprop& rhs) const
		{
			int __cmp = ::stricmp(m_propname,rhs.m_propname);
			if (__cmp != 0) return __cmp > 0;
			return m_objid > rhs.m_objid;
		}
	bool operator >= (const objprop& rhs) const
		{
			int __cmp = ::stricmp(m_propname,rhs.m_propname);
			if (__cmp != 0) return __cmp > 0;
			return m_objid >= rhs.m_objid;
		}
};

struct objprop_hash
{
	size_t operator() (const objprop& __val) const
	{
		size_t __h = 0;
		const char* __c = __val.m_propname;
		for (size_t __l=32; __l; --__l)
			__h = (__h * 131) + *__c++;
		return __h ^ __val.m_objid;
	}
};

#endif // OBJPROP_H
