using System.IO;
using UnrealBuildTool;

public class TerraVoxUE5 : ModuleRules
{
    public TerraVoxUE5(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "ProceduralMeshComponent",
        });
    }
}
