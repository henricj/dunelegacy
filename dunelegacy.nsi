SetCompressor /SOLID lzma

!include x64.nsh
!include MUI2.nsh

Name "Dune Legacy"
BrandingText " http://dunelegacy.sourceforge.net"
!define INSTALLATIONNAME "Dune Legacy"
OutFile "Dune Legacy Setup.exe"
InstallDir "$PROGRAMFILES\${INSTALLATIONNAME}"

RequestExecutionLevel admin

!define MUI_ICON "dunelegacy.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "nsis-background1.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "nsis-background1.bmp"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "nsis-background2.bmp"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "COPYING"

!define MUI_DIRECTORYPAGE_VARIABLE $INSTDIR
!insertmacro MUI_PAGE_DIRECTORY

VAR DunePAKDirectory
!define MUI_DIRECTORYPAGE_VARIABLE $DunePAKDirectory
!define MUI_PAGE_HEADER_TEXT $(PAGE_HEADER_TEXT)
!define MUI_PAGE_HEADER_SUBTEXT $(PAGE_HEADER_SUBTEXT)
!define MUI_DIRECTORYPAGE_TEXT_TOP $(DIRECTORYPAGE_TEXT_TOP)
!define MUI_DIRECTORYPAGE_TEXT_DESTINATION $(DIRECTORYPAGE_TEXT_DESTINATION)
!define MUI_PAGE_CUSTOMFUNCTION_PRE OnDunePakDirectorySelectionPre
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE OnDunePakDirectorySelectionLeave
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH


!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "German"

; English
LangString PAGE_HEADER_TEXT ${LANG_ENGLISH} "Dune II Pak-Files"
LangString PAGE_HEADER_SUBTEXT ${LANG_ENGLISH} "Choose the directory where the installer can copy the Dune II Pak-Files from."
LangString DIRECTORYPAGE_TEXT_TOP ${LANG_ENGLISH} "Dune Legacy needs the PAK-Files from original Dune II. These files can be found in the Dune II folder. The installer will copy the following files from there to the Dune Legacy folder:$\n$\tHARK.PAK$\t$\tSCENARIO.PAK$\t$\tINTRO.PAK$\n$\tATRE.PAK$\t$\tMENTAT.PAK$\t$\tINTROVOC.PAK$\n$\tORDOS.PAK$\t$\tVOC.PAK$\t$\tSOUND.PAK$\n$\tENGLISH.PAK$\t$\tMERC.PAK$\t$\tGERMAN.PAK (if available)$\n$\tDUNE.PAK$\t$\tFINALE.PAK$\t$\tFRENCH.PAK (if available)"
LangString DIRECTORYPAGE_TEXT_DESTINATION ${LANG_ENGLISH} "Dune II Pak-Files Directory"
LangString UNINSTALL_NAME ${LANG_ENGLISH} "Uninstall Dune Legacy"

; German
LangString PAGE_HEADER_TEXT ${LANG_GERMAN} "Dune II Pak-Dateien"
LangString PAGE_HEADER_SUBTEXT ${LANG_GERMAN} "Wählen Sie das Verzeichnis aus, von dem der Installer die Dune II Pak-Dateien kopieren kann."
LangString DIRECTORYPAGE_TEXT_TOP ${LANG_GERMAN} "Dune Legacy benötigt die PAK-Dateien des Originalspiels, welche im Dune II Verzeichnis liegen. Die folgenden Dateien werden von dort in das Dune Legacy Verzeichnis kopiert:$\n$\tHARK.PAK$\t$\tSCENARIO.PAK$\t$\tINTRO.PAK$\n$\tATRE.PAK$\t$\tMENTAT.PAK$\t$\tINTROVOC.PAK$\n$\tORDOS.PAK$\t$\tVOC.PAK$\t$\tSOUND.PAK$\n$\tENGLISH.PAK$\t$\tMERC.PAK$\t$\tGERMAN.PAK (falls vorhanden)$\n$\tDUNE.PAK$\t$\tFINALE.PAK$\t$\tFRENCH.PAK (falls vorhanden)"
LangString DIRECTORYPAGE_TEXT_DESTINATION ${LANG_GERMAN} "Verzeichnis mit Dune II Pak-Dateien"
LangString UNINSTALL_NAME ${LANG_GERMAN} "Dune Legacy deinstallieren"

Function .onInit
  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
    SetRegView 64
    StrCpy $INSTDIR "$PROGRAMFILES64\${INSTALLATIONNAME}"
  ${EndIf}
FunctionEnd

Function un.onInit
  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
    SetRegView 64
    StrCpy $INSTDIR "$PROGRAMFILES64\${INSTALLATIONNAME}"
  ${EndIf}
FunctionEnd

Function OnDunePakDirectorySelectionPre
  IfFileExists "$INSTDIR\HARK.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\ATRE.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\ORDOS.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\ENGLISH.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\DUNE.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\SCENARIO.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\MENTAT.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\VOC.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\MERC.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\FINALE.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\INTRO.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\INTROVOC.PAK" 0 FileNotFound
  IfFileExists "$INSTDIR\SOUND.PAK" 0 FileNotFound

  Abort

FileNotFound:

FunctionEnd

Function OnDunePakDirectorySelectionLeave
  IfFileExists "$DunePAKDirectory\HARK.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\ATRE.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\ORDOS.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\ENGLISH.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\DUNE.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\SCENARIO.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\MENTAT.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\VOC.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\MERC.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\FINALE.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\INTRO.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\INTROVOC.PAK" 0 FileNotFound
  IfFileExists "$DunePAKDirectory\SOUND.PAK" 0 FileNotFound

  CreateDirectory "$INSTDIR"

  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\HARK.PAK" "$INSTDIR" 424
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\ATRE.PAK" "$INSTDIR" 402
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\ORDOS.PAK" "$INSTDIR" 452
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\ENGLISH.PAK" "$INSTDIR" 88
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\DUNE.PAK" "$INSTDIR" 399
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\SCENARIO.PAK" "$INSTDIR" 310
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\MENTAT.PAK" "$INSTDIR" 561
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\VOC.PAK" "$INSTDIR" 257
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\MERC.PAK" "$INSTDIR" 45
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\FINALE.PAK" "$INSTDIR" 569
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\INTRO.PAK" "$INSTDIR" 1196
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\INTROVOC.PAK" "$INSTDIR" 953
  CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\SOUND.PAK" "$INSTDIR" 1280

  IfFileExists "$DunePAKDirectory\GERMAN.PAK" 0 +2
    CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\GERMAN.PAK" "$INSTDIR"

  IfFileExists "$DunePAKDirectory\FRENCH.PAK" 0 +2
    CopyFiles /SILENT /FILESONLY "$DunePAKDirectory\FRENCH.PAK" "$INSTDIR"

  Return

  FileNotFound:
    MessageBox MB_OK 'Cannot find the needed PAK-Files in "$DunePAKDirectory"'
    Abort
FunctionEnd


Section ""
  SetOutPath $INSTDIR\maps\singleplayer
  File "data\maps\singleplayer\*.ini"

  SetOutPath $INSTDIR\maps\multiplayer
  File "data\maps\multiplayer\*.ini"

  SetOutPath $INSTDIR\locale
  File "data\locale\*.po"

  SetOutPath $INSTDIR
  ${If} ${RunningX64}
    File build\x64\dunelegacy.exe
    File build\x64\SDL2.dll
    File build\x64\SDL2_mixer.dll
    File build\x64\libogg-0.dll
    File build\x64\libvorbis-0.dll
    File build\x64\libvorbisfile-3.dll
    File build\x64\libmodplug-1.dll
    File build\x64\libFLAC-8.dll
    File build\x64\smpeg2.dll
  ${Else}
    File build\x86\dunelegacy.exe
    File build\x86\SDL2.dll
    File build\x86\SDL2_mixer.dll
    File build\x86\libogg-0.dll
    File build\x86\libvorbis-0.dll
    File build\x86\libvorbisfile-3.dll
    File build\x86\libmodplug-1.dll
    File build\x86\libFLAC-8.dll
    File build\x86\smpeg2.dll
  ${EndIf}
  File data\LEGACY.PAK
  File data\OPENSD2.PAK
  File data\Dune2-Versions.txt

  File COPYING
  Push "$INSTDIR\COPYING"
  Push "$INSTDIR\License.txt"
  Call unix2dos

  File AUTHORS
  Push "$INSTDIR\AUTHORS"
  Push "$INSTDIR\Authors.txt"
  Call unix2dos

  File README
  Push "$INSTDIR\README"
  Push "$INSTDIR\Readme.txt"
  Call unix2dos

  WriteUninstaller $INSTDIR\uninstall.exe
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "DisplayName" "${INSTALLATIONNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "DisplayIcon" '"$INSTDIR\dunelegacy.exe",0'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}" "NoRepair" 1
SectionEnd

Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\${INSTALLATIONNAME}"
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\Dune Legacy.lnk" "$INSTDIR\dunelegacy.exe" "" "$INSTDIR\dunelegacy.exe" 0
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\Readme.lnk" "$INSTDIR\Readme.txt"
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\License.lnk" "$INSTDIR\License.txt"
  
  WriteINIStr "$INSTDIR\Dune Legacy Website.URL" "InternetShortcut" "URL" "http://dunelegacy.sourceforge.net/"
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\Dune Legacy Website.lnk" "$INSTDIR\Dune Legacy Website.URL"
  
  CreateShortCut "$SMPROGRAMS\${INSTALLATIONNAME}\$(UNINSTALL_NAME).lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

Section "Uninstall"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${INSTALLATIONNAME}"
  RMDir /r "$INSTDIR"

  Delete "$SMPROGRAMS\${INSTALLATIONNAME}\*.*"
  RMDir "$SMPROGRAMS\${INSTALLATIONNAME}"
SectionEnd


Function unix2dos
    ; strips all CRs
    ; and then converts all LFs into CRLFs
    ; (this is roughly equivalent to "cat file | dos2unix | unix2dos")
    ;
    ; usage:
    ;    Push "infile"
    ;    Push "outfile"
    ;    Call unix2dos
    ;
    ; beware that this function destroys $0 $1 $2

    ClearErrors

    Pop $2
    FileOpen $1 $2 w 

    Pop $2
    FileOpen $0 $2 r

    Push $2 ; save name for deleting

    IfErrors unix2dos_done

    ; $0 = file input (opened for reading)
    ; $1 = file output (opened for writing)

unix2dos_loop:
    ; read a byte (stored in $2)
    FileReadByte $0 $2
    IfErrors unix2dos_done ; EOL
    ; skip CR
    StrCmp $2 13 unix2dos_loop
    ; if LF write an extra CR
    StrCmp $2 10 unix2dos_cr unix2dos_write

unix2dos_cr:
    FileWriteByte $1 13

unix2dos_write:
    ; write byte
    FileWriteByte $1 $2
    ; read next byte
    Goto unix2dos_loop

unix2dos_done:

    ; close files
    FileClose $0
    FileClose $1

    ; delete original
    Pop $0
    Delete $0

FunctionEnd

