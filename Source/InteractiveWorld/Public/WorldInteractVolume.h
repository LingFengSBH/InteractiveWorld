// Copyright 2023 Sun BoHeng

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PhysicsVolume.h"
#include "WorldInteractVolume.generated.h"

class UInteractBrush;
class AWorldDrawingBoard;

UCLASS(Blueprintable)
class INTERACTIVEWORLD_API AWorldInteractVolume : public APhysicsVolume
{
	GENERATED_BODY()

	AWorldInteractVolume();

	virtual void BeginPlay() override;

	//Brushes in this volume
	UPROPERTY()
	TArray<UInteractBrush*> OverlappingBrushes;
	UPROPERTY()
	TArray<UInteractBrush*> ManualAddingBrushes;

	//Binding DrawingBoards
	UPROPERTY()
	TArray<AWorldDrawingBoard*> BindingDrawingBoards;
	
	bool bVolumeActive = false;

	bool InteractiveBrushEnter(UInteractBrush* InteractBrush);
	bool InteractiveBrushLeave(UInteractBrush* InteractBrush);
	
	UFUNCTION()
	void OnActorEnteredArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void OnActorLeavedArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	//Get InteractBrushes attached on actor
	static bool GetInteractBrushes(const AActor* Actor,TArray<UInteractBrush*>& OutBrushes);
	static bool GetInteractBrushes(const TArray<AActor*> Actors,TArray<UInteractBrush*>& OutBrushes);

	//If there are suitable brushes,then active.If not,then shut off
	void UpdateDrawingBoardsActive();

	//Clear invalid ones
	void ClearInvalidDrawingBoards();

public:
	
	//Drawing Board//

	//Bind DrawingBoard,and reset active state
	void BindDrawingBoard(AWorldDrawingBoard* DrawingBoard);

	//Unbind DrawingBoard,and reset active state
	void UnBindDrawingBoard(AWorldDrawingBoard* DrawingBoard);

	//Get binding DrawingBoards
	UFUNCTION(BlueprintCallable,BlueprintPure, meta = (DisplayName = "Get DrawingBoards", Keywords = "DrawingBoards"), Category = "World Interact Volume")
	TArray<AWorldDrawingBoard*> GetDrawingBoards() const {return BindingDrawingBoards;}

	//InteractBrush//
	
	void RemoveBrush(UInteractBrush* InteractBrush);

	//Reset active state,this will find overlapping actors and check it they have suitable brushes
	//After binding or unbinding DrawingBoard,call this to update
	void ResetActiveState();
	
	UFUNCTION(BlueprintCallable,meta = (DisplayName = "Manual InteractBrush Enter Area", Keywords = "Interact,Brush"), Category = "World Interact Volume")
	void ManualInteractBrushEnterArea(UInteractBrush* InteractBrush);

	UFUNCTION(BlueprintCallable,meta = (DisplayName = "Manual InteractBrush Leave Area", Keywords = "Interact,Brush"), Category = "World Interact Volume")
	void ManualInteractBrushLeaveArea(UInteractBrush* InteractBrush);
	
};

