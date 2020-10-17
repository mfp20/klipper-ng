from pyknp import ffi,lib

print("- PROTOCOL_MAX_EVENT_BYTES = "+str(lib.PROTOCOL_MAX_EVENT_BYTES))
print("- deltaTime = "+str(lib.deltaTime))
print("- STATUS_SYSEX_START = "+str(hex(lib.STATUS_SYSEX_START)))

lib.encodingSwitch(1)
print("- encodingSwitch(0) = "+str(lib.encodingSwitch(0)))
print("- encodingSwitch(1) = "+str(lib.encodingSwitch(1)))
print("- encodingSwitch(2) = "+str(lib.encodingSwitch(2)))

argv_ptr = ffi.new("uint8_t *", 0)
ev_ptr = ffi.new("uint8_t *", 0)
result = lib.encodeEvent(lib.STATUS_SYSTEM_RESET, 0, argv_ptr, ev_ptr)
#print("- example event STATUS_SYSTEM_RESET: ")
#print(ev_ptr)
#for byte in ev_ptr:
#    print(byte)

