# Tools for the SantaGram Servers

## Mobile Analytics Server
  - analytics\_exfil.py: Given a SQL command, will use the post-authentication vulnerability to execute it and return the result as JSON.
  - analytics\_cookie.php: Creates a cookie which authenticates you as the administrator account.
  
## Exception Handler Server:
  - ex\_php\_command.py: Prompts for a PHP command, and will run it on the server via command injection.
  
## Dungeon Game:
  - cdungeon-decode.c: Decoder for the dtextc.dat Dungeon file. Courtesy Ian Lance Taylor. From http://web.mit.edu/jhawk/src/cdungeon-decode.c
  - dungeon\_parse\_dump.py: Parser for the cdungeon-decode ASCII output, to generate a dot file.
  - dungeon.dot: The resulting dot file. Couldn't get this to render very well.
  - dungeon\_failed\_map.png: The best map which resulted from trying to render the dot file.
  - dungeon\_decode.py: Work in progress Python port of cdungeon-decode.c
  - dungeon\_path\_to\_elf.py: Use dungeon.dot to show the path to the Elf. Requires some unreachable paths to be removed from dungeon.dot first

### Elf Path

    $ python dungeon_path_to_elf.py
    West of House(2) to North of House(3) via "north"
    North of House(3) to Behind House(5) via "east"
    Behind House(5) to Kitchen(6) via "west"
    Kitchen(6) to Living Room(8) via "west"
    Living Room(8) to Cellar(9) via "down"
    Cellar(9) to Troll Room(10) via "east"
    Troll Room(10) to Maze(11) via "south"
    Maze(11) to Maze(14) via "east"
    Maze(14) to Maze(13) via "west"
    Maze(13) to Maze(16) via "up"
    Maze(16) to Maze(18) via "sw"
    Maze(18) to Maze(19) via "east"
    Maze(19) to Maze(30) via "south"
    Maze(30) to Cyclops Room(101) via "ne"
    Cyclops Room(101) to Treasure Room(103) via "up"
    Treasure Room(103) to Small Square Room(188) via "east"
    Small Square Room(188) to Elf Room(192) via "up"
