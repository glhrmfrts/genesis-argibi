import os,sys,json

# map2ctile.py - Generate a C array with solid tile IDs

try:
  filename = sys.argv[1]
except IndexError:
  print(f"Usage: {sys.argv[0]} map.json")
  quit()

with open(filename, 'r') as fh:
  data = json.load(fh)

  clean_filename = filename.replace('.json', '').replace('/', '_').replace('\\', '_')
  dst_filename_h = filename.replace('.json', '_collisiondata.h')
  dst_filename_c = filename.replace('.json', '_collisiondata.c')

  dst_file_h = open(dst_filename_h, 'w')
  dst_file_c = open(dst_filename_c, 'w')

  dst_file_h.write('#pragma once\n#include <genesis.h>\n#include "collisiontile.h"\n')
  dst_file_c.write(f'#include "../{dst_filename_h}"\n')

  for i,layer in enumerate(data['layers']):
    layersize = len(layer['data'])
    non_zero_tiles = 0

    for j,tileid in enumerate(layer['data']):
      if tileid != 0:
        non_zero_tiles += 1

    width = layer['width']
    height = layer['height']

    decl = f"const cTileData {clean_filename}_layer{i}_data[{non_zero_tiles}]"
    dst_file_h.write(f'#define {clean_filename}_layer{i}_len {non_zero_tiles}\n')
    dst_file_h.write(f'#define {clean_filename}_layer{i}_width {width}\n')
    dst_file_h.write(f'#define {clean_filename}_layer{i}_height {height}\n')
    dst_file_h.write('extern ' + decl + ';')
    dst_file_c.write(decl + ' = {\n')

    for i,tileid in enumerate(layer['data']):
      if tileid != 0:
        dst_file_c.write(f'{{ {tileid}, {i} }},\n')

    dst_file_c.write('};\n')
