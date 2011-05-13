/******************************************************************************
 *  darkhook.h
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
/**********************
 * DarkHook2 interface header
 */

#ifndef DARKHOOK_H
#define DARKHOOK_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <lg/config.h>
#include <lg/objstd.h>
#include <lg/iiddef.h>
#include <lg/types.h>
#include <lg/scrmsgs.h>
#include <lg/links.h>
#include <lg/scrmanagers.h>


enum eDarkHookType
{
	kDH_Null = 0,
	kDH_Property,
	kDH_Relation,
	kDH_Object,
	kDH_Trait
};

// Relation
enum eLinkEvent
{
	kRel_Null = 0,
	kRel_Change,
	kRel_Add,
	kRel_Remove
};

struct sDarkRelMsg
{
	eLinkEvent   event;
	IRelation*   pRel;
	const  char*  pszRelName;
	long lLinkId;
	int  iLinkSource;
	int  iLinkDest;
};

enum eTraitEvent
{
	kTrait_AddDonor = 0,
	kTrait_DelDonor = 1,
	kTrait_AddChild = 2,
	kTrait_DelChild = 3
};

struct sDarkTraitMsg
{
	eTraitEvent  event;
	int          idObj;
	int          idSubj;
};

enum ePropEvent
{
	kProp_Null = 0,
    kProp_Change,
    kProp_Add,
    kProp_Remove,
	kProp_Inherited = 16
};

struct sDarkPropMsg
{
	ePropEvent   event;
	const char*  pszPropName;
	int          idObj;
	IProperty*   pProp;
};

enum eObjEvent
{
	kObj_Create  = 0,
    kObj_Destroy = 1
};

struct sDarkObjMsg
{
	eObjEvent    event;
	int          idObj;
};

struct sDHNotifyMsg : sScrMsg {
	eDarkHookType typeDH;
	union {
		sDarkPropMsg   sProp;
		sDarkRelMsg    sRel;
		sDarkObjMsg    sObj;
		sDarkTraitMsg  sTrait;
	};

	virtual ~sDHNotifyMsg()
	{
		if (typeDH == kDH_Relation)
			sRel.pRel->Release();
	}
	sDHNotifyMsg() : sScrMsg(), typeDH(kDH_Null) { }

	virtual const char* __thiscall GetName() const { return "sDHNotifyMsg"; }
};

/*
 * If you require the notification to be asynchronous,
 * then you will receive a message with information in the standard
 * three multiparms, since custom script messages can't be posted.
 *
 * Property: int(kDH_Property), int(event), string("objid,propname")
 * Relation: int(kDH_Relation), int(event), int(linkid)
 * Object: int(kDH_Object), int(event), int(objid)
 * Trait: int(kDH_Trait), int(event), string("objid,subjid")
 */

enum eDHRegisterFlags
{
	kDHNotifyDefault = 0,
	kDHNotifyAsync = 1
};


interface IDarkHookScriptService : IUnknown
{
	// BaseScriptService
	STDMETHOD_(void,Init)(void) PURE;
	STDMETHOD_(void,End)(void) PURE;

	// The hooks.
	STDMETHOD_(Bool,InstallPropHook)(int iAgent, eDHRegisterFlags eFlags, const char * pszProp, int iObj) PURE;
	STDMETHOD_(void,UninstallPropHook)(int iAgent, const char * pszProp, int iObj) PURE;

	// iObj is the source
	STDMETHOD_(Bool,InstallRelHook)(int iAgent, eDHRegisterFlags eFlags, const char * pszRel, int iObj) PURE;
	STDMETHOD_(void,UninstallRelHook)(int iAgent, const char * pszRel, int iObj) PURE;

	STDMETHOD_(Bool,InstallObjHook)(int iAgent, eDHRegisterFlags eFlags, int iObj) PURE;
	STDMETHOD_(void,UninstallObjHook)(int iAgent, int iObj) PURE;

	STDMETHOD_(Bool,InstallHierarchyHook)(int iAgent, eDHRegisterFlags eFlags, int iObj) PURE;
	STDMETHOD_(void,UninstallHierarchyHook)(int iAgent, int iObj) PURE;
};
DEFINE_GUID(IID_IDarkHookScriptService, 0x61CEB223, 0xD820, 0x480A, 0xAA, 0x64, 0xFA, 0x3F, 0x58, 0x82, 0x36, 0x6A);
DEFINE_IIDSTRUCT(IDarkHookScriptService,IID_IDarkHookScriptService);

/*
 * Library Utility Functions
 */
HANDLE DarkHookLoadLibrary(void);
Bool DarkHookInitializeService(IScriptMan* pSM, IMalloc* pMalloc);


#endif // DARKHOOK_H
