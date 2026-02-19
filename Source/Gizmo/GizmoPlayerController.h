#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GizmoPlayerController.generated.h"

UENUM(BlueprintType)
enum class EGizmoAxis : uint8
{
    None,
    X,
    Y,
    Z,
    XY,
    YZ,
    XZ
};

UCLASS()
class GIZMO_API AGizmoPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AGizmoPlayerController();
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(BlueprintReadOnly, Category = "Gizmo")
    TArray<AActor*> SelectedActors;
    FVector GroupPivotLocation;
    TMap<AActor*, FVector> ActorStartOffsets;

    UFUNCTION(BlueprintCallable, Category = "Gizmo")
    void SetComponentHighlight(UStaticMeshComponent* Mesh, bool bEnable);
    void SetActorHighlight(AActor* Actor, bool bEnable);
 
    void OnLeftPressed();
    void OnLeftReleased();

    UFUNCTION(Server, Reliable)
    void Server_MoveActors(const FVector& NewPivotLocation,const TArray<AActor*>& ActorsToMove);


protected:
    virtual void SetupInputComponent() override;

private:


    UPROPERTY()
    AActor* GizmoActor = nullptr;

    bool bDragging = false;
    EGizmoAxis CurrentAxis = EGizmoAxis::None;

    FVector DragStartLocation;
    FVector DragStartMouseWorld;
    FVector DragPlaneNormal;


    FVector GetMouseWorldPoint();
};
