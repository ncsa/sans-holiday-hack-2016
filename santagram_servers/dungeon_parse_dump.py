from __future__ import print_function
from collections import namedtuple
import os

Room = namedtuple("Room", "id name exits")
Exit = namedtuple('Exit', "direction flag id name")

DIRECTIONS = [
    "north", "ne", "east", "se", "south", "sw", "west", "nw", "up",
    "down", "launch", "land", "enter", "exit", "cross"
]

DIRECTION_LINE_PREFIXES = tuple(' {}:'.format(d) for d in DIRECTIONS)

def parse_exit_line(line):
    """>>> parse_exit_line('south: 0 191 (North Pole)')
       Exit(direction='south', room='191')
    """
    name = None
    direction, info = line.lstrip().split(": ", 1)
    parts = info.split(None, 2)
    if len(parts) == 3:
        flag, room_id, name = parts
        name = name.strip("()")
    else:
        flag, room_id = parts
    return Exit(direction, flag, room_id, name)

def get_data():
    with os.popen("./cdungeon-decode -b dtextc.dat -a") as f:
        blocks = f.read().split("\n\n")
        return [b.splitlines() for b in blocks]

def parse(data):
    rooms = (b for b in data if b[0].startswith("Room:"))
    for block in rooms:
        room_info = block[0].split("Room: ")[1]
        room_id, room_name = room_info.split(": ", 1)
        room_name = room_name.strip("()")
        exits = []
        yield Room(room_id, room_name, exits)
        for line in block[1:]:
            if line.startswith(DIRECTION_LINE_PREFIXES):
                exits.append(parse_exit_line(line))

def main():
    data = list(parse(get_data()))
    out = []
    out.append("digraph {")
    for r in data:
        for e in r.exits:
            if e.name != None:
                out.append('"{}({})" -> "{}({})"[label="{}"];'.format(r.name, r.id, e.name, e.id, e.direction))
        print(out[-1])
    out.append("}\n")

    with open("dungeon.dot", 'w') as f:
        f.write("\n".join(out))

if __name__ == "__main__":
    main()
