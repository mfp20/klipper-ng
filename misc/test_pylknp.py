from protocoly import ffi,lib

print(lib.deltaTime)
print(lib.SYSEX_SUB_SCHED_RESET)

lib.encodingSwitch(1)
print(lib.encodingSwitch(0))
print(lib.encodingSwitch(1))
print(lib.encodingSwitch(2))

#PROTOCOL_MAX_EVENT_BYTES

argv_ptr = ffi.new("uint8_t *", 0)
ev_ptr = ffi.new("uint8_t *", 0)
result = lib.encodeEvent(lib.STATUS_SYSTEM_RESET, 0, argv_ptr, ev_ptr)
print(lib.eventBuffer)

