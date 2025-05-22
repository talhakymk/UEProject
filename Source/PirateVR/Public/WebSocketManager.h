#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "WebSocketsModule.h"
#include "GameFramework/Actor.h"
#include "WebSocketManager.generated.h"

class AShipPawn;

UCLASS()
class PIRATEVR_API AWebSocketManager : public AActor
{
    GENERATED_BODY()

public:
    AWebSocketManager();

    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    void InitWebSocket(AShipPawn* InShipPawn);
    void OnWebSocketMessage(const FString& Message);

private:
    TSharedPtr<IWebSocket> Socket;
    AShipPawn* ControlledShip;
};