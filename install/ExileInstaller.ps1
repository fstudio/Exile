<##
#
##>
param(
    [Switch]$Force=$false,
    [Switch]$Reinstall=$false
)

$ExileAppData=$env:APPDATA+"\Exile"

if(!(Test-Path $ExileAppData)){
    mkdir -Path $ExileAppData
}