#pragma once

#include "CoreMinimal.h"

class UMCS_CombatCoreComponent;
class UCanvas;   // Gameplay HUD canvas (UObject)
class FCanvas;   // Engine render context (non-UObject)


/**
 * Global viewport overlay renderer for Motion Combat System debug data.
 * Draws attack selection and situation info when mcs.DebugOverlay = 1.
 */
class FMCS_DebugViewportDrawer
{
public:
    static void Initialize();
    static void Shutdown();

private:
    /** Delegate callback bound to OnEndDraw */
    static FDelegateHandle DrawHandle;

    /** Whether the delegate is registered */
    static bool bIsRegistered;

    /** Called from the OnEndDraw lambda to perform actual drawing */
    static void DrawOverlay();
};