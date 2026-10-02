#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TerraVoxGameMode.generated.h"

UCLASS()
class TERRAVOXUE5_API ATerraVoxGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATerraVoxGameMode();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // atmosphere actors spawned at runtime
    UPROPERTY() class ADirectionalLight* Sun = nullptr;
    UPROPERTY() class ASkyAtmosphere* Sky = nullptr;
    UPROPERTY() class AExponentialHeightFog* Fog = nullptr;
    UPROPERTY() class ASkyLight* SkyLight = nullptr;

    UPROPERTY() class AVoxelWorld* World = nullptr;

    float TimeAccum = 0.f;
    static constexpr float CycleSeconds = 240.f;

    // verified working state while pawn collision (A8) is in flight
    FVector PinnedEye = FVector::ZeroVector;
    bool bHavePinnedEye = false;

    FTimerHandle ProbeTimer;
    FTimerHandle PinTimer;
    FTimerHandle RepinTimer;
    FTimerHandle ShotTimer;
    FTimerHandle ShotTimer2;

    void SpawnAtmosphere();
    void SpawnWorld();
    void ProbeBounds();
    void PinCamera();
    void RepinCamera();
    void Shot();
    void Shot2();
};
