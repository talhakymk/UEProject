#include "WebSocketManager.h"
#include "ShipPawn.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Modules/ModuleManager.h"
#include "Algo/Accumulate.h" // Ortalama hesaplamak için


AWebSocketManager::AWebSocketManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWebSocketManager::BeginPlay()
{
    Super::BeginPlay();
    // Not: InitWebSocket'i d��ar�dan �a��rmal�s�n. Otomatik ba�latmak istersen buraya da koyabilirsin.
}

void AWebSocketManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWebSocketManager::InitWebSocket(AShipPawn* InShipPawn)
{
    ControlledShip = InShipPawn;

    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        FModuleManager::LoadModuleChecked<FWebSocketsModule>("WebSockets");
    }

    Socket = FWebSocketsModule::Get().CreateWebSocket(TEXT("ws://192.168.159.234:81"));

    Socket->OnConnected().AddLambda([]()
        {
            UE_LOG(LogTemp, Log, TEXT("WebSocket connected!"));
        });

    Socket->OnMessage().AddUObject(this, &AWebSocketManager::OnWebSocketMessage);

    Socket->OnConnectionError().AddLambda([](const FString& Error)
        {
            UE_LOG(LogTemp, Error, TEXT("WebSocket connection error: %s"), *Error);
        });

    Socket->Connect();
}

void AWebSocketManager::OnWebSocketMessage(const FString& Message)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        int32 X = JsonObject->GetIntegerField(TEXT("x"));
        int32 Y = JsonObject->GetIntegerField(TEXT("y"));
        int32 PotValue = JsonObject->GetIntegerField(TEXT("pot"));

        FString XStatus = JsonObject->GetStringField(TEXT("xStatus"));
        FString YStatus = JsonObject->GetStringField(TEXT("yStatus"));

        bool Ext2Pressed = JsonObject->GetIntegerField(TEXT("extBtn2")) == 1;
        bool ExtPressed = JsonObject->GetIntegerField(TEXT("extBtn")) == 1;

        if (!ControlledShip) return;

        // Y ekseni için filtreleme
        YBuffer.Add(Y);
        if (YBuffer.Num() > BufferSize)
        {
            YBuffer.RemoveAt(0);
        }

        int32 YAverage = 0;
        if (YBuffer.Num() > 0)
        {
            YAverage = Algo::Accumulate(YBuffer, 0) / YBuffer.Num();
        }

        float ThrottleValue = YAverage;

        // POT (dönüş) için filtreleme
        PotBuffer.Add(PotValue);
        if (PotBuffer.Num() > BufferSize)
        {
            PotBuffer.RemoveAt(0);
        }

        int32 PotAverage = 0;
        if (PotBuffer.Num() > 0)
        {
            PotAverage = Algo::Accumulate(PotBuffer, 0) / PotBuffer.Num();
        }

        //float NormalizedPot = ((float)(PotAverage - 2100) / (4500 - 2100)) * 2.0f - 1.0f;
        float NormalizedPot = ((float)PotAverage - 2048.0f) / 2048.0f;
        NormalizedPot = FMath::Clamp(NormalizedPot, -1.0f, 1.0f);

        // LOG ile test
        UE_LOG(LogTemp, Warning, TEXT("POT raw: %d | avg: %d | normalized: %f"), PotValue, PotAverage, NormalizedPot);

        ControlledShip->MoveForward(ThrottleValue);
        ControlledShip->TargetTurnValue = NormalizedPot;

        if (ExtPressed)
        {
            ControlledShip->Fire_R();
        }

        if (Ext2Pressed)
        {
            ControlledShip->Fire_L();
        }
    }
}
