# Microsoft Developer Studio Project File - Name="libtomcrypt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libtomcrypt - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libtomcrypt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libtomcrypt.mak" CFG="libtomcrypt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libtomcrypt - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libtomcrypt - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libtomcrypt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libtomcrypt___Win32_Release"
# PROP BASE Intermediate_Dir "libtomcrypt___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libtomcrypt___Win32_Release"
# PROP Intermediate_Dir "libtomcrypt___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\libtomcrypt.lib"

!ELSEIF  "$(CFG)" == "libtomcrypt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libtomcrypt___Win32_Debug"
# PROP BASE Intermediate_Dir "libtomcrypt___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libtomcrypt___Win32_Debug"
# PROP Intermediate_Dir "libtomcrypt___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\libtomcryptd.lib"

!ENDIF 

# Begin Target

# Name "libtomcrypt - Win32 Release"
# Name "libtomcrypt - Win32 Debug"
# Begin Source File

SOURCE=.\aes.c
# End Source File
# Begin Source File

SOURCE=.\base64.c
# End Source File
# Begin Source File

SOURCE=.\bits.c
# End Source File
# Begin Source File

SOURCE=.\blowfish.c
# End Source File
# Begin Source File

SOURCE=.\cast5.c
# End Source File
# Begin Source File

SOURCE=.\cbc.c
# End Source File
# Begin Source File

SOURCE=.\cfb.c
# End Source File
# Begin Source File

SOURCE=.\crypt.c
# End Source File
# Begin Source File

SOURCE=.\ctr.c
# End Source File
# Begin Source File

SOURCE=.\des.c
# End Source File
# Begin Source File

SOURCE=.\dh.c
# End Source File
# Begin Source File

SOURCE=.\dsa.c
# End Source File
# Begin Source File

SOURCE=.\ecb.c
# End Source File
# Begin Source File

SOURCE=.\ecc.c
# End Source File
# Begin Source File

SOURCE=.\gf.c
# End Source File
# Begin Source File

SOURCE=.\hash.c
# End Source File
# Begin Source File

SOURCE=.\hmac.c
# End Source File
# Begin Source File

SOURCE=.\keyring.c
# End Source File
# Begin Source File

SOURCE=.\md2.c
# End Source File
# Begin Source File

SOURCE=.\md4.c
# End Source File
# Begin Source File

SOURCE=.\md5.c
# End Source File
# Begin Source File

SOURCE=.\mem.c
# End Source File
# Begin Source File

SOURCE=.\mpi.c
# End Source File
# Begin Source File

SOURCE=.\mycrypt.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_argchk.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_cfg.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_cipher.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_custom.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_gf.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_hash.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_kr.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_macros.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_misc.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_pk.h
# End Source File
# Begin Source File

SOURCE=.\mycrypt_prng.h
# End Source File
# Begin Source File

SOURCE=.\noekeon.c
# End Source File
# Begin Source File

SOURCE=.\ofb.c
# End Source File
# Begin Source File

SOURCE=.\omac.c
# End Source File
# Begin Source File

SOURCE=.\packet.c
# End Source File
# Begin Source File

SOURCE=.\prime.c
# End Source File
# Begin Source File

SOURCE=.\rc2.c
# End Source File
# Begin Source File

SOURCE=.\rc4.c
# End Source File
# Begin Source File

SOURCE=.\rc5.c
# End Source File
# Begin Source File

SOURCE=.\rc6.c
# End Source File
# Begin Source File

SOURCE=.\rmd128.c
# End Source File
# Begin Source File

SOURCE=.\rmd160.c
# End Source File
# Begin Source File

SOURCE=.\rsa.c
# End Source File
# Begin Source File

SOURCE=".\safer+.c"
# End Source File
# Begin Source File

SOURCE=.\safer.c
# End Source File
# Begin Source File

SOURCE=.\safer_tab.c
# End Source File
# Begin Source File

SOURCE=.\sha1.c
# End Source File
# Begin Source File

SOURCE=.\sha256.c
# End Source File
# Begin Source File

SOURCE=.\sha512.c
# End Source File
# Begin Source File

SOURCE=.\skipjack.c
# End Source File
# Begin Source File

SOURCE=.\sprng.c
# End Source File
# Begin Source File

SOURCE=.\strings.c
# End Source File
# Begin Source File

SOURCE=.\tiger.c
# End Source File
# Begin Source File

SOURCE=.\tommath.h
# End Source File
# Begin Source File

SOURCE=.\twofish.c
# End Source File
# Begin Source File

SOURCE=.\twofish_tab.c
# End Source File
# Begin Source File

SOURCE=.\xtea.c
# End Source File
# Begin Source File

SOURCE=.\yarrow.c
# End Source File
# End Target
# End Project
