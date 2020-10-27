# - Pins: pin definitions and management
# - MCU: Interface to Klipper micro-controller code
# - Board: mcu+pins+bus
# - Controller: multiple board controller
#
# Copyright (C) 2020 Anichang <anichang@protonmail.ch>
#
# This file may be distributed under the terms of the GNU GPLv3 license.

import logging, sys, re, math, zlib, collections
from text import msg
from error import KError as error
import tree, timing, pyknp
logger = logging.getLogger(__name__)

class Pin:
    def __init__(self):
        self.name = None
        self.alias = []
        self.caps = []
        self.mode = None
        self.value = None
        self.inuse = None

    def query(self):
        pass

    def reset(self):
        pass

class PinOut(controller.Pin):
    def __init__(self):
        super().__init__()

class PinOutMulti(controller.Pin):
    def __init__(self):
        super().__init__()
        self.sub = []

class PinOutPWM(controller.Pin):
    def __init__(self):
        super().__init__()

class PinOutPWMMulti(controller.Pin):
    def __init__(self):
        super().__init__()

class PinIn(controller.Pin):
    def __init__(self):
        super().__init__()

class PinInADC(controller.Pin):
    def __init__(self):
        super().__init__()

# commport
class Commport:
    def __init__(self):
        pass

    def available(self):
        return False

    def read(self):
        return None

    def write(self, byte):
        pass

    def query(self):
        pass

    def reset(self):
        pass

class Commport1WIRE(controller.Commport):
    def __init__(self):
        super().__init__()

    def available(self):
        return False

    def read(self):
        return None

    def write(self, byte):
        pass

class CommportTTY(controller.Commport):
    def __init__(self):
        super().__init__()

    def available(self):
        return False

    def read(self):
        return None

    def write(self, byte):
        pass

class CommportI2C(controller.Commport):
    def __init__(self):
        super().__init__()

    def available(self):
        return False

    def read(self):
        return None

    def write(self, byte):
        pass

class CommportSPI(controller.Commport):
    def __init__(self):
        super().__init__()

    def available(self):
        return False

    def read(self):
        return None

    def write(self, byte):
        pass

# send the same data to multiple commports
class CommportMulti(controller.Commport):
    def __init__(self):
        super().__init__()
        self.sub = []

    def write(self, byte):
        for s in self.sub:
            s.write(byte)

# bridge 2 commports
class CommportBridge(controller.Commport):
    def __init__(self):
        super().__init__()
        self.sub = []

    def available(self, sub):
        avail = False
        for s in self.sub:
            if s.available():
                avail = True
                break;
        return avail

    def read(self):
        while sub[0].available():
            sub[1].write(sub[0].read())
        while sub[1].available():
            sub[0].write(sub[1].read())

    # inject data
    def write(self, byte):
        for s in self.sub:
            s.write(byte)

# send the same data to multiple commports, receive from multiple commports
class CommportStar(controller.Commport):
    def __init__(self):
        super().__init__()
        self.sub = []

    def available(self):
        avail = False
        for s in self.sub:
            if s.available():
                avail = True
                break;
        return avail

    def read(self):
        received = collections.OrderedDict()
        for s in self.sub:
            received[s] = []
            while s.available():
                byte = s.read()
                received[s] += byte
        return received

    def write(self, byte):
        for s in self.sub:
            s.write(byte)

# broadcast all incoming data (excluding the sender)
class CommportHub(controller.Commport):
    def __init__(self):
        super().__init__()
        self.sub = []

    def available(self):
        avail = False
        for s in self.sub:
            if s.available():
                avail = True
                break;
        return avail

    def read(self):
        received = collections.OrderedDict()
        for s in self.sub:
            received[s] = []
            while s.available():
                byte = s.read()
                for t in self.sub:
                    if t == s: continue
                    t.write(byte)
                received[s] += byte
        return received

    def write(self, byte):
        for s in self.sub:
            s.write(byte)

class MCU:
    def __init__(self):
        self.clock = None

    def init(self,primary=False):
        # TODO init clock
        pass

    def query(self):
        pass

    def reset(self):
        pass

# pin set
class GPIO:
    def __init__(self):
        self.pin = []

    def addPin(self,pin):
        self.pin += pin

    def query(self):
        result = []
        for p in self.pin:
            result += p.query()
            return result

    def reset(self):
        for p in self.pin:
            p.reset()

# on-board device (ex: led)
class Device:
    def __init__(self):
        self.pin = []

    def query(self):
        pass

    def reset(self):
        pass

# manages 1 board := {mcu, GPIOs, Commports, Devices}
class Board(tree.Part):
    def __init__(self, name, hal):
        super().__init__(name, hal = hal)
        self.mcu = MCU()
        self.gpio = GPIO()
        self.commport = []
        self.device = []

    def initMcu(self):
        # TODO connect to serial
        # - perform handshake (protocol, version, encoding)
        # - fetch general info
        # - fetch time info & sync

    def initGpio(self):
        # TODO query all pins and wait for answer
        # for each pin call:
        self.gpio.addPin(None)

    def initCommports(self):
        # TODO query all commports and wait for answer
        # for each commport:
        self.commport += None

    def initDevice(self):
        # TODO query all devices and wait for answer
        # for each device:
        self.device += None

# virtual board:
# - virtual pin, propagates its changes to multiple real pins,
#   across multiple real boards.
# - virtual commport, send the same data to multiple boards
# - bridge, 2-way arbitrary commports bridge
# - star, 1-to-many, many-to-1 arbitrary commports star
# - hub, many-to-many arbitrary commports hub
class BoardVirtual(Board):
    def __init__(self, name, hal):
        super().__init__(name, hal = hal)
        self.multipin = collections.OrderedDict()
        self.multiport = collections.OrderedDict()
        self.bridge = collections.OrderedDict()
        self.star = collections.OrderedDict()
        self.hub = collections.OrderedDict()

    def initMcu(self):
        pass

    def initGpio(self):
        pass

    def initCommports(self):
        pass

    def initDevice(self):
        pass

    def addMultipin(self,name,pins):
        self.multipin += (name,pins)

    def addMultiport(self,name,ports):
        self.multiport += (name,ports)

    def addBridge(self,name,port1,port2):
        self.bridge += (name,[port1,port2])

    def addStar(self,name,ports):
        self.star += (name,ports)

    def addHub(self,name,ports):
        self.hub += (name,ports)

# multiboard mapper := {board1,board2,...,boardN}
# devices manager := {stepper,endstop,heater,thermometer,...}
class Interface(tree.Composite):
    def __init__(self, name, hal):
        super().__init__(name, hal = hal)
        self.board = {}
        self.board_ready = 0
        self.virtual = collections.OrderedDict()
        self.endstop = collections.OrderedDict()
        self.thermometer = collections.OrderedDict()
        self.hygrometer = collections.OrderedDict()
        self.barometer = collections.OrderedDict()
        self.filament = collections.OrderedDict()
        self.stepper = collections.OrderedDict()
        self.servo = collections.OrderedDict()
        self.heater = collections.OrderedDict()
        self.cooler = collections.OrderedDict()
        #
        self.ready = True

    def _show_details(self, indent = 0):
        "Return formatted details about printer controller node."
        txt = "\t"*(indent+1) + "\t(part)\t\t\t(pin type)\t\t\t(used pin)\n"
        used = []
        for kind in [self.virtual, 
                     self.endstop, 
                     self.thermometer, 
                     self.hygrometer, 
                     self.barometer, 
                     self.filament, 
                     self.stepper, 
                     self.servo, 
                     self.heater,
                     self.cooler]:
            for part in sorted(kind):
                for pin in sorted(kind[part].pin):
                    used.append((part, pin, kind[part].pin[pin]))
                    for part, pin, obj in sorted(used):
                        txt = txt + "\t" * (indent+1) + "- " + part.ljust(20, " ") + " " + str(obj).split(" ")[0][1:].ljust(40, " ") + " " + pin + "\n"
                        return txt

    def _register(self):
        for b in self.board:
            self.hal.get_printer().event_register_handler("board:"+b+":ready", self._event_handle_ready)
            self.hal.get_printer().event_register_handler("commander:request_restart", self._event_handle_request_restart)
            # events handlers
            def _event_handle_ready(self):
                self.board_ready = self.board_ready + 1
                if self.board_ready == self.hal.mcu_count:
                    pass
                elif self.board_ready > self.hal.mcu_count:
                    raise error("Controller: too many ready MCUs.")

    def _event_handle_request_restart(self, print_time):
        self.stepper_linetracker.motor_off()

    # setup custom pin aliases
    # TODO
    def setup_custom_alias(self, aliases):
        if aliases.endswith(','):
            aliases = aliases[:-1]
            parts = [a.split('=', 1) for a in aliases.split(',')]
            for pair in parts:
                if len(pair) != 2:
                    raise error("Unable to parse aliases in %s" % (config.get_name(),))
            name, value = [s.strip() for s in pair]
            if value.startswith('<') and value.endswith('>'):
                pin_resolver.reserve_pin(name, value)
            else:
                pin_resolver.alias_pin(name, value)
                # return the pin name part of "board:pin"
                def _b(self, pin):
                    return pin.split(":")[0]

    # return the board name part of "board:pin"
    def _p(self, pin):
        return pin.split(":")[1]

    # return the board from "board:pin"
    def _board(self, pin):
        return self.board[self._b(pin)]

    # returns the mcu from "board:pin"
    def _mcu(self, pin):
        return self._board(pin).mcu

    # return a list of all registered boards
    def board_list(self):
        boards = list()
        for b in self.board:
            boards.append(self.board[b])
            return boards

    # restart all mcus
    def mcu_restart(self):
        for b in self.board_list():
            b.mcu_restart()
            # return a list of all registered mcus
            def mcu_list(self):
                mcus = list()
                for b in self.board:
                    mcus.append(self.board[b].mcu)
                    return mcus

    # return a dict of all available pins and their status
    def pin_dict(self):
        pins = collections.OrderedDict()
        for b in self.board:
            pins[b] = self.board[b].pin.matrix()
            return pins

    # return a dict of all active pins and their status
    def pin_active_dict(self):
        active = collections.OrderedDict()
        for b in self.board:
            active[b] = self.board[b].pin.active
            return active

    # wrappers to call board's methods, pin is "board:pin"
    def pin_register(self, pin, can_invert=False, can_pullup=False, share_type=None):
        return self._board(pin).pin_register(self._p(pin), can_invert, can_pullup, share_type)

    def pin_setup(self, pin_type, pin):
        if self._b(pin) == "virtual":
            return self.virtual["virtual "+self._p(pin)].pin_setup(pin_type,{'chip': None, 'chip_name': self._b(pin), 'pin': self._p(pin), 'invert': False, 'pullup': False})
        return self._board(pin).pin_setup(pin_type, self._p(pin))

    # (un)registers parts
    def register_part(self, node, remove = False):
        if remove:
            # unregister
            for parts in [self.board, self.virtual, self.endstop, self.thermometer, self.hygrometer, self.barometer, self.filament, self.stepper, self.heater, self.cooler]:
                if node.name in parts:
                    if node.name().startswith("mcu "):
                        pass
                    elif node.name().startswith("virtual "):
                        pass
                    elif node.name().startswith("sensor "):
                        if node._type == "endstop":
                            pass
                        if node._type == "thermometer":
                            pass
                        if node._type == "hygrometer":
                            pass
                        if node._type == "barometer":
                            pass
                        if node._type == "filament":
                            pass
                        elif node.name().startswith("stepper "):
                            pass
                        elif node.name().startswith("heater "):
                            pass
                        elif node.name().startswith("cooler "):
                            pass
                    return parts.pop(node.name())
            logger.warning("Part '%s' not registered in controller. Can't de-register.", node.name())
        else:
            # register
            if node.name().startswith("mcu "):
                bname = node.id()
                if bname in self.board:
                    raise error("Duplicate mcu name '%s'" % bname)
                self.board[bname] = node
                self.board[bname].init_mcu()
            elif node.name().startswith("virtual "):
                self.virtual[node.name()] = node
            elif node.name().startswith("sensor "):
                if node._type == "endstop":
                    self.endstop[node.name()] = node
                    if node._type == "thermometer":
                        self.thermometer[node.name()] = node
                        if node._type == "hygrometer":
                            self.hygrometer[node.name()] = node
                            if node._type == "barometer":
                                self.barometer[node.name()] = node
                                if node._type == "filament":
                                    self.filament[node.name()] = node
                                elif node.name().startswith("stepper "):
                                    self.stepper[node.name()] = node
                                    self.stepper_linetracker.register_stepper(node.name(), node.enable)
                                elif node.name().startswith("heater "):
                                    self.heater[node.name()] = node
                                elif node.name().startswith("cooler "):
                                    self.cooler[node.name()] = node
                                else:
                                    raise error("Unknown group for part '%s'. Can't register in controller.", node.name())
        return node

    #
    def cleanup(self):
        pass

    #
    # commands
    def cmd_SHOW_PINS_ALL(self):
        'Shows all pins.'
        # TODO
        self.hal.get_commander().respond_info(self.pin_dict(), log=False)

    def cmd_SHOW_PINS_ACTIVE(self):
        'Shows active pins.'
        # TODO
        self.hal.get_commander().respond_info(self.pin_active_dict(), log=False)

    def cmd_SHOW_ADC_VALUE(self, params):
        'Report the last value of an analog pin.'
        # TODO
        gcode = self.printer.lookup('gcode')
        name = gcode.get_str('NAME', params, None)
        mcu = ""
        value = mcu.adc_query(name)
        if not value:
            objs = ['"%s"' % (n,) for n in sorted(self.adc.keys())]
            msg = "Available ADCs: %s" % (', '.join(objs),)
            gcode.respond_info(msg)
            return
        msg = 'ADC "%s" has value %.6f (timestamp %.3f)' % value
        pullup = gcode.get_float('PULLUP', params, None, above=0.)
        if pullup is not None:
            v = max(.00001, min(.99999, value))
            r = pullup * v / (1.0 - v)
            msg += "\n resistance %.3f (with %.0f pullup)" % (r, pullup)
            gcode.respond_info(msg)

    def cmd_SHOW_ENDSTOPS(self, params):
        'Report on the status of each endstop.'
        # TODO
        msg = " "
        for rail in self.hal.children_deep("rail "):
            msg = msg + ["%s:%s\n" % (name, ["open", "TRIGGERED"][not not t]) for name, t in rail.get_endstops_status()]
            #
            self.hal.get_gcode().respond(msg)

    def cmd_STEPPER_BUZZ(self, params):
        'Oscillate a given stepper to help id it.'
        stepper = self._lookup_stepper(params)
        logger.info("Stepper buzz %s", stepper.get_name())
        was_enable = self.force_enable(stepper)
        toolhead = self.printer.lookup('toolhead')
        dist, speed = BUZZ_DISTANCE, BUZZ_VELOCITY
        if stepper.units_in_radians():
            dist, speed = BUZZ_RADIANS_DISTANCE, BUZZ_RADIANS_VELOCITY
            for i in range(10):
                self.move_force(stepper, dist, speed)
                toolhead.dwell(.050)
                self.move_force(stepper, -dist, speed)
                toolhead.dwell(.450)
                self.restore_enable(stepper, was_enable)

    def cmd_STEPPER_MOVE(self, params):
        'Manually move a stepper (only at idle, see STEPPER_MOVE_FORCE).'
        if 'ENABLE' in params:
            self.do_enable(self.gcode.get_int('ENABLE', params))
            if 'SET_POSITION' in params:
                setpos = self.gcode.get_float('SET_POSITION', params)
                self.do_set_position(setpos)
                sync = self.gcode.get_int('SYNC', params, 1)
                homing_move = self.gcode.get_int('STOP_ON_ENDSTOP', params, 0)
                speed = self.gcode.get_float('SPEED', params, self.velocity, above=0.)
                accel = self.gcode.get_float('ACCEL', params, self.accel, minval=0.)
                if homing_move:
                    movepos = self.gcode.get_float('MOVE', params)
                    self.do_homing_move(movepos, speed, accel, homing_move > 0, abs(homing_move) == 1)
                elif 'MOVE' in params:
                    movepos = self.gcode.get_float('MOVE', params)
                    self.do_move(movepos, speed, accel, sync)
                elif 'SYNC' in params and sync:
                    self.sync_print_time()

    def cmd_STEPPER_MOVE_FORCE(self, params):
        'Manually move a stepper; invalidates kinematics.'
        stepper = self._lookup_stepper(params)
        distance = self.gcode.get_float('DISTANCE', params)
        speed = self.gcode.get_float('VELOCITY', params, above=0.)
        accel = self.gcode.get_float('ACCEL', params, 0., minval=0.)
        logger.warning("STEPPER_MOVE_FORCE %s distance=%.3f velocity=%.3f accel=%.3f", stepper.get_name(), distance, speed, accel)
        self.force_enable(stepper)
        self.move_force(stepper, distance, speed, accel)

    def cmd_STEPPER_SWITCH(self, params):
        'Enable/disable individual stepper by name.'
        stepper_name = self.hal.get_commander().get_str('STEPPER', params, None)
        stepper_enable = self.hal.get_commander().get_int('ENABLE', params, 1)
        self.stepper_linetracker.debug_switch(stepper_name, stepper_enable)

    def cmd_STEPPER_SWITCH_GROUP(self, params):
        'Enable/disable one group of steppers (ex: rail steppers, toolhead steppers) by name.'
        stepper_group = self.hal.get_commander().get_str('GROUP', params, None)
        stepper_enable = self.hal.get_commander().get_int('ENABLE', params, 1)
        self.stepper_linetracker.debug_switch_tracker(stepper_group, stepper_enable)

    def cmd_STEPPER_OFF(self, params):
        'Enable/disable all steppers.'
        self.stepper_linetracker.off()

ATTRS = ("serialport", "baud", "pin_map", "restart_method")
def load_node(name, hal, cparser = None):
    if name == "controller":
        return Interface(name, hal)
    elif name.startswith("mcu "):
        return Board(name, hal)
    elif name.startswith("virtual "):
        return BoardVirtual(name, hal)
    return None

