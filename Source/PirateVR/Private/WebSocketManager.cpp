#include "WebSocketManager.h"
#include "ShipPawn.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Modules/ModuleManager.h"

AWebSocketManager::AWebSocketManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWebSocketManager::BeginPlay()
{
    Super::BeginPlay();
    // Not: InitWebSocket'i dýþarýdan çaðýrmalýsýn. Otomatik baþlatmak istersen buraya da koyabilirsin.
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

    Socket = FWebSocketsModule::Get().CreateWebSocket(TEXT("ws://192.168.106.234:81"));

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
        // Heartbeat mesajlarýný atla
        if (JsonObject->HasField(TEXT("heartbeat")))
        {
            return;
        }

        // Raw deðerler (debug için)
        int32 X = JsonObject->GetIntegerField(TEXT("x"));
        int32 Y = JsonObject->GetIntegerField(TEXT("y"));

        // NORMALIZE EDÝLMÝÞ deðerleri kullan (-1.0 to 1.0)
        float NormalizedX = 0.0f;
        float NormalizedY = 0.0f;

        if (JsonObject->HasField(TEXT("normalizedX")))
        {
            NormalizedX = JsonObject->GetNumberField(TEXT("normalizedX"));
        }

        if (JsonObject->HasField(TEXT("normalizedY")))
        {
            NormalizedY = JsonObject->GetNumberField(TEXT("normalizedY"));
        }

        // Status bilgileri (isteðe baðlý)
        FString XStatus = JsonObject->GetStringField(TEXT("xStatus"));
        FString YStatus = JsonObject->GetStringField(TEXT("yStatus"));

        // Buton durumlarý
        bool JsPressed = JsonObject->GetIntegerField(TEXT("jsBtn")) == 1;
        bool ExtPressed = JsonObject->GetIntegerField(TEXT("extBtn")) == 1;

        if (!ControlledShip) return;

        // Dead zone uygula (küçük hareketleri filtrele)
        const float DeadZone = 0.15f; // %15 dead zone

        float ThrottleValue = 0.0f;
        float TurnValue = 0.0f;

        // Y ekseni  ileri/geri hareket (normalize edilmiþ deðer kullan)
        if (FMath::Abs(NormalizedY) > DeadZone)
        {
            ThrottleValue = -NormalizedY; // Y'yi ters çevir (yukarý = ileri)
            ThrottleValue = FMath::Clamp(ThrottleValue, -1.0f, 1.0f); // Güvenlik sýnýrý
        }

        // X ekseni  sað/sol dönüþ (normalize edilmiþ deðer kullan)
        if (FMath::Abs(NormalizedX) > DeadZone)
        {
            TurnValue = NormalizedX;
            TurnValue = FMath::Clamp(TurnValue, -1.0f, 1.0f); // Güvenlik sýnýrý
        }

        // Debug çýktýsý (deðerleri kontrol et)
        if (ThrottleValue != 0.0f || TurnValue != 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Joystick Input - Throttle: %.3f, Turn: %.3f (Raw X:%d Y:%d)"),
                ThrottleValue, TurnValue, X, Y);
        }

        // Gemi kontrolü
        ControlledShip->MoveForward(ThrottleValue);
        ControlledShip->Turn(TurnValue);

        // Buton kontrolleri (edge detection için static deðiþkenler)
        static bool LastJsPressed = false;
        static bool LastExtPressed = false;

        // Joystick butonu basýldýðýnda (sadece ilk basýþta tetikle)
        if (JsPressed && !LastJsPressed)
        {
            ControlledShip->Fire_L();
            UE_LOG(LogTemp, Log, TEXT("Joystick button pressed - Fire L!"));
        }

        // Harici buton basýldýðýnda (sadece ilk basýþta tetikle)
        if (ExtPressed && !LastExtPressed)
        {
            ControlledShip->Fire_R();
            UE_LOG(LogTemp, Log, TEXT("External button pressed - Fire R!"));
        }

        // Son durumlarý kaydet
        LastJsPressed = JsPressed;
        LastExtPressed = ExtPressed;
    }
}