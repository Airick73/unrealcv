// Weichao Qiu @ 2017
#pragma once

#include "Sensors/CVBaseCameraSensor.h"
#include "CVDepthCamSensor.generated.h"

/** Depth sensor */
UCLASS(meta = (BlueprintSpawnableComponent))
class CVCAMERA_API UCVDepthCamSensor : public UCVBaseCameraSensor
{
	GENERATED_BODY()

public:
	UCVDepthCamSensor(const FObjectInitializer& ObjectInitializer);

	void CaptureDepth(TArray<float>& DepthData, int& Width, int& Height);

	virtual void InitTextureTarget(int FilmWidth, int FilmHeight) override;

	UPROPERTY(EditInstanceOnly, Category = "CVCamera")
	bool bIgnoreTransparentObjects;
};
