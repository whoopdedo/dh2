/******************************************************************************
 *  params.cpp
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
#define INITGUID 1
#include "params.h"

#include <lg/interface.h>
#include <lg/propdefs.h>

#include <algorithm>
#include <memory>
#include <cstring>
#include <cctype>

using namespace std;

sDispatchListenerDesc cScriptParamScriptService::sm_simlistenerdesc = {
	&IID_IScriptParamScriptService,
	0xF,
	SimListener,
	NULL
};
bool cScriptParamScriptService::sm_initialized = false;

int __cdecl cScriptParamScriptService::SimListener(const sDispatchMsg* pSimMsg, const sDispatchListenerDesc* pData)
{
	g_pfnMPrintf("DH2(%s): SimMsg %lu %lu\n",
		reinterpret_cast<cScriptParamScriptService*>(pData->pData)->m_bEnabled?"on":"off",
		pSimMsg->dwEventId, pSimMsg->dwDispatchId);
	if (sm_initialized)
	{
		switch (pSimMsg->dwEventId)
		{
			case kSimStart:
				//reinterpret_cast<cScriptParamScriptService*>(pData->pData)->Enable();
				break;
			case kSimStop:
				reinterpret_cast<cScriptParamScriptService*>(pData->pData)->Reset();
				break;
		}
	}
	return 0;
}

void __stdcall cScriptParamScriptService::PropertyListener(sPropertyListenMsg* pPropMsg, PropListenerData pData)
{
	g_pfnMPrintf("DH2(%s): PropMsg %u %08lX %d:%d\n",
		reinterpret_cast<cScriptParamScriptService*>(pData)->m_bEnabled?"on":"off",
		pPropMsg->event, reinterpret_cast<ulong>(pPropMsg->pData), pPropMsg->iObjId, pPropMsg->iArchetype);
	if (sm_initialized)
	{
		if (!(pPropMsg->event & 8))
		{
			reinterpret_cast<cScriptParamScriptService*>(pData)->Touch(pPropMsg->iObjId);
		}
		else
		{
			reinterpret_cast<cScriptParamScriptService*>(pData)->Reset();
		}
	}
}

cScriptParamScriptService::~cScriptParamScriptService()
{
	Disable();

	m_pSimMan->Unlisten(&IID_IScriptParamScriptService);
	m_pDNProp->Unlisten(m_hListenerHandle);

	sm_initialized = false;
}

cScriptParamScriptService::cScriptParamScriptService(IUnknown* pIFace)
	: m_pLinkMan(pIFace), m_pObjMan(pIFace), m_pTraitMan(pIFace), m_pSimMan(pIFace)
{
	SInterface<IPropertyManager> pPropMan(pIFace);
	SInterface<IPositionProperty> pPosProp = static_cast<IPositionProperty*>(pPropMan->GetPropertyNamed("Position"));
	if (!pPosProp)
		throw no_interface("IID_IPositionProperty");
	m_pPosProp = pPosProp;
	SInterface<IStringProperty> pDNProp = static_cast<IStringProperty*>(pPropMan->GetPropertyNamed("DesignNote"));
	if (!pDNProp)
		throw no_interface("IID_IStringProperty");
	m_pDNProp = pDNProp;

	sm_simlistenerdesc.pData = static_cast<void*>(this);
	m_pSimMan->Listen(&sm_simlistenerdesc);
	m_hListenerHandle = m_pDNProp->Listen(kPropertyFull, PropertyListener, reinterpret_cast<PropListenerData>(this));

	m_iUpdatingObj = 0;
	m_bEnabled = true;

	sm_initialized = true;
}

void cScriptParamScriptService::Enable(void)
{
	m_bEnabled = true;
	Reset();
}

void cScriptParamScriptService::Disable(void)
{
	if (m_bEnabled)
	{
		Reset();
		m_bEnabled = false;
	}
}

// BaseScriptService functions
STDMETHODIMP_(void) cScriptParamScriptService::Init(void)
{
}

STDMETHODIMP_(void) cScriptParamScriptService::End(void)
{
}

// ScriptParamScriptService
STDMETHODIMP cScriptParamScriptService::GetString(int iObjId, const char* pszName, cScrStr& sValue, const char* pszDefault)
{
	try
	{
		const string* psParam = Retrieve(iObjId, pszName);
		if (psParam)
		{
			sValue.Copy(psParam->c_str());
			return S_OK;
		}
		else if (pszDefault)
			sValue.Copy(pszDefault);
		else
			sValue.Copy("");
		return S_FALSE;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

STDMETHODIMP_(int) cScriptParamScriptService::GetInt(int iObjId, const char* pszName, int iDefault)
{
	try
	{
		const string* psParam = Retrieve(iObjId, pszName);
		if (!psParam)
			return iDefault;
		return strtol(psParam->c_str(), NULL, 0);
	}
	catch (...)
	{
		return 0;
	}
}

STDMETHODIMP_(float) cScriptParamScriptService::GetFloat(int iObjId, const char* pszName, float fDefault)
{
	try
	{
		const string* psParam = Retrieve(iObjId, pszName);
		if (!psParam)
			return fDefault;
		return strtod(psParam->c_str(), NULL);
	}
	catch (...)
	{
		return 0;
	}
}

STDMETHODIMP_(Bool) cScriptParamScriptService::GetBool(int iObjId, const char* pszName, Bool bDefault)
{
	try
	{
		const string* psParam = Retrieve(iObjId, pszName);
		if (!psParam)
			return bDefault;
		if (psParam->size() == 0)
			return FALSE;
		if (psParam->size() == 1)
		{
			if (isdigit(psParam->at(0)))
				return psParam->at(0) != '0';
			switch (psParam->at(0))
			{
			case 't': case 'T': case 'y': case 'Y':
				return TRUE;
			default:
				return FALSE;
			}
		}
		if (0 == stricmp(psParam->c_str(), "TRUE"))
			return TRUE;
		if (0 == stricmp(psParam->c_str(), "YES"))
			return TRUE;
		char* end = NULL;
		int val = strtol(psParam->c_str(), &end, 0);
		if (end && *end == '\0')
			return val != 0;
		return FALSE;
	}
	catch (...)
	{
		return 0;
	}
}

STDMETHODIMP cScriptParamScriptService::GetVec(int iObjId, const char* pszName, cScrVec& vValue)
{
	try
	{
		const string* psParam = Retrieve(iObjId, pszName);
		if (psParam)
		{
			float x,y,z;
			if (3 == sscanf(psParam->c_str(),
				(psParam->at(0)=='('?"( %f , %f , %f )":"%f , %f , %f"),
				&x, &y, &z))
			{
				vValue.x = x;
				vValue.y = y;
				vValue.z = z;
				return S_OK;
			}
		}
		vValue.x = 0;
		vValue.y = 0;
		vValue.z = 0;
		return S_FALSE;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

STDMETHODIMP cScriptParamScriptService::Set(int iObjId, const char* pszName, const cMultiParm& mpValue)
{
	try
	{
		if (mpValue.type == kMT_Undef)
			return E_INVALIDARG;
		std::string sVal = static_cast<const char*>(mpValue);
		Update(iObjId, pszName, &sVal);
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

STDMETHODIMP cScriptParamScriptService::Unset(int iObjId, const char* pszName)
{
	try
	{
		Update(iObjId, pszName, NULL);
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

STDMETHODIMP cScriptParamScriptService::Delete(int iObjId, const char* pszName)
{
	try
	{
		Remove(iObjId, pszName);
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

STDMETHODIMP_(Bool) cScriptParamScriptService::IsRelevant(int iObjId, const char* pszName)
{
	try
	{
		const string* psParam = RetrieveSingle(iObjId, pszName);
		return psParam != NULL;
	}
	catch (...)
	{
		return 0;
	}
}

STDMETHODIMP_(Bool) cScriptParamScriptService::Exists(int iObjId, const char* pszName)
{
	try
	{
		const string* psParam = Retrieve(iObjId, pszName);
		return psParam != NULL;
	}
	catch (...)
	{
		return 0;
	}
}

STDMETHODIMP_(int) cScriptParamScriptService::ToTime(const char* pszValue)
{
	char* end = NULL;
	double t = strtod(pszValue, &end);
	if (t <= 0)
		return 0;
	if (end)
	{
		if ((*end | 0x20) == 'm')
			t *= 60000;
		else if ((*end | 0x20) == 's')
			t *= 1000;
	}
	return int(t);
}

static const unsigned char g_cHexTable[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
	0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

struct colorname
{
	const char* n;
	unsigned long c;
	bool operator== (const char* _rhs) const
		{ return (stricmp(n, _rhs) == 0); }
	bool operator< (const char* _rhs) const
		{ return (stricmp(n, _rhs) < 0); }
};
static const colorname g_sColorNames[] = {
	{ "aqua",	0xFFFF00 },
	{ "black",	0x080808 },
	{ "blue",	0xFF0000 },
	{ "chartreuse",	0x80FF80 },
	{ "dodger",	0x0080FF },
	{ "fuchsia",	0xFF00FF },
	{ "gold",	0x80FFFF },
	{ "gray",	0x808080 },
	{ "green",	0x008000 },
	{ "grey",	0x808080 },
	{ "lawn",	0x00FF80 },
	{ "lime",	0x00FF00 },
	{ "maroon",	0x000080 },
	{ "navy",	0x800000 },
	{ "olive",	0x008080 },
	{ "orange",	0x0080FF },
	{ "orchid",	0xFF80FF },
	{ "pink",	0x8000FF },
	{ "purple",	0x800080 },
	{ "red",	0x0000FF },
	{ "rose",	0x8080FF },
	{ "silver",	0xC0C0C0 },
	{ "slate",	0xFF8080 },
	{ "spring",	0x80FF00 },
	{ "steel",	0xFF8000 },
	{ "teal",	0x808000 },
	{ "turquoise",	0xFFFF80 },
	{ "violet",	0xFF0080 },
	{ "white",	0xFFFFFF },
	{ "yellow",	0x00FFFF }
};
static const unsigned int g_iNumColors = sizeof(g_sColorNames)/sizeof(g_sColorNames[0]);
static const colorname* g_sColorNameLast = g_sColorNames + g_iNumColors;

STDMETHODIMP_(int) cScriptParamScriptService::ToColor(const char* pszValue)
{
	const unsigned char* c = reinterpret_cast<const unsigned char*>(pszValue);
	if (*c == '#')
	{
		if (strlen(reinterpret_cast<const char*>(++c)) < 6)
			return 0;
		unsigned char r,g,b;
		r  = g_cHexTable[*c++] << 4;
		r |= g_cHexTable[*c++];
		g  = g_cHexTable[*c++] << 4;
		g |= g_cHexTable[*c++];
		b  = g_cHexTable[*c++] << 4;
		b |= g_cHexTable[*c];
		return (r|(g<<8)|(b<<16));
	}
	if (strchr(pszValue, ','))
	{
		unsigned int r,g,b;
		if (3 == sscanf(pszValue, "%u , %u , %u", &r, &g, &b))
		{
			return ((r&0xFF)|((g&0xFF)<<8)|((b&0xFF)<<16));
		}
	}
	else
	{
		const colorname* clr = lower_bound(g_sColorNames, g_sColorNameLast, pszValue);
		if (clr != g_sColorNameLast && *clr == pszValue)
			return clr->c;
	}
	return 0;
}

int cScriptParamScriptService::FindClosest(int iObj, const char* pszName)
{
	int iFound = 0;
	float fDistance = numeric_limits<float>::infinity();
	cScrVec vObj;
	sPosition* pos;
	if (!m_pPosProp->Get(iObj, &pos))
		return 0;
	vObj = pos->Location;
	int iArc = m_pObjMan->GetObjectNamed(pszName);
	if (iArc == 0)
		return 0;
	SInterface<IObjectQuery> pQuery = m_pTraitMan->Query(iArc, kTraitQueryFull|kTraitQueryChildren);
	if (!pQuery)
		return 0;
	for (; !pQuery->Done(); pQuery->Next())
	{
		int obj = pQuery->Object();
		if (obj > 0)
		{
			if (m_pPosProp->Get(obj, &pos))
			{
				if (pos->Cell == -1)
					continue;
				float dist = (vObj - pos->Location).MagSquared();
				if (dist < fDistance)
				{
					fDistance = dist;
					iFound = obj;
				}
			}
		}
	}
	return iFound;
}

int cScriptParamScriptService::ObjectNamed(const char* pszName, int iDefault)
{
	int obj = m_pObjMan->GetObjectNamed(pszName);
	if (obj == 0)
	{
		char* end = NULL;
		obj = strtol(pszName, &end, 10);
		if (obj == 0 && end && *end == 0)
			obj = iDefault;
		else if (!m_pObjMan->Exists(obj))
			obj = 0;
	}
	return obj;
}

STDMETHODIMP_(int) cScriptParamScriptService::ToObject(const char* pszValue, int iDest, int iSource)
{
	try
	{
		if (!pszValue || !*pszValue)
			return iDest;
		int obj;
		if (pszValue[0] == '^')
		{
			obj = FindClosest(iDest, pszValue+1);
		}
		else
		{
			if (!stricmp(pszValue, "self"))
				obj = iDest;
			else if (!stricmp(pszValue, "source"))
				obj = iSource;
			else
				obj = ObjectNamed(pszValue, iDest);
		}
		return obj;
	}
	catch (...)
	{
		return 0;
	}
}

STDMETHODIMP_(IObjectQuery*) cScriptParamScriptService::QueryObjects(const char* pszValue, int iDest)
{
	try
	{
		return NULL;
	}
	catch (...)
	{
		return NULL;
	}
}

//

static char* strqsep(char** str)
{
	char* p = *str;
	if (!p) return NULL;
	while (*p)
	{
		if (*p == '\"' || *p == '\'')
		{
			char* t = p;
			do
			{
				t = strchr(t+1, *p);
				if (!t)
				{
					p = *str;
					*str = NULL;
					return p;
				}
			}
			while (*(t-1) == '\\');
			p = t;
		}
		else if (*p == ';')
		{
			char* t = *str;
			*p++ = '\0';
			*str = p;
			return t;
		}
		++p;
	}
	p = *str;
	*str = NULL;
	return p;
}

void cScriptParamScriptService::Parse(tParamEntryMap& cache, const char* pszDN)
{
	if (!pszDN)
		return;

	auto_ptr<char> szString(new char[strlen(pszDN) + 1]);
	strcpy(szString.get(), pszDN);
	char* pszSep, *pszToken;
	for (pszSep = szString.get(), pszToken = strqsep(&pszSep);
	     pszToken; pszToken = strqsep(&pszSep))
	{
		while (isspace(*pszToken)) ++pszToken;
		if (*pszToken == '\0' || *pszToken == '=')
			continue;
		char* pszStart = strchr(pszToken, '=');
		if (pszStart)
		{
			*pszStart++ = '\0';
			if (*pszStart == '!')
			{
				// out-of-bound data trick
				cache.insert(make_pair(string(pszToken),string(1,0)));
			}
			else
			{
				while (isspace(*pszStart)) ++pszStart;
				char* pszEnd;
				if (*pszStart == '\"' || *pszStart == '\'')
				{
					char* pszChar = pszStart + 1;
					pszEnd = pszChar;
					while (*pszChar)
					{
						if (*pszChar == '\\')
						{
							if (*(pszChar+1) == *pszStart
							 || *(pszChar+1) == '\\')
							{
								++pszChar;
							}
						}
						else if (*pszChar == *pszStart)
							break;
						*pszEnd++ = *pszChar++;
					}
					*pszEnd = '\0';
					++pszStart;
				}
				else
					pszEnd = pszStart + strlen(pszStart);
				cache.insert(make_pair(string(pszToken),string(pszStart,pszEnd)));
			}
		}
		else
		{
			cache.insert(make_pair(string(pszToken),string()));
		}
	}
}

string cScriptParamScriptService::Unparse(tParamEntryMap& cache)
{
	string sDN;
	size_t len = 0;
	for (tParamEntryMap::const_iterator param = cache.begin();
	    param != cache.end(); ++param)
	{
		// first="second" + extra if escapes are needed
		len += param->first.size() + param->second.size() + 5;
	}
	sDN.reserve(len);
	for (tParamEntryMap::const_iterator param = cache.begin();
	     param != cache.end(); ++param)
	{
		const string& value = param->second;
		sDN.append(param->first);
		sDN.append(1, '=');
		if (value.size() == 1 && value.at(0) == '\0')
		{
			// out-of-bound data trick
			sDN.append(1, '!');
		}
		else if (value.size() == 0)
		{
			sDN.append(2, '"');
		}
		else
		{
			if (string::npos == value.find_first_of(";\"'\\ !"))
			{
				sDN.append(value);
			}
			else
			{
				if (string::npos == value.find('"'))
				{
					sDN.append(1, '"');
					size_t start = 0;
					size_t pos;
					while (string::npos != (pos = value.find_first_of("\"\\", start)))
					{
						sDN.append(value, start, start-pos);
						sDN.append(1, '\\');
						sDN.append(value, pos, 1);
						start = pos + 1;
					}
					sDN.append(value, start, string::npos);
					sDN.append(1, '"');
				}
				else
				{
					sDN.append(1, '\'');
					size_t start = 0;
					size_t pos;
					while (string::npos != (pos = value.find_first_of("\'\\", start)))
					{
						sDN.append(value, start, start-pos);
						sDN.append(1, '\\');
						sDN.append(value, pos, 1);
						start = pos + 1;
					}
					sDN.append(value, start, string::npos);
					sDN.append(1, '\'');
				}
			}
		}
		sDN.append(1, ';');
	}
	return sDN;
}

cScriptParamScriptService::tParamEntryMap* cScriptParamScriptService::Read(int iObj)
{
	if (m_pDNProp->IsSimplyRelevant(iObj))
	{
		const char* pszDesignNote;
		m_pDNProp->GetSimple(iObj, &pszDesignNote);

		tParamEntryMap& entry = m_mapParamCache[iObj];
		Parse(entry, pszDesignNote);
		return &entry;
	}
	return NULL;
}

const string* cScriptParamScriptService::ReadParam(int iObj, const string& sParamName)
{
	const string* psParam = NULL;
	tParamEntryMap* entry = Read(iObj);
	if (entry)
	{
		tParamEntryMap::const_iterator param_entry = entry->find(sParamName);
		if (param_entry != entry->end())
		{
			psParam = &(param_entry->second);
		}
	}
	return psParam;
}

void cScriptParamScriptService::Write(int iObj)
{
	m_iUpdatingObj = iObj;
	try
	{
		tParamCacheMap::iterator entry = m_mapParamCache.find(iObj);
		if (entry != m_mapParamCache.end())
		{
			string sDN = Unparse(entry->second);
			m_pDNProp->Set(iObj, sDN.c_str());
		}
	}
	catch (exception& e)
	{
		m_iUpdatingObj = 0;
		throw e;
	}
	m_iUpdatingObj = 0;
}

const string* cScriptParamScriptService::RetrieveCached(int iObj, const string& sParamName)
{
	const string* psParam = NULL;
	if (m_bEnabled)
	{
		tParamCacheMap::const_iterator cached_entry = m_mapParamCache.find(iObj);
		if (cached_entry != m_mapParamCache.end())
		{
			tParamEntryMap::const_iterator param_entry = cached_entry->second.find(sParamName);
			if (param_entry != cached_entry->second.end())
			{
				psParam = &(param_entry->second);
			}
		}
		else
		{
			psParam = ReadParam(iObj, sParamName);
		}
	}
	else // not enabled
	{
		psParam = ReadParam(iObj, sParamName);
	}
	return psParam;
}

const string* cScriptParamScriptService::Retrieve(int iObj, const string& sParamName)
{
	const string* psParam = NULL;
	// Trait manager does the recursion for us.
	// How convenient that the query object is included in the result set
	// (But only for MetaProp+Full, go figure.)
	SInterface<IObjectQuery> pInheritance = m_pTraitMan->Query(iObj, kTraitQueryMetaProps|kTraitQueryFull);
	if (!pInheritance)
		return NULL;
	for (; ! pInheritance->Done(); pInheritance->Next())
	{
		psParam = RetrieveCached(pInheritance->Object(), sParamName);
		if (psParam)
			break;
	}
	if (psParam)
	{
		// Check for the block-inheritance sentinal.
		if (psParam->size() == 1 && psParam->at(0) == '\0')
			return NULL;
	}
	return psParam;
}

const string* cScriptParamScriptService::RetrieveSingle(int iObj, const string& sParamName)
{
	const string* psParam = RetrieveCached(iObj, sParamName);
	if (psParam)
	{
		// Check for the block-inheritance sentinal.
		if (psParam->size() == 1 && psParam->at(0) == '\0')
			return NULL;
	}
	return psParam;
}

void cScriptParamScriptService::Update(int iObj, const string& sParamName, const string* sParamValue)
{
	tParamEntryMap* cache;
	if (m_bEnabled)
	{
		tParamCacheMap::iterator cached_entry = m_mapParamCache.find(iObj);
		if (cached_entry != m_mapParamCache.end())
		{
			cache = &(cached_entry->second);
		}
		else
		{
			cache = &(m_mapParamCache[iObj]);
			Read(iObj);
		}
	}
	else // not enabled
	{
		cache = &(m_mapParamCache[iObj]);
		Read(iObj);
	}

	(*cache)[sParamName] = (sParamValue != NULL) ? *sParamValue : string(1,0);
	Write(iObj);
}

void cScriptParamScriptService::Remove(int iObj, const string& sParamName)
{
	tParamEntryMap* cache;
	if (m_bEnabled)
	{
		tParamCacheMap::iterator cached_entry = m_mapParamCache.find(iObj);
		if (cached_entry != m_mapParamCache.end())
		{
			cache = &(cached_entry->second);
		}
		else
		{
			cache = &(m_mapParamCache[iObj]);
			Read(iObj);
		}
	}
	else // not enabled
	{
			cache = &(m_mapParamCache[iObj]);
			Read(iObj);
	}

	cache->erase(sParamName);
	Write(iObj);
}

void cScriptParamScriptService::Touch(int iObj)
{
	if (m_bEnabled && iObj != m_iUpdatingObj)
	{
		tParamCacheMap::iterator cached_entry = m_mapParamCache.find(iObj);
		if (cached_entry != m_mapParamCache.end())
		{
			m_mapParamCache.erase(cached_entry);
		}
	}
}

void cScriptParamScriptService::Reset(void)
{
	m_mapParamCache.clear();
}
