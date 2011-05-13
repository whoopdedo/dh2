/******************************************************************************
 *  dh2.h
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
 * DarkHook2
 */

#ifndef DH2_H
#define DH2_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "darkhook.h"
#include "objprop.h"
#include <lg/scrmanagers.h>
#include <lg/properties.h>
#include <lg/links.h>
#include <lg/objects.h>
#include <lg/interface.h>
#include <lg/interfaceimp.h>

#include <unordered_map>
#include <list>


class cDH2ScriptService : public cInterfaceImp<IDarkHookScriptService>
{
public:
	STDMETHOD_(void,Init)(void);
	STDMETHOD_(void,End)(void);

	STDMETHOD_(Bool,InstallPropHook)(int iAgent, eDHRegisterFlags eFlags, const char * pszProp, int iObj);
	STDMETHOD_(void,UninstallPropHook)(int iAgent, const char * pszProp, int iObj);

	STDMETHOD_(Bool,InstallRelHook)(int iAgent, eDHRegisterFlags eFlags, const char * pszRel, int iObj);
	STDMETHOD_(void,UninstallRelHook)(int iAgent, const char * pszRel, int iObj);

	STDMETHOD_(Bool,InstallObjHook)(int iAgent, eDHRegisterFlags eFlags, int iObj);
	STDMETHOD_(void,UninstallObjHook)(int iAgent, int iObj);

	STDMETHOD_(Bool,InstallHierarchyHook)(int iAgent, eDHRegisterFlags eFlags, int iObj);
	STDMETHOD_(void,UninstallHierarchyHook)(int iAgent, int iObj);

	cDH2ScriptService(IUnknown* pIFace);
	virtual ~cDH2ScriptService();

	static bool sm_initialized;

private:
	SInterface<IScriptMan> m_pScriptMan;
	SInterface<IPropertyManager> m_pPropMan;
	SInterface<ILinkManager> m_pLinkMan;
	SInterface<IObjectSystem> m_pObjMan;
	SInterface<ITraitManager> m_pTraitMan;
	SInterface<ISimManager> m_pSimMan;

	static void __stdcall PropertyListener(sPropertyListenMsg* pPropMsg, PropListenerData hData);
	static void __stdcall RelationListener(sRelationListenMsg* pRelMsg, void* pData);
	static void __cdecl ObjectListener(int iObjId, unsigned long uEvent, void* pData);
	static void __stdcall HierarchyListener(const sHierarchyMsg* pTraitMsg, void* pData);
	static int __cdecl SimListener(const sDispatchMsg* pSimMsg, const sDispatchListenerDesc* pDesc);

	static sDispatchListenerDesc sm_simlistenerdesc;

	typedef std::unordered_map<int,PropListenerHandle> prophook_map;
	typedef std::unordered_map<int,bool> relhook_map;
	static prophook_map sm_prophookhandles;
	static relhook_map  sm_relhookactive;
	static int 	 sm_objhookhandle;
	static bool	 sm_traithookactive;

	typedef std::list<std::pair<int,eDHRegisterFlags> > objlist;
	typedef std::unordered_map<objprop,objlist,objprop_hash> objprop_map;
	typedef std::unordered_map<int,objlist> obj_map;
	objprop_map m_proptree;
	objprop_map	m_reltree;
	obj_map m_objtree;
	obj_map m_traittree;

	void HandleProperty(sPropertyListenMsg* pPropMsg);
	void HandleRelation(sRelationListenMsg* pRelMsg);
	void HandleObject(int iObjId, unsigned long uEvent);
	void HandleHierarchy(const sHierarchyMsg* pTraitMsg);

	void DoPropertyNotify(int iAgent, eDHRegisterFlags eFlags, unsigned int uEvent, const char* pszName, int iObjId, IProperty* pProp);
	void DoRelationNotify(int iAgent, eDHRegisterFlags eFlags, unsigned int uEvent, const char* pszName, long lLinkId, int iLinkSource, int iLinkDest, IRelation* pRel);
	void DoObjectNotify(int iAgent, eDHRegisterFlags eFlags, unsigned int uEvent, int iObjId);
	void DoHierarchyNotify(int iAgent, eDHRegisterFlags eFlags, unsigned int uEvent, int iObjId, int iSubjId);

	void Reset(void);
};

#endif // DH2_H
