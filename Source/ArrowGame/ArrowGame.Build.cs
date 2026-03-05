using UnrealBuildTool;

public class ArrowGame : ModuleRules
{
    public ArrowGame(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput", "Niagara","AnimGraphRuntime",
            "UMG", "Slate", "SlateCore","OnlineSubsystem",
            "OnlineSubsystemUtils",
            "OnlineSubsystemSteam"
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}