$ErrorActionPreference = 'Stop'
$binDir = 'C:\Users\DINESHBABU\AppData\Local\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT.LLVM_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin'
$gpp = Join-Path $binDir 'g++.exe'
$root = $PSScriptRoot

$env:Path = $binDir + ';' + $env:Path

Write-Host "Compiling BlockVault C++20 Test Suites..." -ForegroundColor Cyan

$inc1 = "-I" + (Join-Path $root 'include')
$inc2 = "-I" + (Join-Path $root 'third_party\sqlite')
$incSodium = "-I" + (Join-Path $root 'third_party\libsodium\libsodium-win64\include')
$libSodium = "-L" + (Join-Path $root 'third_party\libsodium\libsodium-win64\lib')

$cryptoSources = @(
    (Join-Path $root 'src\crypto\SecureBuffer.cpp'),
    (Join-Path $root 'src\crypto\CryptoManager.cpp'),
    (Join-Path $root 'src\crypto\PasswordGenerator.cpp')
)

$allSources = $cryptoSources + @(
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
    (Join-Path $root 'src\vault\CredentialManager.cpp')
)

$sqliteObj = Join-Path $root 'third_party\sqlite\sqlite3.o'

$commonFlags = @(
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
)

# ------------------------------------------------------------------------------
# 1. Compile and run Cryptography Test Suite (test_crypto.exe)
# ------------------------------------------------------------------------------
Write-Host "--- 1. Building Libsodium Cryptography Test Suite (test_crypto.exe) ---" -ForegroundColor Yellow
$cryptoTestExe = Join-Path $root 'tests\test_crypto.exe'
$cryptoTestArgs = $commonFlags + $cryptoSources + @(
    (Join-Path $root 'tests\test_crypto.cpp'),
    '-lsodium',
    '-o', $cryptoTestExe
)

& $gpp @cryptoTestArgs
if ($LASTEXITCODE -eq 0 -and (Test-Path $cryptoTestExe)) {
    Write-Host "Running Cryptography Tests..." -ForegroundColor Green
    & $cryptoTestExe
} else {
    Write-Host "Cryptography test compilation failed!" -ForegroundColor Red
    exit 1
}

# ------------------------------------------------------------------------------
# 2. Compile and run Password Generator Test Suite (test_password_generator.exe)
# ------------------------------------------------------------------------------
Write-Host "`n--- 2. Building Password Generator Test Suite (test_password_generator.exe) ---" -ForegroundColor Yellow
$pwdTestExe = Join-Path $root 'tests\test_password_generator.exe'
$pwdTestArgs = $commonFlags + $cryptoSources + @(
    (Join-Path $root 'tests\test_password_generator.cpp'),
    '-lsodium',
    '-o', $pwdTestExe
)

& $gpp @pwdTestArgs
if ($LASTEXITCODE -eq 0 -and (Test-Path $pwdTestExe)) {
    Write-Host "Running Password Generator Tests..." -ForegroundColor Green
    & $pwdTestExe
} else {
    Write-Host "Password Generator test compilation failed!" -ForegroundColor Red
    exit 1
}

# ------------------------------------------------------------------------------
# 3. Compile and run Database Test Suite (test_database.exe)
# ------------------------------------------------------------------------------
Write-Host "`n--- 3. Building Database Test Suite (test_database.exe) ---" -ForegroundColor Yellow
$dbTestExe = Join-Path $root 'tests\test_database.exe'
$dbTestArgs = $commonFlags + $allSources + @(
    (Join-Path $root 'tests\test_database.cpp'),
    $sqliteObj,
    '-lsodium',
    '-o', $dbTestExe
)

& $gpp @dbTestArgs
if ($LASTEXITCODE -eq 0 -and (Test-Path $dbTestExe)) {
    Write-Host "Running Database Tests..." -ForegroundColor Green
    & $dbTestExe
} else {
    Write-Host "Database test compilation failed!" -ForegroundColor Red
    exit 1
}

# ------------------------------------------------------------------------------
# 4. Compile and run End-to-End Test Suite (test_runner.exe)
# ------------------------------------------------------------------------------
Write-Host "`n--- 4. Building End-to-End Test Suite (test_runner.exe) ---" -ForegroundColor Yellow
$e2eTestExe = Join-Path $root 'tests\test_runner.exe'
$e2eTestArgs = $commonFlags + $allSources + @(
    (Join-Path $root 'tests\main_test.cpp'),
    $sqliteObj,
    '-lsodium',
    '-o', $e2eTestExe
)

& $gpp @e2eTestArgs
if ($LASTEXITCODE -eq 0 -and (Test-Path $e2eTestExe)) {
    Write-Host "Running End-to-End Tests..." -ForegroundColor Green
    & $e2eTestExe
} else {
    Write-Host "End-to-End test compilation failed!" -ForegroundColor Red
    exit 1
}
