import pathlib
import os
import re
import cffi

DIR_H="include"
DIR_BUILD="build"
DIR_PY=DIR_BUILD+"/py"
FILES=(
        DIR_BUILD+"/objs/src/hal/platform.h.i",
        DIR_H+"/pyknp_head.h",
        DIR_H+"/hal/arch.h",
        DIR_H+"/hal/board.h",
        DIR_H+"/hal.h",
        DIR_H+"/protocol.h",
        DIR_H+"/protocol_custom.h",
        DIR_H+"/libknp.h",
        DIR_H+"/pyknp_tail.h",
        )
DEST=DIR_PY+'/pyknp.h'
DEFINES_HEAD=(
        "__FIRMWARE_ARCH_",
        "__FIRMWARE_MCU_",
        "__FIRMWARE_BOARD_",
        "PIN_TOTAL",
        "PWM_CH",
        "ADC_CH",
        "ONEWIRE_CH",
        "TTY_CH",
        "I2C_CH",
        "SPI_CH"
        )

print("- Composing pyknp.h file.")

ifdef=0
includeguard=0
cppextern=0
ifelse=0
def removeCFFIUnsupported(line):
    global ifdef
    global includeguard
    global cppextern
    global ifelse
    # remove 'next line'
    if ifdef:
        ifdef=0
        return ''
    if includeguard:
        includeguard=0
        return ''
    if cppextern:
        cppextern=0
        return ''
    if ifelse:
        ifelse=0
        return ''
    x = re.findall("^#", line)
    if x:
        # remove ifdef
        y = re.findall("^#ifdef", line)
        if y:
            ifdef = 1
            return ''
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
        # remove else
        y = re.findall("^#else", line)
        if y:
            ifelse = 1
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

def searchInfoFiles(dirName):
    # create a list of file and sub directories names in the given directory
    listOfFile = os.listdir(dirName)
    iFiles = list()
    # Iterate over all the entries
    for entry in listOfFile:
        # Create full path
        fullPath = os.path.join(dirName, entry)
        # If entry is a directory then get the list of files in this directory
        if os.path.isdir(fullPath):
            iFiles = iFiles + searchInfoFiles(fullPath)
        elif (fullPath.endswith(".i")):
            iFiles.append(fullPath)
    return iFiles

def searchDefine(name,iFiles):
    found = ""
    for iFile in iFiles:
        searchfile = open(iFile, "r")
        for line in searchfile:
            if "#define "+name in line:
                found = line
                break
        searchfile.close()
        if found!="":
            break
    return found

def addDefines(destination):
    iFiles = searchInfoFiles("build/")
    line = ""
    for define in DEFINES_HEAD:
        line = searchDefine(define,iFiles)
        if line=="":
            print("WARNING: '#define "+define+"' NOT FOUND!!!")
        else:
            print(line)
            destination.write(line)

destination = open(DEST,'w')
addDefines(destination)
for f in FILES:
    header = open(f, 'r')
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
h_file_name = this_dir / DEST
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

