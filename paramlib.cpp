/******************************************************************************
 *  paramlib.cpp
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
#include "scriptparam.h"
#include <windows.h>

/* OSL sorts next to OSM nicely,
 * And will be copied by DarkLoader.
 */
#define SCRIPTPARAM_MODULENAME "PARAMS.OSL"
#define SCRIPTPARAM_INITPROCNAME "_ScriptParamInit"

typedef Bool (__cdecl *InitProc)(IScriptMan* pScriptMan, IMalloc* pMalloc);

HMODULE ScriptParamLoadLibrary(void)
{
	HMODULE hDLL = ::GetModuleHandleA(SCRIPTPARAM_MODULENAME);
	if (!hDLL)
		hDLL = ::LoadLibraryA(SCRIPTPARAM_MODULENAME);
	return hDLL;
}

Bool ScriptParamInitializeService(IScriptMan* pSM, IMalloc* pMalloc)
{
	HMODULE hDLL = ScriptParamLoadLibrary();
	if (!hDLL)
		return FALSE;

	InitProc pfnInit = reinterpret_cast<InitProc>(::GetProcAddress(hDLL, SCRIPTPARAM_INITPROCNAME));
	if (!pfnInit)
		return FALSE;

	return pfnInit(pSM, pMalloc);
}
