#include "ShipPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "WebSocketManager.h"
#include "EngineUtils.h"

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
    SpringArm->bUsePawnControlRotation = false;
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

        // WebSocketManager aktörünü sahnede bul
        for (TActorIterator<AWebSocketManager> It(GetWorld()); It; ++It)
        {
            AWebSocketManager* WSManager = *It;
            if (WSManager)
            {
                WSManager->InitWebSocket(this); // Gemi referansýný gönder
                break;
            }
        }
    }
}

void AShipPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Ývme hesaplama
    if (InputThrottle == 0.0f)
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
    else if (InputThrottle >= 3000.0f)
    {
        if (CurrentSpeed > 0.0f)
        {
            CurrentSpeed = FMath::Max(CurrentSpeed - Deceleration * DeltaTime, 0.0f);
        }
        else
        {
            //CurrentSpeed = FMath::Clamp(CurrentSpeed - Acceleration * DeltaTime, -MaxSpeed, 0.0f);
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
    CurrentTurnValue = FMath::FInterpTo(CurrentTurnValue, TargetTurnValue, DeltaTime, InterpSpeed);
    AddActorLocalRotation(FRotator(0.0f, CurrentTurnValue * TurnSpeed * DeltaTime, 0.0f));

    FRotator TargetRotation = GetActorRotation();
    TargetRotation.Pitch = 0.0f; // Ýstersen eðimi sabitle
    TargetRotation.Roll = 0.0f;  // Sallanma efekti sadece mesh'te kalmalý
    SpringArm->SetWorldRotation(TargetRotation);

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

    //PlayerInputComponent->BindAxis("MoveForward", this, &AShipPawn::MoveForward);
    //PlayerInputComponent->BindAxis("Turn", this, &AShipPawn::Turn);
    //PlayerInputComponent->BindAxis("TurnCamera", this, &AShipPawn::TurnCamera);

    //PlayerInputComponent->BindAction("Fire_R", IE_Pressed, this, &AShipPawn::Fire_R);
    //PlayerInputComponent->BindAction("Fire_L", IE_Pressed, this, &AShipPawn::Fire_L);
}

void AShipPawn::MoveForward(float Value)
{
    UE_LOG(LogTemp, Warning, TEXT("MoveForward from WebSocket: %f"), Value);

    InputThrottle = Value;

}

void AShipPawn::Turn(float Value)
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();

    // Interpolation speed ayarý
    //float InterpSpeed = 3.0f;           // FInterpTo için
    float ConstantInterpSpeed = 2.0f;   // FInterpConstantTo için

    // Sabit hýzla yaklaþma
    //CurrentTurnValue = FMath::FInterpConstantTo(CurrentTurnValue, Value, DeltaTime, ConstantInterpSpeed);

    // Alternatif: Yorum açarsan bu satýrý test et
     CurrentTurnValue = FMath::FInterpTo(CurrentTurnValue, Value, DeltaTime, InterpSpeed);

    UE_LOG(LogTemp, Log, TEXT("TURN (smoothed): %f | Target: %f"), CurrentTurnValue, Value);

    AddActorLocalRotation(FRotator(0.0f, CurrentTurnValue * TurnSpeed * DeltaTime, 0.0f));

}

/*void AShipPawn::TurnCamera(float Value)
{
    AddControllerYawInput(Value);
}*/

void AShipPawn::Fire_R()
{
    if (IsPlayer)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastFireTime_R < FireCooldown)
        {
            UE_LOG(LogTemp, Warning, TEXT("Fire_R on cooldown!"));
            return;
        }

        LastFireTime_R = CurrentTime;
    }

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
    if (IsPlayer)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastFireTime_R < FireCooldown)
        {
            UE_LOG(LogTemp, Warning, TEXT("Fire_L on cooldown!"));
            return;
        }

        LastFireTime_R = CurrentTime;
    }

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





