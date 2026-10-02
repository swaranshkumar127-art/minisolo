# TERRAVOX UE5 - quick launch (game mode)
$UE = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$Proj = 'C:\Users\admin\Documents\MIIIIIINNEECRAFFTUU\TerraVoxUE5.uproject'
Get-Process -Name UnrealEditor -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2
Start-Process -FilePath $UE -ArgumentList "-project=$Proj",'-game','-ResX=1280','-ResY=720','-windowed','-NoSplash','-log','-d3d11','-unattended' -WorkingDirectory 'C:\Users\admin\Documents\MIIIIIINNEECRAFFTUU' -WindowStyle Minimized
Write-Host 'TERRAVOX UE5 launching...'
