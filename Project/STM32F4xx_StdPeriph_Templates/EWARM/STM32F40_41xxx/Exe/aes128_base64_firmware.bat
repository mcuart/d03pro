cd /d %~dp0 
for %%f in (*.bin) do (
set bin_file_name=%%~nf.bin
)
::aes128_base64_firmware.exe d06pro_stm8_v1.0.bin
aes128_base64_firmware.exe %bin_file_name%

::自动上传脚本
::auto_sftp.bat