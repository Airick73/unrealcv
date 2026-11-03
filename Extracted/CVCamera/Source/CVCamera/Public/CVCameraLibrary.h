// Extracted from UnrealCV (https://github.com/unrealcv/unrealcv), MIT License.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CVCameraLibrary.generated.h"

class ACVCameraActor;

/** Convenience helpers for spawning CV cameras and writing their output to disk. */
UCLASS()
class CVCAMERA_API UCVCameraLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Spawn a camera at a world transform and set its resolution / FOV. */
	UFUNCTION(BlueprintCallable, Category = "CVCamera", meta = (WorldContext = "WorldContextObject"))
	static ACVCameraActor* SpawnCVCamera(UObject* WorldContextObject, FVector Location, FRotator Rotation,
		int32 Width = 640, int32 Height = 480, float FOV = 90.0f);

	/** Save BGRA pixels (lit, normal, flow, segmentation) as a PNG. */
	UFUNCTION(BlueprintCallable, Category = "CVCamera|IO")
	static bool SaveColorImageAsPng(const TArray<FColor>& Pixels, int32 Width, int32 Height, const FString& Filename);

	/** Save a float image (e.g. depth in cm) as a float32 NumPy .npy file with shape (Height, Width). */
	UFUNCTION(BlueprintCallable, Category = "CVCamera|IO")
	static bool SaveFloatImageAsNpy(const TArray<float>& Pixels, int32 Width, int32 Height, const FString& Filename);
};
