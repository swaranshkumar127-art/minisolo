#include "PlayerPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "VoxelWorld.h"

APlayerPawn::APlayerPawn()
{
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetCapsuleComponent());
    SpringArm->TargetArmLength = 0.f;
    SpringArm->bUsePawnControlRotation = true;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    bUseControllerRotationYaw = true;

    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->MaxWalkSpeed = 440.f;
    Move->JumpZVelocity = 430.f;
    Move->GravityScale = 1.8f;
    Move->AirControl = 0.28f;

    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &APlayerPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &APlayerPawn::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &APlayerPawn::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &APlayerPawn::LookUp);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerPawn::StartJump);
    PlayerInputComponent->BindAction("FireL", IE_Pressed, this, &APlayerPawn::OnBreak);
    PlayerInputComponent->BindAction("FireR", IE_Pressed, this, &APlayerPawn::OnPlace);
}

void APlayerPawn::MoveForward(float V)
{
    if (Controller && V != 0.f)
    {
        AddMovementInput(GetActorForwardVector(), V);
    }
}

void APlayerPawn::MoveRight(float V)
{
    if (Controller && V != 0.f)
    {
        AddMovementInput(GetActorRightVector(), V);
    }
}

void APlayerPawn::Turn(float V)
{
    AddControllerYawInput(V);
}

void APlayerPawn::LookUp(float V)
{
    AddControllerPitchInput(V);
}

void APlayerPawn::StartJump()
{
    Jump();
}

void APlayerPawn::OnBreak()
{
    FVector Start;
    FRotator Rot;
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->GetPlayerViewPoint(Start, Rot);
    }
    else
    {
        return;
    }

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(VoxelTrace), false, this);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + Rot.Vector() * 700.f, ECC_WorldStatic, Params))
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor && Hit.GetComponent())
        {
            HitActor = Hit.GetComponent()->GetOwner();
        }
        AVoxelWorld* VW = Cast<AVoxelWorld>(HitActor);
        if (VW)
        {
            // step slightly into the hit block
            FVector Local = VW->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint - Hit.ImpactNormal * 0.5f);
            int32 X = FMath::RoundToInt(Local.X - 0.5f);
            int32 Y = FMath::RoundToInt(Local.Y - 0.5f);
            int32 Z = FMath::RoundToInt(Local.Z - 0.5f);
            VW->SetBlock(X, Y, Z, 0);
        }
    }
}

void APlayerPawn::OnPlace()
{
    FVector Start;
    FRotator Rot;
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->GetPlayerViewPoint(Start, Rot);
    }
    else
    {
        return;
    }

    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(VoxelTrace), false, this);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + Rot.Vector() * 700.f, ECC_WorldStatic, Params))
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor && Hit.GetComponent())
        {
            HitActor = Hit.GetComponent()->GetOwner();
        }
        AVoxelWorld* VW = Cast<AVoxelWorld>(HitActor);
        if (VW)
        {
            // step out of the hit block along its face normal, place plank (id 4)
            FVector Local = VW->GetActorTransform().InverseTransformPosition(Hit.ImpactPoint + Hit.ImpactNormal * 0.5f);
            int32 X = FMath::RoundToInt(Local.X - 0.5f);
            int32 Y = FMath::RoundToInt(Local.Y - 0.5f);
            int32 Z = FMath::RoundToInt(Local.Z - 0.5f);
            VW->SetBlock(X, Y, Z, 4);
        }
    }
}
