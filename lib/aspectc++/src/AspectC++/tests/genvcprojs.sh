#!/bin/sh

# to be executed in the AspectC++/tests directory

find . -maxdepth 2 -mindepth 2 -type f -name "Makefile" |
while read testmakefile; do
  testdir=`dirname "$testmakefile"`
  testname=`basename "$testdir"`

  echo "Generating project file $testdir/$testname.vcproj"
  cd "$testdir"
  rm -f "$testname.vcproj"
  
  REALINST=`grep -q "real-instances" Makefile && echo ";AC_OPTIONS=&quot;--real-instances&quot;"`

  cat >> "$testname.vcproj" << EOF
<?xml version="1.0" encoding = "Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="7.00"
	Name="$testname"
	ProjectGUID="{37F3A735-0447-48C0-8FC5-832F01725A47}"
	Keyword="Win32Proj">
	<Platforms>
		<Platform
			Name="Win32"/>
	</Platforms>
	<Configurations>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory="Debug"
			IntermediateDirectory="Debug"
			ConfigurationType="1"
			CharacterSet="2">
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				PreprocessorDefinitions="_ASPECTC,WIN32;_DEBUG;_CONSOLE$REALINST"
				MinimalRebuild="TRUE"
				BasicRuntimeChecks="3"
				RuntimeLibrary="5"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="TRUE"
				DebugInformationFormat="4"/>
			<Tool
				Name="VCCustomBuildTool"/>
			<Tool
				Name="VCLinkerTool"
				OutputFile="\$(OutDir)/feature.exe"
				LinkIncremental="2"
				GenerateDebugInformation="TRUE"
				ProgramDatabaseFile="\$(OutDir)/feature.pdb"
				SubSystem="1"
				TargetMachine="1"/>
			<Tool
				Name="VCMIDLTool"/>
			<Tool
				Name="VCPostBuildEventTool"/>
			<Tool
				Name="VCPreBuildEventTool"/>
			<Tool
				Name="VCPreLinkEventTool"/>
			<Tool
				Name="VCResourceCompilerTool"/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"/>
			<Tool
				Name="VCWebDeploymentTool"/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory="Release"
			IntermediateDirectory="Release"
			ConfigurationType="1"
			CharacterSet="2">
			<Tool
				Name="VCCLCompilerTool"
				Optimization="2"
				InlineFunctionExpansion="1"
				OmitFramePointers="TRUE"
				PreprocessorDefinitions="_ASPECTC,WIN32;NDEBUG;_CONSOLE$REALINST"
				StringPooling="TRUE"
				RuntimeLibrary="4"
				EnableFunctionLevelLinking="TRUE"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="TRUE"
				DebugInformationFormat="3"/>
			<Tool
				Name="VCCustomBuildTool"/>
			<Tool
				Name="VCLinkerTool"
				OutputFile="\$(OutDir)/feature.exe"
				LinkIncremental="1"
				GenerateDebugInformation="TRUE"
				SubSystem="1"
				OptimizeReferences="2"
				EnableCOMDATFolding="2"
				TargetMachine="1"/>
			<Tool
				Name="VCMIDLTool"/>
			<Tool
				Name="VCPostBuildEventTool"/>
			<Tool
				Name="VCPreBuildEventTool"/>
			<Tool
				Name="VCPreLinkEventTool"/>
			<Tool
				Name="VCResourceCompilerTool"/>
			<Tool
				Name="VCWebServiceProxyGeneratorTool"/>
			<Tool
				Name="VCWebDeploymentTool"/>
		</Configuration>
	</Configurations>
	<Files>
		<Filter
			Name="Source Files"
			Filter="cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm">
EOF

  find . -maxdepth 1 -type f -regex "\./.*\.\(c\|cc\|cxx\|cpp\)" -printf "%f\n" |
  while read transunit; do
    cat >> "$testname.vcproj" << EOF
			<File RelativePath="$transunit"></File>
EOF
  done

  cat >> "$testname.vcproj" << EOF
		</Filter>
		<Filter
			Name="Header Files"
			Filter="h;hpp;hxx;hm;inl;inc">
EOF

  find . -maxdepth 1 -type f -regex "\./.*\.\(h\|hxx\|hpp\)" -printf "%f\n" |
  while read include; do
    cat >> "$testname.vcproj" << EOF
			<File RelativePath="$include"></File>
EOF
  done

  cat >> "$testname.vcproj" << EOF
		</Filter>
		<Filter
			Name="Resource Files"
			Filter="rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe">
		</Filter>
		<Filter
			Name="Aspect Files"
			Filter="ah;acc">
			<Filter
				Name="Active Aspects"
				Filter="ah;acc">
EOF

  rm -f "${testname}_DummyAspect.ah"

  find . -maxdepth 1 -type f -regex "\./.*\.\(ah\|acc\)" -printf "%f\n" |
  while read aspect; do
    cat >> "$testname.vcproj" << EOF
				<File RelativePath="$aspect"></File>
EOF
  done
  
  if [ -z "`find . -maxdepth 1 -type f -regex "\./.*\.\(ah\|acc\)" -printf "%f\n"`" ]; then
    echo "  add dummy aspect"
    cat >> "$testname.vcproj" << EOF
				<File RelativePath="${testname}_DummyAspect.ah"></File>
EOF
    echo " " > "${testname}_DummyAspect.ah"
  fi

  cat >> "$testname.vcproj" << EOF
			</Filter>
			<Filter
				Name="Inactive Aspects"
				Filter="ah;acc">
			</Filter>
		</Filter>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
EOF
  
#  recode lat1..ibmpc -q "$testname.vcproj"
  cd ..
done

echo "Generating solution file WeaverTests.sln"
rm -f "WeaverTests.sln"

cat >> "WeaverTests.sln" << EOF
Microsoft Visual Studio Solution File, Format Version 7.00
EOF

find . -maxdepth 2 -mindepth 2 -type f -name "Makefile" |
while read testmakefile; do
  testdir=`dirname "$testmakefile"`
  testname=`basename "$testdir"`
  cat >> "WeaverTests.sln" << EOF
Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "$testname", "$testname\\$testname.vcproj", "{37F3A735-0447-48C0-8FC5-832F01725A47}"
EndProject
EOF
done

cat >> "WeaverTests.sln" << EOF
Global
	GlobalSection(SolutionConfiguration) = preSolution
		ConfigName.0 = Debug
		ConfigName.1 = Release
	EndGlobalSection
	GlobalSection(ProjectDependencies) = postSolution
	EndGlobalSection
	GlobalSection(ProjectConfiguration) = postSolution
		{37F3A735-0447-48C0-8FC5-832F01725A47}.Debug.ActiveCfg = Debug|Win32
		{37F3A735-0447-48C0-8FC5-832F01725A47}.Debug.Build.0 = Debug|Win32
		{37F3A735-0447-48C0-8FC5-832F01725A47}.Release.ActiveCfg = Release|Win32
		{37F3A735-0447-48C0-8FC5-832F01725A47}.Release.Build.0 = Release|Win32
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
	EndGlobalSection
	GlobalSection(ExtensibilityAddIns) = postSolution
	EndGlobalSection
EndGlobal
EOF

#recode lat1..ibmpc -q "WeaverTests.sln"
