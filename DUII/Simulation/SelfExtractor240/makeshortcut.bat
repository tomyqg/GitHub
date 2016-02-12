IF EXIST "%SYSTEMDRIVE%\Program Files (x86)" (
IF NOT EXIST "%SYSTEMDRIVE%\Program Files (x86)\Track Systems\TraqDash Simulator" (
mkdir "%SYSTEMDRIVE%\Program Files (x86)\Track Systems\TraqDash Simulator"
)
xcopy "%SYSTEMDRIVE%\Program Files\Track Systems\TraqDash Simulator\*.*" "%SYSTEMDRIVE%\Program Files (x86)\Track Systems\TraqDash Simulator" /S /I /Y /Q
copy "%SYSTEMDRIVE%\Program Files (x86)\Track Systems\TraqDash Simulator\TraqDashPC.lnk" "%USERPROFILE%\Desktop\"
chdir "%SYSTEMDRIVE%\Program Files (x86)\Track Systems\TraqDash Simulator\"
) ELSE (
chdir "%USERPROFILE%\Desktop\"
copy "%SYSTEMDRIVE%\Program Files\Track Systems\TraqDash Simulator\TraqDashPC.lnk" .
chdir "%SYSTEMDRIVE%\Program Files\Track Systems\TraqDash Simulator\"
)
start .\TraqDashPC.exe
