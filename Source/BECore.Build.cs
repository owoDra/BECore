// Copyright owoDra

using UnrealBuildTool;

public class BECore : ModuleRules
{
	public BECore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreOnline",
                "CoreUObject",
                "ApplicationCore",
                "Engine",
                "PhysicsCore",
                "GameplayTags",
                "GameplayTasks",
                "GameplayAbilities",
                "AIModule",
                "ModularGameplay",
                "ModularGameplayActors",
                "DataRegistry",
                "ReplicationGraph",
                "GameFeatures",
                "Hotfix",
                "CommonLoadingScreen",
                "Niagara",
                "AsyncMixin",
                "ControlFlows",
                "PropertyPath"
            }
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "InputCore",
                "Slate",
                "SlateCore",
                "RenderCore",
                "DeveloperSettings",
                "EnhancedInput",
                "NetCore",
                "RHI",
                "Projects",
                "Gauntlet",
                "UMG",
                "CommonUI",
                "CommonInput",
                "GameSettings",
                "CommonGame",
                "CommonUser",
                "GameSubtitles",
                "GameplayMessageRuntime",
                "AudioMixer",
                "NetworkReplayStreaming",
                "UIExtension",
                "ClientPilot",
                "AudioModulation",
                "EngineSettings",
                "DTLSHandlerComponent",
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        SetupGameplayDebuggerSupport(Target);
        SetupIrisSupport(Target);
    }
}
