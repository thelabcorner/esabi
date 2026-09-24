param(
    [string]$Clang = ""
)

$ErrorActionPreference = "Stop"
$buildRoot = Join-Path $PSScriptRoot ".build"
$build = Join-Path $buildRoot ("run-" + [Guid]::NewGuid().ToString("N"))

if (-not $Clang) {
    if ($env:CLANG_PATH -and (Test-Path $env:CLANG_PATH)) {
        $Clang = $env:CLANG_PATH
    } else {
        $cmd = Get-Command clang -ErrorAction SilentlyContinue
        if ($cmd) {
            $Clang = $cmd.Source
        } elseif (Test-Path "C:\Program Files\LLVM\bin\clang.exe") {
            $Clang = "C:\Program Files\LLVM\bin\clang.exe"
        } else {
            throw "clang not found; pass -Clang or set CLANG_PATH"
        }
    }
}

$llvmDir = Split-Path -Parent $Clang
$clangCl = Join-Path $llvmDir "clang-cl.exe"
$nm = Join-Path $llvmDir "llvm-nm.exe"
$readobj = Join-Path $llvmDir "llvm-readobj.exe"
foreach ($tool in @($clangCl, $nm, $readobj)) {
    if (-not (Test-Path $tool)) { throw "required LLVM tool not found: $tool" }
}

New-Item -ItemType Directory -Force -Path $build | Out-Null

function Invoke-Checked([string]$Exe, [string[]]$Arguments) {
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Exe failed with exit code $LASTEXITCODE"
    }
}

function Assert-CoffExports([string]$Dll, [string[]]$Expected) {
    $output = & $readobj --coff-exports $Dll
    if ($LASTEXITCODE -ne 0) { throw "llvm-readobj failed for $Dll" }
    foreach ($name in $Expected) {
        $escaped = [regex]::Escape($name)
        if (-not ($output -match "(?m)^\s*Name:\s+$escaped\s*$")) {
            throw "missing exact export '$name' in $Dll"
        }
    }
}

function Invoke-MSVCX86ExportCheck([string[]]$Expected) {
    $programFilesX86 = [Environment]::GetFolderPath("ProgramFilesX86")
    $vswhere = Join-Path $programFilesX86 "Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        Write-Host "MSVC x86 export check skipped (vswhere not found)"
        return
    }

    $install = (& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath | Select-Object -First 1)
    if (-not $install) {
        Write-Host "MSVC x86 export check skipped (VC tools not found)"
        return
    }

    $devcmd = Join-Path $install "Common7\Tools\VsDevCmd.bat"
    if (-not (Test-Path $devcmd)) {
        Write-Host "MSVC x86 export check skipped (VsDevCmd.bat not found)"
        return
    }

    $source = Join-Path $PSScriptRoot "export_smoke.c"
    $object = Join-Path $build "msvc_x86.obj"
    $dll = Join-Path $build "msvc_x86.dll"
    $dump = Join-Path $build "msvc_x86_exports.txt"
    $cmdFile = Join-Path $build "msvc_x86.cmd"

    @(
        ('@call "{0}" -no_logo -arch=x86 -host_arch=x64' -f $devcmd),
        'if errorlevel 1 exit /b 1',
        ('cl /nologo /TC /std:c11 /W4 /WX /c "{0}" /Fo"{1}"' -f $source, $object),
        'if errorlevel 1 exit /b 1',
        ('link /nologo /DLL /NOENTRY /NODEFAULTLIB /OUT:"{0}" "{1}"' -f $dll, $object),
        'if errorlevel 1 exit /b 1',
        ('dumpbin /nologo /exports "{0}" > "{1}"' -f $dll, $dump),
        'if errorlevel 1 exit /b 1'
    ) | Set-Content -Encoding ASCII $cmdFile

    & cmd.exe /d /c $cmdFile
    if ($LASTEXITCODE -ne 0) { throw "MSVC x86 export check failed" }

    $output = Get-Content -Raw $dump
    foreach ($name in $Expected) {
        $escaped = [regex]::Escape($name)
        if (-not ($output -match "(?m)^\s+\d+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+$escaped\s*$")) {
            throw "MSVC x86 DLL missing exact export '$name'"
        }
    }
}

$expectedExports = @(
    "ESFreeMem",
    "ESGetVersion",
    "ESInitialize",
    "ESTerminate",
    "ping",
    "add_one",
    "invert_bool",
    "half_value",
    "text_value",
    "script_value",
    "init_argc"
)

try {
    Invoke-Checked $Clang @("-std=c11", "-Wall", "-Wextra", "-Werror", (Join-Path $PSScriptRoot "layout_smoke.c"), "-o", (Join-Path $build "layout_long32.exe"))
    & (Join-Path $build "layout_long32.exe")
    if ($LASTEXITCODE -ne 0) { throw "LONG32 layout_smoke failed" }

    Invoke-Checked $Clang @("-DESABI_ABI_PROFILE=ESABI_ABI_PROFILE_LONG64", "-std=c11", "-Wall", "-Wextra", "-Werror", (Join-Path $PSScriptRoot "layout_smoke.c"), "-o", (Join-Path $build "layout_long64.exe"))
    & (Join-Path $build "layout_long64.exe")
    if ($LASTEXITCODE -ne 0) { throw "LONG64 layout_smoke failed" }

    Invoke-Checked $Clang @("-std=c99", "-Wall", "-Wextra", "-Werror", "-c", (Join-Path $PSScriptRoot "compile_smoke.c"), "-o", (Join-Path $build "c99.obj"))

    Invoke-Checked $clangCl @("/nologo", "/std:c11", "/W4", "/WX", "/c", (Join-Path $PSScriptRoot "compile_smoke.c"), "/Fo$(Join-Path $build 'clangcl.obj')")

    Invoke-Checked $Clang @("-DESABI_EXPORT=", "-x", "c++", "-std=c++11", "-Wall", "-Wextra", "-Werror", (Join-Path $PSScriptRoot "cpp_smoke.cpp"), "-o", (Join-Path $build "cpp_smoke.exe"))
    & (Join-Path $build "cpp_smoke.exe")
    if ($LASTEXITCODE -ne 0) { throw "cpp_smoke failed" }

    Invoke-Checked $Clang @("--target=i686-pc-windows-msvc", "-mrtd", "-std=c11", "-Wall", "-Wextra", "-Werror", "-c", (Join-Path $PSScriptRoot "compile_smoke.c"), "-o", (Join-Path $build "x86_hostile_cc.obj"))
    $symbols = & $nm (Join-Path $build "x86_hostile_cc.obj")
    if ($LASTEXITCODE -ne 0) { throw "llvm-nm failed" }
    if (-not ($symbols -match "(?m)^\s*[0-9A-Fa-f]+\s+T\s+_esabi_compile_smoke\s*$")) {
        throw "x86 direct method is not the expected cdecl symbol"
    }

    Invoke-Checked $Clang @("-shared", "-nostdlib", "-Wl,/noentry", "-std=c11", "-Wall", "-Wextra", "-Werror", (Join-Path $PSScriptRoot "export_smoke.c"), "-o", (Join-Path $build "export_x64.dll"))
    Assert-CoffExports (Join-Path $build "export_x64.dll") $expectedExports

    Invoke-Checked $Clang @("--target=i686-pc-windows-msvc", "-mrtd", "-shared", "-nostdlib", "-Wl,/noentry", "-std=c11", "-Wall", "-Wextra", "-Werror", (Join-Path $PSScriptRoot "export_smoke.c"), "-o", (Join-Path $build "export_x86.dll"))
    Assert-CoffExports (Join-Path $build "export_x86.dll") $expectedExports

    Invoke-MSVCX86ExportCheck $expectedExports

    Write-Host "esabi validation passed"
} finally {
    if (Test-Path $build) {
        try {
            Remove-Item -Recurse -Force $build -ErrorAction Stop
        } catch {
            Write-Warning ("Could not remove this run's build directory: " + $_.Exception.Message)
        }
    }
}
