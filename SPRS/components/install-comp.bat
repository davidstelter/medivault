SET fire="C:\Program Files\Mozilla Firefox"
SET comp="C:\Program Files\Mozilla Firefox\components\"
echo %comp%
REM SET SDKBIN=../../../gecko-sdk/bin

copy xpcom_wrapper_interface.xpt %comp% 
copy Debug\xpcom_wrapper_interface.dll  %comp%
..\..\..\gecko-sdk\bin\regxpcom.exe -x %comp% 
