# SANS 2016 Holiday Hack Challenge

This repository contains various tools that we've written for the 2016 SANS Holiday Hack Challenge.

Our full report is at: https://ncsa.github.io/sans-holiday-hack-2016/

## Tools for the SantaGram Servers

### Mobile Analytics Server
  - analytics\_exfil.py: Given a SQL command, will use the post-authentication vulnerability to execute it and return the result as JSON.
  - analytics\_cookie.php: Creates a cookie which authenticates you as the administrator account.
  
### Exception Handler Server:
  - ex\_php\_command.py: Prompts for a PHP command, and will run it on the server via command injection.
  
### Dungeon Game:
  - cdungeon-decode.c: Decoder for the dtextc.dat Dungeon file. Courtesy Ian Lance Taylor. From http://web.mit.edu/jhawk/src/cdungeon-decode.c
  - dungeon\_parse\_dump.py: Parser for the cdungeon-decode ASCII output, to generate a dot file.
  - dungeon.dot: The resulting dot file. Couldn't get this to render very well.
  - dungeon\_failed\_map.png: The best map which resulted from trying to render the dot file.
  - dungeon\_decode.py: Work in progress Python port of cdungeon-decode.c

## Tools for the CranPi Terminals

### General
  - tty\_oneshot.py: Runs a command on one of the terminals. Note that this won't work on the Train consoles, which require a user-specific QUEST\_UID.
  
### Wumpus
  - srand.c: Used with LD\_PRELOAD to fix the random seed so that the Wumpus game is always the same.

## Miscellaneous Tools:
  - part1\_twitter.py: A script to download all of Santa's tweets. Note that you'll need a settings file with your own Twitter API access information.
