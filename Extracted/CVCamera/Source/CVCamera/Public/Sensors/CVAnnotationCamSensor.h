// Weichao Qiu @ 2017
#pragma once

#include "Sensors/CVBaseCameraSensor.h"
#include "CVAnnotationCamSensor.generated.h"

/** Annotation sensor, utilize the UAnnnotationComponent */
UCLASS(meta = (BlueprintSpawnableComponent))
class CVCAMERA_API UCVAnnotationCamSensor : public UCVBaseCameraSensor
{
	GENERATED_BODY()

public:
	UCVAnnotationCamSensor(const FObjectInitializer& ObjectInitializer);

	static void GetAnnotationComponents(UWorld* World, TArray<TWeakObjectPtr<UPrimitiveComponent> >& ComponentList);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * T);

	void CaptureSeg(TArray<FColor>& ImageData, int& Width, int& Height);

	void InitTextureTarget(int FilmWidth, int FilmHeight);
};
