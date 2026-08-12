# PowerShell wrapper for PMDoT CLI
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
python "$scriptDir\pmdot.py" @args
