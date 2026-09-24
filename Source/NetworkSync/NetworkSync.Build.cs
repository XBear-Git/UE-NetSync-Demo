// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NetworkSync : ModuleRules
{
	public NetworkSync(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Niagara",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"NetworkSync",
			"NetworkSync/Variant_Platforming",
			"NetworkSync/Variant_Platforming/Animation",
			"NetworkSync/Variant_Combat",
			"NetworkSync/Variant_Combat/AI",
			"NetworkSync/Variant_Combat/Animation",
			"NetworkSync/Variant_Combat/Gameplay",
			"NetworkSync/Variant_Combat/Interfaces",
			"NetworkSync/Variant_Combat/UI",
			"NetworkSync/Variant_SideScrolling",
			"NetworkSync/Variant_SideScrolling/AI",
			"NetworkSync/Variant_SideScrolling/Gameplay",
			"NetworkSync/Variant_SideScrolling/Interfaces",
			"NetworkSync/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
