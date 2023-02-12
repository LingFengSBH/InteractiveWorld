// Copyright 2023 Sun BoHeng

#include "WorldInteractVolume.h"
#include "InteractBrush.h"
#include "WorldDrawingBoard.h"
#include "Components/BrushComponent.h"
#include "Kismet/GameplayStatics.h"


AWorldInteractVolume::AWorldInteractVolume()
{
}

void AWorldInteractVolume::BeginPlay()
{
 Super::BeginPlay();

 GetBrushComponent()->OnComponentBeginOverlap.AddDynamic(this, &AWorldInteractVolume::OnActorEnteredArea);
 GetBrushComponent()->OnComponentEndOverlap.AddDynamic(this, &AWorldInteractVolume::OnActorLeavedArea);

 //I don't know why there are Error when I use TActor Type  TActorIterator<AWorldDrawingBoard>.S
 //'TActorIterator<AWorldDrawingBoard>' is incomplete,why???
 TArray<AActor*> FindDrawingBoards;
 UGameplayStatics::GetAllActorsOfClass(GetWorld(),AWorldDrawingBoard::StaticClass(),FindDrawingBoards);
 for (const auto FindActor : FindDrawingBoards)
 {
  if (Cast<AWorldDrawingBoard>(FindActor)->GetUseInteractVolume()&&Cast<AWorldDrawingBoard>(FindActor)->GetInteractVolumes().Find(this) != -1)
  {
   BindingDrawingBoards.AddUnique(Cast<AWorldDrawingBoard>(FindActor));
  }
 }
 ResetActiveState();
}

void AWorldInteractVolume::OnActorEnteredArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                              const FHitResult& SweepResult)
{
 TArray<UInteractBrush*> ActorBrushes;
 if (GetInteractBrushes(OtherActor, ActorBrushes))
 {
  bool bHasSuitableBrush = false;
  for (auto InteractBrush : ActorBrushes)
  {
   for (const auto DrawingBoard : BindingDrawingBoards)
   {
    if (!DrawingBoard)
    {
     continue;
    }
    if (!InteractBrush->bUseDrawOnlyDrawingBoardsClassList || InteractBrush->DrawOnlyDrawingBoardsClassList.Find(
     DrawingBoard->GetClass()) != -1)
    {
     bHasSuitableBrush = true;
     InteractBrush->EnterArea(this);
     OverlappingBrushes.Add(InteractBrush);
     break;
    }
   }
  }
  if (bHasSuitableBrush)
  {
   UpdateDrawingBoardsActive();
  }
 }
}

void AWorldInteractVolume::OnActorLeavedArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
 TArray<UInteractBrush*> ActorBrushes;
 if (GetInteractBrushes(OtherActor, ActorBrushes))
 {
  bool bHasSuitableBrush = false;
  for (const auto InteractBrush : ActorBrushes)
  {
   for (const auto DrawingBoard : BindingDrawingBoards)
   {
    if (!DrawingBoard)
    {
     continue;
    }
    if (!InteractBrush->bUseDrawOnlyDrawingBoardsClassList || InteractBrush->DrawOnlyDrawingBoardsClassList.Find(
     DrawingBoard->GetClass()) != -1)
    {
     bHasSuitableBrush = true;
     InteractBrush->LeaveArea(this);
     OverlappingBrushes.Remove(InteractBrush);
     break;
    }
   }
  }
  if (bHasSuitableBrush)
  {
   UpdateDrawingBoardsActive();
  }
 }
}

bool AWorldInteractVolume::GetInteractBrushes(const AActor* Actor, TArray<UInteractBrush*>& OutBrushes)
{
 Actor->GetComponents<UInteractBrush>(OutBrushes);
 return OutBrushes.Num() > 0;
}

void AWorldInteractVolume::UpdateDrawingBoardsActive()
{
 if (bVolumeActive && OverlappingBrushes.Num() == 0)
 {
  for (const auto DrawingBoard : BindingDrawingBoards)
  {
   DrawingBoard->InteractVolumeActive(false, this);
  }
  bVolumeActive = false;
 }
 if (!bVolumeActive && OverlappingBrushes.Num() > 0)
 {
  for (const auto DrawingBoard : BindingDrawingBoards)
  {
   DrawingBoard->InteractVolumeActive(true, this);
  }
  bVolumeActive = true;
 }
}

void AWorldInteractVolume::ClearInvalidDrawingBoards()
{
 TArray<AWorldDrawingBoard*> TempDrawingBoards;
 for (const auto DrawingBoard : BindingDrawingBoards)
 {
  if (DrawingBoard)
  {
   TempDrawingBoards.AddUnique(DrawingBoard);
  }
 }
 BindingDrawingBoards = TempDrawingBoards;
}

void AWorldInteractVolume::BindDrawingBoard(AWorldDrawingBoard* DrawingBoard)
{
 BindingDrawingBoards.AddUnique(DrawingBoard);
 ResetActiveState();
}

void AWorldInteractVolume::UnBindDrawingBoard(AWorldDrawingBoard* DrawingBoard)
{
 BindingDrawingBoards.Remove(DrawingBoard);
 ResetActiveState();
}

void AWorldInteractVolume::RemoveBrush(UInteractBrush* InteractBrush)
{
 OverlappingBrushes.Remove(InteractBrush);
 UpdateDrawingBoardsActive();
}

void AWorldInteractVolume::ResetActiveState()
{
 for (const auto InteractBrush : OverlappingBrushes)
 {
  InteractBrush->LeaveArea(this);
 }
 for (const auto DrawingBoard : BindingDrawingBoards)
 {
  DrawingBoard->InteractVolumeActive(false, this);
 }
 bVolumeActive = false;

 TArray<AActor*> OverlappingActors;
 GetOverlappingActors(OverlappingActors);

 TArray<UInteractBrush*> ActorBrushes;
 bool bHasSuitableBrush = false;
 for (const auto OverlappingActor : OverlappingActors)
 {
  if (GetInteractBrushes(OverlappingActor, ActorBrushes))
  {
   for (const auto InteractBrush : ActorBrushes)
   {
    for (const auto DrawingBoard : BindingDrawingBoards)
    {
     if (!DrawingBoard)
     {
      continue;
     }
     if (!InteractBrush->bUseDrawOnlyDrawingBoardsClassList || InteractBrush->DrawOnlyDrawingBoardsClassList.Find(
      DrawingBoard->GetClass()) != -1)
     {
      bHasSuitableBrush = true;
      InteractBrush->EnterArea(this);
      OverlappingBrushes.Add(InteractBrush);
      break;
     }
    }
   }
  }
  if (bHasSuitableBrush)
  {
   UpdateDrawingBoardsActive();
  }
 }
}