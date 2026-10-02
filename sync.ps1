# minisolo auto-sync: changes detect → commit (human-style message) → push
$repo = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $repo

$messages = @(
  "fix: small bug in render loop",
  "feat: tweak terrain generation",
  "refactor: cleanup block placement logic",
  "chore: update world config",
  "fix: correct chunk loading",
  "feat: improve first-person controls",
  "perf: reduce mesh rebuilds",
  "style: format code",
  "docs: update README",
  "feat: add new block textures",
  "fix: player spawn position",
  "chore: tweak settings",
  "feat: day-night cycle polish",
  "fix: inventory UI glitch",
  "refactor: world gen module"
)

Write-Host "minisolo auto-sync chal raha hai... (Ctrl+C se band)"

while ($true) {
  $status = git status --porcelain
  if ($status) {
    git add -A
    $msg = $messages | Get-Random
    git commit -m $msg | Out-Null
    git push origin main | Out-Null
    $time = Get-Date -Format "HH:mm:ss"
    Write-Host "[$time] commit+push: $msg"
  }
  # 45 se 180 second ke beech random gap (human jaisa feel)
  $wait = Get-Random -Minimum 45 -Maximum 180
  Start-Sleep -Seconds $wait
}
