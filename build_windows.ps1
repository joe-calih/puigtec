[CmdletBinding()]
param(
    [switch]$SkipBuild,
    [switch]$RequireInstaller
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $projectRoot

$cmake = Get-Command cmake -ErrorAction SilentlyContinue
$cmakePath = if ($cmake) { $cmake.Source } else { $null }
if ($null -eq $cmakePath) {
    foreach ($candidate in @(
        'C:\Program Files\CMake\bin\cmake.exe',
        'C:\Program Files (x86)\CMake\bin\cmake.exe',
        "$env:LOCALAPPDATA\Programs\CMake\bin\cmake.exe"
    )) {
        if (Test-Path -LiteralPath $candidate) {
            $cmakePath = $candidate
            break
        }
    }
}

if ($null -eq $cmakePath) {
    $portableCMakeRoot = Join-Path $env:LOCALAPPDATA 'Programs\CMake'
    if (Test-Path -LiteralPath $portableCMakeRoot) {
        $portableCMake = Get-ChildItem -LiteralPath $portableCMakeRoot -Directory |
            Sort-Object Name -Descending |
            ForEach-Object {
                Join-Path $_.FullName 'bin\cmake.exe'
            } |
            Where-Object { Test-Path -LiteralPath $_ } |
            Select-Object -First 1
        if ($portableCMake) {
            $cmakePath = $portableCMake
        }
    }
}

if ($null -eq $cmakePath) {
    throw 'CMake is required. Install CMake 3.22 or later, then run this script again.'
}

if (-not $SkipBuild) {
    $visualStudioPath = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools'
    $visualStudioEnvironment = Join-Path $visualStudioPath 'VC\Auxiliary\Build\vcvars64.bat'
    $ctest = Join-Path (Split-Path -Parent $cmakePath) 'ctest.exe'
    if (-not (Test-Path -LiteralPath $visualStudioEnvironment)) {
        throw "Visual Studio 2022 x64 C++ Build Tools were not found at $visualStudioPath."
    }
    if (-not (Test-Path -LiteralPath $ctest)) {
        throw "CMake's test runner was not found at $ctest."
    }

    $commands = 'call "' + $visualStudioEnvironment +
        '" >nul && "' + $cmakePath +
        '" -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release' +
        ' && "' + $cmakePath +
        '" --build build' +
        ' && "' + $ctest +
        '" --test-dir build -C Release --output-on-failure'
    & $env:ComSpec /d /c $commands
    if ($LASTEXITCODE -ne 0) {
        throw "Windows x64 Release build or DSP tests failed with exit code $LASTEXITCODE."
    }
}

$buildRoot = Join-Path $projectRoot 'build'
if (-not (Test-Path -LiteralPath $buildRoot)) {
    throw "Build output was not found at $buildRoot."
}

$pluginCandidates = @(
    Get-ChildItem -LiteralPath $buildRoot -Directory -Filter 'Blue Steel EQ.vst3' -Recurse |
        Sort-Object FullName
)
if ($pluginCandidates.Count -eq 0) {
    $pluginCandidates = @(
        Get-ChildItem -LiteralPath $buildRoot -Directory -Filter '*.vst3' -Recurse |
            Where-Object {
                Test-Path -LiteralPath (Join-Path $_.FullName 'Contents')
            } |
            Sort-Object FullName
    )
}
if ($pluginCandidates.Count -eq 0) {
    throw 'Could not find the built VST3 bundle. Build the BlueSteelEQ_VST3 target first.'
}

$appCandidates = @(
    Get-ChildItem -LiteralPath $buildRoot -File -Filter '*.exe' -Recurse |
        Where-Object {
            $_.FullName -notmatch '\.vst3[\\/]' -and
            (
                $_.Directory.Name -eq 'Standalone' -or
                $_.BaseName -like 'Blue Steel EQ*' -or
                $_.BaseName -eq 'BlueSteelEQ_Standalone'
            )
        } |
        Sort-Object FullName
)
if ($appCandidates.Count -eq 0) {
    throw 'Could not find the standalone Blue Steel EQ executable in the build output.'
}

$stagingRoot = Join-Path $projectRoot 'installer\staging'
$stagedPlugin = Join-Path $stagingRoot 'VST3\Blue Steel EQ v2.vst3'
$stagedApp = Join-Path $stagingRoot 'Standalone\Blue Steel EQ.exe'
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $stagedPlugin) | Out-Null
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $stagedApp) | Out-Null

if (Test-Path -LiteralPath $stagedPlugin) {
    Remove-Item -LiteralPath $stagedPlugin -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $stagedPlugin | Out-Null
Copy-Item -Path (Join-Path $pluginCandidates[0].FullName '*') `
    -Destination $stagedPlugin -Recurse -Force
Copy-Item -LiteralPath $appCandidates[0].FullName -Destination $stagedApp -Force

$juceLicense = Join-Path $buildRoot '_deps\juce-src\LICENSE.md'
if (-not (Test-Path -LiteralPath $juceLicense)) {
    $juceLicense = Join-Path $buildRoot '_deps\JUCE-src\LICENSE.md'
}
if (-not (Test-Path -LiteralPath $juceLicense)) {
    throw "The JUCE third-party license was not found in $buildRoot\_deps."
}
Copy-Item -LiteralPath $juceLicense `
    -Destination (Join-Path $stagingRoot 'Standalone\JUCE-LICENSE.md') -Force
Copy-Item -LiteralPath (Join-Path $projectRoot 'LICENSES\AGPL-3.0.txt') `
    -Destination (Join-Path $stagingRoot 'Standalone\AGPL-3.0.txt') -Force

$outputDirectory = Join-Path $projectRoot 'installer\output'
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
$distributionArchives = @(
    @{
        Name = 'Blue-Steel-EQ-VST3.zip'
        Sources = @(
            $stagedPlugin,
            (Join-Path $projectRoot 'LICENSE'),
            (Join-Path $projectRoot 'LICENSES\AGPL-3.0.txt'),
            (Join-Path $stagingRoot 'Standalone\JUCE-LICENSE.md')
        )
    },
    @{
        Name = 'Blue-Steel-EQ-Standalone.zip'
        Sources = @(
            $stagedApp,
            (Join-Path $projectRoot 'LICENSE'),
            (Join-Path $projectRoot 'LICENSES\AGPL-3.0.txt'),
            (Join-Path $stagingRoot 'Standalone\JUCE-LICENSE.md'),
            (Join-Path $projectRoot 'README.md'),
            (Join-Path $projectRoot 'docs\V2-RELEASE-NOTES.md')
        )
    }
)

foreach ($archive in $distributionArchives) {
    $archivePath = Join-Path $outputDirectory $archive.Name
    if (Test-Path -LiteralPath $archivePath) {
        Remove-Item -LiteralPath $archivePath -Force
    }

    Compress-Archive `
        -LiteralPath $archive.Sources `
        -DestinationPath $archivePath `
        -CompressionLevel Optimal
    if (-not (Test-Path -LiteralPath $archivePath)) {
        throw "The standalone VST3 or application package could not be created: $archivePath."
    }
    Write-Output "Distribution archive created: $archivePath"
}

$innoCompiler = Get-Command ISCC.exe -ErrorAction SilentlyContinue
$innoCompilerPath = $null
if ($null -eq $innoCompiler) {
    $knownInnoPaths = @(
        'C:\Program Files (x86)\Inno Setup 6\ISCC.exe',
        'C:\Program Files\Inno Setup 6\ISCC.exe'
    )
    $innoPath = $knownInnoPaths | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
    if ($null -ne $innoPath) {
        $innoCompilerPath = $innoPath
    }
}
else {
    $innoCompilerPath = $innoCompiler.Source
}

if ($null -eq $innoCompilerPath) {
    Write-Output 'The VST3 and standalone executable are ready under installer\staging\.'
    if ($RequireInstaller) {
        throw 'Inno Setup 6 is required to create the requested Windows installer executable.'
    }
    Write-Output 'Install Inno Setup 6 to create installer\output\Blue-Steel-EQ-Setup.exe.'
    exit 0
}

$env:BLUE_STEEL_STAGE = $stagingRoot
& $innoCompilerPath (Join-Path $projectRoot 'installer\BlueSteelEQ.iss')
if ($LASTEXITCODE -ne 0) {
    throw "Inno Setup failed with exit code $LASTEXITCODE."
}

$env:BLUE_STEEL_STAGE = $null
$installer = Join-Path $projectRoot 'installer\output\Blue-Steel-EQ-Setup.exe'
if (-not (Test-Path -LiteralPath $installer)) {
    throw 'Inno Setup completed without producing the Windows installer.'
}

Write-Output "Installer created: $installer"
