// Copyright 2023 Sun BoHeng

#include "InteractBrush.h"

#include "InteractiveWorldBPLibrary.h"
#include "InteractiveWorldSubsystem.h"
#include "WorldInteractVolume.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"

#include "Logging/TokenizedMessage.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"


// Sets default values for this component's properties
UInteractBrush::UInteractBrush()
{
}

bool UInteractBrush::PrepareForDrawing(TArray<TSubclassOf<AWorldDrawingBoard>>& NoVolumeDrawingBoardClass)
{
	bSucceededDrawnThisTime = false;
	PreviousT = CurrentT;
	CurrentT = GetComponentTransform();
	//This brush is not in suitable InteractVolume,so it can only draw on DrawingBoards that do not use InteractVolume
	if (!GetBrushActiveInVolume())
	{
		if (NoVolumeDrawingBoardClass.Num() == 0)
		{
			//No DrawingBoards that do not use InteractVolume,so do not draw
			return false;
		}
		if (bUseDrawOnlyDrawingBoardsClassList)
		{
			bool HasSuitableClass = false;
			for (const auto Class : NoVolumeDrawingBoardClass)
			{
				if (DrawOnlyDrawingBoardsClassList.Find(Class) != -1)
				{
					//Find suitable DrawingBoards that do not use InteractVolume
					HasSuitableClass = true;
					break;
				}
			}
			if (!HasSuitableClass)
			{
				return false;
			}
		}
	}
	//bDrawEveryFrame,bDrawOnMovement and moved,or called draw manually
	if (bDrawEveryFrame || bDrawOnce || (!UKismetMathLibrary::NearlyEqual_TransformTransform(
		CurrentT, PreviousT, MovementTolerance.X, MovementTolerance.Y, MovementTolerance.Z) && bDrawOnMovement))
	{
		//Reset bDrawOnce
		bDrawOnce = false;
		//For Blueprint part
		if (UpdateDrawInfo())
		{
			return true;
		}
	}
	//We didn't succeed to draw
	return false;
}

bool UInteractBrush::ShouldDrawOn(AWorldDrawingBoard* DrawingBoard) const
{
	//If DrawingBoard uses InteractVolume,we should make sure we are in the same volume
	//If DrawingBoard doesn't use InteractVolume,check if we use DrawOnlyDrawingBoardsClassList and find if is suitable
	return DrawOnDrawingBoards.Find(DrawingBoard) != -1
		|| (!DrawingBoard->GetUseInteractVolume()
			&& (!bUseDrawOnlyDrawingBoardsClassList
				|| DrawOnlyDrawingBoardsClassList.Find(DrawingBoard->GetClass()) != -1));
}

void UInteractBrush::PreDrawOnRT(AWorldDrawingBoard* DrawingBoard, UCanvas* CanvasDrawOn, FVector2D CanvasSize)
{
	const float TraveledDistance = UKismetMathLibrary::Distance2D(
		UInteractiveWorldBPLibrary::Vector3ToVector2(CurrentT.GetLocation()),
		UInteractiveWorldBPLibrary::Vector3ToVector2(PreviousT.GetLocation()));
	if (bUseMultiDraw && TraveledDistance > MaxDrawDistance && bSucceededDrawnLastTime)
	{
		//Draw many times between two location
		const int32 DrawTimes = FMath::CeilToInt(TraveledDistance / MaxDrawDistance);
		for (int32 i = 1; i < DrawTimes; i++)
		{
			DrawOnRT(DrawingBoard, CanvasDrawOn, CanvasSize,
			         UKismetMathLibrary::Conv_IntToFloat(i) / UKismetMathLibrary::Conv_IntToFloat(DrawTimes), DrawTimes);
		}
	}
	DrawOnRT(DrawingBoard, CanvasDrawOn, CanvasSize, 1, 1);
    bSucceededDrawnThisTime = true;
}

void UInteractBrush::FinishDraw()
{
	PreviousT = CurrentT;
	bSucceededDrawnLastTime = bSucceededDrawnThisTime;
	if (bSucceededDrawnLastTime)
	{
		PreviousDrawnT = CurrentT;	
	}
}

bool UInteractBrush::UpdateDrawInfo_Implementation()
{
	return true;
}


void UInteractBrush::DrawOnRT_Implementation(AWorldDrawingBoard* DrawingBoard, UCanvas* CanvasDrawOn,
                                             FVector2D CanvasSize, float InterpolateRate, int32 DrawTimes)
{
}


void UInteractBrush::EnterArea(AWorldInteractVolume* InteractVolume)
{
	OverlappingInteractVolumes.Add(InteractVolume);
	UpdateActiveState();
}

void UInteractBrush::LeaveArea(AWorldInteractVolume* InteractVolume)
{
	OverlappingInteractVolumes.Remove(InteractVolume);
	UpdateActiveState();
}

// Called when the game starts
void UInteractBrush::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetSubsystem<UInteractiveWorldSubsystem>()->RegisterBrush(this);
	UpdateActiveState();

	//Check whether the Actor this Interact Brush attach to has collision with World Interact Volume
	auto BrushCollisionWarning = [&]()->void
	{
		FMessageLog("PIE").Warning()
		->AddToken(FTextToken::Create(FText::FromString(FString(TEXT("Interact Brush: ")))))
		->AddToken(FUObjectToken::Create(this))
		->AddToken(FTextToken::Create(FText::FromString(FString(TEXT("ISN'T attached to an Actor with Collisitn. Interact Volume will not work with it.")))))
		->AddToken(FTextToken::Create(FText::FromString(FString(TEXT("Please make Actor: ")))))
		->AddToken(FUObjectToken::Create(this->GetOwner()))
		->AddToken(FTextToken::Create(FText::FromString(FString(TEXT("has query collision to 'WorldDynamic'")))));
	};
	if(!GetOwner()->GetComponentByClass(UPrimitiveComponent::StaticClass()))
	{
		BrushCollisionWarning();
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetOwner()->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	bool bHasCollision = false;
	for (const auto& Comp : PrimitiveComponents)
	{
		if((Comp->GetCollisionEnabled()==ECollisionEnabled::QueryOnly || Comp->GetCollisionEnabled()==ECollisionEnabled::QueryAndPhysics)
			&& Comp->GetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic)!= ECollisionResponse::ECR_Ignore)
		{
			bHasCollision = true;
			break;
		}
	}
	if (!bHasCollision)
	{
		BrushCollisionWarning();
	}
	GetOwner()->UpdateOverlaps();
}

void UInteractBrush::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetSubsystem<UInteractiveWorldSubsystem>()->UnregisterBrush(this);
	for (const auto InteractVolume : OverlappingInteractVolumes)
	{
		InteractVolume->RemoveBrush(this);
	}
}

void UInteractBrush::UpdateActiveState()
{
	UpdateDrawOnDrawingBoards();
	if (DrawOnDrawingBoards.Num() > 0 && !bBrushActiveInVolume)
	{
		bBrushActiveInVolume = true;
		bSucceededDrawnLastTime = false;
		bSucceededDrawnThisTime = false;
	}
	if (DrawOnDrawingBoards.Num() == 0 && bBrushActiveInVolume)
	{
		bBrushActiveInVolume = false;
		bSucceededDrawnLastTime = false;
		bSucceededDrawnThisTime = false;
	}
}

void UInteractBrush::UpdateDrawOnDrawingBoards()
{
	//Find suitable DrawingBoards bind to InteractVolume
	DrawOnDrawingBoards.Empty();
	if (bUseDrawOnlyDrawingBoardsClassList)
	{
		for (const auto Volume : OverlappingInteractVolumes)
		{
			for (auto DrawingBoard : Volume->GetDrawingBoards())
			{
				if (DrawOnlyDrawingBoardsClassList.Find(DrawingBoard->GetClass()) != -1)
				{
					DrawOnDrawingBoards.AddUnique(DrawingBoard);
				}
			}
		}
	}
	else
	{
		for (const auto Volume : OverlappingInteractVolumes)
		{
			for (auto DrawingBoard : Volume->GetDrawingBoards())
			{
				DrawOnDrawingBoards.AddUnique(DrawingBoard);
			}
		}
	}
}