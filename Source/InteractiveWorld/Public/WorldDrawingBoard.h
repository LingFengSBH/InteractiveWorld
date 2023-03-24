// Copyright 2023 Sun BoHeng

#pragma once

#include "CoreMinimal.h"
#include "InteractBrush.h"
#include "GameFramework/Actor.h"
#include "WorldDrawingBoard.generated.h"

UCLASS()
class INTERACTIVEWORLD_API AWorldDrawingBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldDrawingBoard();

private:
	//InteractVolumes that make this DrawingBoard stay active
	UPROPERTY()
	TArray<AWorldInteractVolume*> ActiveVolumes;
	
	//No InteractVolume makes this DrawingBoard stay active,so it shut off itself
	bool bDrawingBoardActiveAuto;

	//No InteractVolume makes this DrawingBoard stay active,so it shut off itself
	FTimerHandle StopActiveHandle;

	//This if for timer to shut off DrawingBoard,don't call manually
	//No InteractVolume makes this DrawingBoard stay active,so it shut off itself
	void ShutOff(){bDrawingBoardActiveAuto=false;}

	//Should this DrawingBoard active?If bUseInteractVolume = true,it well check volumes and decide if we need switch active state 
	void UpdateActive();

	//Set PreviousPosition as CurrentPosition,for next update
	void SetPreviousParameters();

	//Bind or Unbind this DrawingBoard to InteractVolumes
	void ReBindInteractVolumes(bool bBind);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	//Auto Sleep//
	
	// If no brush draw on this board in this time,DrawingBoard will not simulate.
	// If 0,will only simulate when brush draw on.
	// If less than 0,It will not sleep.
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Drawing Board | Sleep")
	float SleepTime;

	//Save how long time form last time there are brush draw on this DrawingBoard
	float TimeFromLastDraw;

	//Simulating RT//
	
	//The RT that all brush will draw on this frame
	//Please Use "Set RT Draw On" to set it, and "Get RT Draw On"
	UPROPERTY()
	UTextureRenderTarget2D* RTBrushDrawOn;

	//RenderTarget size
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Drawing Board | Simulating RT")
	FVector2D RTSize = FVector2D(1024,1024);
	UPROPERTY(BlueprintReadWrite,Category = "World Drawing Board | Simulating")
	FVector2D PreviousRTSize = FVector2D(1024,1024);

	//How big a RenderTarget pixel is in world
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Drawing Board | Simulating RT")
	FVector2D PixelWorldSize;
	UPROPERTY(BlueprintReadWrite,Category = "World Drawing Board | Simulating RT")
	FVector2D PreviousPixelWorldSize;

	//DrawingBoard Canvas//

	//The canvas location in world
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Drawing Board | Canvas")
	FVector2D CanvasWorldLocation = FVector2D(0,0);
	UPROPERTY(BlueprintReadWrite,Category = "World Drawing Board | Canvas")
	FVector2D PreviousCanvasWorldLocation= FVector2D(0,0);;
	
	//The canvas size in world
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Drawing Board | Canvas")
	FVector2D CanvasWorldSize = FVector2D(1024,1024);
	UPROPERTY(BlueprintReadWrite,Category = "World Drawing Board | Canvas")
	FVector2D PreviousCanvasWorldSize = FVector2D(1024,1024);

	//The canvas yaw in world
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Drawing Board | Canvas")
	float CanvasWorldYaw;
	UPROPERTY(BlueprintReadWrite,Category = "World Drawing Board | Canvas")
	float PreviousCanvasWorldYaw;

	//Simulating//
	
	//If this DrawingBoard should move with RenderTarget pixel aligned with last time.
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Drawing Board | Simulating")
	bool bPixelAlignedMove = true;

	//This is for brush to calculate draw intensity,like snow,this can be the snow thickness
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "World Drawing Board | Simulating")
	float InteractHeight = 30.0;

	//Active Mode//
	
	//Active this DrawingBoard
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "World Drawing Board | Active Mode")
	bool bActive = true;

	//If you want to use InteractVolume to define area thia this DrawingBoard active
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "World Drawing Board | Active Mode")
	bool bUseInteractVolume = false;

	//If you want to use InteractVolume to define area thia this DrawingBoard active,this array stores InteractVolumes
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "World Drawing Board | Active Mode",meta = (editcondition = "bUseInteractVolume"))
	TArray<AWorldInteractVolume*> InteractVolumes;

public:
	
	//Before we allocate brushes and simulate,we should update something,like position
	UFUNCTION(BlueprintNativeEvent,meta=(DisplayName="Update DrawingBoard State"))
	void UpdateDrawingBoardState();

	//SubSystem allocated InteractBrushes,and can this function.
	//Receive InteractBrushes,and prepare for simulate
	void PrepareForSimulate(TArray<UInteractBrush*> Brushes);
	//No InteractBrush for this DrawingBoard.Prepare for simulate
	void PrepareForSimulate();

	//Before drawing brushes
	UFUNCTION(BlueprintNativeEvent,meta=(DisplayName="Pre Simulate"))
	void PreSimulate();

	//Draw brushes
	void DrawBrushes(TArray<UInteractBrush*> Brushes,UTextureRenderTarget2D* RTDrawOn);

	// After drawing brushes
	UFUNCTION(BlueprintNativeEvent,meta=(DisplayName="Post Simulate"))
	void PostSimulate();

	//Canvas Setting//

	//Set CanvasWorldLocation
	//bPixelAligned : If this DrawingBoard should move with RenderTarget pixel aligned with last time
	//PixelSizeScale : When bPixelAligned is true,scale pixel size
	UFUNCTION(BlueprintCallable,Category = "World Drawing Board | Runtime",meta=(DisplayName="Set Canvas World Location"))
	void SetCanvasWorldLocation(FVector2D NewLocation,bool bPixelAligned,float PixelSizeScale = 1);
	
	//Transform World to Canvas//
	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="World to Canvas Rotation"), Category="World Drawing Board | World to Canvas")
	float WorldToCanvasRotation(float WorldRotation);

	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="World to Canvas Size"), Category="World Drawing Board | World to Canvas")
	FVector2D WorldToCanvasSize(FVector2D WorldSize);

	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="World to Canvas UV"), Category="World Drawing Board | World to Canvas")
	FVector2D WorldToCanvasUV(FVector2D WorldLocation);

	//This is actually a box SDF
	float GetNearestDistance(FVector2D WorldLocation) const;

	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="World to Canvas Brush"), Category="World Drawing Board | World to Canvas")
	void WorldToCanvasBrush(FVector2D BrushLocation,FVector2D BrushSize,float BrushRotation,FVector2D& OutScreenPosition,FVector2D& OutScreenSize,float& OutScreenRotation);


    //Interact Volume//
	
	UFUNCTION(BlueprintCallable,meta=(DisplayName="Reset Use InteractVolume"), Category="World Drawing Board | Interact Volume")
	void ResetUseInteractVolume(bool NewUseInteractVolume = false);
	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="Get Use InteractVolume"), Category="World Drawing Board | Interact Volume")
	bool GetUseInteractVolume() const {return bUseInteractVolume;};
	
	UFUNCTION(BlueprintCallable,meta=(DisplayName="Reset InteractVolumes"), Category="World Drawing Board | Interact Volume")
	void ResetInteractVolumes(TArray<AWorldInteractVolume*> NewInteractVolumes);

	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="Get InteractVolumes"), Category="World Drawing Board | Interact Volume")
	TArray<AWorldInteractVolume*> GetInteractVolumes() const {return InteractVolumes;};

	//This if for InteractVolumes to active this drawing board
	void InteractVolumeActive(bool bActive, AWorldInteractVolume* TargetInteractVolume);

	//Active State//

	//Set active
	UFUNCTION(BlueprintCallable,meta=(DisplayName="Reset Active State"), Category="World Drawing Board")
	void SetDrawingBoardActive(bool Active = true);

	//InteractBrushes leaved volumes,so we do not need to allocate brushes for it
	bool GetShouldDrawOn() const {return bActive&&(!bUseInteractVolume||ActiveVolumes.Num() > 0);}
	
	//If there are any InteractBrush in InteractVolume,we should stay active.But shut off when there are no InteractBrush after SleepTime
	//So,this will be later than GetShouldDrawOn,this will be false after SleepTime
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="Get Active State"), Category="World Drawing Board")
	bool GetActiveState() const {return  bDrawingBoardActiveAuto && bActive;}

	//Although there may be some InteractBrushes,but there are no successful draw,then this DrawingBoard will sleep 
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="Get Is Simulating"), Category="World Drawing Board")
	bool GetIsSimulating(); 

	//Get private properties//
	
	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="Get Interact Height"), Category="World Drawing Board")
	float GetInteractHeight() const {return  InteractHeight;}

	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="Get Time from Last Draw"), Category="World Drawing Board")
	float GetTimeFromLastDraw() const {return  TimeFromLastDraw;}

	//RT Draw On
	UFUNCTION(BlueprintCallable,meta=(DisplayName="Set RT Draw On"), Category="World Drawing Board")
	void SetRTDrawOn(UTextureRenderTarget2D* NewRT);

	UFUNCTION(BlueprintCallable,BlueprintPure,meta=(DisplayName="Get RT Draw On"), Category="World Drawing Board")
	UTextureRenderTarget2D* GetRTDrawOn() const {return RTBrushDrawOn;}
};