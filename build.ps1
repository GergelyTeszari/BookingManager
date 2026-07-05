$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $vsWhere))
{
    Write-Error "Visual Studio Installer was not found."
    exit 1
}

$installationPath = & $vsWhere `
    -latest `
    -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath

if (-not $installationPath)
{
    Write-Error "A Visual Studio installation with C++ build tools was not found."
    exit 1
}

$devShellModule = Join-Path `
    $installationPath `
    "Common7\Tools\Microsoft.VisualStudio.DevShell.dll"

Import-Module $devShellModule
Enter-VsDevShell `
    -VsInstallPath $installationPath `
    -SkipAutomaticLocation

Set-Location $PSScriptRoot

cl /nologo /EHsc /std:c++20 `
    BookingManager.cpp `
    BookingManagerTests.cpp `
    /Fe:BookingManagerTests.exe

if ($LASTEXITCODE -ne 0)
{
    Write-Host " ########## Build failed. ##########"
    exit $LASTEXITCODE
}

Write-Host " ########## Build successful. ##########"

& "$PSScriptRoot\BookingManagerTests.exe"

if ($LASTEXITCODE -ne 0)
{
    Write-Host " ########## Tests failed. ##########"
    exit $LASTEXITCODE
}

Write-Host " ########## All tests passed. ##########"