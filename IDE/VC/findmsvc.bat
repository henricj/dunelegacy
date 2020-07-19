@if exist "%VS150COMNTOOLS%VsDevCmd.bat" goto skipwhere

@if not defined VSWHWERE set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

@if not exist %VSWHERE% set VSWHERE="%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

@if not exist %VSWHERE% goto errorexit

@for /f "usebackq tokens=*" %%i in (`%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do @(
  @set VS150COMNTOOLS=%%i\Common7\Tools\
)

@if not exist "%VS150COMNTOOLS%VsDevCmd.bat" goto errorexit

:skipwhere

@call "%VS150COMNTOOLS%VsDevCmd.bat" %*

@exit /b 0

:errorexit
@echo ***** Can't find MSVC ***** %0 %*
@exit /b 1
