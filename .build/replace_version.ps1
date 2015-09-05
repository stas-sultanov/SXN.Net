# Enable -Verbose option
[CmdletBinding()]

# A list of files templates on which current script should be applied
$TargetFiles = '*AssemblyInfo.cs', '*.nuspec'

# A regular expression pattern to find the version in the TFS build number variable
$TfsVersionRegex = "\d+\.\d+\.\d+\.\d+"

# A regular expression pattern to find the version in the target files
$VersionRegex = "255\.255\.255\.255"

# Check if source code directory is available
if (-not $Env:BUILD_SOURCESDIRECTORY)
{
	Write-Error ("BUILD_SOURCESDIRECTORY environment variable is missing.")

	exit 1
}
elseif (-not (Test-Path $Env:BUILD_SOURCESDIRECTORY))
{
	Write-Error "BUILD_SOURCESDIRECTORY does not exist: $Env:BUILD_SOURCESDIRECTORY"

	exit 1
}

Write-Verbose "BUILD_SOURCESDIRECTORY: $Env:BUILD_SOURCESDIRECTORY"

# Check if build number is available
if (-not $Env:BUILD_BUILDNUMBER)
{
	Write-Error ("BUILD_BUILDNUMBER environment variable is missing.")

	exit 1
}

Write-Verbose "BUILD_BUILDNUMBER: $Env:BUILD_BUILDNUMBER"

# Get and validate the version data
$VersionData = [regex]::matches($Env:BUILD_BUILDNUMBER,$TfsVersionRegex)

switch($VersionData.Count)
{
	0
		{ 
			Write-Error "Could not find version number data in BUILD_BUILDNUMBER."

			exit 1
		}
	1
		{
		}
	default
		{ 
			Write-Warning "Found more than instance of version data in BUILD_BUILDNUMBER."

			Write-Warning "Will assume first instance is version."
		}
}

# Get current version
$CurrentVersion = $VersionData[0]

Write-Verbose "Current Version Is : $CurrentVersion"

# Enumerate files which contains version information
$files = @()

foreach ($targetFile in $TargetFiles)
{
	$files += Get-ChildItem -Path $Env:BUILD_SOURCESDIRECTORY -Recurse -File -Include $targetFile
}

# Apply version to files
foreach ($file in $files)
{
	Write-Host "Applying version to: $file"

	# Get file content
	$filecontent = Get-Content($file)

	# Clear read-only flag
	attrib $file -r

	# Replace version
	$filecontent -replace $VersionRegex, $CurrentVersion | Out-File $file
}
