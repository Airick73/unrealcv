// Extracted from UnrealCV (https://github.com/unrealcv/unrealcv), MIT License.
#include "CVAnnotationSubsystem.h"
#include "CVAnnotationComponent.h"
#include "CVCameraLog.h"
#include "Components/MeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

bool UCVAnnotationSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	// Only game and PIE worlds; skip editor, preview and thumbnail worlds.
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UCVAnnotationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	if (bAutoAnnotateOnBeginPlay)
	{
		AnnotateWorld();
	}
}

void UCVAnnotationSubsystem::AnnotateWorld()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogCVCamera, Warning, TEXT("Can not annotate world, the world is not valid"));
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AnnotateActor(*It);
	}
	UE_LOG(LogCVCamera, Log, TEXT("Annotated %d actors"), AnnotationColors.Num());
}

void UCVAnnotationSubsystem::AnnotateActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}
	TArray<UMeshComponent*> MeshComponents;
	Actor->GetComponents<UMeshComponent>(MeshComponents);
	if (MeshComponents.Num() == 0)
	{
		return; // Nothing to render, don't use up a colour.
	}
	SetAnnotationColor(Actor, GetDefaultColor(Actor));
}

void UCVAnnotationSubsystem::SetAnnotationColor(AActor* Actor, FColor AnnotationColor)
{
	if (!IsValid(Actor))
	{
		return;
	}
	TArray<UCVAnnotationComponent*> AnnotationComponents;
	Actor->GetComponents<UCVAnnotationComponent>(AnnotationComponents);
	if (AnnotationComponents.Num() == 0)
	{
		CreateAnnotationComponents(Actor, AnnotationColor);
	}
	else
	{
		UpdateAnnotationComponents(Actor, AnnotationColor);
	}
	AnnotationColor.A = 255;
	AnnotationColors.Emplace(Actor->GetName(), AnnotationColor);
	AnnotatedActors.Emplace(Actor->GetName(), Actor);
}

bool UCVAnnotationSubsystem::GetAnnotationColor(AActor* Actor, FColor& AnnotationColor) const
{
	if (!IsValid(Actor))
	{
		return false;
	}
	TArray<UCVAnnotationComponent*> AnnotationComponents;
	Actor->GetComponents<UCVAnnotationComponent>(AnnotationComponents);
	if (AnnotationComponents.Num() == 0)
	{
		return false;
	}
	AnnotationColor = AnnotationComponents[0]->GetAnnotationColor();
	return true;
}

AActor* UCVAnnotationSubsystem::FindActorByColor(FColor Color) const
{
	for (const TPair<FString, FColor>& Pair : AnnotationColors)
	{
		if (Pair.Value.R == Color.R && Pair.Value.G == Color.G && Pair.Value.B == Color.B)
		{
			if (const TWeakObjectPtr<AActor>* Found = AnnotatedActors.Find(Pair.Key))
			{
				return Found->Get();
			}
		}
	}
	return nullptr;
}

void UCVAnnotationSubsystem::CreateAnnotationComponents(AActor* Actor, const FColor& AnnotationColor)
{
	TArray<UMeshComponent*> MeshComponents;
	Actor->GetComponents<UMeshComponent>(MeshComponents);
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		UCVAnnotationComponent* AnnotationComponent = NewObject<UCVAnnotationComponent>(MeshComponent);
		AnnotationComponent->SetupAttachment(MeshComponent);
		AnnotationComponent->RegisterComponent();
		// Set annotation color after the component is registered
		AnnotationComponent->SetAnnotationColor(AnnotationColor);
		AnnotationComponent->MarkRenderStateDirty();
	}
}

void UCVAnnotationSubsystem::UpdateAnnotationComponents(AActor* Actor, const FColor& AnnotationColor)
{
	TArray<UCVAnnotationComponent*> AnnotationComponents;
	Actor->GetComponents<UCVAnnotationComponent>(AnnotationComponents);
	for (UCVAnnotationComponent* AnnotationComponent : AnnotationComponents)
	{
		AnnotationComponent->SetAnnotationColor(AnnotationColor);
		AnnotationComponent->MarkRenderStateDirty();
	}
}

FColor UCVAnnotationSubsystem::GetDefaultColor(AActor* Actor) const
{
	if (const FColor* Existing = AnnotationColors.Find(Actor->GetName()))
	{
		return *Existing;
	}
	return GetColorFromColorMap(AnnotationColors.Num());
}

// ---------------------------------------------------------------------------
// Colour palette (identical to UnrealCV's FColorGenerator)
// ---------------------------------------------------------------------------
namespace CVColorMap
{
	static int32 GetChannelValue(uint32 Index)
	{
		// The fill loop below writes indices 0..257 (UnrealCV sized this 256, overflowing by two).
		static int32 Values[258] = { 0 };
		static bool bInit = false;
		if (!bInit)
		{
			float Step = 256;
			uint32 Iter = 0;
			Values[0] = 0;
			while (Step >= 1)
			{
				for (uint32 Value = Step - 1; Value <= 256; Value += Step * 2)
				{
					Iter++;
					Values[Iter] = Value;
				}
				Step /= 2;
			}
			bInit = true;
		}
		return Index <= 255 ? Values[Index] : 0;
	}

	static void GetColors(int32 MaxVal, bool Fix1, bool Fix2, bool Fix3, TArray<FColor>& ColorMap)
	{
		for (int32 I = 0; I <= (Fix1 ? 0 : MaxVal - 1); I++)
		{
			for (int32 J = 0; J <= (Fix2 ? 0 : MaxVal - 1); J++)
			{
				for (int32 K = 0; K <= (Fix3 ? 0 : MaxVal - 1); K++)
				{
					uint8 R = (uint8)GetChannelValue(Fix1 ? MaxVal : I);
					uint8 G = (uint8)GetChannelValue(Fix2 ? MaxVal : J);
					uint8 B = (uint8)GetChannelValue(Fix3 ? MaxVal : K);
					ColorMap.Add(FColor(R, G, B, 255));
				}
			}
		}
	}
}

FColor UCVAnnotationSubsystem::GetColorFromColorMap(int32 ObjectIndex)
{
	static TArray<FColor> ColorMap;
	const int32 NumPerChannel = 32;
	if (ColorMap.Num() == 0)
	{
		for (int32 MaxChannelIndex = 0; MaxChannelIndex < NumPerChannel; MaxChannelIndex++)
		{
			CVColorMap::GetColors(MaxChannelIndex, false, false, true, ColorMap);
			CVColorMap::GetColors(MaxChannelIndex, false, true, false, ColorMap);
			CVColorMap::GetColors(MaxChannelIndex, false, true, true, ColorMap);
			CVColorMap::GetColors(MaxChannelIndex, true, false, false, ColorMap);
			CVColorMap::GetColors(MaxChannelIndex, true, false, true, ColorMap);
			CVColorMap::GetColors(MaxChannelIndex, true, true, false, ColorMap);
			CVColorMap::GetColors(MaxChannelIndex, true, true, true, ColorMap);
		}
	}
	if (!ColorMap.IsValidIndex(ObjectIndex))
	{
		UE_LOG(LogCVCamera, Error, TEXT("Object index %d is out of the color map boundary [0, %d)"), ObjectIndex, ColorMap.Num());
		return FColor::White;
	}
	return ColorMap[ObjectIndex];
}
