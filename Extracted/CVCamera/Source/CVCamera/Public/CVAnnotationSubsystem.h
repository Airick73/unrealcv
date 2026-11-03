// Extracted from UnrealCV (https://github.com/unrealcv/unrealcv), MIT License.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CVAnnotationSubsystem.generated.h"

/**
 * Gives every mesh actor a unique colour for the segmentation (object mask) image.
 * Replaces UnrealCV's FObjectAnnotator, which was owned by its WorldController.
 *
 * By default the whole world is annotated when play begins. To turn that off, add this to
 * Config/DefaultGame.ini:
 *     [/Script/CVCamera.CVAnnotationSubsystem]
 *     bAutoAnnotateOnBeginPlay=False
 *
 * Actors spawned after BeginPlay are not annotated automatically: call AnnotateActor (or
 * AnnotateWorld again, which skips actors that already have a colour).
 *
 * The depth sensor also uses these annotation proxies, by default, to see through
 * transparent surfaces (see UCVDepthCamSensor::bIgnoreTransparentObjects).
 */
UCLASS(Config = Game)
class CVCAMERA_API UCVAnnotationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Annotate every actor with a mesh in this world. Already-annotated actors keep their colour. */
	UFUNCTION(BlueprintCallable, Category = "CVCamera|Annotation")
	void AnnotateWorld();

	/** Annotate one actor, giving it the next unused colour unless it already has one. */
	UFUNCTION(BlueprintCallable, Category = "CVCamera|Annotation")
	void AnnotateActor(AActor* Actor);

	/** Set an explicit annotation colour for an actor (creates the annotation components if needed). */
	UFUNCTION(BlueprintCallable, Category = "CVCamera|Annotation")
	void SetAnnotationColor(AActor* Actor, FColor AnnotationColor);

	/** Returns false if the actor has no annotation. */
	UFUNCTION(BlueprintCallable, Category = "CVCamera|Annotation")
	bool GetAnnotationColor(AActor* Actor, FColor& AnnotationColor) const;

	/** Actor name -> colour for every annotated actor. Use it to turn segmentation pixels back into objects. */
	UFUNCTION(BlueprintPure, Category = "CVCamera|Annotation")
	TMap<FString, FColor> GetAnnotationColors() const { return AnnotationColors; }

	/** Find the actor whose annotation colour matches a segmentation pixel (alpha is ignored). */
	UFUNCTION(BlueprintCallable, Category = "CVCamera|Annotation")
	AActor* FindActorByColor(FColor Color) const;

	/** Deterministic colour for the N-th object (same palette as UnrealCV). */
	static FColor GetColorFromColorMap(int32 ObjectIndex);

	UPROPERTY(Config, EditAnywhere, Category = "CVCamera|Annotation")
	bool bAutoAnnotateOnBeginPlay = true;

private:
	void CreateAnnotationComponents(AActor* Actor, const FColor& AnnotationColor);
	void UpdateAnnotationComponents(AActor* Actor, const FColor& AnnotationColor);
	FColor GetDefaultColor(AActor* Actor) const;

	TMap<FString, FColor> AnnotationColors;
	TMap<FString, TWeakObjectPtr<AActor>> AnnotatedActors;
};
