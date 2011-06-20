/******************************************************************************
 *  scriptparam.h
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
 * Script Params interface header
 */

#ifndef SCRIPTPARAM_H
#define SCRIPTPARAM_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <lg/config.h>
#include <lg/objstd.h>
#include <lg/iiddef.h>
#include <lg/types.h>

interface IMalloc;
interface IScriptMan;
interface IObjectQuery;

interface IScriptParamScriptService : IUnknown
{
	// BaseScriptService
	STDMETHOD_(void,Init)(void) PURE;
	STDMETHOD_(void,End)(void) PURE;

	// Fetch a parameter. Caller is responsible for freeing the string.
	STDMETHOD(GetString)(int iObjId, const char* pszName, cScrStr& sValue, const char* pszDefault = NULL) PURE;
	// Fetch parameter as integer.
	STDMETHOD_(int,GetInt)(int iObjId, const char* pszName, int iDefault = 0) PURE;
	// Fetch parameter as float.
	STDMETHOD_(float,GetFloat)(int iObjId, const char* pszName, float fDefault = 0) PURE;
	// Fetch parameter as boolean.
	STDMETHOD_(Bool,GetBool)(int iObjId, const char* pszName, Bool bDefault = FALSE) PURE;
	// Fetch parameter as vector.
	STDMETHOD(GetVec)(int iObjId, const char* pszName, cScrVec& vValue) PURE;
	// Fetch parameter and convert using ToTime.
	STDMETHOD_(int,GetTime)(int iObjId, const char* pszName, int iDefault = 0) PURE;
	// Fetch parameter and convert using ToColor.
	STDMETHOD_(int,GetColor)(int iObjId, const char* pszName, int iDefault = 0) PURE;
	// Fetch parameter and convert using ToObject.
	STDMETHOD_(int,GetObject)(int iObjId, const char* pszName, int iDest, int iSource = 0, int iDefault = 0) PURE;
	// Fetch parameter and convert using QueryObjects.
	STDMETHOD_(IObjectQuery*,GetManyObjects)(int iObjId, const char* pszName, int iDest) PURE;
	// Modify parameter.
	STDMETHOD(Set)(int iObjId, const char* pszName, const cMultiParm& mpValue) PURE;
	// Mark a parameter as not-set without deleting from an archetype.
	STDMETHOD(Unset)(int iObjId, const char* pszName) PURE;
	// Removes a parameter, even from an archetype.
	STDMETHOD(Delete)(int iObjId, const char* pszName) PURE;
	// Check if a parameter is set directly on the object.
	STDMETHOD_(Bool,IsRelevant)(int iObjId, const char* pszName) PURE;
	// Check if a parameter is set.
	STDMETHOD_(Bool,Exists)(int iObjId, const char* pszName) PURE;
	// Convert a string to a time value.
	STDMETHOD_(int,ToTime)(const char* pszValue) PURE;
	// Convert a string to a color value.
	STDMETHOD_(int,ToColor)(const char* pszValue) PURE;
	// Find an object relative to other objects (or just use the name or ID)
	// "self"=>iDest, "source"=>iSource, "^" archetype=>closest to iDest,
	// "&" flavor=>first link from iDest, "%" flavor=>link from iSource
	// "&?" flavor=>random link, "%?" flavor=>random link
	STDMETHOD_(int,ToObject)(const char* pszValue, int iDest, int iSource = 0) PURE;
	// Find many objects.
	// Recognizes ToObject strings (except "source") and:
	// "*" archetype=>direct concrete descendants
	// "@" archetype=>all concrete descendants
	// "&" flavor=>link query
	// distance "<" archetype=>nearby objects
	// distance "<" height "<" archetype=>nearby with max. Z-displacement
	STDMETHOD_(IObjectQuery*,QueryObjects)(const char* pszValue, int iDest) PURE;
};
DEFINE_GUID(IID_IScriptParamScriptService, 0xCECACA8D, 0x1429, 0x4404, 0xAE, 0xB2, 0x6A, 0xEF, 0xC5, 0x74, 0x46, 0xCF);
DEFINE_IIDSTRUCT(IScriptParamScriptService,IID_IScriptParamScriptService);

/*
 * Library Utility Functions
 */
HANDLE ScriptParamLoadLibrary(void);
Bool ScriptParamInitializeService(IScriptMan* pSM, IMalloc* pMalloc);


#endif // SCRIPTPARAM_H
