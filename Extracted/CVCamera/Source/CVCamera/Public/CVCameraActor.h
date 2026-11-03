// Extracted from UnrealCV (https://github.com/unrealcv/unrealcv), MIT License.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CVFusionCamSensor.h"
#include "CVCameraActor.generated.h"

/**
 * A spawnable camera that captures lit (RGB), depth, surface normal, optical flow and
 * object-mask (segmentation) images. Equivalent to UnrealCV's AFusionCameraActor.
 *
 *   ACVCameraActor* Cam = GetWorld()->SpawnActor<ACVCameraActor>(Location, Rotation);
 *   Cam->GetSensor()->GetLit(Pixels, Width, Height);
 *
 * You can also add a UCVFusionCamSensor component to any actor of your own instead.
 */
UCLASS(Blueprintable)
class CVCAMERA_API ACVCameraActor : public AActor
{
	GENERATED_BODY()

public:
	ACVCameraActor();

	UFUNCTION(BlueprintPure, Category = "CVCamera")
	UCVFusionCamSensor* GetSensor() const { return FusionCamSensor; }

private:
	UPROPERTY(Category = "CVCamera", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCVFusionCamSensor> FusionCamSensor;
};
