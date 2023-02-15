// Copyright 2023 Sun BoHeng

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "InteractBrush.h"
#include "WorldDrawingBoard.h"

#include "InteractiveWorldSubsystem.generated.h"


UCLASS()
class INTERACTIVEWORLD_API UInteractiveWorldSubsystem : public UWorldSubsystem,public FTickableGameObject
{
	GENERATED_BODY()

public:
	//Begin FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return !IsTemplate(); }
	virtual TStatId GetStatId() const override{RETURN_QUICK_DECLARE_CYCLE_STAT(UInteractiveWorldSubsystem, STATGROUP_Tickables);}
	//End FTickableGameObject Interface
	
	//Register a InteractBrush to subsystem,then the brush can be allocated to WorldDrawingBoards and draw trails
	UFUNCTION(BlueprintCallable,Category = "Interactive World Subsystem | Register",meta=(DisplayName="Register Brush"))
	void RegisterBrush(UInteractBrush* Brush);

	//Unregister a InteractBrush
	UFUNCTION(BlueprintCallable,Category = "Interactive World Subsystem | Register",meta=(DisplayName="Unregister Brush"))
	void UnregisterBrush(UInteractBrush* Brush);

	//Register a DrawingBoard to subsystem,then the brush can be allocated to it and draw trails
	UFUNCTION(BlueprintCallable,Category = "Interactive World Subsystem | Register",meta=(DisplayName="Register Drawing Board"))
	void RegisterDrawingBoard(AWorldDrawingBoard* DrawingBoard);

	//Unregister a DrawingBoard
	UFUNCTION(BlueprintCallable,Category = "Interactive World Subsystem | Register",meta=(DisplayName="Unregister Drawing Board"))
	void UnregisterDrawingBoard(AWorldDrawingBoard* DrawingBoard);

	//Distance from player camera,brushes out of range will not be drawn.If less than 0,will not cull brushes.
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Interactive World Subsystem | Culling")
	float BrushCullDistance = -1;

	//For Debugging
	UFUNCTION(BlueprintCallable,Category = "Interactive World Subsystem | Debug",meta=(DisplayName="Get Registered Drawing Boards"))
	TArray<AWorldDrawingBoard*> GetRegisteredDrawingBoards(){return DrawingBoards;}

	UFUNCTION(BlueprintCallable,Category = "Interactive World Subsystem | Debug",meta=(DisplayName="Get Registered Interact Brushes"))
	TArray<UInteractBrush*> GetRegisteredInteractBrushes(){return Brushes;}
	
private:
	//Brushes that registered
	UPROPERTY()
	TArray<UInteractBrush*> Brushes;

	//DrawingBoards that registered
	UPROPERTY()
	TArray<AWorldDrawingBoard*> DrawingBoards;

	//Prepare InteractBrushes.This will cull invalid and far InteractBrushes
	bool PrepareBrushes(TArray<UInteractBrush*>& BrushesNeedDrawing);

	//Allocate InteractBrushes for DrawingBoards
	void AllocateBrushes();

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};