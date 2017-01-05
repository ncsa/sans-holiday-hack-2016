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
