import pathlib
import cffi

print("- Building CFFI Module")

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
print("* Complete")

