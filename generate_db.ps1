$ErrorActionPreference = "Stop"

$workspace = $PSScriptRoot
$ideDataPath = "$workspace\.pio\build\m5stack-core2\idedata.json"
$compileDbPath = "$workspace\compile_commands.json"

if (-not (Test-Path $ideDataPath)) {
    Write-Error "idedata.json not found"
    exit 1
}

$ideData = Get-Content $ideDataPath -Raw | ConvertFrom-Json

# Use full compiler path so clangd query-driver can locate and execute it
$cxx = if ($ideData.cxx_path) { $ideData.cxx_path } else { "xtensa-esp32-elf-g++" }
$flags = $ideData.cxx_flags

# Locate toolchain root for system fallback headers (in correct order: after platform_include)
$toolchainDir = Split-Path (Split-Path $cxx -Parent) -Parent
$toolchainIncludes = @(
    "$toolchainDir\xtensa-esp32-elf\include\c++\8.4.0",
    "$toolchainDir\xtensa-esp32-elf\include\c++\8.4.0\xtensa-esp32-elf",
    "$toolchainDir\lib\gcc\xtensa-esp32-elf\8.4.0\include",
    "$toolchainDir\lib\gcc\xtensa-esp32-elf\8.4.0\include-fixed",
    "$toolchainDir\xtensa-esp32-elf\include"
)

# Format defines properly: do NOT wrap numbers in quotes, and escape strings properly
$defineArgs = @()
$defineCmdParts = @()
foreach ($d in $ideData.defines) {
    $defineArgs += "-D$d"
    if ($d -match '\s') {
        # Argument contains spaces (e.g. ARDUINO_BOARD="M5Stack Core2")
        $escaped = $d -replace '"', '\"'
        $defineCmdParts += "`"-D$escaped`""
    } elseif ($d -match '"') {
        # String define with quotes (e.g. MBEDTLS_CONFIG_FILE="mbedtls/esp_config.h")
        $escaped = $d -replace '"', '\"'
        $defineCmdParts += "-D$escaped"
    } else {
        # Bare or numeric define (e.g. F_CPU=240000000L, ARDUINO=10812)
        $defineCmdParts += "-D$d"
    }
}

$buildIncludes = @($ideData.includes.build)
$includeArgs = @()
$includeCmdParts = @()
foreach ($inc in $buildIncludes) {
    $includeArgs += "-I$inc"
    $includeCmdParts += "-I$inc"
}

# Add toolchain includes after build includes with -isystem
$toolchainArgs = @()
$toolchainCmdParts = @()
foreach ($tinc in $toolchainIncludes) {
    $toolchainArgs += "-isystem"
    $toolchainArgs += $tinc
    $toolchainCmdParts += "-isystem `"$tinc`""
}

# Get all .cpp files in src
$srcFiles = Get-ChildItem -Path "$workspace\src" -Recurse -Filter "*.cpp"

$newEntries = @()
foreach ($file in $srcFiles) {
    $rel = $file.FullName.Substring($workspace.Length + 1)
    $obj = ".pio\build\m5stack-core2\src\$rel.o"

    # Build command string
    $cmdParts = @(
        "`"$cxx`"",
        "-o", "`"$obj`"",
        "-c"
    ) + $flags + $defineCmdParts + $includeCmdParts + $toolchainCmdParts + @("`"$($file.FullName)`"")
    $cmd = $cmdParts -join " "

    # Build arguments array
    $argList = @(
        $cxx,
        "-o", $obj,
        "-c"
    ) + $flags + $defineArgs + $includeArgs + $toolchainArgs + @($file.FullName)

    $newEntries += [PSCustomObject]@{
        directory = $workspace
        file      = $file.FullName
        command   = $cmd
        arguments = $argList
        output    = $obj
    }
}

# Read existing compile_commands.json if exists
$existingEntries = @()
if (Test-Path $compileDbPath) {
    $existingEntries = Get-Content $compileDbPath -Raw | ConvertFrom-Json
    # Filter out existing src entries if any
    $existingEntries = @($existingEntries | Where-Object { $_.file -notmatch '\\src\\' })
}

$allEntries = $newEntries + $existingEntries
$json = $allEntries | ConvertTo-Json -Depth 10
[System.IO.File]::WriteAllText($compileDbPath, $json, [System.Text.Encoding]::UTF8)

Write-Host "Successfully updated compile_commands.json with $($newEntries.Count) project files. Total entries: $($allEntries.Count)"
