#include "ShipPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

AShipPawn::AShipPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create Static Mesh Component
    BoatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoatMesh"));
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")); // Yeni bir RootComponent ekleniyor
    BoatMesh->SetupAttachment(RootComponent); // BoatMesh'i yeni RootComponent'e baðlýyoruz

    BoatMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f)); 

    // Disable physics simulation and gravity
    BoatMesh->SetSimulatePhysics(false);
    BoatMesh->SetEnableGravity(false);
    BoatMesh->SetMobility(EComponentMobility::Movable);

    // Create a Spring Arm for the camera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent); // SpringArm artýk RootComponent'e baðlý
    SpringArm->TargetArmLength = 300.0f; // Distance from the ship
    SpringArm->bUsePawnControlRotation = true; // Rotate based on controller input
    SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // Position above the ship

    // Create a Camera and attach it to the Spring Arm
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    // Automatically possess this pawn as Player 0
    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// BeginPlay
void AShipPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AShipPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Eðer ileri hareket varsa
    if (InputThrottle > 0.0f)
    {
        // Eðer gemi geri gidiyorsa, önce durmasý gerekiyor
        if (CurrentSpeed < 0.0f)
        {
            CurrentSpeed = FMath::Min(CurrentSpeed + Deceleration * DeltaTime, 0.0f); // Yavaþça sýfýra getir
        }
        else
        {
            CurrentSpeed = FMath::Clamp(CurrentSpeed + Acceleration * DeltaTime, 0.0f, MaxSpeed); // Hýzlan
        }
    }
    // Eðer geri hareket varsa
    else if (InputThrottle < 0.0f)
    {
        // Eðer gemi ileri gidiyorsa, önce durmasý gerekiyor
        if (CurrentSpeed > 0.0f)
        {
            CurrentSpeed = FMath::Max(CurrentSpeed - Deceleration * DeltaTime, 0.0f); // Yavaþça sýfýra getir
        }
        else
        {
            CurrentSpeed = FMath::Clamp(CurrentSpeed - Acceleration * DeltaTime, -MaxSpeed, 0.0f); // Geriye hýzlan
        }
    }
    // Eðer giriþ yoksa (InputThrottle == 0.0f)
    else
    {
        float FrictionDeceleration = 2.0f; // Sürtünme katsayýsý

        if (CurrentSpeed > 0.0f)
        {
            CurrentSpeed = FMath::Max(0.0f, CurrentSpeed - FrictionDeceleration * DeltaTime); // Ýleri hareketi yavaþlat
        }
        else if (CurrentSpeed < 0.0f)
        {
            CurrentSpeed = FMath::Min(0.0f, CurrentSpeed + FrictionDeceleration * DeltaTime); // Geri hareketi yavaþlat
        }
    }

    // Gemiyi hareket ettir
    FVector ForwardVector = GetActorForwardVector();
    FVector NewLocation = GetActorLocation() + (ForwardVector * CurrentSpeed * DeltaTime);

    // Sabit su seviyesi
    NewLocation.Z = DesiredWaterLevelZ;
    SetActorLocation(NewLocation);

    // Sallanma efekti
    SwayTime += DeltaTime;
    float SwayOffset = FMath::Sin(SwayTime * SwayFrequency) * SwayAmplitude;
    FRotator NewMeshRotation = BoatMesh->GetRelativeRotation();
    NewMeshRotation.Roll = SwayOffset;
    BoatMesh->SetRelativeRotation(NewMeshRotation);
}

// Input setup
void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShipPawn::MoveForward);
	PlayerInputComponent->BindAxis("Turn", this, &AShipPawn::Turn);

    PlayerInputComponent->BindAxis("TurnCamera", this, &AShipPawn::TurnCamera);

    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShipPawn::Fire);
}

// Forward movement input
void AShipPawn::MoveForward(float Value)
{
	InputThrottle = Value;
}

// Turning input
void AShipPawn::Turn(float Value)
{
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += Value * TurnSpeed * GetWorld()->GetDeltaSeconds();
	SetActorRotation(NewRotation);
}

void AShipPawn::TurnCamera(float Value)
{
    AddControllerYawInput(Value);
}

void AShipPawn::Fire()
{
    if (CannonballClass)
    {
        // Gemi pozisyonu ve yönü
        FVector SpawnLocation = BoatMesh->GetComponentLocation() + (GetActorForwardVector() * 200.0f); // Gemi önünden ateþle
        FRotator SpawnRotation = GetActorRotation();

        // Cannonball'u spawn et
        GetWorld()->SpawnActor<AActor>(CannonballClass, SpawnLocation, SpawnRotation);
    }
}