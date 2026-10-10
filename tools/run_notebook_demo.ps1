$ErrorActionPreference = 'Stop'
$study = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$python = Join-Path $env:USERPROFILE '.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
if (-not (Test-Path -LiteralPath $python)) { $python = 'python' }
Write-Host '=== TESTES DOS COMPONENTES ISOLADOS ===' -ForegroundColor Cyan
$studyWsl = (& wsl wslpath -a "$study").Trim()
& wsl -d Ubuntu -- bash -lc "cd '$studyWsl' && bash tests/run-component-checks.sh"
Write-Host '=== VERIFICACAO DOS HASHES DA FONTE ORIGINAL ===' -ForegroundColor Cyan
& $python (Join-Path $study 'tools\record_component_validation.py')
Write-Host '=== ARQUIVOS DE DEMONSTRACAO ===' -ForegroundColor Cyan
Write-Host (Join-Path $study 'prepared\star-2500-corner\layout.svg')
Write-Host (Join-Path $study 'study\STATUS.md')
Start-Process (Join-Path $study 'prepared\star-2500-corner\layout.svg')
Start-Process notepad.exe (Join-Path $study 'study\STATUS.md')
