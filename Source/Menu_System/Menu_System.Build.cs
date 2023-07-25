// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Menu_System : ModuleRules
{
	public Menu_System(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"HeadMountedDisplay",
			"EnhancedInput", 
			"OnlineSubsystemSteam",
			"OnlineSubsystem"
		}
		);
	}
}
