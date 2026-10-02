#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerPawn.generated.h"

UCLASS()
class TERRAVOXUE5_API APlayerPawn : public ACharacter
{
    GENERATED_BODY()

public:
    APlayerPawn();

    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere) class USpringArmComponent* SpringArm;
    UPROPERTY(VisibleAnywhere) class UCameraComponent* Camera;

    void MoveForward(float V);
    void MoveRight(float V);
    void Turn(float V);
    void LookUp(float V);
    void StartJump();
    void OnBreak();
    void OnPlace();
};
