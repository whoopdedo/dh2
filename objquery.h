/******************************************************************************
 *  objquery.h
 *
 *  This file is part of Dark Hook 2
 *  Copyright (C) 2011 Tom N Harris <telliamed@whoopdedo.org>
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
#ifndef OBJQUERY_H
#define OBJQUERY_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <lg/config.h>
#include <lg/objstd.h>
#include <lg/interface.h>
#include <lg/interfaceimp.h>
#include <lg/types.h>
#include <lg/objects.h>
#include <lg/links.h>

class cObjQuery : public cInterfaceImp<IObjectQuery>
{
public:
	STDMETHOD_(Bool,Done)(void)
	{
		return (m_iObj == 0);
	}
	STDMETHOD_(int,Object)(void)
	{
		return m_iObj;
	}
	STDMETHOD(Next)(void) PURE;

protected:
	int m_iObj;
};

class cConstObjQuery : public cObjQuery
{
public:
	STDMETHOD(Next)(void)
	{
		m_iObj = 0;
		return S_OK;
	}

	cConstObjQuery(int iObj) { m_iObj = iObj; }

};

class cConcreteObjQuery : public cObjQuery
{
private:
	SInterface<IObjectQuery> m_pQuery;

	void GetNext(void)
	{
		m_iObj = 0;
		for (; !m_pQuery->Done(); m_pQuery->Next())
		{
			int iObj = m_pQuery->Object();
			if (iObj > 0)
			{
				m_iObj = iObj;
				break;
			}
		}
	}

public:
	STDMETHOD(Next)(void)
	{
		if (m_pQuery && !m_pQuery->Done())
		{
			m_pQuery->Next();
			GetNext();
		}
		else
			m_iObj = 0;
		return S_OK;
	}

	cConcreteObjQuery(IObjectQuery* pQuery)
		: m_pQuery(pQuery)
	{
		m_iObj = 0;
		if (m_pQuery)
			GetNext();
	}

};

class cLinkObjQuery : public cObjQuery
{
private:
	SInterface<ILinkQuery> m_pQuery;

public:
	STDMETHOD(Next)(void)
	{
		if (m_pQuery && !m_pQuery->Done())
		{
			sLink sl;
			m_pQuery->Next();
			m_pQuery->Link(&sl);
			m_iObj = sl.dest;
		}
		else
			m_iObj = 0;
		return S_OK;
	}

	cLinkObjQuery(ILinkQuery* pQuery)
		: m_pQuery(pQuery)
	{
		m_iObj = 0;
		if (m_pQuery && !m_pQuery->Done())
		{
			sLink sl;
			m_pQuery->Link(&sl);
			m_iObj = sl.dest;
		}
	}
};

template <class TFilter>
class cFilterObjQuery : public cObjQuery
{
private:
	SInterface<IObjectQuery> m_pQuery;
	TFilter m_filter;

	void GetNext(void)
	{
		m_iObj = 0;
		for (; !m_pQuery->Done(); m_pQuery->Next())
		{
			int iObj = m_pQuery->Object();
			if (m_filter(iObj))
			{
				m_iObj = iObj;
				break;
			}
		}
	}

public:
	STDMETHOD(Next)(void)
	{
		if (m_pQuery && !m_pQuery->Done())
		{
			m_pQuery->Next();
			GetNext();
		}
		else
			m_iObj = 0;
		return S_OK;
	}

	cFilterObjQuery(IObjectQuery* pQuery, const TFilter& filter)
		: m_pQuery(pQuery), m_filter(filter)
	{
		m_iObj = 0;
		if (m_pQuery)
			GetNext();
	}
};

#endif // OBJQUERY_H
