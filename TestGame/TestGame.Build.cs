// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TestGame : ModuleRules
{
	public TestGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "NavigationSystem", "AIModule", "GameplayTags", "OnlineSubsystem", "Sockets", "Networking", "GameplayAbilities", "GameplayTasks", "UMG", "Paper2D", "GeometryCollectionEngine", "EnhancedInput", "Niagara", "Chaos", "PhysicsCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { "OnlineSubsystem" });
		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true\

		DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
	}
}
