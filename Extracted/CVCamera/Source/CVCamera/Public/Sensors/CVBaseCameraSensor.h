// Weichao Qiu @ 2017
// Extracted from UnrealCV. Should not be used directly in blueprint; use UCVFusionCamSensor.
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "CineCameraComponent.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

#include "CVBaseCameraSensor.generated.h"

/**
 * A base camera sensor for ground truth capture
 */
UCLASS(abstract)
class CVCAMERA_API UCVBaseCameraSensor : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:
	UCVBaseCameraSensor(const FObjectInitializer& ObjectInitializer);

	/** Render one frame and read it back (synchronous: blocks until the GPU finishes) */
	void Capture(TArray<FColor>& ImageData, int& Width, int& Height);

	/** Get/set the sensor location / rotation */
	FVector GetSensorLocation()
	{
		return this->GetComponentLocation(); // World space
	}
	void SetSensorLocation(FVector Location)
	{
		this->SetWorldLocation(Location);
	}

	FRotator GetSensorRotation()
	{
		return this->GetComponentRotation(); // World space
	}
	void SetSensorRotation(FRotator Rotator)
	{
		this->SetWorldRotation(Rotator);
	}

	/** Get/set the horizontal FOV. When Cine is enabled, this updates the physical lens. */
	float GetFOV() const;
	void SetFOV(float FOV);

	void SetCineCameraEnabled(bool bEnabled);
	bool IsCineCameraEnabled() const { return bCineCameraEnabled; }
	UCineCameraComponent* GetCineCameraComponent() const { return CineCameraModel; }

	/** Synchronize the physical camera model with this scene-capture component. */
	void UpdateCineCameraView(float DeltaTime = 0.0f);

	/** Get/set the sensor film size */
	void SetFilmSize(int Width, int Height);
	int GetFilmWidth();
	int GetFilmHeight();

	virtual void InitTextureTarget(int FilmWidth, int FilmHeight);

	void SetPostProcessMaterial(UMaterial* PostProcessMaterial);

	/** Similar function to GetCameraView in UCameraComponent, without lockToHMD feature */
	void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

protected:
	/** Check whether the TextureTarget is correctly initialized */
	bool CheckTextureTarget();

	int FilmWidth;

	int FilmHeight;

	UPROPERTY(Transient)
	TObjectPtr<UCineCameraComponent> CineCameraModel;

	bool bCineCameraEnabled = false;
	bool bHasLegacyPostProcessState = false;
	FPostProcessSettings LegacyPostProcessSettings;
	float LegacyPostProcessBlendWeight = 0.0f;
};
