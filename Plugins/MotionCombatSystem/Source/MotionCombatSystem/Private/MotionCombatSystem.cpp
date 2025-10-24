// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotionCombatSystem.h"
#include "Debug/MCS_DebugViewportDrawer.h"

#define LOCTEXT_NAMESPACE "FMotionCombatSystemModule"

void FMotionCombatSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
#if WITH_EDITORONLY_DATA
	FWorldDelegates::OnPostWorldInitialization.AddLambda([ ] (UWorld* World, const UWorld::InitializationValues IVS)
		{
			if (!World)
				return;

			// Only initialize for PIE or Game worlds (not editor previews)
			if (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game)
			{
				// Make sure viewport exists before initializing
				if (GEngine && GEngine->GameViewport)
				{
					FMCS_DebugViewportDrawer::Initialize();
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[MCS] GameViewport not ready yet, delaying MCS DebugViewportDrawer setup."));
					FCoreDelegates::OnEndFrame.AddStatic(&FMCS_DebugViewportDrawer::Initialize);
				}
			}
		});

	// Clean up when world is torn down
	FWorldDelegates::OnPreWorldFinishDestroy.AddLambda([ ] (UWorld* World)
		{
			if (World && (World->WorldType == EWorldType::PIE || World->WorldType == EWorldType::Game))
			{
				FMCS_DebugViewportDrawer::Shutdown();
			}
		});
#endif
}

void FMotionCombatSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
#if WITH_EDITORONLY_DATA
	FMCS_DebugViewportDrawer::Shutdown();
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMotionCombatSystemModule, MotionCombatSystem)