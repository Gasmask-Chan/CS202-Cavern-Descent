import os, glob, re

SOLID_TILES = {1, 2, 3, 5, 6, 7, 8}
PLATFORMS = {4}

def parse_arrays(arr_str):
    nums = re.findall(r'\d+', arr_str)
    nums = [int(n) for n in nums]
    grids = []
    idx = 0
    while idx < len(nums):
        grid = []
        for y in range(10):
            row = []
            for x in range(10):
                if idx < len(nums):
                    row.append(nums[idx])
                    idx += 1
                else:
                    row.append(0)
            grid.append(row)
        grids.append(grid)
    return grids

def grid_to_str(grid):
    res = '    {\n'
    for r in grid:
        res += '        {' + ', '.join(f'{n:2}' for n in r) + '},\n'
    res += '    }'
    return res

def process_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Find the terrain array
    room_match = None
    for match in re.finditer(r'static const int (\w+)\[(\d+)\]\[10\]\[10\]\s*=\s*\{(.*?)\n\};', content, re.DOTALL):
        if not match.group(1).endswith('_npc') and not match.group(1).endswith('_npcs') and not match.group(1).endswith('_loot') and not match.group(1).endswith('_mugshots'):
            room_match = match
            break

    # Find the NPC array
    npc_match = None
    for match in re.finditer(r'static const int (\w+)\[(\d+)\]\[10\]\[10\]\s*=\s*\{(.*?)\n\};', content, re.DOTALL):
        if match.group(1).endswith('_npc') or match.group(1).endswith('_npcs'):
            npc_match = match
            break
            
    if not room_match or not npc_match:
        print(f"Skipping {filepath} - arrays not found")
        return False
        
    num_templates = int(room_match.group(2))
    
    room_grids = parse_arrays(room_match.group(3))
    npc_grids = parse_arrays(npc_match.group(3))
    
    if len(room_grids) != num_templates or len(npc_grids) != num_templates:
        print(f"Skipping {filepath} - template count mismatch (Expected: {num_templates}, Room: {len(room_grids)}, NPC: {len(npc_grids)})")
        return False

    for i in range(num_templates):
        rooms = room_grids[i]
        npcs = npc_grids[i]
        
        # 1. Open up space (hạn chế ngõ cụt)
        for y in range(3, 7):
            for x in range(3, 7):
                if rooms[y][x] in SOLID_TILES:
                    rooms[y][x] = 0 # Air
        
        # 2. Fix NPC placements
        for y in range(10):
            for x in range(10):
                npc = npcs[y][x]
                if npc == 0: continue
                
                # If NPC is inside a solid wall, clear it
                if rooms[y][x] in SOLID_TILES:
                    npcs[y][x] = 0
                    continue
                    
                if npc == 1: # Snake - Needs ground
                    if y == 9 or (rooms[y+1][x] not in SOLID_TILES and rooms[y+1][x] not in PLATFORMS):
                        ny = y
                        while ny < 9 and rooms[ny+1][x] not in SOLID_TILES and rooms[ny+1][x] not in PLATFORMS:
                            ny += 1
                        npcs[y][x] = 0
                        if ny < 9 and rooms[ny][x] not in SOLID_TILES:
                            left_solid = (x == 0) or (rooms[ny][x-1] in SOLID_TILES)
                            right_solid = (x == 9) or (rooms[ny][x+1] in SOLID_TILES)
                            if left_solid and right_solid:
                                npcs[ny][x] = 0 # Trapped, delete snake
                            else:
                                npcs[ny][x] = 1
                        
                elif npc == 3: # Spider - Needs ceiling
                    if y == 0 or rooms[y-1][x] not in SOLID_TILES:
                        ny = y
                        while ny > 0 and rooms[ny-1][x] not in SOLID_TILES:
                            ny -= 1
                        npcs[y][x] = 0
                        if ny > 0 and rooms[ny][x] not in SOLID_TILES:
                            npcs[ny][x] = 3

        # 3. Limit to 1 of each NPC type per room maximum
        seen_types = set()
        for y in range(10):
            for x in range(10):
                npc = npcs[y][x]
                if npc > 0:
                    if npc in seen_types:
                        npcs[y][x] = 0
                    else:
                        seen_types.add(npc)

    new_rooms_str = '\n' + ',\n'.join(grid_to_str(g) for g in room_grids) + '\n'
    new_npcs_str = '\n' + ',\n'.join(grid_to_str(g) for g in npc_grids) + '\n'
    
    content = content[:room_match.start(3)] + new_rooms_str + content[room_match.end(3):]
    
    # Must re-search for npc_match because indices changed!
    for match in re.finditer(r'static const int (\w+)\[(\d+)\]\[10\]\[10\]\s*=\s*\{(.*?)\n\};', content, re.DOTALL):
        if match.group(1).endswith('_npc') or match.group(1).endswith('_npcs'):
            content = content[:match.start(3)] + new_npcs_str + content[match.end(3):]
            break
        
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)
    return True

files = glob.glob('src/level/rooms/*.h')
for f in files:
    if process_file(f):
        print('Processed:', f)
