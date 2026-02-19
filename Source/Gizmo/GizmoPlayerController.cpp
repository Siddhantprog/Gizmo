#include "GizmoPlayerController.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"

AGizmoPlayerController::AGizmoPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    bDragging = false;
    CurrentAxis = EGizmoAxis::None;
}

void AGizmoPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindAction("LeftClick", IE_Pressed, this, &AGizmoPlayerController::OnLeftPressed);
    InputComponent->BindAction("LeftClick", IE_Released, this, &AGizmoPlayerController::OnLeftReleased);
}

void AGizmoPlayerController::OnLeftPressed()
{
    FHitResult Hit;
    GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (!Hit.bBlockingHit)
        return;

    CurrentAxis = EGizmoAxis::None;

    // Axis detection
    if (Hit.Component.IsValid())
    {
        if (Hit.Component->ComponentHasTag("Axis_X"))      CurrentAxis = EGizmoAxis::X;
        else if (Hit.Component->ComponentHasTag("Axis_Y")) CurrentAxis = EGizmoAxis::Y;
        else if (Hit.Component->ComponentHasTag("Axis_Z")) CurrentAxis = EGizmoAxis::Z;
        else if (Hit.Component->ComponentHasTag("Plane_XY")) CurrentAxis = EGizmoAxis::XY;
        else if (Hit.Component->ComponentHasTag("Plane_YZ")) CurrentAxis = EGizmoAxis::YZ;
        else if (Hit.Component->ComponentHasTag("Plane_XZ")) CurrentAxis = EGizmoAxis::XZ;
    }

    if (CurrentAxis != EGizmoAxis::None && SelectedActors.Num() > 0)
    {
        bDragging = true;

        SetIgnoreLookInput(true);
        SetIgnoreMoveInput(true);

        DragStartLocation = GroupPivotLocation;

        //  Calculate Drag Plane Normal ONCE 

        FVector AxisX = FVector::ForwardVector;
        FVector AxisY = FVector::RightVector;
        FVector AxisZ = FVector::UpVector;

        switch (CurrentAxis)
        {
        case EGizmoAxis::X:
            DragPlaneNormal = FVector::CrossProduct(
                AxisX,
                PlayerCameraManager->GetActorForwardVector()
            );
            break;

        case EGizmoAxis::Y:
            DragPlaneNormal = FVector::CrossProduct(
                AxisY,
                PlayerCameraManager->GetActorForwardVector()
            );
            break;

        case EGizmoAxis::Z:
            DragPlaneNormal = FVector::CrossProduct(
                AxisZ,
                PlayerCameraManager->GetActorForwardVector()
            );
            break;

        case EGizmoAxis::XY: DragPlaneNormal = AxisZ; break;
        case EGizmoAxis::YZ: DragPlaneNormal = AxisX; break;
        case EGizmoAxis::XZ: DragPlaneNormal = AxisY; break;
        }

        if (DragPlaneNormal.IsNearlyZero())
        {
            DragPlaneNormal = FVector::UpVector;
        }

        DragPlaneNormal.Normalize();

        // Calculate Drag Start Mouse World

        FVector RayStart, RayDir;
        DeprojectMousePositionToWorld(RayStart, RayDir);

        FPlane DragPlane(DragStartLocation, DragPlaneNormal);

        DragStartMouseWorld = FMath::LinePlaneIntersection(
            RayStart,
            RayStart + RayDir * 10000.f,
            DragPlane
        );
        ActorStartOffsets.Empty();

        for (AActor* Actor : SelectedActors)
        {
            ActorStartOffsets.Add(
                Actor,
                Actor->GetActorLocation() - GroupPivotLocation
            );
        }

        return;
    }

    //Actor Selection Logic

    AActor* HitActor = Hit.GetActor();

    if (!HitActor || HitActor->ActorHasTag("Gizmo"))
        return;

    bool bCtrlPressed = IsInputKeyDown(EKeys::LeftControl);

    //Single click (no ctrl)
    if (!bCtrlPressed)
    {
        // If clicking the only selected actor → deselect it
        if (SelectedActors.Num() == 1 && SelectedActors.Contains(HitActor))
        {
            SetActorHighlight(HitActor, false);
            SelectedActors.Empty();

            // Destroy gizmo
            if (GizmoActor)
            {
                GizmoActor->Destroy();
                GizmoActor = nullptr;
            }

            return;
        }

        // Otherwise clear previous selection
        for (AActor* Actor : SelectedActors)
        {
            SetActorHighlight(Actor, false);
        }

        SelectedActors.Empty();

        // Select new actor
        SelectedActors.Add(HitActor);
        SetActorHighlight(HitActor, true);
    }
    //Ctrl toggle selection
    else
    {
        if (SelectedActors.Contains(HitActor))
        {
            SetActorHighlight(HitActor, false);
            SelectedActors.Remove(HitActor);
        }
        else
        {
            SelectedActors.Add(HitActor);
            SetActorHighlight(HitActor, true);
        }
    }


    //Update gizmo position or destroy if nothing selected
    if (SelectedActors.Num() > 0)
    {
        FVector Center = FVector::ZeroVector;

        for (AActor* Actor : SelectedActors)
        {
            Center += Actor->GetActorLocation();
        }

        GroupPivotLocation = Center / SelectedActors.Num();

        if (GizmoActor)
        {
            GizmoActor->Destroy();
            GizmoActor = nullptr;
        }

        UClass* GizmoClass = LoadClass<AActor>(
            nullptr,
            TEXT("/Game/BP_RuntimeTransformGizmo.BP_RuntimeTransformGizmo_C")
        );

        if (GizmoClass)
        {
            GizmoActor = GetWorld()->SpawnActor<AActor>(
                GizmoClass,
                GroupPivotLocation,
                FRotator::ZeroRotator
            );
        }
    }
    else
    {
        if (GizmoActor)
        {
            GizmoActor->Destroy();
            GizmoActor = nullptr;
        }
    }

}

void AGizmoPlayerController::OnLeftReleased()
{
    bDragging = false;
    CurrentAxis = EGizmoAxis::None;

    SetIgnoreLookInput(false);
    SetIgnoreMoveInput(false);
}

// Main dragging logic

void AGizmoPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDragging || SelectedActors.Num() == 0) return;

    FVector RayStart, RayDir;
    DeprojectMousePositionToWorld(RayStart, RayDir);

    FVector AxisX = FVector::ForwardVector;
    FVector AxisY = FVector::RightVector;
    FVector AxisZ = FVector::UpVector;

    FPlane DragPlane(DragStartLocation, DragPlaneNormal);

    FVector Intersection = FMath::LinePlaneIntersection(
        RayStart,
        RayStart + RayDir * 10000.f,
        DragPlane
    );

    FVector Delta = Intersection - DragStartMouseWorld;
    FVector NewLocation = DragStartLocation;
    FVector NewPivot = DragStartLocation + Delta;

    switch (CurrentAxis)
    {
    case EGizmoAxis::X:  NewLocation += AxisX * FVector::DotProduct(Delta, AxisX); break;
    case EGizmoAxis::Y:  NewLocation += AxisY * FVector::DotProduct(Delta, AxisY); break;
    case EGizmoAxis::Z:  NewLocation += AxisZ * FVector::DotProduct(Delta, AxisZ); break;
    case EGizmoAxis::XY: NewLocation += Delta - FVector::DotProduct(Delta, AxisZ) * AxisZ; break;
    case EGizmoAxis::YZ: NewLocation += Delta - FVector::DotProduct(Delta, AxisX) * AxisX; break;
    case EGizmoAxis::XZ: NewLocation += Delta - FVector::DotProduct(Delta, AxisY) * AxisY; break;
    default: break;
    }

	// Smooth movement for local player, server update for others

    if (!NewLocation.Equals(GroupPivotLocation, 0.1f))
    {
        if (!HasAuthority())
        {
            Server_MoveActors(NewLocation, SelectedActors);
        }
        for (AActor* Actor : SelectedActors)
        {
            FVector Offset = ActorStartOffsets[Actor];
            Actor->SetActorLocation(NewLocation + Offset);
        }

        GroupPivotLocation = NewLocation;
    }

	// If location didn't change much, just update gizmo and pivot to avoid jitter

    for (AActor* Actor : SelectedActors)
    {
        if (!ActorStartOffsets.Contains(Actor)) continue;

        FVector Offset = ActorStartOffsets[Actor];
        Actor->SetActorLocation(NewLocation + Offset);
    }

    if (GizmoActor)
    {
        GizmoActor->SetActorLocation(NewLocation);
    }

    GroupPivotLocation = NewLocation;
}

// Utility functions

FVector AGizmoPlayerController::GetMouseWorldPoint()
{
    FVector RayStart, RayDir;
    DeprojectMousePositionToWorld(RayStart, RayDir);

    FPlane Plane(GroupPivotLocation, FVector::UpVector);

    return FMath::LinePlaneIntersection(
        RayStart,
        RayStart + RayDir * 10000.f,
        Plane
    );
}

// Highlighting logic: assumes the material has "HighlightColor" and "EmissiveStrength" parameters
void AGizmoPlayerController::SetComponentHighlight(UStaticMeshComponent* Mesh, bool bEnable)
{
    if (!Mesh) return;

  
        // Get existing material
        UMaterialInterface* BaseMat = Mesh->GetMaterial(0);

        // Try to get existing dynamic instance
        UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(BaseMat);

        // If not dynamic yet, create one
        if (!DynMat)
        {
            DynMat = Mesh->CreateAndSetMaterialInstanceDynamic(0);
        }

        if (DynMat)
        {
            // Set highlight color
            DynMat->SetVectorParameterValue(TEXT("HighlightColor"), FLinearColor::Green );

            // Enable / Disable glow
            DynMat->SetScalarParameterValue(TEXT("EmissiveStrength"),bEnable ? 2.f : 0.f );
        }
}

void AGizmoPlayerController::SetActorHighlight(AActor* Actor, bool bEnable)
{
    TArray<UStaticMeshComponent*> Meshes;
    Actor->GetComponents<UStaticMeshComponent>(Meshes);

    for (UStaticMeshComponent* Mesh : Meshes)
    {
        SetComponentHighlight(Mesh, bEnable);
    }
}


void AGizmoPlayerController::Server_MoveActors_Implementation(const FVector& NewPivotLocation, const TArray<AActor*>& ActorsToMove)
{
    for (AActor* Actor : ActorsToMove)
    {
        if (!Actor) continue;

        FVector Offset = Actor->GetActorLocation() - GroupPivotLocation;
        Actor->SetActorLocation(NewPivotLocation + Offset);
    }

    GroupPivotLocation = NewPivotLocation;
}
