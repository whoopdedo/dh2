/******************************************************************************
 *  dh2lib.cpp
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
#include "darkhook.h"
#include <windows.h>

/* OSL sorts next to OSM nicely,
 * And will be copied by DarkLoader.
 */
#define DH2_MODULENAME "DH2.OSL"
#define DH2_INITPROCNAME "_DH2Init"

typedef Bool (__cdecl *DHInitProc)(IScriptMan* pScriptMan, IMalloc* pMalloc);

HANDLE DarkHookLoadLibrary(void)
{
	HMODULE hDH2 = ::GetModuleHandleA(DH2_MODULENAME);
	if (!hDH2)
		hDH2 = ::LoadLibraryA(DH2_MODULENAME);
	return reinterpret_cast<HANDLE>(hDH2);
}

Bool DarkHookInitializeService(IScriptMan* pSM, IMalloc* pMalloc)
{
	HMODULE hDH2 = reinterpret_cast<HMODULE>(DarkHookLoadLibrary());
	if (!hDH2)
		return FALSE;

	DHInitProc pfnDHInit = reinterpret_cast<DHInitProc>(::GetProcAddress(hDH2, DH2_INITPROCNAME));
	if (!pfnDHInit)
		return FALSE;

	return pfnDHInit(pSM, pMalloc);
}
