/******************************************************************************
 *  dh2.cpp
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
#define INITGUID 1
#include "dh2.h"

#include <functional>
#include <algorithm>

using namespace std;


sDispatchListenerDesc cDH2ScriptService::sm_simlistenerdesc = {
	&IID_IDarkHookScriptService,
	0xF,
	SimListener,
	NULL
};

bool cDH2ScriptService::sm_initialized = false;
cDH2ScriptService::prophook_map cDH2ScriptService::sm_prophookhandles;
cDH2ScriptService::relhook_map cDH2ScriptService::sm_relhookactive;
int cDH2ScriptService::sm_objhookhandle = -1;
bool cDH2ScriptService::sm_traithookactive = false;

// Hooks

void __stdcall cDH2ScriptService::PropertyListener(sPropertyListenMsg* pPropMsg, PropListenerData pData)
{
	if (!sm_initialized)
		return;

	if ((pPropMsg->event & 8) != 0 || (pPropMsg->event & 48) == kPropertyInherited)
		return;

	try
	{
		reinterpret_cast<cDH2ScriptService*>(pData)->HandleProperty(pPropMsg);
	}
	catch (...)
	{
	}
}

void cDH2ScriptService::HandleProperty(sPropertyListenMsg* pPropMsg)
{
	//if (!(m_pSimMan->LastMsg() & 9))
	//	return;

	static const char __event[] = { 0,1,2,2,3,3,3,3,0,1,2,2,3,3,3,3 };
	int event = __event[pPropMsg->event & 0xF];
	if (pPropMsg->event & kPropertyInherited)
		event |= kProp_Inherited;

	SInterface<IProperty> pProp = m_pPropMan->GetProperty(pPropMsg->iPropId);
	const sPropertyDesc* pPropDesc = pProp->Describe();

	objprop_map::const_iterator i = m_proptree.find(objprop(pPropDesc->szName,pPropMsg->iObjId));
	if (i != m_proptree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoPropertyNotify(l->first, l->second, event, pPropDesc->szName, pPropMsg->iObjId, pProp);
		}
	}
	i = m_proptree.find(objprop(pPropDesc->szName,0));
	if (i != m_proptree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoPropertyNotify(l->first, l->second, event, pPropDesc->szName, pPropMsg->iObjId, pProp);
		}
	}
	/* Have to find a way to register a listener for every property
	i = m_proptree.find(objprop("",pPropMsg->iObjId));
	if (i != m_proptree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoPropertyNotify(l->first, l->second, event, pPropDesc->szName, pPropMsg->iObjId, pProp);
		}
	}
	*/

}

void __stdcall cDH2ScriptService::RelationListener(sRelationListenMsg* pRelMsg, void* pData)
{
	if (!sm_initialized)
		return;

	if (!(pRelMsg->event & 0xF))
		return;

	try
	{
		reinterpret_cast<cDH2ScriptService*>(pData)->HandleRelation(pRelMsg);
	}
	catch (...)
	{
	}
}

void cDH2ScriptService::HandleRelation(sRelationListenMsg* pRelMsg)
{
	//if (!(m_pSimMan->LastMsg() & 9))
	//	return;

	static const char __event[] = { 0,1,2,2,3,3,3,3,3,3,3,3,3,3,3,3 };
	int event = __event[pRelMsg->event & 0xF];

	SInterface<IRelation> pRel = m_pLinkMan->GetRelation(pRelMsg->flavor);
	const sRelationDesc* pRelDesc = pRel->Describe();

	objprop_map::const_iterator i = m_reltree.find(objprop(pRelDesc->szName,pRelMsg->source));
	if (i != m_reltree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoRelationNotify(l->first, l->second, event, pRelDesc->szName, pRelMsg->lLink, pRelMsg->source, pRelMsg->dest, pRel);
		}
	}
	i = m_reltree.find(objprop(pRelDesc->szName,0));
	if (i != m_reltree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoRelationNotify(l->first, l->second, event, pRelDesc->szName, pRelMsg->lLink, pRelMsg->source, pRelMsg->dest, pRel);
		}
	}
	/* Have to find a way to register a listener for every link
	m_reltree.find(objprop("",pRelMsg->source));
	if (i != m_reltree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoRelationNotify(l->first, l->second, event, pRelDesc->szName, pRelMsg->lLink, pRelMsg->source, pRelMsg->dest, pRel);
		}
	}
	*/
}

void __cdecl cDH2ScriptService::ObjectListener(int iObjId, unsigned long uEvent, void* pData)
{
	if (!sm_initialized)
		return;

	// PostLoad and BeginCreate events are ignored
	if (uEvent > 2)
		return;

	try
	{
		reinterpret_cast<cDH2ScriptService*>(pData)->HandleObject(iObjId, uEvent);
	}
	catch (...)
	{
	}
}

void cDH2ScriptService::HandleObject(int iObjId, unsigned long uEvent)
{
	//if (!(m_pSimMan->LastMsg() & 9))
	//	return;

	obj_map::const_iterator i = m_objtree.find(iObjId);
	if (i != m_objtree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoObjectNotify(l->first, l->second, uEvent, iObjId);
		}
	}

	i = m_objtree.find(0);
	if (i != m_objtree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoObjectNotify(l->first, l->second, uEvent, iObjId);
		}
	}
}

void __stdcall cDH2ScriptService::HierarchyListener(const sHierarchyMsg* pTraitMsg, void* pData)
{
	if (!sm_initialized)
		return;

	try
	{
		reinterpret_cast<cDH2ScriptService*>(pData)->HandleHierarchy(pTraitMsg);
	}
	catch (...)
	{
	}
}

void cDH2ScriptService::HandleHierarchy(const sHierarchyMsg* pTraitMsg)
{
	//if (!(m_pSimMan->LastMsg() & 9))
	//	return;

	obj_map::const_iterator i = m_traittree.find(pTraitMsg->iSubjId);
	if (i != m_traittree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoHierarchyNotify(l->first, l->second, pTraitMsg->event, pTraitMsg->iSubjId, pTraitMsg->iObjId);
		}
	}
	i = m_traittree.find(0);
	if (i != m_traittree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoHierarchyNotify(l->first, l->second, pTraitMsg->event, pTraitMsg->iSubjId, pTraitMsg->iObjId);
		}
	}
	// And again with the inverse context
	i = m_traittree.find(pTraitMsg->iObjId);
	if (i != m_traittree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoHierarchyNotify(l->first, l->second, pTraitMsg->event+2, pTraitMsg->iObjId, pTraitMsg->iSubjId);
		}
	}
	i = m_traittree.find(0);
	if (i != m_traittree.end())
	{
		objlist::const_iterator l = i->second.begin();
		for (; l != i->second.end(); ++l)
		{
			DoHierarchyNotify(l->first, l->second, pTraitMsg->event+2, pTraitMsg->iObjId, pTraitMsg->iSubjId);
		}
	}
}

int __cdecl cDH2ScriptService::SimListener(const sDispatchMsg* pSimMsg, const sDispatchListenerDesc* pDesc)
{
	try
	{
		if (pSimMsg->dwEventId == kSimStop)
			reinterpret_cast<cDH2ScriptService*>(pDesc->pData)->Reset();
	}
	catch (...)
	{
	}
	return 0;
}


// DH2 class

cDH2ScriptService::~cDH2ScriptService()
{
	sm_initialized = false;

	Reset();

	m_pSimMan->Unlisten(&IID_IDarkHookScriptService);
	m_pSimMan->Release();
	m_pTraitMan->Release();
	m_pObjMan->Release();
	m_pLinkMan->Release();
	m_pPropMan->Release();
	m_pScriptMan->Release();
}

cDH2ScriptService::cDH2ScriptService(IUnknown* pIFace)
{
	m_pScriptMan = pIFace;
	m_pLinkMan = pIFace;
	m_pLinkMan = pIFace;
	m_pObjMan = pIFace;
	m_pTraitMan = pIFace;
	m_pSimMan = pIFace;

	sm_simlistenerdesc.pData = reinterpret_cast<void*>(this);
	m_pSimMan->Listen(&sm_simlistenerdesc);

	sm_initialized = true;
}

// BaseScriptService functions
STDMETHODIMP_(void) cDH2ScriptService::Init(void)
{
}

STDMETHODIMP_(void) cDH2ScriptService::End(void)
{
}

void cDH2ScriptService::Reset(void)
{
	if (sm_objhookhandle != -1)
	{
		m_pObjMan->Unlisten(sm_objhookhandle);
		sm_objhookhandle = -1;
	}

	prophook_map::iterator i = sm_prophookhandles.begin();
	for (; i != sm_prophookhandles.end(); ++i)
	{
		IProperty* pProp = m_pPropMan->GetProperty(i->first);
		pProp->Unlisten(i->second);
		pProp->Release();
	}
	sm_prophookhandles.clear();

	m_proptree.clear();
	m_reltree.clear();
	m_objtree.clear();
	m_traittree.clear();
}

inline bool operator== (const pair<int,eDHRegisterFlags>& __x, int __y)
{
	return __x.first == __y;
}

// DarkHookScriptService
STDMETHODIMP_(Bool) cDH2ScriptService::InstallPropHook(int iAgent, eDHRegisterFlags eFlags, const char * pszProp, int iObj)
{
	if (! pszProp || ! *pszProp)
		return FALSE;

	SInterface<IProperty> pProp = m_pPropMan->GetPropertyNamed(pszProp);
	if (!pProp || pProp->GetID() == -1)
		return FALSE;
	prophook_map::iterator k = sm_prophookhandles.find(pProp->GetID());
	if (k == sm_prophookhandles.end())
	{
		// We don't increment the reference count on pProp to avoid the hassle
		// of having to release it later... Shouldn't be a problem since the
		// listener will be called by the same instance anyway (we hope).
		PropListenerHandle h = pProp->Listen(kPropertyFull, PropertyListener, reinterpret_cast<PropListenerData>(this));
		sm_prophookhandles.insert(make_pair(pProp->GetID(),h));
	}

	objprop proptag(pszProp,iObj);
	objlist& l = m_proptree[proptag];
	objlist::iterator i = find(l.begin(), l.end(), iAgent);
	if (i == l.end())
		l.push_back(make_pair(iAgent,eFlags));
	else
		i->second = eFlags;

	return TRUE;
}

void __stdcall cDH2ScriptService::UninstallPropHook(int iAgent, const char * pszProp, int iObj)
{
	if (! pszProp || ! *pszProp)
		return;

	SInterface<IProperty> pProp = m_pPropMan->GetPropertyNamed(pszProp);
	if (!pProp)
		return;

	objprop_map::iterator i = m_proptree.find(objprop(pszProp,iObj));
	if (i != m_proptree.end())
	{
		remove(i->second.begin(), i->second.end(), iAgent);
		if (i->second.empty())
		{
			m_proptree.erase(i);

			prophook_map::iterator k = sm_prophookhandles.find(pProp->GetID());
			if (k != sm_prophookhandles.end())
			{
				pProp->Unlisten(k->second);
				sm_prophookhandles.erase(k);
			}
		}
	}
}

STDMETHODIMP_(Bool) cDH2ScriptService::InstallRelHook(int iAgent, eDHRegisterFlags eFlags, const char * pszRel, int iObj)
{
	if (! pszRel || ! *pszRel)
		return FALSE;

	SInterface<IRelation> pRel = m_pLinkMan->GetRelationNamed(pszRel);
	if (!pRel || pRel->GetID() == 0)
		return FALSE;
	relhook_map::iterator k = sm_relhookactive.find(pRel->GetID());
	if (k == sm_relhookactive.end())
	{
		pRel->Listen(kRelationFull, RelationListener, reinterpret_cast<void*>(this));
		sm_relhookactive.insert(make_pair(pRel->GetID(),true));
	}

	objprop reltag(pszRel,iObj);
	objlist& l = m_reltree[reltag];
	objlist::iterator i = find(l.begin(), l.end(), iAgent);
	if (i == l.end())
		l.push_back(make_pair(iAgent,eFlags));
	else
		i->second = eFlags;

	return TRUE;
}

STDMETHODIMP_(void) cDH2ScriptService::UninstallRelHook(int iAgent, const char * pszRel, int iObj)
{
	if (! pszRel || ! *pszRel)
		return;

	SInterface<IRelation> pRel = m_pLinkMan->GetRelationNamed(pszRel);
	if (!pRel)
		return;

	objprop_map::iterator i = m_reltree.find(objprop(pszRel,iObj));
	if (i != m_reltree.end())
	{
		remove(i->second.begin(), i->second.end(), iAgent);
		if (i->second.empty())
		{
			m_reltree.erase(i);
			// Can't unregister link listeners...
			// lets just hope the relation IDs don't change behind our back
		}
	}
}

STDMETHODIMP_(Bool) cDH2ScriptService::InstallObjHook(int iAgent, eDHRegisterFlags eFlags, int iObj)
{
	static const sObjListenerDesc _desc = { ObjectListener, reinterpret_cast<void*>(this) };

	objlist& l = m_objtree[iObj];
	objlist::iterator i = find(l.begin(), l.end(), iAgent);
	if (i == l.end())
		l.push_back(make_pair(iAgent,eFlags));
	else
		i->second = eFlags;

	if (sm_objhookhandle == -1)
		sm_objhookhandle = m_pObjMan->Listen(const_cast<sObjListenerDesc*>(&_desc));
	return TRUE;
}

STDMETHODIMP_(void) cDH2ScriptService::UninstallObjHook(int iAgent, int iObj)
{
	obj_map::iterator i = m_objtree.find(iObj);
	if (i != m_objtree.end())
	{
		remove(i->second.begin(), i->second.end(), iAgent);
		if (i->second.empty())
			m_objtree.erase(i);
	}
}

STDMETHODIMP_(Bool) cDH2ScriptService::InstallHierarchyHook(int iAgent, eDHRegisterFlags eFlags, int iObj)
{
	objlist& l = m_traittree[iObj];
	objlist::iterator i = find(l.begin(), l.end(), iAgent);
	if (i == l.end())
		l.push_back(make_pair(iAgent,eFlags));
	else
		i->second = eFlags;

	if (! sm_traithookactive)
	{
		m_pTraitMan->Listen(HierarchyListener, reinterpret_cast<void*>(this));
		sm_traithookactive = true;
	}
	return TRUE;
}

STDMETHODIMP_(void) cDH2ScriptService::UninstallHierarchyHook(int iAgent, int iObj)
{
	obj_map::iterator i = m_traittree.find(iObj);
	if (i != m_traittree.end())
	{
		remove(i->second.begin(), i->second.end(), iAgent);
		if (i->second.empty())
			m_traittree.erase(i);
	}
}

// Notifiers

static void DoPostMessage(IScriptMan* pSM, int iSrc, int iDest, const char* psz, const cMultiParm& data1, const cMultiParm& data2, const cMultiParm& data3)
{
#ifdef __GNUC__
	asm("push edi\n"
	"\tmov edi,esp\n"
	"\tsub esp,0x20\n"
	"\tmov eax,%0\n"
	"\tmov edx,dword ptr [eax]\n"
	"\tmov dword ptr [esp+0x1C],0x8\n"
	"\tmov ecx,%6\n"
	"\tmov dword ptr [esp+0x18],ecx\n"
	"\tmov ecx,%5\n"
	"\tmov dword ptr [esp+0x14],ecx\n"
	"\tmov ecx,%4\n"
	"\tmov dword ptr [esp+0x10],ecx\n"
	"\tmov ecx,%3\n"
	"\tmov dword ptr [esp+0xC],ecx\n"
	"\tmov ecx,%2\n"
	"\tmov dword ptr [esp+0x8],ecx\n"
	"\tmov ecx,%1\n"
	"\tmov dword ptr [esp+0x4],ecx\n"
	"\tmov dword ptr [esp],eax\n"
	"\tcall dword ptr [edx+0x6C]\n"
	"\tmov esp,edi\n"
	"\tpop edi\n"
	:: "g"(pSM), "g"(iSrc), "g"(iDest), "g"(psz), "g"(&data1), "g"(&data2), "g"(&data3)
	);
#else
	_asm {
		push edi
		mov  edi,esp
		sub  esp,0x20
		mov  eax,pSM
		mov  edx,dword ptr [eax]
		mov  dword ptr [esp+0x1C],kScrMsgPostToOwner
		mov  ecx,&data3
		mov  dword ptr [esp+0x18],ecx
		mov  ecx,&data2
		mov  dword ptr [esp+0x14],ecx
		mov  ecx,&data1
		mov  dword ptr [esp+0x10],ecx
		mov  ecx,psz
		mov  dword ptr [esp+0xC],ecx
		mov  ecx,dword ptr iDest
		mov  dword ptr [esp+0x8],ecx
		mov  ecx,dword ptr iSrc
		mov  dword ptr [esp+0x4],ecx
		mov  dword ptr [esp],eax
		call dword ptr [edx+0x6C]
		mov  esp,edi
		pop  edi
	}
#endif
}

void cDH2ScriptService::DoPropertyNotify(int iAgent, eDHRegisterFlags eFlags, unsigned int uEvent, const char* pszName, int iObjId, IProperty* pProp)
{
	if (eFlags & kDHNotifyAsync)
	{
		char szData[64];
		sprintf(szData, "%d,%s", iObjId, pszName);
		DoPostMessage(m_pScriptMan,0,iAgent,"DHNotifyAsync",int(kDH_Property),int(uEvent),szData);
	}
	else
	{
		cMultiParm mpReply;
		sDHNotifyMsg* pMsg = new sDHNotifyMsg();
		pMsg->to = iAgent;
		pMsg->message = "DHNotify";
		pMsg->typeDH = kDH_Property;
		pMsg->sProp.event = ePropEvent(uEvent);
		pMsg->sProp.idObj = iObjId;
		pMsg->sProp.pszPropName = pszName;
		pMsg->sProp.pProp = pProp;
		m_pScriptMan->SendMessage(pMsg,&mpReply);
		pMsg->Release();
	}
}

void cDH2ScriptService::DoRelationNotify(int iAgent, eDHRegisterFlags eFlags, unsigned int uEvent, const char* pszName, long lLinkId, int iLinkSource, int iLinkDest, IRelation* pRel)
{
	if (eFlags & kDHNotifyAsync)
	{
		DoPostMessage(m_pScriptMan,0,iAgent,"DHNotifyAsync",int(kDH_Relation),int(uEvent),int(lLinkId));
	}
	else
	{
		cMultiParm mpReply;
		sDHNotifyMsg* pMsg = new sDHNotifyMsg();
		pMsg->to = iAgent;
		pMsg->message = "DHNotify";
		pMsg->typeDH = kDH_Relation;
		pMsg->sRel.event = eLinkEvent(uEvent);
		pMsg->sRel.pszRelName = pszName;
		pMsg->sRel.pRel = pRel;
		pMsg->sRel.lLinkId = lLinkId;
		pMsg->sRel.iLinkSource = iLinkSource;
		pMsg->sRel.iLinkDest = iLinkDest;
		m_pScriptMan->SendMessage(pMsg,&mpReply);
		pMsg->Release();
	}
}

void cDH2ScriptService::DoObjectNotify(int iAgent, eDHRegisterFlags eFlags, unsigned int uEvent, int iObjId)
{
	if (eFlags & kDHNotifyAsync)
	{
		DoPostMessage(m_pScriptMan,0,iAgent,"DHNotifyAsync",int(kDH_Object),int(uEvent),iObjId);
	}
	else
	{
		cMultiParm mpReply;
		sDHNotifyMsg* pMsg = new sDHNotifyMsg();
		pMsg->to = iAgent;
		pMsg->message = "DHNotify";
		pMsg->typeDH = kDH_Object;
		pMsg->sObj.event = eObjEvent(uEvent);
		pMsg->sObj.idObj = iObjId;
		m_pScriptMan->SendMessage(pMsg,&mpReply);
		pMsg->Release();
	}
}

void cDH2ScriptService::DoHierarchyNotify(int iAgent, eDHRegisterFlags eFlags, unsigned int uEvent, int iObjId, int iSubjId)
{
	if (eFlags & kDHNotifyAsync)
	{
		char szData[32];
		sprintf(szData, "%d,%d", iObjId, iSubjId);
		DoPostMessage(m_pScriptMan,0,iAgent,"DHNotifyAsync",int(kDH_Trait),int(uEvent),szData);
	}
	else
	{
		cMultiParm mpReply;
		sDHNotifyMsg* pMsg = new sDHNotifyMsg();
		pMsg->to = iAgent;
		pMsg->message = "DHNotify";
		pMsg->typeDH = kDH_Trait;
		pMsg->sTrait.event = eTraitEvent(uEvent);
		pMsg->sTrait.idObj = iObjId;
		pMsg->sTrait.idSubj = iSubjId;
		m_pScriptMan->SendMessage(pMsg,&mpReply);
		pMsg->Release();
	}
}

