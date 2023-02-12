// Copyright 2023 Sun BoHeng

#pragma once

#include "Modules/ModuleManager.h"

class FInteractiveWorldModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};