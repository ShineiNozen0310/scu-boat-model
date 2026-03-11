param(
    [string]$GccExe = "C:\msys64\ucrt64\bin\gcc.exe"
)

if (-not (Test-Path $GccExe)) {
    throw "MSYS2 gcc not found: $GccExe"
}

$msysBin = Split-Path -Parent $GccExe
$arguments = @(
    '-std=c11',
    '-Wall',
    '-Wextra',
    '-Werror',
    '-Ifirmware/app',
    'test/host/crsf_host_selftest.c',
    'firmware/app/main_app.c',
    'firmware/app/control/boat_controller.c',
    'firmware/app/safety/boat_safety.c',
    'firmware/app/drivers/boat_crsf.c',
    'firmware/app/drivers/boat_ir_decoder.c',
    'firmware/app/drivers/ir_capture_queue.c',
    'firmware/app/drivers/boat_radio_protocol.c',
    '-o',
    'test/host/crsf_host_selftest.exe'
)

& $GccExe @arguments

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$env:PATH = "$msysBin;$env:PATH"
& '.\test\host\crsf_host_selftest.exe'
exit $LASTEXITCODE
