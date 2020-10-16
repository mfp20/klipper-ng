from libc cimport stdint

ctypedef stdint.uint8_t uint8_t
ctypedef stdint.uint16_t uint16_t

ctypedef void (*cbf_void_t)();
ctypedef void (*cbf_short_t)(uint8_t);
ctypedef void (*cbf_long_t)(uint16_t);
ctypedef void (*cbf_ss_t)(uint8_t,uint8_t);
ctypedef void (*cbf_sl_t)(uint8_t,uint16_t);
ctypedef void (*cbf_sss_t)(uint8_t,uint8_t,uint8_t);

cdef extern from "include/libknp.h":
    void initHost();
    void runHost();
    void eventRunHost(uint8_t size, uint8_t *event);
    extern cbf_ss_t cbPinMode;
    extern cbf_ss_t cbDigitalPortData;
    extern cbf_ss_t cbDigitalPinData;
    extern cbf_ss_t cbAnalogPinData;
    extern cbf_ss_t cbDigitalPortReport;
    extern cbf_ss_t cbDigitalPinReport;
    extern cbf_ss_t cbAnalogPinReport;
    extern cbf_void_t cbMicrostampReport;
    extern cbf_void_t cbTimestampReport;
    extern cbf_short_t cbCongestionReport;
    extern cbf_void_t cbVersionReport;
    extern cbf_void_t cbInterrupt;
    extern cbf_short_t cbEncodingSwitch;
    extern cbf_void_t cbEmergencyStop1;
    extern cbf_void_t cbEmergencyStop2;
    extern cbf_void_t cbEmergencyStop3;
    extern cbf_void_t cbEmergencyStop4;
    extern cbf_void_t cbSystemPause;
    extern cbf_void_t cbSystemResume;
    extern cbf_void_t cbSystemHalt;
    extern cbf_void_t cbSystemReset;
    extern cbf_void_t cbSysexDigitalPinData;
    extern cbf_void_t cbSysexAnalogPinData;
    extern cbf_void_t cbSysexSchedulerData;
    extern cbf_void_t cbSysexOneWireData;
    extern cbf_void_t cbSysexUartData;
    extern cbf_void_t cbSysexI2CData;
    extern cbf_void_t cbSysexSPIData;
    extern cbf_void_t cbSysexStringData;
    extern cbf_void_t cbSysexDigitalPinReport;
    extern cbf_void_t cbSysexAnalogPinReport;
    extern cbf_void_t cbSysexVersionReport;
    extern cbf_void_t cbSysexFeaturesReport;
    extern cbf_void_t cbSysexPinCapsReq;
    extern cbf_void_t cbSysexPinCapsRep;
    extern cbf_void_t cbSysexPinMapReq;
    extern cbf_void_t cbSysexPinMapRep;
    extern cbf_void_t cbSysexPinStateReq;
    extern cbf_void_t cbSysexPinStateRep;
    extern cbf_void_t cbSysexDeviceReq;
    extern cbf_void_t cbSysexDeviceRep;
    extern cbf_void_t cbSysexRCSwitchIn;
    extern cbf_void_t cbSysexRCSwitchOut;
    extern cbf_ss_t cbSchedCreate;
    extern cbf_short_t cbSchedDelete;
    extern cbf_sss_t cbSchedAdd;
    extern cbf_sl_t cbSchedSchedule;
    extern cbf_long_t cbSchedDelay;
    extern cbf_void_t cbSchedQueryList;
    extern cbf_short_t cbSchedQueryTask;
    extern cbf_void_t cbSchedReset;

