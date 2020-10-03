# Printer heat controller, sensors and heaters lookup
#
# Copyright (C) 2016-2018  Kevin O'Connor <kevin@koconnor.net>
# Copyright (C) 2020 Anichang <anichang@protonmail.ch>
#
# This file may be distributed under the terms of the GNU GPLv3 license.

import logging, sys
from text import msg
from error import KError as error
import tree, governor
logger = logging.getLogger(__name__)

#
# Verifier: periodical temperature checks
#
# TODO
HINT_THERMAL = """
See the 'verify_heater' section in config/example-extras.cfg
for the parameters that control this check.
"""
class Verifier:
    def __init__(self, config):
        self.printer = config.get_printer()
        self.printer.event_register_handler("klippy:connect", self._event_handle_connect)
        self.printer.event_register_handler("klippy:shutdown", self._event_handle_shutdown)
        self.heater_name = config.get_name().split()[1]
        self.heater = None
        self.hysteresis = config.getfloat('hysteresis', 5., minval=0.)
        self.max_error = config.getfloat('max_error', 120., minval=0.)
        self.heating_gain = config.getfloat('heating_gain', 2., above=0.)
        default_gain_time = 20.
        if self.heater_name == 'heater_bed':
            default_gain_time = 60.
        self.check_gain_time = config.getfloat('check_gain_time', default_gain_time, minval=1.)
        self.approaching_target = self.starting_approach = False
        self.last_target = self.goal_temp = self.error = 0.
        self.goal_systime = self.printer.get_reactor().NEVER
        self.check_timer = None
    def _event_handle_connect(self):
        if self.printer.get_args().get('output_debug') is not None:
            # Disable verify_heater if outputting to a debug file
            return
        pheater = self.printer.lookup('heater')
        self.heater = pheater.lookup_heater(self.heater_name)
        logger.info("Starting heater checks for %s", self.heater_name)
        reactor = self.printer.get_reactor()
        self.check_timer = reactor.register_timer(self.check_event, reactor.NOW)
    def _event_handle_shutdown(self):
        if self.check_timer is not None:
            reactor = self.printer.get_reactor()
            self.printer.get_reactor().update_timer(self.check_timer, reactor.NEVER)
    def check_event(self, eventtime):
        temp, target = self.heater.get_temp(eventtime)
        if temp >= target - self.hysteresis or target <= 0.:
            # Temperature near target - reset checks
            if self.approaching_target and target:
                logger.info("Heater %s within range of %.3f",
                             self.heater_name, target)
            self.approaching_target = self.starting_approach = False
            if temp <= target + self.hysteresis:
                self.error = 0.
            self.last_target = target
            return eventtime + 1.
        self.error += (target - self.hysteresis) - temp
        if not self.approaching_target:
            if target != self.last_target:
                # Target changed - reset checks
                logger.info("Heater %s approaching new target of %.3f",
                             self.heater_name, target)
                self.approaching_target = self.starting_approach = True
                self.goal_temp = temp + self.heating_gain
                self.goal_systime = eventtime + self.check_gain_time
            elif self.error >= self.max_error:
                # Failure due to inability to maintain target temperature
                return self.heater_fault()
        elif temp >= self.goal_temp:
            # Temperature approaching target - reset checks
            self.starting_approach = False
            self.error = 0.
            self.goal_temp = temp + self.heating_gain
            self.goal_systime = eventtime + self.check_gain_time
        elif eventtime >= self.goal_systime:
            # Temperature is no longer approaching target
            self.approaching_target = False
            logger.info("Heater %s no longer approaching target %.3f",
                         self.heater_name, target)
        elif self.starting_approach:
            self.goal_temp = min(self.goal_temp, temp + self.heating_gain)
        self.last_target = target
        return eventtime + 1.
    def heater_fault(self):
        msg = "Heater %s not heating at expected rate" % (self.heater_name,)
        logger.error(msg)
        self.printer.call_shutdown(msg + HINT_THERMAL)
        return self.printer.get_reactor().NEVER

#
# PID calibration tool/command
#
# TODO
class PIDCalibrate:
    def __init__(self, config):
        self.printer = config.get_printer()
        self.gcode = self.printer.lookup('gcode')
        self.gcode.register_command('PID_CALIBRATE', self.cmd_PID_CALIBRATE, desc=self.cmd_PID_CALIBRATE_help)
    cmd_PID_CALIBRATE_help = "Run PID calibration test"
    def cmd_PID_CALIBRATE(self, params):
        heater_name = self.gcode.get_str('HEATER', params)
        target = self.gcode.get_float('TARGET', params)
        write_file = self.gcode.get_int('WRITE_FILE', params, 0)
        pheater = self.printer.lookup('heater')
        try:
            heater = pheater.lookup_heater(heater_name)
        except self.printer.config_error as e:
            raise self.gcode.error(str(e))
        self.printer.lookup('toolhead').get_last_move_time()
        calibrate = ControlAutoTune(heater, target)
        old_control = heater.set_control(calibrate)
        try:
            heater.set_temp(target)
        except self.printer.command_error as e:
            heater.set_control(old_control)
            raise
        self.gcode.wait_for_temperature(heater)
        heater.set_control(old_control)
        if write_file:
            calibrate.write_file('/tmp/heattest.txt')
        # Log and report results
        Kp, Ki, Kd = calibrate.calc_final_pid()
        logger.info("Autotune: final: Kp=%f Ki=%f Kd=%f", Kp, Ki, Kd)
        self.gcode.respond_info(
            "PID parameters: pid_Kp=%.3f pid_Ki=%.3f pid_Kd=%.3f\n"
            "The SAVE_CONFIG command will update the printer config file\n"
            "with these parameters and restart the printer." % (Kp, Ki, Kd))
        # Store results for SAVE_CONFIG
        configfile = self.printer.lookup('configfile')
        configfile.set(heater_name, 'control', 'pid')
        configfile.set(heater_name, 'pid_Kp', "%.3f" % (Kp,))
        configfile.set(heater_name, 'pid_Ki', "%.3f" % (Ki,))
        configfile.set(heater_name, 'pid_Kd', "%.3f" % (Kd,))

'''
logger.info(temperature.sensor_factories)
INFO:root:{'NTC 100K beta 3950': <function <lambda> at 0x70751013a150>, 'Honeywell 100K 135-104LAG-J01': <function <lambda> at 0x70751011ac50>, 'NTC 100K MGB18-104F39050L32': <function <lambda> at 0x70750feb6a50>, 'PT100 INA826': <function <lambda> at 0x70750feb6dd0>, 'ATC Semitec 104GT-2': <function <lambda> at 0x70751011ae50>, 'MAX31855': <class extras.spi_temperature.MAX31855 at 0x70750fec4360>, 'MAX31856': <class extras.spi_temperature.MAX31856 at 0x70750fec42f0>, 'AD8496': <function <lambda> at 0x70750feb6cd0>, 'AD8497': <function <lambda> at 0x70750feb6d50>, 'AD8494': <function <lambda> at 0x70750feb6bd0>, 'AD8495': <function <lambda> at 0x70750feb6c50>, 'MAX31865': <class extras.spi_temperature.MAX31865 at 0x70750fec4440>, 'EPCOS 100K B57560G104F': <function <lambda> at 0x70750feb6ad0>, 'PT1000': <function <lambda> at 0x70750feb6e50>, 'AD595': <function <lambda> at 0x70750feb6b50>, 'BME280': <class extras.bme280.BME280 at 0x70750fec4590>, 'MAX6675': <class extras.spi_temperature.MAX6675 at 0x70750fec43d0>}

logger.info(temperature.heaters)
INFO:root:{'heater_bed': <heater.Heater instance at 0x70750fecb230>, 'extruder': <heater.Heater instance at 0x70750f8afd70>}

logger.info(temperature.gcode_id_to_sensor)
INFO:root:{'B': <heater.Heater instance at 0x70750fecb230>, 'T0': <heater.Heater instance at 0x70750f8afd70>}

logger.info(temperature.available_heaters)
INFO:root:['heater_bed', 'extruder']

logger.info(temperature.available_sensors)
INFO:root:['heater_bed', 'extruder']
'''

class Manager(tree.Part):
    def __init__(self, name, hal):
        super().__init__(name, hal = hal)
        self.govoff = governor.AlwaysOff()
        self.govon = governor.AlwaysOn()
        self.tcontroller = {}
        self.ready = True
        #self.printer.try_load_module(config, "verify_heater %s" % (self.name,))
    def _show_details(self, indent = 0):
        "Return formatted details about temperature controller node."
        return ""
    def register(self):
        self.hal.get_printer().event_register_handler("commander:request_restart", self._event_handler_off_all_actuators)
    # handlers
    def _command_handler_off_all_heaters(self, print_time=0.):
        for tcontrol in self.tcontroller.values():
            tcontrol.set_off_heaters()
    def _command_handler_off_all_coolers(self, print_time=0.):
        for tcontrol in self.tcontroller.values():
            tcontrol.set_off_coolers()
    def _event_handler_off_all_actuators(self, print_time=0.):
        self._command_handler_off_all_heaters()
        self._command_handler_off_all_coolers()
    # (un)register temperature controller to the given commander
    def tc_register(self, tc, gcode=None):
        # unregister
        if tc == None:
            self.tcontroller.pop(tc.name)
            if gcode:
                gcode = self.hal.get_my_gcode(tc)
                # TODO remove command gcode.register_mux_command("SET_TEMPERATURE", "TCONTROL", self.name, self.cmd_SET_TEMPERATURE, desc=self.cmd_SET_TEMPERATURE_help)
            else:
                # TODO remove command from commander self.hal.get_commander().register_mux_command("SET_TEMPERATURE", "TCONTROL", self.name, self.cmd_SET_TEMPERATURE, desc=self.cmd_SET_TEMPERATURE_help)
                pass
            return
        # register
        if gcode:
            self.tcontroller[gcode] = tc
            # TODO register commands
        else:
            if tc.name == None:
                raise error("Can't register temperature controller.")
            if tc.name in self.tcontroller:
                raise error("Temperature controller '%s' already registered" % (tc.name,))
            self.tcontroller[tc.name] = tc
    def get_tc(self, name):
        return self.tcontroller[name]
    def get_current_temp(tcname, sensorname = None):
        tc = self.get_tc(tcname)
        sensor = None
        if sensorname:
            sensor = tc.get_thermometer(sensorname)
        return tc.get_temp(self.hal.get_reactor().monotonic(), sensor)
    def set_target_temp(tcname, temp):
        self.get_tc(tcname).set_temp(temp)
    #
    def cleanup(self):
        pass
    #
    # commands
    def _cmd__SET_TEMPERATURE(self, params):
        'Sets the temperature for the given controller.'
        # TODO
        name = self.gcode.get_float('NAME', params, 0.)
        temp = self.gcode.get_float('TARGET', params, 0.)
        self._set_target_temp(name, temp)
    def _cmd__TEMPERATURE_HEATERS_SWITCH(self, params):
        ''
        # TODO
        self._off_all_heaters()
    def _cmd__TEMPERATURE_COOLERS_SWITCH(self, params):
        'Turn on/off all coolers'
        # TODO
        self._off_all_coolers()
    def _cmd__TEMPERATURE_SWITCH(self, params):
        'Turn on/off all temperature actuators (ie: heaters and coolers)'
        # TODO
        self._event_handler_off_all_actuators()
    def get_status(self, eventtime):
        # TODO
        return {'available_heaters': self.available_heaters, 'available_sensors': self.available_sensors}

def load_node(name, hal, cparser = None):
    return Manager(name, hal)

