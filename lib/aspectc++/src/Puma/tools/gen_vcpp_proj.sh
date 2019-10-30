#!/bin/bash

# This file is part of PUMA.
# Copyright (C) The PUMA developer team.
#                                                                
# This program is free software;  you can redistribute it and/or 
# modify it under the terms of the GNU General Public License as 
# published by the Free Software Foundation; either version 2 of 
# the License, or (at your option) any later version.            
#                                                                
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
# GNU General Public License for more details.                   
#                                                                
# You should have received a copy of the GNU General Public      
# License along with this program; if not, write to the Free     
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
# MA  02111-1307  USA                                            

if [ "$1" = "debug" ]; then
  TARGET=debug
  PACKAGE="PumaVCProj_d.zip"
  PRJFILE="Puma_d.vcproj"
else
  TARGET=release
  PACKAGE="PumaVCProj.zip"
  PRJFILE="Puma.vcproj"
fi

echo "Generating Puma project file $PRJFILE..."

rm -f "$PRJFILE"
  
cat >> "$PRJFILE" << EOF
<?xml version="1.0" encoding="Windows-1252"?>
<VisualStudioProject
	ProjectType="Visual C++"
	Version="7.00"
	Name="Puma"
	ProjectGUID="{37F3A735-0447-48C0-8FC5-832F01725A47}"
	Keyword="Win32Proj">
	<Platforms>
		<Platform
			Name="Win32"/>
	</Platforms>
	<Configurations>
		<Configuration
			Name="Debug|Win32"
			OutputDirectory=".\lib\\\$(ConfigurationName)"
			IntermediateDirectory=".\lib"
			ConfigurationType="4"
			CharacterSet="2">
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="&quot;.\gen-${TARGET}\step2\inc&quot;;&quot;.\gen-${TARGET}\step2\src&quot;;.\src\win32\regex"
				PreprocessorDefinitions="WIN32;_DEBUG;_LIB;__SIZE_TYPE__=size_t;__PTRDIFF_TYPE__=ptrdiff_t;HAVE_STRING_H;STDC_HEADERS"
				MinimalRebuild="true"
				BasicRuntimeChecks="3"
				RuntimeLibrary="3"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				DebugInformationFormat="4"
				DisableSpecificWarnings="4800;4355;4996;4244;4267"/>
			<Tool
				Name="VCPostBuildEventTool"
				CommandLine="copy &quot;\$(ProjectDir)\src\win32\regex\regex.h&quot; &quot;\$(ProjectDir)\include&quot;"/>
		</Configuration>
		<Configuration
			Name="Release|Win32"
			OutputDirectory=".\lib\\\$(ConfigurationName)"
			IntermediateDirectory=".\lib"
			ConfigurationType="4"
			CharacterSet="2"
			WholeProgramOptimization="1">
			<Tool
				Name="VCCLCompilerTool"
				AdditionalIncludeDirectories="&quot;.\gen-${TARGET}\step2\inc&quot;;&quot;.\gen-${TARGET}\step2\src&quot;;.\src\win32\regex"
				PreprocessorDefinitions="WIN32;NDEBUG;_LIB;__SIZE_TYPE__=size_t;__PTRDIFF_TYPE__=ptrdiff_t;HAVE_STRING_H;STDC_HEADERS"
				RuntimeLibrary="2"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				DebugInformationFormat="0"
				DisableSpecificWarnings="4355;4800;4996;4244;4267"/>
			<Tool
				Name="VCPostBuildEventTool"
				CommandLine="copy &quot;\$(ProjectDir)\src\win32\regex\regex.h&quot; &quot;\$(ProjectDir)\include&quot;"/>
		</Configuration>
		<Configuration
			Name="Debug DLL|Win32"
			OutputDirectory=".\lib\\\$(ConfigurationName)"
			IntermediateDirectory=".\lib"
			ConfigurationType="2"
			CharacterSet="2">
			<Tool
				Name="VCCLCompilerTool"
				Optimization="0"
				AdditionalIncludeDirectories="&quot;.\gen-${TARGET}\step2\inc&quot;;&quot;.\gen-${TARGET}\step2\src&quot;;.\src\win32\regex"
				PreprocessorDefinitions="WIN32;_DEBUG;_LIB;__SIZE_TYPE__=size_t;__PTRDIFF_TYPE__=ptrdiff_t;HAVE_STRING_H;STDC_HEADERS"
				MinimalRebuild="true"
				BasicRuntimeChecks="3"
				RuntimeLibrary="3"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				DebugInformationFormat="4"
				DisableSpecificWarnings="4800;4355;4996;4244;4267"/>
			<Tool
				Name="VCLinkerTool"
				GenerateDebugInformation="true"/>
			<Tool
				Name="VCPostBuildEventTool"
				CommandLine="copy &quot;\$(ProjectDir)\src\win32\regex\regex.h&quot; &quot;\$(ProjectDir)\include&quot;"/>
		</Configuration>
		<Configuration
			Name="Release DLL|Win32"
			OutputDirectory=".\lib\\\$(ConfigurationName)"
			IntermediateDirectory=".\lib"
			ConfigurationType="2"
			CharacterSet="2"
			WholeProgramOptimization="1">
			<Tool
				Name="VCCLCompilerTool"
				AdditionalIncludeDirectories="&quot;.\gen-${TARGET}\step2\inc&quot;;&quot;.\gen-${TARGET}\step2\src&quot;;.\src\win32\regex"
				PreprocessorDefinitions="WIN32;NDEBUG;_LIB;__SIZE_TYPE__=size_t;__PTRDIFF_TYPE__=ptrdiff_t;HAVE_STRING_H;STDC_HEADERS"
				RuntimeLibrary="2"
				UsePrecompiledHeader="0"
				WarningLevel="3"
				Detect64BitPortabilityProblems="true"
				DebugInformationFormat="0"
				DisableSpecificWarnings="4355;4800;4996;4244;4267"/>
			<Tool
				Name="VCPostBuildEventTool"
				CommandLine="copy &quot;\$(ProjectDir)\src\win32\regex\regex.h&quot; &quot;\$(ProjectDir)\include&quot;"/>
		</Configuration>
	</Configurations>
	<Files>
		<Filter
			Name="Source Files"
			Filter="cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx">
EOF

function add_files() {
  find $1 -type f -regex "$2" -printf "%f\n" | \
  while read file; do
    echo "			<File RelativePath=\"$3\\$file\"></File>" >> "$PRJFILE"
  done
}

add_files "./gen-${TARGET}/step2/src" "\./.*\.\(c\|cc\|cxx\|cpp\)" ".\gen-${TARGET}\step2\src"
add_files "./src/win32/regex" "\./.*\.\(c\|cc\|cxx\|cpp\)" ".\src\win32\regex"

cat >> "$PRJFILE" << EOF
		</Filter>
		<Filter
			Name="Header Files"
			Filter="h;hpp;hxx;ah;ot;hm;inl;inc;xsd">
EOF

add_files "./gen-${TARGET}/step2/inc/Puma" "\./.*\.\(h\|ah\|hpp\|ot\)" ".\gen-${TARGET}\step2\inc\Puma"
add_files "./gen-${TARGET}/step2/src" "\./.*\.\(h\|ah\|hpp\|ot\)" ".\gen-${TARGET}\step2\src"
add_files "./src/win32/regex" "\./.*\.\(h\|ah\|hpp\|ot\)" ".\src\win32\regex"

cat >> "$PRJFILE" << EOF
		</Filter>
	</Files>
	<Globals>
	</Globals>
</VisualStudioProject>
EOF
  
recode lat1..ibmpc -q "$PRJFILE"

if [ "$2" = "pack" ]; then
  echo "Creating Puma source package $PACKAGE..."
  zip -q -r "$PACKAGE" "./gen-${TARGET}/step2/inc" "./gen-${TARGET}/step2/src" ./include
  zip -q "$PACKAGE" "$PRJFILE" COPYING README ./src/win32/regex/regex.* ./lib
fi
