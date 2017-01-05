#!/usr/bin/env python

import struct

# Low-level functions for parsing the data

def parse_data(f, s):
    """A simple wrapper around struct.unpack"""
    size = struct.calcsize(s)
    return struct.unpack(s, f.read(size))[0]

def read_char(f):
    """Read a single character (unsigned char)"""
    return parse_data(f, 'B')

def read_flags(f, c):
    """Read a number of flags, which each are a char"""
    result = []
    for i in range(c):
        result.append(read_char(f))
    return result

def read_int(f):
    """Read an integer, in a format specific to the game"""
    i = read_char(f)
    if i > 127:
        i = i - 256
    return i * 256 + read_char(f)

def read_ints(f, c):
    """Read a number of integers"""
    result = []
    for i in range(c):
        result.append(read_int(f))
    return result

def read_partial_ints(f, c):
    """This is basically a sparse array of integers"""
    result = [0]*c
    while True:
        if c < 255:
            i = read_char(f)
            if i == 255:
                return result
        else:
            i = read_int(f)
            if i == -1:
                return result
        if i >= c:
            raise IndexError("Bad index")
        result[i] = read_int(f)

def read_string(f, offset, base):
    """Strings are XOR-encoded"""
    key = "IanLanceTaylorJr"
    pos = ((-offset) - 1) * 8
    f.seek(pos + base)

    result = [ord(x) for x in f.read(1024)]
    result.append(0)
    for i in range(1024):
        k = (ord(key[pos & 0xf]) ^ pos) & 0xff
        result[i] = (result[i] ^ k) & 0xff
        pos += 1
        if result[i] == 0:
            break
    trim = result.index(0)
    return ''.join(chr(x) for x in result[:trim])

# Main parsing helper

def parse(f, num=1, is_int=True, is_partial=False, is_flags=False):
    if is_flags:
        # We assume that if it's a flag, that trumps it being an integer
        return read_flags(f, num)

    if is_int:
        if num == 1:
            # Single int
            return read_int(f)
        else:
            # Multiple ints
            if is_partial:
                return read_partial_ints(f, num)
            else:
                return read_ints(f, num)

def parse_general_information(f, data):
    """Parses version numbers, max score, etc."""
    data['version'] = {}
    for i in ['major', 'minor', 'edit']:
        data['version'][i] = parse(f)

    for i in ['max_score', 'stars', 'max_end_score']:
        data[i] = parse(f)

def parse_room_information(f, data):
    """Parses room descriptions, exits, flags, etc."""
    max_room = data['max_room'] = parse(f)

    data['room_desc1'] = parse(f, num=max_room)
    data['room_desc2'] = parse(f, num=max_room)
    data['room_exit'] = parse(f, num=max_room)
    data['room_action'] = parse(f, num=max_room, is_partial=True)
    data['room_val'] = parse(f, num=max_room, is_partial=True)
    data['room_flag'] = parse(f, num=max_room)

    return max_room

def parse_exit_information(f, data):
    """Parses exit mappings"""
    max_exit = data['max_exit'] = parse(f)

    data['exits'] = parse(f, num=max_exit)

    return max_exit

def parse_object_information(f, data):
    """Parses object descriptions, actions, rooms, etc."""
    max_object = data['max_object'] = parse(f)

    data['obj_desc1'] = parse(f, max_object)
    data['obj_desc2'] = parse(f, max_object)
    data['obj_desco'] = parse(f, max_object, is_partial=True)
    data['obj_caction'] = parse(f, max_object, is_partial=True)
    data['obj_flag1'] = parse(f, max_object)
    data['obj_flag2'] = parse(f, max_object, is_partial=True)
    data['obj_fval'] = parse(f, max_object, is_partial=True)
    data['obj_tval'] = parse(f, max_object, is_partial=True)
    data['obj_size'] = parse(f, max_object)
    data['obj_capacity'] = parse(f, max_object, is_partial=True)
    data['obj_room'] = parse(f, max_object)
    data['obj_adventurer'] = parse(f, max_object, is_partial=True)
    data['obj_container'] = parse(f, max_object, is_partial=True)
    data['obj_read'] = parse(f, max_object, is_partial=True)

    return max_object

def parse_double_room_information(f, data):
    """Parses double room information. Whatever that is..."""
    max_room2 = data['max_room2'] = parse(f)

    data['room2_o'] = parse(f, max_room2)
    data['room2_i'] = parse(f, max_room2)

    return max_room2

def parse_clock_information(f, data):
    """Parses clock ticks, actions, etc."""
    max_clock = data['max_clock'] = parse(f)

    data['clock_tick'] = parse(f, max_clock)
    data['clock_action'] = parse(f, max_clock)
    data['clock_flag'] = parse(f, max_clock, is_flags=True)

    return max_clock

def parse_villain_information(f, data):
    """Parses villain probabilities, melee stats, etc."""
    max_villain = data['max_villain'] = parse(f)

    data['villains'] = parse(f, max_villain)
    data['villain_prob'] = parse(f, max_villain, is_partial=True)
    data['villian_opps'] = parse(f, max_villain, is_partial=True)
    data['villain_best'] = parse(f, max_villain)
    data['villain_melee'] = parse(f, max_villain)

    return max_villain

def parse_adventurer_information(f, data):
    """Parses adventurer score, vehicles, flags, etc."""
    max_adventurer = data['max_adventurer'] = parse(f)

    data['adv_room'] = parse(f, max_adventurer)
    data['adv_score'] = parse(f, max_adventurer, is_partial=True)
    data['adv_vehicle'] = parse(f, max_adventurer, is_partial=True)
    data['adv_obj'] = parse(f, max_adventurer)
    data['adv_action'] = parse(f, max_adventurer)
    data['adv_strength'] = parse(f, max_adventurer)
    data['adv_flag'] = parse(f, max_adventurer, is_partial=True)

    return max_adventurer

def parse_message(f, data):
    """Parses messages for rooms, objects, etc."""
    max_message = data['max_message'] = parse(f)
    data['room_text'] = parse(f, max_message)

    message_base = data['message_base'] = f.tell()
    data['room_desc1_parsed'] = []
    data['room_desc2_parsed'] = []
    for i in range(data['max_room']):
        data['room_desc1_parsed'].append(read_string(f, data['room_desc1'][i], message_base))
        data['room_desc2_parsed'].append(read_string(f, data['room_desc2'][i], message_base))

    data['obj_desc1_parsed'] = []
    data['obj_desc2_parsed'] = []
    data['obj_desco_parsed'] = []
    data['obj_read_parsed'] = []
    for i in range(data['max_object']):
        data['obj_desc1_parsed'].append(read_string(f, data['obj_desc1'][i], message_base))
        data['obj_desc2_parsed'].append(read_string(f, data['obj_desc2'][i], message_base))
        data['obj_desco_parsed'].append(read_string(f, data['obj_desco'][i], message_base))
        data['obj_read_parsed'].append(read_string(f, data['obj_read'][i], message_base))

    data['message_parsed'] = []
    for i in range(max_message):
        data['message_parsed'].append(read_string(f, data['room_text'][i], message_base))

    return max_message

def process_exits(data):
    exits = {}
    for i in range(0, data['max_room']):
        exits[i+1] = {}
        k = -1
        while True:
            exit = data['exits'][data['room_exit'][i] + k]
            if exit & 0x7c00:
                dir = exit & 0x7c00
                if dir == 0x400:
                    direction = "North"
                    port = "n"
                elif dir == 0x800:
                    direction = "Northeast"
                    port = "ne"
                elif dir == 0xc00:
                    direction = "East"
                    port = "e"
                elif dir == 0x1000:
                    direction = "Southeast"
                    port = "se"
                elif dir == 0x1400:
                    direction = "South"
                    port = "s"
                elif dir == 0x1800:
                    direction = "Southwest"
                    port = "sw"
                elif dir == 0x1c00:
                    direction = "West"
                    port = "w"
                elif dir == 0x2000:
                    direction = "Northwest"
                    port = "nw"
                else:
                    port = None
                # if dir == 0x2400:
                #     direction = "Up"
                # if dir == 0x2800:
                #     direction = "Down"
                # if dir == 0x2c00:
                #     direction = "Launch"
                # if dir == 0x3000:
                #     direction = "Land"
                # if dir == 0x3400:
                #     direction = "Enter"
                # if dir == 0x3800:
                #     direction = "Exit"
                # if dir == 0x3c00:
                #     direction = "Cross"
            type = exit & 0x300
            if type:
                # Next word is a string
                k += 1
            if type > 1:
                # And then, the next word is a pair of bytes
                k += 1
            #if type == 2:
            #    print "Failure flag"
            #if type == 3:
            #    print "Failure door"
            if exit & 0xff and type != 1 and port:
                destination = exit & 0xff
                exits[i+1][destination] = (port, direction)
            if exit & 0x8000:
                break
            k += 1
    return exits


def print_dotfile(exits, data):
    print "digraph {"
    #print "  splines=false;"
    #print "  rank=same;"
    print "  rankdir=BT;"
    #print "  overlap=false;"
    print "  overlap=prism;"
    for source in exits:
        for dest in exits[source]:
            port, direction = exits[source][dest]
            if dest in exits and source in exits[dest]:
                return_port, return_direction = exits[dest][source]
                print "\"%s (%d)\":%s -> \"%s (%d)\":%s[label=\"%s\"];" % (data['room_desc2_parsed'][source-1], source, port, data['room_desc2_parsed'][dest-1], dest, return_port, direction)
            else:
               print "\"%s (%d)\":%s -> \"%s (%d)\"[label=\"%s\"];" % (data['room_desc2_parsed'][source-1], source, port, data['room_desc2_parsed'][dest-1], dest, direction)
    print "}"





def main():
    f = open('dtextc.dat', 'rb')
    data = {}

    parse_general_information(f, data)
    max_room = parse_room_information(f, data)
    max_exit = parse_exit_information(f, data)
    max_object = parse_object_information(f, data)
    max_room2 = parse_double_room_information(f, data)
    max_clock = parse_clock_information(f, data)
    max_villain = parse_villain_information(f, data)
    max_adventurer = parse_adventurer_information(f, data)

    melee_base = data['melee_base'] = parse(f)

    max_message = parse_message(f, data)
    exits = process_exits(data)
    print_exits(exits, data)

if __name__ == "__main__":
    main()




