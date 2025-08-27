; filepath: d:\Projects\LED\Alex\WiFi_AHT21\PC\HTview\HT_view\installer.nsi
!define APP_NAME "HT View"
!define APP_VERSION "1.0"
!define APP_PUBLISHER "Temperature & Humidity Monitor"
!define APP_EXE "HT_view.exe"
!define APP_DESCRIPTION "Temperature and Humidity Data Viewer"

!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "x64.nsh"

; Подключить функцию для подсчета размера
!insertmacro GetSize

Name "${APP_NAME} ${APP_VERSION}"
OutFile "HT_View_Setup.exe"

; 64-битная установка
!ifdef NSIS_WIN32_MAKENSIS
  !define INSTALL_DIR_32 "$PROGRAMFILES32\${APP_NAME}"
  !define INSTALL_DIR_64 "$PROGRAMFILES64\${APP_NAME}"
!else
  !define INSTALL_DIR_32 "$PROGRAMFILES\${APP_NAME}"
  !define INSTALL_DIR_64 "$PROGRAMFILES\${APP_NAME}"
!endif

InstallDir "${INSTALL_DIR_64}"
RequestExecutionLevel admin

; Современный интерфейс
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis3-metro.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\nsis3-metro.bmp"

!define MUI_WELCOMEPAGE_TITLE "Welcome to ${APP_NAME} Setup"
!define MUI_WELCOMEPAGE_TEXT "${APP_DESCRIPTION}$\r$\n$\r$\nThis application allows you to view and analyze temperature and humidity data from CSV files with advanced charting capabilities.$\r$\n$\r$\nClick Next to continue."

; Настройка финальной страницы
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${APP_NAME}"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.md"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show User Guide"
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED

; Страницы установки
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Страницы удаления
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; Функция инициализации
Function .onInit
  ${If} ${RunningX64}
    SetRegView 64
    StrCpy $INSTDIR "${INSTALL_DIR_64}"
  ${Else}
    MessageBox MB_YESNO|MB_ICONQUESTION "This is a 64-bit application. It may not work correctly on 32-bit Windows.$\r$\n$\r$\nDo you want to continue anyway?" IDYES continue
    Abort
    continue:
    StrCpy $INSTDIR "${INSTALL_DIR_32}"
  ${EndIf}
FunctionEnd

; Основная секция установки
Section "Main Application" SecMain
    SetOutPath "$INSTDIR"
    
    ; Основные файлы
    File "deploy\HT_view.exe"
    File "deploy\*.dll"
    File "deploy\qt.conf"
    
    ; Документация и скриншоты
    File "README.md"
    File /nonfatal "screenshot.png"
    File "deploy\config.ini"
    
    ; Плагины платформы (обязательные)
    SetOutPath "$INSTDIR\platforms"
    File "deploy\platforms\*.*"
    
    ; Стили Windows (если есть)
    IfFileExists "deploy\styles\*.*" 0 +3
    SetOutPath "$INSTDIR\styles"
    File "deploy\styles\*.*"
    
    ; Форматы изображений (если есть)
    IfFileExists "deploy\imageformats\*.*" 0 +3
    SetOutPath "$INSTDIR\imageformats"
    File "deploy\imageformats\*.*"
    
    ; Иконки (если есть)
    IfFileExists "deploy\iconengines\*.*" 0 +3
    SetOutPath "$INSTDIR\iconengines"
    File "deploy\iconengines\*.*"
    
    ; Создать ярлыки
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\User Guide.lnk" "$INSTDIR\README.md" "" "" 0
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"
    CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_EXE}" 0
    
    ; Ассоциация с CSV файлами
    WriteRegStr HKLM "SOFTWARE\Classes\.csv\OpenWithList\${APP_EXE}" "" ""
    WriteRegStr HKLM "SOFTWARE\Classes\Applications\${APP_EXE}\SupportedTypes" ".csv" ""
    WriteRegStr HKLM "SOFTWARE\Classes\Applications\${APP_EXE}" "FriendlyAppName" "${APP_NAME}"
    WriteRegStr HKLM "SOFTWARE\Classes\Applications\${APP_EXE}\shell\open\command" "" '"$INSTDIR\${APP_EXE}" "%1"'
    
    ; Регистрация в Add/Remove Programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME} (64-bit)"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" "${APP_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayIcon" "$INSTDIR\${APP_EXE}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "HelpLink" "$INSTDIR\README.md"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoRepair" 1
    
    ; Вычислить размер установки
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "EstimatedSize" "$0"
    
    WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd

Function un.onInit
  ${If} ${RunningX64}
    SetRegView 64
  ${EndIf}
FunctionEnd

Section "Uninstall"
    Delete "$INSTDIR\${APP_EXE}"
    Delete "$INSTDIR\*.dll"
    Delete "$INSTDIR\qt.conf"
    Delete "$INSTDIR\README.md"
    Delete "$INSTDIR\config.ini"
    Delete "$INSTDIR\uninstall.exe"
    
    RMDir /r "$INSTDIR\platforms"
    RMDir /r "$INSTDIR\styles"
    RMDir /r "$INSTDIR\imageformats"
    RMDir /r "$INSTDIR\iconengines"
    RMDir "$INSTDIR"
    
    Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\User Guide.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"
    RMDir "$SMPROGRAMS\${APP_NAME}"
    Delete "$DESKTOP\${APP_NAME}.lnk"
    
    DeleteRegKey HKLM "SOFTWARE\Classes\Applications\${APP_EXE}"
    DeleteRegValue HKLM "SOFTWARE\Classes\.csv\OpenWithList\${APP_EXE}" ""
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
SectionEnd