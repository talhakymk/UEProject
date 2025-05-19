#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "ShipPawn.generated.h"

// Forward declarations
class USpringArmComponent;
class UCameraComponent;
class AActor;
class UProjectileMovementComponent;
class USceneComponent;

UCLASS()
class PIRATEVR_API AShipPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AShipPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Whether this is the player's ship
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Control")
	bool IsPlayer = true;

	
private:
	// Root component (needed for projectile movement)
	UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	// Mesh component for the boat
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BoatMesh;

	// Spring Arm for the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	// Camera attached to the Spring Arm
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	// Movement Component (replaces SetActorLocation movement)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovementComponent;

	// Desired water level (Z-coordinate for the boat to stay on water)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float DesiredWaterLevelZ = 1020.0f;

	// Movement variables
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float Acceleration = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float Deceleration = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TurnSpeed = 50.0f;

	float CurrentSpeed = 0.0f;
	float InputThrottle = 0.0f;

	// Oscillation variables for swaying effect
	UPROPERTY(EditAnywhere, Category = "Sway")
	float SwayAmplitude = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Sway")
	float SwayFrequency = 2.0f;

	float SwayTime = 0.0f;

	// Movement logic
	void MoveForward(float Value);
	void Turn(float Value);
	void TurnCamera(float Value);

	// Combat logic
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Fire_R();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Fire_L();

	// Cannonball Blueprint Reference
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AActor> CannonballClass;
};
