$ErrorActionPreference = 'Stop'
$binDir = 'C:\Users\DINESHBABU\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT.LLVM_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin'
$gpp = Join-Path $binDir 'g++.exe'
$root = $PSScriptRoot

$env:Path = $binDir + ';' + $env:Path

Write-Host "Building BlockVault Executable..." -ForegroundColor Cyan

$outDir = Join-Path $root 'bin'
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$inc1 = "-I" + (Join-Path $root 'include')
$inc2 = "-I" + (Join-Path $root 'third_party\sqlite')
$incSodium = "-I" + (Join-Path $root 'third_party\libsodium\libsodium-win64\include')
$libSodium = "-L" + (Join-Path $root 'third_party\libsodium\libsodium-win64\lib')

$sources = @(
    (Join-Path $root 'src\crypto\SecureBuffer.cpp'),
    (Join-Path $root 'src\crypto\CryptoManager.cpp'),
    (Join-Path $root 'src\crypto\PasswordGenerator.cpp'),
    (Join-Path $root 'src\models\User.cpp'),
    (Join-Path $root 'src\models\Credential.cpp'),
    (Join-Path $root 'src\models\Block.cpp'),
    (Join-Path $root 'src\models\Vault.cpp'),
    (Join-Path $root 'src\db\PreparedStatement.cpp'),
    (Join-Path $root 'src\db\Transaction.cpp'),
    (Join-Path $root 'src\db\MigrationManager.cpp'),
    (Join-Path $root 'src\db\DatabaseManager.cpp'),
    (Join-Path $root 'src\ledger\AuditLedger.cpp'),
    (Join-Path $root 'src\auth\SessionManager.cpp'),
    (Join-Path $root 'src\auth\AuthManager.cpp'),
    (Join-Path $root 'src\vault\CredentialManager.cpp'),
    (Join-Path $root 'src\main.cpp')
)

$sqliteObj = Join-Path $root 'third_party\sqlite\sqlite3.o'
$outExe = Join-Path $outDir 'blockvault.exe'

$args = @(
    '-std=c++20',
    '-O2',
    '-Wall',
    '-Wextra',
    '-static',
    '-static-libgcc',
    '-static-libstdc++',
    $inc1,
    $inc2,
    $incSodium,
    $libSodium
) + $sources + @($sqliteObj, '-lsodium', '-o', $outExe)

& $gpp @args
if ($LASTEXITCODE -eq 0 -and (Test-Path $outExe)) {
    Write-Host "Build complete: $outExe" -ForegroundColor Green
    & $outExe
}
else {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}