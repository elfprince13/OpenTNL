# Microsoft Developer Studio Project File - Name="tnl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=tnl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tnl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tnl.mak" CFG="tnl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tnl - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "tnl - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tnl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GR /GX /O2 /I "../libtomcrypt" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\tnl.lib"

!ELSEIF  "$(CFG)" == "tnl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GR /GX /ZI /Od /I "../libtomcrypt" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "TNL_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\tnld.lib"

!ENDIF 

# Begin Target

# Name "tnl - Win32 Release"
# Name "tnl - Win32 Debug"
# Begin Source File

SOURCE=.\assert.cpp
# End Source File
# Begin Source File

SOURCE=.\assert.h
# End Source File
# Begin Source File

SOURCE=.\asymmetricKey.cpp
# End Source File
# Begin Source File

SOURCE=.\asymmetricKey.h
# End Source File
# Begin Source File

SOURCE=.\bitSet.h
# End Source File
# Begin Source File

SOURCE=.\bitStream.cpp
# End Source File
# Begin Source File

SOURCE=.\bitStream.h
# End Source File
# Begin Source File

SOURCE=.\byteBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\byteBuffer.h
# End Source File
# Begin Source File

SOURCE=.\certificate.cpp
# End Source File
# Begin Source File

SOURCE=.\certificate.h
# End Source File
# Begin Source File

SOURCE=.\clientPuzzle.cpp
# End Source File
# Begin Source File

SOURCE=.\clientPuzzle.h
# End Source File
# Begin Source File

SOURCE=.\connectionStringTable.cpp
# End Source File
# Begin Source File

SOURCE=.\connectionStringTable.h
# End Source File
# Begin Source File

SOURCE=.\dataChunker.cpp
# End Source File
# Begin Source File

SOURCE=.\dataChunker.h
# End Source File
# Begin Source File

SOURCE=.\endian.h
# End Source File
# Begin Source File

SOURCE=.\eventConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\eventConnection.h
# End Source File
# Begin Source File

SOURCE=.\ghostConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\ghostConnection.h
# End Source File
# Begin Source File

SOURCE=.\huffmanStringProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\huffmanStringProcessor.h
# End Source File
# Begin Source File

SOURCE=.\log.cpp
# End Source File
# Begin Source File

SOURCE=.\log.h
# End Source File
# Begin Source File

SOURCE=.\netBase.cpp
# End Source File
# Begin Source File

SOURCE=.\netBase.h
# End Source File
# Begin Source File

SOURCE=.\netConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\netConnection.h
# End Source File
# Begin Source File

SOURCE=.\netEvent.h
# End Source File
# Begin Source File

SOURCE=.\netInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\netInterface.h
# End Source File
# Begin Source File

SOURCE=.\netObject.cpp
# End Source File
# Begin Source File

SOURCE=.\netObject.h
# End Source File
# Begin Source File

SOURCE=.\netStringTable.cpp
# End Source File
# Begin Source File

SOURCE=.\netStringTable.h
# End Source File
# Begin Source File

SOURCE=.\nonce.h
# End Source File
# Begin Source File

SOURCE=.\platform.cpp
# End Source File
# Begin Source File

SOURCE=.\platform.h
# End Source File
# Begin Source File

SOURCE=.\random.cpp
# End Source File
# Begin Source File

SOURCE=.\random.h
# End Source File
# Begin Source File

SOURCE=.\rpc.cpp
# End Source File
# Begin Source File

SOURCE=.\rpc.h
# End Source File
# Begin Source File

SOURCE=.\symmetricCipher.cpp
# End Source File
# Begin Source File

SOURCE=.\symmetricCipher.h
# End Source File
# Begin Source File

SOURCE=.\tnl.h
# End Source File
# Begin Source File

SOURCE=.\types.h
# End Source File
# Begin Source File

SOURCE=.\udp.cpp
# End Source File
# Begin Source File

SOURCE=.\udp.h
# End Source File
# Begin Source File

SOURCE=.\vector.cpp
# End Source File
# Begin Source File

SOURCE=.\vector.h
# End Source File
# End Target
# End Project
