import pathlib
import re
import cffi

DIR_H="include"
DIR_BUILD="./build"
DIR_PY=DIR_BUILD+"/py"
FILES=(
        "pyknp_head.h",
        # "defines.h",
        "hal/arch.h",
        "hal/board.h",
        "hal.h",
        "protocol.h",
        "protocol_custom.h",
        "libknp.h",
        "pyknp_tail.h",
        )
DEST=DIR_PY+'/pyknp.h'

print("- Composing pyknp.h file.")

includeguard=0
cppextern=0
def removeCFFIUnsupported(line):
    global includeguard
    global cppextern
    # remove 'next line'
    if includeguard:
        includeguard=0
        return ''
    if cppextern:
        cppextern=0
        return ''
    x = re.findall("^#", line)
    if x:
        # remove include guard
        y = re.findall("(^#ifndef.*_H$)", line)
        if y:
            includeguard = 1
            return ''
        # remove C++ extern
        y = re.findall("^#ifdef __cplusplus", line)
        if y:
            cppextern = 1
            return ''
        # remove endif
        y = re.findall("^#endif", line)
        if y:
            return ''
        # remove #include
        y = re.findall("^#include", line)
        if y:
            return ''
    return line

destination = open(DEST,'w')
for f in FILES:
    header = open(DIR_H+"/"+f, 'r')
    for line in header.readlines():
        good = removeCFFIUnsupported(line)
        if good:
            destination.write(good)
    header.close()
destination.close()


print("- Building pyknp CFFI Python module")

ffi = cffi.FFI()
# read header
this_dir = pathlib.Path().absolute()
h_file_name = this_dir / "build/py/pyknp.h"
with open(h_file_name) as h_file:
    ffi.cdef(h_file.read())
# configure
ffi.set_source(
    'pyknp',
    '#include "pyknp.h"',
    #extra_compile_args=[],
    extra_link_args=["-l:../objs/libknp.hostlinux.a"],
)
# compile
ffi.compile(tmpdir="build/py/", verbose=False, debug=None)
# report
print("- pyknp ready to import")

