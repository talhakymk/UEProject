#include "ShipPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AShipPawn::AShipPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = Root;

    // Mesh
    BoatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoatMesh"));
    BoatMesh->SetupAttachment(RootComponent);
    BoatMesh->SetSimulatePhysics(false);
    BoatMesh->SetEnableGravity(false);
    BoatMesh->SetMobility(EComponentMobility::Movable);

    // Camera System
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 300.0f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));


    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    
    // Movement Component
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->UpdatedComponent = RootComponent;
    ProjectileMovementComponent->bRotationFollowsVelocity = false;
    ProjectileMovementComponent->bShouldBounce = false;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AShipPawn::BeginPlay()
{
    Super::BeginPlay();

    if (!IsPlayer)
    {
        if (SpringArm) SpringArm->Deactivate();
        if (Camera) Camera->Deactivate();
    }

    if (IsPlayer)
    {
        AutoPossessPlayer = EAutoReceiveInput::Player0;
    }
}

void AShipPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Ývme hesaplama
    if (InputThrottle > 0.0f)
    {
        if (CurrentSpeed < 0.0f)
        {
            CurrentSpeed = FMath::Min(CurrentSpeed + Deceleration * DeltaTime, 0.0f);
        }
        else
        {
            CurrentSpeed = FMath::Clamp(CurrentSpeed + Acceleration * DeltaTime, 0.0f, MaxSpeed);
        }
    }
    else if (InputThrottle < 0.0f)
    {
        if (CurrentSpeed > 0.0f)
        {
            CurrentSpeed = FMath::Max(CurrentSpeed - Deceleration * DeltaTime, 0.0f);
        }
        else
        {
            CurrentSpeed = FMath::Clamp(CurrentSpeed - Acceleration * DeltaTime, -MaxSpeed, 0.0f);
        }
    }
    else
    {
        float FrictionDeceleration = 2.0f;

        if (CurrentSpeed > 0.0f)
        {
            CurrentSpeed = FMath::Max(0.0f, CurrentSpeed - FrictionDeceleration * DeltaTime);
        }
        else if (CurrentSpeed < 0.0f)
        {
            CurrentSpeed = FMath::Min(0.0f, CurrentSpeed + FrictionDeceleration * DeltaTime);
        }
    }

    // Projectile Movement ile ileri yönlü hareket
    FVector ForwardVector = GetActorForwardVector();
    ProjectileMovementComponent->Velocity = ForwardVector * CurrentSpeed;

    // Z seviyesi sabitleniyor
    FVector Location = GetActorLocation();
    Location.Z = DesiredWaterLevelZ;
    SetActorLocation(Location);


    // Sallanma efekti
    SwayTime += DeltaTime;
    float SwayOffset = FMath::Sin(SwayTime * SwayFrequency) * SwayAmplitude;
    FRotator NewMeshRotation = BoatMesh->GetRelativeRotation();
    NewMeshRotation.Roll = SwayOffset;
    BoatMesh->SetRelativeRotation(NewMeshRotation);
}

void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AShipPawn::MoveForward);
    PlayerInputComponent->BindAxis("Turn", this, &AShipPawn::Turn);
    PlayerInputComponent->BindAxis("TurnCamera", this, &AShipPawn::TurnCamera);

    PlayerInputComponent->BindAction("Fire_R", IE_Pressed, this, &AShipPawn::Fire_R);
    PlayerInputComponent->BindAction("Fire_L", IE_Pressed, this, &AShipPawn::Fire_L);
}

void AShipPawn::MoveForward(float Value)
{
    InputThrottle = Value;
}

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

void AShipPawn::Fire_R()
{
    if (CannonballClass)
    {
        FVector SpawnLocation = BoatMesh->GetComponentLocation() +
            (GetActorForwardVector() * 15.0f) +
            (GetActorRightVector() * 30.0f);

        FRotator SpawnRotation = GetActorRightVector().Rotation();
        SpawnRotation.Pitch += 5.0f;

        GetWorld()->SpawnActor<AActor>(CannonballClass, SpawnLocation, SpawnRotation);
    }
}

void AShipPawn::Fire_L()
{
    if (CannonballClass)
    {
        FVector SpawnLocation = BoatMesh->GetComponentLocation() +
            (GetActorForwardVector() * 55.0f) -
            (GetActorRightVector() * 30.0f);

        FRotator SpawnRotation = (-GetActorRightVector()).Rotation();
        SpawnRotation.Pitch += 5.0f;

        GetWorld()->SpawnActor<AActor>(CannonballClass, SpawnLocation, SpawnRotation);
    }
}



