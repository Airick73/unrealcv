// Weichao Qiu @ 2017
#pragma once

#include "Sensors/CVBaseCameraSensor.h"
#include "CVNormalCamSensor.generated.h"

/** Surface normal sensor */
UCLASS()
class CVCAMERA_API UCVNormalCamSensor : public UCVBaseCameraSensor
{
	GENERATED_BODY()

public:
	UCVNormalCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void InitTextureTarget(int filmWidth, int filmHeight) override;

	void CaptureNormal(TArray<FColor>& ImageData, int& Width, int& Height);

private:
	/** Post process material to extract surface normal data */
	UPROPERTY()
	UMaterial* SurfaceNormalPPMaterial;
};
