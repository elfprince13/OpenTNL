ZAP 1.3

Introduction
------------

Welcome to ZAP, the retro multiplayer team action game!  Zap is a
game of action and strategy.  In Zap, the goal of the game varies
from level to level, from the following game types:

Capture the Flag - Team game where the objective is to take the
enemy's flag and return it to your flag.  Each capture earns your
team one point.  Take care to defend your flag from the enemy -- 
you can only score if it is at home!

Soccer - Team game where the objective is to move the white circle
(the ball) into the goal of the opponent's color.

Hunters - Solo game where the objective is to collect flags from
other players and return them to the Nexus for points.  Each player
starts with one flag, and drops it if he or she is zapped.  Scoring
in Hunters is based on how many flags the player is carrying when
touching the open Nexus.  The first flag is worth one point, the
second is worth two, the third 3, and so on.  So the total value
of capturing 5 flags would be 5 + 4 + 3 + 2 + 1 = 15 points.  If
the Nexus is dark, it is closed -- the upper timer in the lower
right corner counts down to when it will next be open.

Zapmatch - Solo game, often of short duration between levels.  Just
zap as many other players as you can!


Ship configuration:
Each ship can be configured with 2 modules and 3 weapons.  Pressing
the loadout select screen allows the player to choose the next loadout
for his or her ship.  This loadout will not become available until the
player either flies over a resupply area (team-color-coded patch), or
respawns (only if there are no resupply areas on the level).

Modules are special powers that can be activated by pressing the appropriate
module activation key.  The modules in Zap, and their function are:

1. Boost - Gives the ship a boost of speed
2. Shield - Creates a defensive barrier around the ship that absorbs shots
3. Repair - Repairs self and nearby teammates that are damaged
4. Sensor - Boosts the screen visible distance of the player
5. Cloak - Turns the ship invisible
6. Engineer - Allows the ship to turn Resource items into base defense objects

The Engineering bay allows the ship to pick up and use the white star
"Resources".  Ships can only carry one resource at a time.  Using
the engineering bay while carrying a resource will bring up a menu
of items that can be built.  All constructed objects must be built
on a wall of the level.


Changes since version 1.2.2:
----------------------------
- Added built-in level editor!
- Added custom ship modules
- Added Hunters game type
- Added many new maps
- Added different weapon types
- Added deployable objects
- Added support for more controllers
- Fixed many bugs

Controls:

w - move up
S - move down
A - move left
D - move right

T - chat to team
G - chat global
V - open quick chat menu
R - record voice chat

C - toggle commander map
SPACE - activate primary module (default = boost)
SHIFT - activate secondary module (default = shield)

TAB - show scores

E - next weapon
Q - select weapon and module loadout

Mouse:

Mouse button 1 - fire weapon
Mouse button 2 - activate secondary module

Dual-Analog Controller:

Left stick - move
Right stick - aim and fire

right trigger - activate primary module (default = boost)
left trigger - activate secondary module (default = shield)

button 1 - cycle to next weapon
button 2 - toggle commander map
button 3 - open quick chat menu
button 4 - select weapon and module loadout 
button 5 - show scores
button 6 - record voice

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
		Saitek P880 Dual Analog - joystick 1
		PS 2 Dual Shock w/USB - joystick 2
		XBox controller - joystick 3
        
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

0...9 - set the active team for item construction
T - construct a ResourceItem at the mouse point
G - construct a Spawn point at the mouse point
B - construct a RepairItem at the mouse point

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