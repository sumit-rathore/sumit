#!/usr/bin/env python

fout = open ('../inc/istatus.h', 'w')
fin = open ('../../../fg_icron_files/istatus', 'r')

fout.write('#ifndef ISTATUS_H\n')
fout.write('#define ISTATUS_H\n')
fout.write('enum IStatusIdentifier {\n')

for line in fin:
    if "L:" in line:
        level = line.split()
        enumLevel = level[0].split(":")
        output = '     %s,\n' %(enumLevel[1].strip())
        fout.write(output)

fout.write('};\n')
fout.write('#endif // ISTATUS_H\n')

fout.close()
fin.close()

