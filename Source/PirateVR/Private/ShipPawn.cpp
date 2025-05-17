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
    BoatMesh->SetupAttachment(RootComponent); // BoatMesh'i yeni RootComponent'e ba�l�yoruz

    BoatMesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

    // Disable physics simulation and gravity
    BoatMesh->SetSimulatePhysics(false);
    BoatMesh->SetEnableGravity(false);
    BoatMesh->SetMobility(EComponentMobility::Movable);

    // Create a Spring Arm for the camera
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent); // SpringArm art�k RootComponent'e ba�l�
    SpringArm->TargetArmLength = 300.0f; // Distance from the ship
    SpringArm->bUsePawnControlRotation = true; // Rotate based on controller input
    SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); // Position above the ship

    // Create a Camera and attach it to the Spring Arm
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

}

// BeginPlay
void AShipPawn::BeginPlay()
{
    Super::BeginPlay();

    // Kamera kapatma i�lemi
    if (!IsPlayer)
    {
        if (SpringArm) SpringArm->Deactivate();
        if (Camera) Camera->Deactivate();
    }

    if (IsPlayer) {
        // Automatically possess this pawn as Player 0
        AutoPossessPlayer = EAutoReceiveInput::Player0;
    }

}

void AShipPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // E�er ileri hareket varsa
    if (InputThrottle > 0.0f)
    {
        // E�er gemi geri gidiyorsa, �nce durmas� gerekiyor
        if (CurrentSpeed < 0.0f)
        {
            CurrentSpeed = FMath::Min(CurrentSpeed + Deceleration * DeltaTime, 0.0f); // Yava��a s�f�ra getir
        }
        else
        {
            CurrentSpeed = FMath::Clamp(CurrentSpeed + Acceleration * DeltaTime, 0.0f, MaxSpeed); // H�zlan
        }
    }
    // E�er geri hareket varsa
    else if (InputThrottle < 0.0f)
    {
        // E�er gemi ileri gidiyorsa, �nce durmas� gerekiyor
        if (CurrentSpeed > 0.0f)
        {
            CurrentSpeed = FMath::Max(CurrentSpeed - Deceleration * DeltaTime, 0.0f); // Yava��a s�f�ra getir
        }
        else
        {
            CurrentSpeed = FMath::Clamp(CurrentSpeed - Acceleration * DeltaTime, -MaxSpeed, 0.0f); // Geriye h�zlan
        }
    }
    // E�er giri� yoksa (InputThrottle == 0.0f)
    else
    {
        float FrictionDeceleration = 2.0f; // S�rt�nme katsay�s�

        if (CurrentSpeed > 0.0f)
        {
            CurrentSpeed = FMath::Max(0.0f, CurrentSpeed - FrictionDeceleration * DeltaTime); // �leri hareketi yava�lat
        }
        else if (CurrentSpeed < 0.0f)
        {
            CurrentSpeed = FMath::Min(0.0f, CurrentSpeed + FrictionDeceleration * DeltaTime); // Geri hareketi yava�lat
        }
    }

    // Gemiyi hareket ettir
    FVector ForwardVector = GetActorForwardVector();
    FVector NewLocation = GetActorLocation() + (ForwardVector * CurrentSpeed * DeltaTime);

    // Sabit su seviyesi
    NewLocation.Z = DesiredWaterLevelZ;
    SetActorLocation(NewLocation, true);

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
    if (GEngine)
    {
        // Ekrana mesaj yazd�r
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Fire function called!"));
    }

    if (CannonballClass)
    {
        // Gemi sa� taraf�na g�re spawn pozisyonunu belirle
        FVector SpawnLocation = BoatMesh->GetComponentLocation() +
            (GetActorForwardVector() * 15.0f) +  // Geminin �n k�sm�na bir miktar uzakl�k
            (GetActorRightVector() * 30.0f);    // Geminin sa� taraf�na bir miktar uzakl�k

        // Gemi sa� y�n�ne do�ru rotasyonu ayarla
        FRotator SpawnRotation = GetActorRightVector().Rotation();
        SpawnRotation.Pitch += 5.0f; // Negatif pitch yukar� y�nl�d�r


        // Mermiyi spawn et
        AActor* SpawnedCannonball = GetWorld()->SpawnActor<AActor>(CannonballClass, SpawnLocation, SpawnRotation);

        if (SpawnedCannonball)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Cannonballlll spawned successfully to the right!"));
            }
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to spawn Cannonball!"));
            }
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("CannonballClass is not set!"));
        }
    }
}