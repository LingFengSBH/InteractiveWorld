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

 //Waiting for one frame for all overlapping finished.
 GetWorldTimerManager().SetTimerForNextTick(this,&AWorldInteractVolume::ResetActiveState);
 //ResetActiveState();
}

bool AWorldInteractVolume::InteractiveBrushEnter(UInteractBrush* InteractBrush)
{
 for (auto DrawingBoard : BindingDrawingBoards)
 {
  if (!DrawingBoard)
  {
   continue;
  }
  if (!InteractBrush->bUseDrawOnlyDrawingBoardsClassList || InteractBrush->DrawOnlyDrawingBoardsClassList.Find(
   DrawingBoard->GetClass()) != -1)
  {
   InteractBrush->EnterArea(this);
   OverlappingBrushes.AddUnique(InteractBrush);
   return true;
  }
 }
 return false;
}

bool AWorldInteractVolume::InteractiveBrushLeave(UInteractBrush* InteractBrush)
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
   InteractBrush->LeaveArea(this);
   OverlappingBrushes.Remove(InteractBrush);
   ManualAddingBrushes.Remove(InteractBrush);
  return true;
  }
 }
 return false;
}

void AWorldInteractVolume::OnActorEnteredArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                              const FHitResult& SweepResult)
{
 TArray<UInteractBrush*> ActorBrushes;
 if (GetInteractBrushes(OtherActor, ActorBrushes))
 {
  bool bHasSuitableBrush = false;
  for (const auto InteractBrush : ActorBrushes)
  {
   bHasSuitableBrush |= InteractiveBrushEnter(InteractBrush);
   //If you write
   //bHasSuitableBrush = bHasSuitableBrush || InteractiveBrushEnter(InteractBrush);
   //will cause bugs,because || will not call left function when bHasSuitableBrush equals true.
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
   bHasSuitableBrush |= InteractiveBrushLeave(InteractBrush);
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

bool AWorldInteractVolume::GetInteractBrushes(const TArray<AActor*> Actors, TArray<UInteractBrush*>& OutBrushes)
{
 OutBrushes.Empty();
 TArray<UInteractBrush*> TempBrushes;
 for (auto& Actor : Actors )
 {
  GetInteractBrushes(Actor,TempBrushes);
  OutBrushes.Append(TempBrushes);
 }
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

 GetInteractBrushes(OverlappingActors, ActorBrushes);
 ActorBrushes.Append(ManualAddingBrushes);
 {
  for (const auto InteractBrush : ActorBrushes)
  {
   bHasSuitableBrush |= InteractiveBrushEnter(InteractBrush);
  }
 }
 if (bHasSuitableBrush)
 {
  UpdateDrawingBoardsActive();
 }
}

void AWorldInteractVolume::ManualInteractBrushEnterArea(UInteractBrush* InteractBrush)
{
 ManualAddingBrushes.AddUnique(InteractBrush);
 if(InteractiveBrushEnter(InteractBrush))
 {
  UpdateDrawingBoardsActive();
 }
}

void AWorldInteractVolume::ManualInteractBrushLeaveArea(UInteractBrush* InteractBrush)
{
 ManualAddingBrushes.Remove(InteractBrush);
 if(InteractiveBrushLeave(InteractBrush))
 {
  UpdateDrawingBoardsActive();
 }
}
