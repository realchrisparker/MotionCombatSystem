#include <Debug/MCS_DebugViewportDrawer.h>
#include <Components/MCS_CombatCoreComponent.h>
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"


static TAutoConsoleVariable<bool> CVarMCS_DebugOverlay(
    TEXT("mcs.DebugOverlay"),
    false,
    TEXT("Toggle Motion Combat System Debug Overlay"),
    ECVF_Default);

bool FMCS_DebugViewportDrawer::bIsRegistered = false;
FDelegateHandle FMCS_DebugViewportDrawer::DrawHandle;

void FMCS_DebugViewportDrawer::Initialize()
{
    if (bIsRegistered || !GEngine || !GEngine->GameViewport)
        return;

    // Register a parameter-less lambda to the OnEndDraw delegate
    DrawHandle = GEngine->GameViewport->OnEndDraw().AddLambda([ ] ()
        {
            FMCS_DebugViewportDrawer::DrawOverlay();
        });

    bIsRegistered = true;
    UE_LOG(LogTemp, Log, TEXT("[MCS] Debug viewport drawer registered (OnEndDraw lambda)."));
}

void FMCS_DebugViewportDrawer::Shutdown()
{
    if (!bIsRegistered || !GEngine || !GEngine->GameViewport)
        return;

    GEngine->GameViewport->OnEndDraw().Remove(DrawHandle);
    bIsRegistered = false;
    UE_LOG(LogTemp, Log, TEXT("[MCS] Debug viewport drawer unregistered."));
}

void FMCS_DebugViewportDrawer::DrawOverlay()
{
    // Skip if disabled
    if (!CVarMCS_DebugOverlay.GetValueOnGameThread())
        return;

    if (!GEngine || !GEngine->GameViewport || !GWorld)
        return;

    // Grab the debug canvas from the viewport
    FViewport* Viewport = GEngine->GameViewport->Viewport;
    FCanvas* Canvas = Viewport ? Viewport->GetDebugCanvas() : nullptr;
    if (!Canvas)
        return;

    for (FConstPlayerControllerIterator It = GWorld->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get())
        {
            if (!PC->IsLocalController())
                continue;

            if (APawn* Pawn = PC->GetPawn())
            {
                if (UMCS_CombatCoreComponent* Core = Pawn->FindComponentByClass<UMCS_CombatCoreComponent>())
                {
                    float Y = 50.f;
                    Core->DrawDebugOverlay(Canvas, Y);
                }
            }
        }
    }
}