using UnrealBuildTool;
using System.Collections.Generic;

public class TerraVoxUE5Target : TargetRules
{
    public TerraVoxUE5Target(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

        // ProceduralMesh is required by the runtime module AND the non-editor target,
        // otherwise -game launches fail to link (hard-won lesson).
        ExtraModuleNames.Add("ProceduralMeshComponent");
    }
}
