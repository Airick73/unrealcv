// Extracted from UnrealCV (https://github.com/unrealcv/unrealcv), MIT License.
using UnrealBuildTool;

public class CVCamera : ModuleRules
{
	public CVCamera(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"RenderCore",
			"RHI",
			"CinematicCamera",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"ImageWrapper",
		});
	}
}
