import networkx as nx
g=nx.nx_pydot.read_dot("dungeon.dot")
path = nx.shortest_path(g, source='West of House(2)', target='Elf Room(192)')

for a,b in zip(path, path[1:]):
    print a, 'to', b, 'via', g[a][b][0]['label']
