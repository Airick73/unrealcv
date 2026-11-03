// Weichao Qiu @ 2017
#pragma once

#include "Sensors/CVBaseCameraSensor.h"
#include "CVLitCamSensor.generated.h"

/** 
 * RGB color sensor 
 * The alias issue was reported here
 * https://forums.unrealengine.com/development-discussion/rendering/59403-scenecapturecomponent2d-antialiasing
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class CVCAMERA_API UCVLitCamSensor : public UCVBaseCameraSensor
{
	GENERATED_BODY()

public:
	UCVLitCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void InitTextureTarget(int FilmWidth, int FilmHeight) override;

	void CaptureLit(TArray<FColor>& Image, int& Width, int& Height);
};
