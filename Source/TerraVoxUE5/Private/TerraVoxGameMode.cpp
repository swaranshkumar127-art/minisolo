#include "TerraVoxGameMode.h"
#include "VoxelWorld.h"
#include "PlayerPawn.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerCameraManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/MovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

ATerraVoxGameMode::ATerraVoxGameMode()
{
    DefaultPawnClass = APlayerPawn::StaticClass();
    SetActorTickEnabled(true);
    PrimaryActorTick.bCanEverTick = true;
}

void ATerraVoxGameMode::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("TERRAVOX BOOT TEST: GameMode BeginPlay ran - world streaming started"));

    SpawnWorld();
    SpawnAtmosphere();

    GetWorldTimerManager().SetTimer(ProbeTimer, this, &ATerraVoxGameMode::ProbeBounds, 2.f, false);
    GetWorldTimerManager().SetTimer(PinTimer, this, &ATerraVoxGameMode::PinCamera, 1.f, false);
    GetWorldTimerManager().SetTimer(ShotTimer, this, &ATerraVoxGameMode::Shot, 6.f, false);
}

void ATerraVoxGameMode::SpawnWorld()
{
    World = GetWorld()->SpawnActor<AVoxelWorld>(AVoxelWorld::StaticClass(), FVector::ZeroVector, FRotator(0.f, 0.f, 90.f));
}

void ATerraVoxGameMode::SpawnAtmosphere()
{
    UWorld* W = GetWorld();

    Sun = W->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-45.f, -35.f, 0.f));
    if (Sun)
    {
        Sun->GetLightComponent()->SetIntensity(3.4f);
        Sun->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.97f, 0.90f));
        Sun->GetLightComponent()->bAtmosphereSunLight = true;
    }

    Sky = W->SpawnActor<ASkyAtmosphere>();

    Fog = W->SpawnActor<AExponentialHeightFog>();
    if (Fog)
    {
        if (UExponentialHeightFogComponent* FC = Fog->GetComponentByClass<UExponentialHeightFogComponent>())
        {
            FC->SetFogDensity(0.003f);
            FC->SetFogHeightFalloff(0.05f);
            FC->FogInscatteringColor = FColor(143, 158, 173);
        }
    }

    SkyLight = W->SpawnActor<ASkyLight>();
    if (SkyLight)
    {
        if (USkyLightComponent* SLC = SkyLight->GetComponentByClass<USkyLightComponent>())
        {
            SLC->SetLightColor(FLinearColor(0.8f, 0.85f, 0.95f));
            SLC->bRealTimeCapture = false;
        }
    }
}

void ATerraVoxGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeAccum += DeltaTime;
    const float Phase = FMath::Frac(TimeAccum / CycleSeconds);
    const float Elev = FMath::Sin(Phase * 2.f * PI) * 70.f;
    const float Azim = -35.f + Phase * 360.f;

    if (Sun)
    {
        Sun->SetActorRotation(FRotator(Elev, Azim, 0.f));
        const float Day = FMath::Clamp(FMath::Sin(Phase * 2.f * PI), 0.f, 1.f);
        Sun->GetLightComponent()->SetIntensity(FMath::Lerp(0.9f, 3.5f, Day));
        Sun->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.97f, 0.90f) * Day + FLinearColor(0.3f, 0.4f, 0.7f) * (1.f - Day));
    }
    if (Fog)
    {
        const float Day = FMath::Clamp(FMath::Sin(Phase * 2.f * PI), 0.f, 1.f);
        if (UExponentialHeightFogComponent* FC = Fog->GetComponentByClass<UExponentialHeightFogComponent>())
        {
            FC->SetFogDensity(FMath::Lerp(0.008f, 0.003f, Day));
            FC->FogInscatteringColor = FColor((uint8)(FMath::Lerp(70.f, 143.f, Day)), (uint8)(FMath::Lerp(75.f, 158.f, Day)), (uint8)(FMath::Lerp(110.f, 173.f, Day)));
        }
    }
}

void ATerraVoxGameMode::ProbeBounds()
{
    if (World)
    {
        UE_LOG(LogTemp, Log, TEXT("TERRAVOX BOUNDS: origin=%s extent=%s"), *World->VoxelMesh->Bounds.Origin.ToString(), *World->VoxelMesh->Bounds.BoxExtent.ToString());
        DrawDebugBox(GetWorld(), World->VoxelMesh->Bounds.Origin, World->VoxelMesh->Bounds.BoxExtent, FColor::Red, true, 5.f);
    }
}

void ATerraVoxGameMode::PinCamera()
{
    if (!World)
    {
        return;
    }

    // flat-spot search over the voxel grid (gx 2..40, gz 2..55 step 3)
    int32 BestGX = 2, BestGZ = 2;
    float BestScore = TNumericLimits<float>::Max();
    for (int32 GX = 2; GX <= 40; GX += 3)
    {
        for (int32 GZ = 2; GZ <= 55; GZ += 3)
        {
            const float H = (float)World->GetTerrainHeight(GX, GZ);
            float S = 0.f;
            S += FMath::Abs((float)World->GetTerrainHeight(GX + 3, GZ) - H);
            S += FMath::Abs((float)World->GetTerrainHeight(GX - 3, GZ) - H);
            S += FMath::Abs((float)World->GetTerrainHeight(GX, GZ + 3) - H);
            S += FMath::Abs((float)World->GetTerrainHeight(GX, GZ - 3) - H);
            if (S < BestScore)
            {
                BestScore = S;
                BestGX = GX;
                BestGZ = GZ;
            }
        }
    }

    const float GroundH = (float)World->GetTerrainHeight(BestGX, BestGZ);
    // voxel grid (X, Z) == world (X, -Y); voxel +Y == world +Z
    PinnedEye = FVector((float)BestGX, -(float)BestGZ, GroundH + 3.5f);
    bHavePinnedEye = true;

    // freeze the pawn while physics (A8) is broken
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* P = PC->GetPawn())
        {
            P->SetActorEnableCollision(false);
            TArray<UActorComponent*> Comps;
            P->GetComponents(UMovementComponent::StaticClass(), Comps);
            for (UActorComponent* C : Comps)
            {
                C->SetComponentTickEnabled(false);
            }
        }

        UE_LOG(LogTemp, Log, TEXT("TERRAVOX pinned eye to (%s) groundH=%.1f"), *PinnedEye.ToString(), GroundH);
        PC->ClientSetLocation(PinnedEye, FRotator(-8.f, 0.f, 0.f));
    }

    GetWorldTimerManager().SetTimer(RepinTimer, this, &ATerraVoxGameMode::RepinCamera, 0.5f, true);
}

void ATerraVoxGameMode::RepinCamera()
{
    if (!bHavePinnedEye)
    {
        return;
    }
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->ClientSetLocation(PinnedEye, FRotator(-8.f, 0.f, 0.f));
    }
}

void ATerraVoxGameMode::Shot()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (PC->PlayerCameraManager)
        {
            UE_LOG(LogTemp, Log, TEXT("DBG CAM pos=%s rot=%s fov=%.1f"),
                *PC->PlayerCameraManager->GetCameraLocation().ToString(),
                *PC->PlayerCameraManager->GetCameraRotation().ToString(),
                PC->PlayerCameraManager->GetFOVAngle());
        }
        GetWorld()->Exec(GetWorld(), TEXT("show Default"));
        GetWorldTimerManager().SetTimer(ShotTimer2, this, &ATerraVoxGameMode::Shot2, 0.8f, false);
    }
}

void ATerraVoxGameMode::Shot2()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        GetWorld()->Exec(GetWorld(), TEXT("HighResShot 1280x720"));
        UE_LOG(LogTemp, Log, TEXT("TERRAVOX HighResShot requested"));
    }
}
