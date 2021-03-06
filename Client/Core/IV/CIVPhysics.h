//============== IV:Multiplayer - https://github.com/Neproify/ivmultiplayer ==============
//
// File: CIVPhysics.h
// Project: Client.Core
// Author(s): XForce
// License: See LICENSE in root directory
//
//==============================================================================

#pragma once

class IVPhysics
{
public:
	virtual ~IVPhysics();
};

class CIVPhysics
{
private:
	IVPhysics* m_pPhysics;

public:
	CIVPhysics();
	~CIVPhysics();
};