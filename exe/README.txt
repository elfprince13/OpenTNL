ZAP 1.2.0

Welcome to ZAP, the retro multiplayer team action game!

Changes since version 1.1.2:
- Added shields, boost and ship energy
- Added Soccer game type
- Added level cycling and victory conditions
- Added deathmatch mini level between team levels
- Removed player particle trails and added motion trails
- Added README
- Added credits on exit
- Added a new CTF level
- Added voice recording and playback (recording on Win32 only for now)
- Added more sounds

Controls:

W - move up
S - move down
A - move left
D - move right

T - chat to team
G - chat global
V - open quick chat menu
R - record voice chat

C - toggle commander map
F - toggle shields
SPACE - boost

TAB - show scores

Mouse:

Aim and fire with mouse button 1

Dual-Analog Controller:

Left stick - move
Right stick - aim and fire

left trigger - shield
right trigger - boost

button 1 - record voice chat
button 3 - toggle commander map
button 4 - show scores

In the game you can press ESC to go to the game options screen.  
From there you can access the main options which include setting 
full screen mode, enabling relative controls and on the Windows 
platform enabling dual analog controller support.

Command Line Options:

Note - zap addresses are of the form transport:address:port like:
IP:127.0.0.1:28000
or IP:Any:28000
or IP:www.foobar.com:24601

-server [bindAddress] hosts a game server/client on the specified 
        bind address.
-connect [connectAddress] starts as a game client and attempts 
        to connect to the server at the specified address.
-master [masterAddress] specfies the address of the master server 
        to connect to.
-dedicated [bindAddress] starts Zap as a dedicated server
-name [playerName] sets the client's name to the specified name 
        and skips the name entry screen.
-levels ["level1 level2 level3 ... leveln"] sets the specified level 
		rotation for games
-hostname [hostname] sets the name that will appear in the server 
        browser when searching for servers.
-maxplayers [number] sets the maximum number of players allowed 
        on the server
-joystick [joystickType] enables dual analog control pad.  The
        joystickType argument can be either 0 or 1.  If the right
        stick doesn't aim shots properly with 0, try 1.

Credits:

ZAP is free software provided by GarageGames.com, Inc.
Team members are:
 
    Mark Frohnmayer
    Ben Garney
    John Quigley
    Robert Blanchett