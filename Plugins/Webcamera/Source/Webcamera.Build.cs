// Copyright (C) 2016 Fredrik Lindh. All Rights Reserved.
// Created by Fredrik Lindh (Temaran) on 2016-01-28

using UnrealBuildTool;

public class Webcamera : ModuleRules
{
	public Webcamera(TargetInfo Target)
	{		
		PublicIncludePaths.AddRange(
			new string[] 
            {
				"Public"
			});
				
		
		PrivateIncludePaths.AddRange(
			new string[] 
            {
				"Private",
			});
			
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Engine",
				"Core",
                "CoreUObject",
                "Projects",
                "RHI",
                "RenderCore",
                "ShaderCore"
			});
	}
}
