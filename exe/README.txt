ZAP 1.3

Welcome to ZAP, the retro multiplayer team action game!

Changes since version 1.2.2:
- Added built-in level editor!

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
button 6 - open quick chat menu

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
        joystickType argument can be either 0, 1 or 2.  If the right
        stick doesn't aim shots properly with 0, try 1 or 2.
        
        Known controllers:
		Logitech Wingman cordless - joystick 0
		PS 2 Dual Shock w/USB - joystick 2
        
-jsave [journalName] saves the log of the play session to the specified
        journal file.
-jplay [journalName] replays a saved journal.
-edit [levelName] starts Zap in level editing mode, loading and saving the
		specified level.
		
Level editor instructions:

Currently the level editor allows you to edit the barrier objects within
levels, and will display spawn points and some mission objects.

Mouse functions:

Left-click - select and move.  Clicking on vertices allows movement
		of verts, clicking on edges allows movement of the entire barrier border.
		To move a vertex or border, click and hold as you drag the object around.
		Left click also completes a new barrier border.
		If no object is under the mouse, left-clicking will create a 
		drag selection box for selecting multiple objects.  Holding down
		the shift key also allows multiple selection of objects.

Right-click - add barrier vertex.  If right clicking on an existing barrier edge,
		this will insert a new vertex along that edge at the click point.  Otherwise
		this either begins a new barrier border or adds a new vertex to the current
		new barrier border.

Keyboard functions:

W - scroll map up
S - scroll map down
A - scroll map left
D - scroll map right
C - zoom out
E - zoom in
R - reset view to 0,0 in the top left corner
F - flip current selection horizontally
V - flip current selection vertically
CTRL-D - duplicate current selection
CTRL-Z - undo last operation

ESC - bring up editor menu

Credits:

ZAP is free software provided by GarageGames.com, Inc.
Team members are:
 
    Mark Frohnmayer
    Ben Garney
    John Quigley
    Robert Blanchett