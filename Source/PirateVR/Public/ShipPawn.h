#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShipPawn.generated.h"


// Forward declarations
class USpringArmComponent;
class UCameraComponent;
class AActor;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ship Control")
	bool IsPlayer = true;




private:
	// Mesh component for the boat
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BoatMesh;

	// Spring Arm for the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	// Camera attached to the Spring Arm
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	// Desired water level (Z-coordinate for the boat to stay on water)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float DesiredWaterLevelZ = 1020.0f; // Adjust this value to match your water surface level

	// Movement variables
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeed = 1200.0f; // Maximum speed

	UPROPERTY(EditAnywhere, Category = "Movement")
	float Acceleration = 200.0f; // Acceleration rate

	UPROPERTY(EditAnywhere, Category = "Movement")
	float Deceleration = 300.0f; // Deceleration rate

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TurnSpeed = 50.0f; // Turning speed

	float CurrentSpeed = 0.0f; // Current speed of the boat
	float InputThrottle = 0.0f; // Forward/backward input

	// Oscillation variables for swaying effect
	UPROPERTY(EditAnywhere, Category = "Sway")
	float SwayAmplitude = 5.0f; // Amplitude of the sway

	UPROPERTY(EditAnywhere, Category = "Sway")
	float SwayFrequency = 2.0f; // Frequency of the sway

	float SwayTime = 0.0f; // Internal timer for sway effect


	// Movement logic
	void MoveForward(float Value);
	void Turn(float Value);
	void TurnCamera(float Value);

	// Fire Logic
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Fire(); //Fire Function

	// Cannonball Blueprint Reference
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AActor> CannonballClass;
};