# Microsoft Developer Studio Project File - Name="zap" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=zap - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zap.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zap.mak" CFG="zap - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zap - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "zap - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zap - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "zap___Win32_Release"
# PROP BASE Intermediate_Dir "zap___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "zap___Win32_Release"
# PROP Intermediate_Dir "zap___Win32_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "../tnl" /I "../glut" /I "../openal" /I "c:/dxsdk/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 dsound.lib dinput.lib dinput8.lib dxguid.lib ../openal/alut.lib ../openal/openal32.lib wsock32.lib opengl32.lib glu32.lib ../glut/glut32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"../exe/zap.exe" /libpath:"c:/dxsdk/lib"

!ELSEIF  "$(CFG)" == "zap - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "zap___Win32_Debug"
# PROP BASE Intermediate_Dir "zap___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "zap___Win32_Debug"
# PROP Intermediate_Dir "zap___Win32_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "../tnl" /I "../glut" /I "../openal" /I "c:/dxsdk/include" /D "TNL_DEBUG" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dsound.lib dinput.lib dinput8.lib dxguid.lib ../openal/alut.lib ../openal/openal32.lib wsock32.lib opengl32.lib glu32.lib ../glut/glut32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../exe/zapd.exe" /pdbtype:sept /libpath:"c:/dxsdk/lib"

!ENDIF 

# Begin Target

# Name "zap - Win32 Release"
# Name "zap - Win32 Debug"
# Begin Source File

SOURCE=.\barrier.cpp
# End Source File
# Begin Source File

SOURCE=.\barrier.h
# End Source File
# Begin Source File

SOURCE=.\collision.h
# End Source File
# Begin Source File

SOURCE=.\CTFGame.cpp
# End Source File
# Begin Source File

SOURCE=.\CTFGame.h
# End Source File
# Begin Source File

SOURCE=.\game.cpp
# End Source File
# Begin Source File

SOURCE=.\game.h
# End Source File
# Begin Source File

SOURCE=.\gameConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\gameConnection.h
# End Source File
# Begin Source File

SOURCE=.\gameLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\gameLoader.h
# End Source File
# Begin Source File

SOURCE=.\gameNetInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\gameNetInterface.h
# End Source File
# Begin Source File

SOURCE=.\gameObject.cpp
# End Source File
# Begin Source File

SOURCE=.\gameObject.h
# End Source File
# Begin Source File

SOURCE=.\gameType.cpp
# End Source File
# Begin Source File

SOURCE=.\gameType.h
# End Source File
# Begin Source File

SOURCE=.\gridDB.cpp
# End Source File
# Begin Source File

SOURCE=.\gridDB.h
# End Source File
# Begin Source File

SOURCE=.\gsm.h
# End Source File
# Begin Source File

SOURCE=.\gsm_decode.c
# End Source File
# Begin Source File

SOURCE=.\gsm_encode.c
# End Source File
# Begin Source File

SOURCE=.\gsm_state.c
# End Source File
# Begin Source File

SOURCE=.\item.cpp
# End Source File
# Begin Source File

SOURCE=.\item.h
# End Source File
# Begin Source File

SOURCE=.\lpc10.h
# End Source File
# Begin Source File

SOURCE=.\lpc10dec.c
# End Source File
# Begin Source File

SOURCE=.\lpc10enc.c
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\masterConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\masterConnection.h
# End Source File
# Begin Source File

SOURCE=..\master\masterInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\moveObject.cpp
# End Source File
# Begin Source File

SOURCE=.\moveObject.h
# End Source File
# Begin Source File

SOURCE=.\point.h
# End Source File
# Begin Source File

SOURCE=.\projectile.cpp
# End Source File
# Begin Source File

SOURCE=.\projectile.h
# End Source File
# Begin Source File

SOURCE=.\quickChat.cpp
# End Source File
# Begin Source File

SOURCE=.\quickChat.h
# End Source File
# Begin Source File

SOURCE=.\sfx.cpp
# End Source File
# Begin Source File

SOURCE=.\sfx.h
# End Source File
# Begin Source File

SOURCE=.\ship.cpp
# End Source File
# Begin Source File

SOURCE=.\ship.h
# End Source File
# Begin Source File

SOURCE=.\sparkManager.cpp
# End Source File
# Begin Source File

SOURCE=.\sparkManager.h
# End Source File
# Begin Source File

SOURCE=.\SweptEllipsoid.cpp
# End Source File
# Begin Source File

SOURCE=.\SweptEllipsoid.h
# End Source File
# Begin Source File

SOURCE=.\teleporter.cpp
# End Source File
# Begin Source File

SOURCE=.\teleporter.h
# End Source File
# Begin Source File

SOURCE=.\UI.cpp
# End Source File
# Begin Source File

SOURCE=.\UI.h
# End Source File
# Begin Source File

SOURCE=.\UICredits.cpp
# End Source File
# Begin Source File

SOURCE=.\UiCredits.h
# End Source File
# Begin Source File

SOURCE=.\UIGame.cpp
# End Source File
# Begin Source File

SOURCE=.\UIGame.h
# End Source File
# Begin Source File

SOURCE=.\UIMenus.cpp
# End Source File
# Begin Source File

SOURCE=.\UIMenus.h
# End Source File
# Begin Source File

SOURCE=.\UINameEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\UINameEntry.h
# End Source File
# Begin Source File

SOURCE=.\UIQueryServers.cpp
# End Source File
# Begin Source File

SOURCE=.\UIQueryServers.h
# End Source File
# Begin Source File

SOURCE=.\voiceCodec.cpp
# End Source File
# Begin Source File

SOURCE=.\voiceCodec.h
# End Source File
# Begin Source File

SOURCE=.\winJoystick.cpp
# End Source File
# End Target
# End Project
