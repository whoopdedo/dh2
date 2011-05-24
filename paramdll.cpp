/******************************************************************************
 *  paramdll.cpp
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
#include "params.h"
#include <lg/scrmanagers.h>
#include <windows.h>

IMalloc* g_pMalloc = NULL;

extern "C" __declspec(dllexport)
Bool __cdecl ScriptParamInit(IScriptMan* pScriptMan, IMalloc* pMalloc)
{
	try
	{
		if (! cScriptParamScriptService::sm_initialized)
		{
			g_pMalloc = pMalloc;
			cScriptParamScriptService* pScrParams = new cScriptParamScriptService(pScriptMan);
			pScriptMan->ExposeService(pScrParams, IID_IScriptParamScriptService);
			pScrParams->Init(); // why doesn't ExposeService do this?
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}

extern "C"
BOOL WINAPI DllMain (HINSTANCE h, DWORD dwReason, PVOID pv)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		::DisableThreadLibraryCalls(h);
		return TRUE;
	}
	return TRUE;
#ifdef __GNUC__
	pv = pv;
#endif
#ifdef __BORLANDC__
#pragma argsused(dwReason)
#endif
}
