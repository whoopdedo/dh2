/******************************************************************************
 *  params.h
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
/**********************
 * Script Params
 */

#ifndef PARAMS_H
#define PARAMS_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "scriptparam.h"
#include <lg/scrmanagers.h>
#include <lg/propdefs.h>
#include <lg/properties.h>
#include <lg/objects.h>
#include <lg/interface.h>
#include <lg/interfaceimp.h>

#include <unordered_map>
#include <string>
#include "strnocase.h"
#include <random>

class cScriptParamScriptService : public cInterfaceImp<IScriptParamScriptService>
{
private:
	typedef std::unordered_map<std::string, std::string, strnocase_hash, strnocase_equal> tParamEntryMap;
	typedef std::unordered_map<int, tParamEntryMap> tParamCacheMap;

public:
	STDMETHOD_(void,Init)(void);
	STDMETHOD_(void,End)(void);

	STDMETHOD(GetString)(int iObjId, const char* pszName, cScrStr& sValue, const char* pszDefault = NULL);
	STDMETHOD_(int,GetInt)(int iObjId, const char* pszName, int iDefault = 0);
	STDMETHOD_(float,GetFloat)(int iObjId, const char* pszName, float fDefault = 0);
	STDMETHOD_(Bool,GetBool)(int iObjId, const char* pszName, Bool bDefault = FALSE);
	STDMETHOD(GetVec)(int iObjId, const char* pszName, cScrVec& vValue);
	STDMETHOD(Set)(int iObjId, const char* pszName, const cMultiParm& mpValue);
	STDMETHOD(Unset)(int iObjId, const char* pszName);
	STDMETHOD(Delete)(int iObjId, const char* pszName);
	STDMETHOD_(Bool,IsRelevant)(int iObjId, const char* pszName);
	STDMETHOD_(Bool,Exists)(int iObjId, const char* pszName);
	STDMETHOD_(int,ToTime)(const char* pszValue);
	STDMETHOD_(int,ToColor)(const char* pszValue);
	STDMETHOD_(int,ToObject)(const char* pszValue, int iDest, int iSource = 0);
	STDMETHOD_(IObjectQuery*,QueryObjects)(const char* pszValue, int iDest);

	cScriptParamScriptService(IUnknown* pIFace);
	virtual ~cScriptParamScriptService();

	static bool sm_initialized;

private:
	SInterface<IStringProperty> m_pDNProp;
	SInterface<IPositionProperty> m_pPosProp;
	SInterface<ILinkManager> m_pLinkMan;
	SInterface<IObjectSystem> m_pObjMan;
	SInterface<ITraitManager> m_pTraitMan;
	SInterface<ISimManager> m_pSimMan;

	std::default_random_engine m_rand;
	tParamCacheMap m_mapParamCache;
	PropListenerHandle m_hListenerHandle;
	int m_iUpdatingObj;
	bool m_bEnabled;

	static sDispatchListenerDesc sm_simlistenerdesc;

	static int __cdecl SimListener(const sDispatchMsg* pSimMsg, const sDispatchListenerDesc* pData);
	static void __stdcall PropertyListener(sPropertyListenMsg* pPropMsg, PropListenerData pData);

	void Enable(void);
	void Disable(void);

	void Parse(tParamEntryMap& cache, const char* pszDN);
	std::string Unparse(tParamEntryMap& cache);
	tParamEntryMap* Read(int iObj);
	const std::string* ReadParam(int iObj, const std::string& sParamName);
	void Write(int iObj);
	const std::string* RetrieveCached(int iObj, const std::string& sParamName);
	const std::string* Retrieve(int iObj, const std::string& sParamName);
	const std::string* RetrieveSingle(int iObj, const std::string& sParamName);
	void Update(int iObj, const std::string& sParamName, const std::string* sParamValue);
	void Remove(int iObj, const std::string& sParamName);
	void Touch(int iObj);
	void Reset(void);

	int ObjectNamed(const char* pszName, int iDefault);
	int FindOneClosest(int iObj, const char* pszName);
	int FindOneLink(int iFrom, const char *pszLink);
	int FindAnyLink(int iFrom, const char *pszLink);
	IObjectQuery* FindAllLinks(int iFrom, const char *pszLink);
	IObjectQuery* FindAllObjects(int iFlags, const char* pszName);
	IObjectQuery* FindAllClosest(int iObj, const char* pszName);
};

class ClosestObjFilter
{
public:
	bool operator() (int iObj) const;
	bool UpdateIfCloser(int iObj);
	bool Valid() const { return m_iObj != 0; }

	ClosestObjFilter(int iObj, float fMaxDistance, SInterface<IPositionProperty>& pPosProp);
	ClosestObjFilter(const ClosestObjFilter& rCpy)
		: m_iObj(rCpy.m_iObj), m_fMaxSquared(rCpy.m_fMaxSquared), m_vPosition(rCpy.m_vPosition),
		  m_pPosProp(rCpy.m_pPosProp)
	{ }

protected:
	int m_iObj;
	float m_fMaxSquared;
	cScrVec m_vPosition;
	SInterface<IPositionProperty> m_pPosProp;

	float Distance(int iObj) const;

};

class ClosestZOffsetObjFilter
{
public:
	bool operator() (int iObj) const;
	bool UpdateIfCloser(int iObj);

	ClosestZOffsetObjFilter(int iObj, float fMaxDistance, float fMaxHeight, SInterface<IPositionProperty>& pPosProp);
	ClosestZOffsetObjFilter(const ClosestZOffsetObjFilter& rCpy)
		: m_iObj(rCpy.m_iObj), m_fMaxSquared(rCpy.m_fMaxSquared), m_fMaxHeight(rCpy.m_fMaxHeight),
		  m_vPosition(rCpy.m_vPosition), m_pPosProp(rCpy.m_pPosProp)
	{ }

protected:
	int m_iObj;
	float m_fMaxSquared, m_fMaxHeight;
	cScrVec m_vPosition;
	SInterface<IPositionProperty> m_pPosProp;

	float Distance(int iObj, float *fHeight) const;

};

typedef int (__cdecl *MPrintfProc)(const char*, ...);
extern MPrintfProc g_pfnMPrintf;

#endif // PARAMS_H
