using UnrealBuildTool;
using System.Collections.Generic;

public class TerraVoxUE5EditorTarget : TargetRules
{
    public TerraVoxUE5EditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        ExtraModuleNames.Add("ProceduralMeshComponent");
    }
}
