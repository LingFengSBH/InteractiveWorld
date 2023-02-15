// Copyright 2023 Sun BoHeng

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WorldInteractVolume.h"
#include "InteractBrush.generated.h"

class AWorldDrawingBoard;

UCLASS(Blueprintable,ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INTERACTIVEWORLD_API UInteractBrush : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractBrush();

	//Basic//

	//World size that the brush will draw in
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractBrush|Drawing")
	FVector2D Size;

	//Current transform of this brush
	UFUNCTION(BlueprintCallable,Category = "InteractBrush|Trasnform",meta=(DisplayName="Get Current Transform"))
	FTransform GetCurrentTransform() const {return CurrentT;}
	
	//Culling//

	//If this brush is outside the DrawingBoard range,it will be culled,CullRadius will be calculated be size.Enable if you want to override that value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "InteractBrush|Culling")
	bool bOverrideCullRadius = false;

	//If this brush is outside the DrawingBoard range,it will be culled.This will override that value if bOverrideCullRadius = true
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "InteractBrush|Culling",meta = (editcondition = "bOverrideCullRadius"))
	float CullRadiusOverride = 0;

	//Get CullRadius of this brush
	UFUNCTION(BlueprintCallable,Category = "InteractBrush|Culling",meta=(DisplayName="Get Cull Radius"))
	float GetCullRadius() const {return bOverrideCullRadius ? CullRadiusOverride : Size.Length();}

	//Drawing Mode//
	
	//Whether to draw every frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractBrush|DrawingMode")
	bool bDrawEveryFrame = false;

	//Whether to draw on movement.  Disable if you are going to draw manually from blueprint.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractBrush|DrawingMode",meta = (editcondition = "!bDrawEveryFrame"))
	bool bDrawOnMovement = true;

	//Tolerance of movement.X:Position tolerance,Y:Rotation tolerance,Z:Scale tolerance.If greater than that will be considered as movement.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractBrush|DrawingMode",meta = (editcondition = "!bDrawEveryFrame"))
	FVector MovementTolerance = FVector(0.1,0.1,0.1);

	//DrawingBoard Type//
	
	//Enable if you'd like this brush can only draw in specific DrawingBoards.
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractBrush|DrawingBoardType")
	bool bUseDrawOnlyDrawingBoardsClassList = false;

	//You'd like this brush can only draw in specific DrawingBoards class,bUseDrawOnlyDrawingBoardsClassList should be true if you want to use it.
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractBrush|DrawingBoardType")
	TArray<TSubclassOf<AWorldDrawingBoard>> DrawOnlyDrawingBoardsClassList;

	//MultiDraw//

	//If you want to draw more than once every frame.
	//If true,the drawing times will be depending on distance moved,but more than once each update.
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractBrush|MultiDraw")
	bool bUseMultiDraw = false;

	//If bUseMultiDraw = true,drawing times will be depending on distance moved,then how far do you want each draw between
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "InteractBrush|MultiDraw",meta = (editcondition = "bUseMultiDraw"))
	float MaxDrawDistance = 10;
	
	//Drawing//
	
	//Do some preparation for drawing,and return a boolean which decides whether ot need to be drawn or not 
	bool PrepareForDrawing(TArray<TSubclassOf<AWorldDrawingBoard>>& NoVolumeDrawingBoardClass);

	//Draw manually.If you set bDrawEveryFrame and bDrawOnMovement false,you should call this to draw
	UFUNCTION(BlueprintCallable,Category = "InteractBrush|Drawing",meta=(DisplayName="Draw Brush"))
	void DrawBrush(){bDrawOnce=true;};
	
	//Function for blueprint children to override,update information and return a boolean which decides whether ot need to be drawn or not
	UFUNCTION(BlueprintNativeEvent,Category = "InteractBrush|Drawing",meta=(DisplayName="Update Draw Info"))
	bool UpdateDrawInfo();

	//Check if this brush should draw on specific DrawingBoard
	bool ShouldDrawOn(AWorldDrawingBoard* DrawingBoard) const;

	//Call this to let brush draw on canvas
	void PreDrawOnRT(AWorldDrawingBoard* DrawingBoard,UCanvas* CanvasDrawOn,FVector2D CanvasSize);

	//The actual event to draw brush.if you opened bUseMultiDraw,InterpolateRate will interpolation from 0 to 1 
	UFUNCTION(BlueprintNativeEvent,Category = "InteractBrush|Drawing",meta=(DisplayName="Draw on RT"))
	void DrawOnRT(AWorldDrawingBoard* DrawingBoard, UCanvas* CanvasDrawOn, FVector2D CanvasSize, float InterpolateRate, int32 DrawTimes);

	void FinishDraw();
	
	//If we "PrepareForDrawing",but didn't draw successfully,it will be false.
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "InteractBrush|Drawing",meta=(DisplayName="Get Last Draw Succeed"))
	bool GetLastDrawSucceed() const {return bSucceededDrawnLastTime;}
	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "InteractBrush|Drawing",meta=(DisplayName="Get Current Draw Succeed"))
	bool GetCurrentDrawSucceed() const {return bSucceededDrawnThisTime;}
	//Interact Volume//
	
	//These Functions are designed for InteractVolume,when this brush enter an InteractVolume,it will be called
	void EnterArea(AWorldInteractVolume* InteractVolume);
	//These Functions are designed for InteractVolume,when this brush leave an InteractVolume,it will be called
	void LeaveArea(AWorldInteractVolume* InteractVolume);
	//Clear OverlappingInteractVolumes
	void ResetBrush(){OverlappingInteractVolumes.Empty();UpdateActiveState();}
	//Get BrushActiveInVolume
	bool GetBrushActiveInVolume() const {return bBrushActiveInVolume;}

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//Current Transform
	UPROPERTY(BlueprintReadOnly,Category = "InteractBrush|Transform")
	FTransform CurrentT;

	//Previous Transform,each time we call "PrepareForDrawing",CurrentT will be set,and PreviousT saves old CurrentT
	UPROPERTY(BlueprintReadOnly,Category = "InteractBrush|Transform")
	FTransform PreviousT;

	//Only update when we draw on drawing board,this saves the transform last time it draw
	UPROPERTY(BlueprintReadOnly,Category = "InteractBrush|Transform")
	FTransform PreviousDrawnT;

	//If ture,draw once and become false after drawn.This is for function DrawBrush to draw manually
	bool bDrawOnce;

	//If use interact volume,and leaved all suitable volumes,the brush will not draw on DrawingBoards which use interact volume
	void UpdateActiveState();

private:
	//Currently OverlappingInteractVolumes
	UPROPERTY()
	TArray<AWorldInteractVolume*> OverlappingInteractVolumes;
	
	//DrawingBoards which use InteractVolume that this brush will draw on 
	TArray<AWorldDrawingBoard*> DrawOnDrawingBoards;

	//If use interact volume,and leaved all suitable volumes,the brush will not draw on DrawingBoards which use interact volume
	bool bBrushActiveInVolume = false;

	//Save state for bSucceededDrawnLastTime
	bool bSucceededDrawnThisTime = false;
	
	//If we "PrepareForDrawing",but didn't draw successfully,it will be false.
	bool bSucceededDrawnLastTime = false;

	//Find DrawingBoards which use InteractVolume that this brush will draw on from InteractVolumes
	void UpdateDrawOnDrawingBoards();
};