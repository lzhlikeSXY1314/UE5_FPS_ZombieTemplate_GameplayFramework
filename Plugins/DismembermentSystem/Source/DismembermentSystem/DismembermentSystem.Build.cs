using UnrealBuildTool;

public class DismembermentSystem : ModuleRules
{
    public DismembermentSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[] { });
        PrivateIncludePaths.AddRange(new string[] { });

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Niagara", // 移到公共依赖，外部项目可访问Niagara类型
				"RenderCore",
                "RHI",
                "MeshDescription"
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
            });

        DynamicallyLoadedModuleNames.AddRange(new string[] { });
    }
}