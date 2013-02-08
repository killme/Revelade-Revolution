Revelade Revolution

Change history:
======================================================================
1.0.6 (2/3/2013):
-Features/additions:
    Added new server XRRS (eXtensible Revelade Revolution Server) based on XSBS.

-Fixes:
    Fixed invisible fire bug.
    Fixed small survival bug that crashed the game when starting a local match.
    Fixed bug that caused players to get full ammo when picking up mortar ammo.
    Fixed bug where zombies in survival are not taking or giving any damage.
    Fixed bug where infected players can pick up mortar in infection mode.

-Other/tweaks:
    /reconnect will work after game restart.
    Radio voice playback set to load sound from the same directory (male/female) as the speaker.
======================================================================
1.0.5 and older (newest first):
-Features/additions:
    Added controller support.
    Added auto update tool which is used by the game to install updates.
    Added third optional parameter to the "download" command which specifies the filename and one corresponding parameters to "newver" and "newpatch" commands.
    Version, build number, and date are displayed in the lower left corner of the main menu.
    Teammate shooters (not necessarily team killers) can be killed by anyone without a penalty.
    When in death cam mode you can now rotate between players by using the left and right mouse buttons (attack commands).
    When "data/botnames.cfg" is not available random names are generated and given to bots.
    Coop survival is now a death-match mode. When everyone is dead or round time is up (1.5 minutes) the winner (player with the most frags) is announced and all players are spawned. Until then the player is in chase camera mode.
    Coop survival is now overtime (normal time X 1.5).
    Added buying to coop survival (when you kill a zombie you get an amout of guts that varies depending on the zombie type (frequency*health/20) (average = 60)). When a player buys an item the name of that item appears above its head.
    Added time limit server var "timelimit <i>" (default = 10) which controls the game length (in minutes). In team games (overtime, default = 15) this value is multiplied by 1.5.
    Added server var "allowedweaps <i>", if set to 0 both primary and secondary fire modes are allowed, if set to 1 only primary is allowed, and for 2 only secondary is allowed. This can allow instagib servers to prevent players from using one of the crossbow's fire modes.
    Added auto update in-game download.
    Added server var "flaglimit" which controls the number of flags to win in CTF modes.
    Added the new blips to radar, but they disappear when they reach the edge (no fade, which means slightly better performance).
    New explosion effects (can be toggled via "newexplosion").
    Added radar for all modes except coop edit (toggled via "showradar"), and now in addition to flags it shows teammate positions (in blue), ammo positions (in grey), health positions (in green), and armor positions (in yellow). Radar texture transparency can now be controlled via "radartexalpha" (default = 0.5).
    Added IRC chat to the game (copied from Red Eclipse).
    Added "voice" option under "customize" for switching between "male" and "female" voices.
    Added Mortar weapon.

-Fixes:
    Fixed bug that caused the hud gun to disappear.
    Fixed bug where bots would appear even when "include bots" is not checked.
    Fixed bug where players would keep burning until death.
    Fixed several buffer overflows that happen sometimes when downloading or updating from master.
    Fixed issue with libcurl linking against "msvcr100.dll".
    Fixed teleport bug.
    Mortar pickup model points to "ammo/mortar".
    Fixed bug where particles would appear incorrectly.
    Fixed 2 duplicate radio commands.
    Improved damage calculation:
        Fixed ray weapon damage (now slugshot does damage correctly).
        Fixed explosion radial damage application.
    Fixed "taunt" issue where "thirdperson" was set to 2.
    Fixed getmap and getdemo.
    Fixed the bug where the default class is set to -1.
    Fixed bug which crashes the game when bots are added (my bad, I forgot to add the Mortar's model to the vwep list).
    Fixed DMSP lag, which was caused by dead monsters never being removed.
    Fixed infection mode zombie icons.
    Fixed AI for coop survival.

-Other/tweaks:
    Removed support for "version.txt".
    New options for installtion will now appear when after downloading an update.
    Added "download and install with external tool" option to "new version/patch" dialog.
    Increased Medic speed (90 -> 100).
    Swapped Offense's Pistol with Defense's Grenadier.
    Changed Stealth's armour type (green -> blue (weeker)) and reduced armour (40 -> 25).
    Bumped protocol version (262 -> 263).
    When guts change the amount earned or lost is diaplayed above the amount of guts available for ~2.5 secs.
    Moved clock and fps counter on HUD up to make room for the above.
    Teams are now assigned correctly in coop survival.
    Teamkillers must pay a penalty of 700 guts (guts can become negative) and are not allowed to spawn the next time they die.
    The game prints "Revelade Revolution [version_string] - [version_date]" at launch.
    "Round over" and "round draw" will now play from "data/sounds/voice/round_over.ogg" and "data/sounds/voice/round_draw.ogg".
    When downloading a file the game will now check the header received from the server for the file name. If there is one the file is renamed to its proper name once the download is over.
    Bumped protocol version (261 -> 262).
    Tweaked trail particle effect of several weapon, and readded trail for arrows.
    Several coop survival changes.
    Moved fire damage calculation and application to server.
    Reduced health of all zombies in coop survival (including support troopers) to 75%.
    Changed coop survival difficulty level to 1.
    Removed heavy class and gave every other class a third weapon.
    Radar is only displayed in team modes.
    Ranged damage calculation is only done for weapons with ranges higher than or equal to 20.
    Down powered rocket launcher: attack delay (800->1200, 1600->2000), kick (10->15, 10->15), damage (140->110, 170->100), projectile radius (60, 90->60), now normal rocket is more powerful than guided rocket.
    Down powered mortar: damage (300->70, 300->70), offset (3->4, 3->4), projectile/explosion radius (400->200, 400->200), projectile gravity (240->260, 140->160), basically it now does less damage and travels for a shorter distance.
    Tweaked pistol: attack delay (500->250, 1000->500), max ammo (45->60), damage (25->20, 12->10).
    Increased jet pack's attack delay by 40% (180->300).
    Fixed IRC.
    Added IRC chat button to multiplayer menu.
    Various particle and weapon effect changes.
    Reduced team HUD size.
    Menus disappear correctly after a match starts.
    Brought motion blur back to life.
    Increased max team name length to 10.
    Changed team names to "survivor" and "scavenger".
    Flame Jet now does damage more frequently at the time of the hit than at the end.
    Flame Jet damage reduced to 10 (50%).
    Added PT_SPIN, PT_SPIN_FAST, PT_EXPAND, PT_EXPAND_FAST, PT_CONTRACT, PT_CONTRACT_FAST, PT_GROW, PT_SHRINK, and PT_PULSE new particle types.
    Flamejet flame now uses PT_SPIN and PT_EXPAND.
    The grenadier projectile is now a bouncer again.
    Coop survival now plays single player music (zombie ambience).
    Brought back hold mode.
    Lowered slughshot secondary fire damage to 60%.
    Improved infection mode AI.
    Added secondary fire support for bots.
======================================================================
