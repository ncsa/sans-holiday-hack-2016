# Tools for the CranPi Terminals

## General
  - tty.py: Drops you into an interactive session on one of the terminals.  Doesn't properly unbuffer the terminal or turn off echo, but it works well enough to poke around.
  - tty\_oneshot.py: Runs a command on one of the terminals. Note that this won't work on the Train consoles, which require a user-specific QUEST_UID.
  
## Wumpus
  - srand.c: Used with LD\_PRELOAD to fix the random seed so that the Wumpus game is always the same.  Compile with ```gcc -o srand.so -ldl -shared -fPIC srand.c```
