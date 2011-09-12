//============== IV: Multiplayer - http://code.iv-multiplayer.com ==============
//
// File: CModule.cpp
// Project: Server.Core
// Author(s): MaVe
// License: See LICENSE in root directory
//
//==============================================================================

#include "Main.h"
#include "CModule.h"
#include <SharedUtility.h>
#include "CNetworkManager.h"
#include "CPlayerManager.h"
#include "CVehicleManager.h"
#include "CObjectManager.h"
#include "CBlipManager.h"
#include "CActorManager.h"
#include "CCheckpointManager.h"
#include "CPickupManager.h"
#include "CModuleManager.h"
#include "Scripting/CScriptingManager.h"
#include <CSettings.h>
#include <Game/CTime.h>
#include <Game/CTrafficLights.h>
#include "CEvents.h"
#include <CLogFile.h>

extern CNetworkManager * g_pNetworkManager;
extern CPlayerManager * g_pPlayerManager;
extern CVehicleManager * g_pVehicleManager;
extern CObjectManager * g_pObjectManager;
extern CBlipManager * g_pBlipManager;
extern CActorManager * g_pActorManager;
extern CPickupManager * g_pPickupManager;
extern CCheckpointManager * g_pCheckpointManager;
extern CScriptingManager * g_pScriptingManager;
extern CModuleManager * g_pModuleManager;
extern CTime * g_pTime;
extern CEvents * g_pEvents;
extern CTrafficLights * g_pTrafficLights;

CModule::CModule(const char * szName)
{
	String strModuleName(szName);
	SharedUtility::RemoveIllegalCharacters(strModuleName);
	String strModulePath(SharedUtility::GetAbsolutePath("modules/%s", strModuleName.Get()));
#ifdef WIN32
	// In windows replace all '/' with '\\' or the module will not load
	strModulePath.Substitute("/", "\\");
#endif
	m_pLibrary = new CLibrary();

	if(!m_pLibrary)
		return;

	if(!m_pLibrary->Load(strModulePath.Get()))
	{
		delete m_pLibrary;
		m_pLibrary = NULL;
		return;
	}

	m_ModuleFunctions.pfnSetupFunctions = (SetupFunctions_t)m_pLibrary->GetProcedureAddress("SetupFunctions");
	m_ModuleFunctions.pfnSetupInterfaces = (SetupInterfaces_t)m_pLibrary->GetProcedureAddress("SetupInterfaces");
	m_ModuleFunctions.pfnInitModule = (InitModule_t)m_pLibrary->GetProcedureAddress("InitModule");
	m_ModuleFunctions.pfnScriptLoad = (ScriptLoad_t)m_pLibrary->GetProcedureAddress("ScriptLoad");
	m_ModuleFunctions.pfnScriptUnload = (ScriptUnload_t)m_pLibrary->GetProcedureAddress("ScriptUnload");
	m_ModuleFunctions.pfnPulse = (Pulse_t)m_pLibrary->GetProcedureAddress("Pulse");

	if(!IsValid())
	{
		delete m_pLibrary;
		m_pLibrary = NULL;
		return;
	}

	// Setup the functions
	m_ModuleFunctions.pfnSetupFunctions(FunctionContainer);

	// Setup the pointers
	InterfacesContainer[0] = (void*)g_pNetworkManager;
	InterfacesContainer[1] = (void*)g_pPlayerManager;
	InterfacesContainer[2] = (void*)g_pVehicleManager;
	InterfacesContainer[3] = (void*)g_pObjectManager;
	InterfacesContainer[4] = (void*)g_pBlipManager;
	InterfacesContainer[5] = (void*)g_pActorManager;
	InterfacesContainer[6] = (void*)g_pPickupManager;
	InterfacesContainer[7] = (void*)g_pCheckpointManager;
	InterfacesContainer[8] = (void*)NULL; // model manager
	InterfacesContainer[9] = (void*)g_pScriptingManager;
	InterfacesContainer[10] = (void*)g_pModuleManager;
	InterfacesContainer[11] = (void*)g_pNetworkManager->GetNetServer();
	InterfacesContainer[12] = (void*)NULL; // FIXME
	InterfacesContainer[13] = (void*)g_pTime;
	InterfacesContainer[14] = (void*)g_pTrafficLights;
	InterfacesContainer[15] = (void*)g_pEvents;
	InterfacesContainer[16] = new CSquirrelArgumentManager();

	// Send it
	if(m_ModuleFunctions.pfnSetupInterfaces)
		m_ModuleFunctions.pfnSetupInterfaces(InterfacesContainer);

	char szModuleName[64];
	strcpy(szModuleName, szName);

	if(m_ModuleFunctions.pfnInitModule(szModuleName))
		CLogFile::Printf("Loaded module '%s'", szModuleName);
}

CModule::~CModule()
{
	if(m_pLibrary)
	{
		m_pLibrary->Unload();
		delete m_pLibrary;
	}
}

bool CModule::IsValid()
{
	if(!m_pLibrary)
		return false;

	if(!m_ModuleFunctions.pfnSetupFunctions || !m_ModuleFunctions.pfnInitModule)
		return false;

	return true;
}

void CModule::ScriptLoad(HSQUIRRELVM pVM)
{
	if(!m_pLibrary || !m_ModuleFunctions.pfnScriptLoad)
		return;

	m_ModuleFunctions.pfnScriptLoad(pVM);
}

void CModule::ScriptUnload(HSQUIRRELVM pVM)
{
	if(!m_pLibrary || !m_ModuleFunctions.pfnScriptUnload)
		return;

	m_ModuleFunctions.pfnScriptUnload(pVM);
}

void CModule::Pulse()
{
	if(!m_pLibrary || !m_ModuleFunctions.pfnPulse)
		return;

	return m_ModuleFunctions.pfnPulse();
}
