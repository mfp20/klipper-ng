# Support for UC1701 (and similar) 128x64 graphics LCD displays
#
# Copyright (C) 2018-2019  Kevin O'Connor <kevin@koconnor.net>
# Copyright (C) 2018  Eric Callahan  <arksine.code@gmail.com>
#
# This file may be distributed under the terms of the GNU GPLv3 license.
import logging
import icons, font8x14, extras.bus

BACKGROUND_PRIORITY_CLOCK = 0x7fffffff00000000

TextGlyphs = { 'right_arrow': '\x1a', 'degrees': '\xf8' }

class DisplayBase:
    def __init__(self, io, columns=128):
        self.send = io.send
        # framebuffers
        self.columns = columns
        self.vram = [bytearray(self.columns) for i in range(8)]
        self.all_framebuffers = [(self.vram[i], bytearray('~'*self.columns), i)
                                 for i in range(8)]
        # Cache fonts and icons in display byte order
        self.font = [self._swizzle_bits(bytearray(c))
                     for c in font8x14.VGA_FONT]
        self.icons = {}
        for name, icon in icons.Icons16x16.items():
            top1, bot1 = self._swizzle_bits([d >> 8 for d in icon])
            top2, bot2 = self._swizzle_bits(icon)
            self.icons[name] = (top1 + top2, bot1 + bot2)
    def flush(self):
        # Find all differences in the framebuffers and send them to the chip
        for new_data, old_data, page in self.all_framebuffers:
            if new_data == old_data:
                continue
            # Find the position of all changed bytes in this framebuffer
            diffs = [[i, 1] for i, (n, o) in enumerate(zip(new_data, old_data))
                     if n != o]
            # Batch together changes that are close to each other
            for i in range(len(diffs)-2, -1, -1):
                pos, count = diffs[i]
                nextpos, nextcount = diffs[i+1]
                if pos + 5 >= nextpos and nextcount < 16:
                    diffs[i][1] = nextcount + (nextpos - pos)
                    del diffs[i+1]
            # Transmit changes
            for col_pos, count in diffs:
                # Set Position registers
                ra = 0xb0 | (page & 0x0F)
                ca_msb = 0x10 | ((col_pos >> 4) & 0x0F)
                ca_lsb = col_pos & 0x0F
                self.send([ra, ca_msb, ca_lsb])
                # Send Data
                self.send(new_data[col_pos:col_pos+count], is_data=True)
            old_data[:] = new_data
    def _swizzle_bits(self, data):
        # Convert 8x16 data into display col/row order
        bits_top = [0] * 8
        bits_bot = [0] * 8
        for row in range(8):
            for col in range(8):
                bits_top[col] |= ((data[row] >> (8 - col)) & 1) << row
                bits_bot[col] |= ((data[row + 8] >> (8 - col)) & 1) << row
        return (bits_top, bits_bot)
    def write_text(self, x, y, data):
        if x + len(data) > 16:
            data = data[:16 - min(x, 16)]
        pix_x = x * 8
        page_top = self.vram[y * 2]
        page_bot = self.vram[y * 2 + 1]
        for c in data:
            bits_top, bits_bot = self.font[ord(c)]
            page_top[pix_x:pix_x+8] = bits_top
            page_bot[pix_x:pix_x+8] = bits_bot
            pix_x += 8
    def write_graphics(self, x, y, row, data):
        if x + len(data) > 16:
            data = data[:16 - min(x, 16)]
        page = self.vram[y * 2 + (row >= 8)]
        bit = 1 << (row % 8)
        pix_x = x * 8
        for bits in data:
            for col in range(8):
                if (bits << col) & 0x80:
                    page[pix_x] ^= bit
                pix_x += 1
    def write_glyph(self, x, y, glyph_name):
        icon = self.icons.get(glyph_name)
        if icon is not None and x < 15:
            # Draw icon in graphics mode
            pix_x = x * 8
            page_idx = y * 2
            self.vram[page_idx][pix_x:pix_x+16] = icon[0]
            self.vram[page_idx + 1][pix_x:pix_x+16] = icon[1]
            return 2
        char = TextGlyphs.get(glyph_name)
        if char is not None:
            # Draw character
            self.write_text(x, y, char)
            return 1
        return 0
    def clear(self):
        zeros = bytearray(self.columns)
        for page in self.vram:
            page[:] = zeros
    def get_dimensions(self):
        return (16, 4)

# IO wrapper for "4 wire" spi bus (spi bus with an extra data/control line)
class SPI4wire:
    def __init__(self, config, data_pin_name):
        self.spi = extras.bus.MCU_SPI_from_config(config, 0,
                                                  default_speed=10000000)
        dc_pin = config.get(data_pin_name)
        self.mcu_dc = extras.bus.MCU_digital_out_queued(
            self.spi.get_mcu(), dc_pin, self.spi.get_command_queue())
    def send(self, cmds, is_data=False):
        self.mcu_dc.update_digital_out(is_data,
                                       reqclock=BACKGROUND_PRIORITY_CLOCK)
        self.spi.spi_send(cmds, reqclock=BACKGROUND_PRIORITY_CLOCK)

# IO wrapper for i2c bus
class I2C:
    def __init__(self, config, default_addr):
        self.i2c = extras.bus.MCU_I2C_from_config(
            config, default_addr=default_addr, default_speed=400000)
    def send(self, cmds, is_data=False):
        if is_data:
            hdr = 0x40
        else:
            hdr = 0x00
        cmds = bytearray(cmds)
        cmds.insert(0, hdr)
        self.i2c.i2c_write(cmds, reqclock=BACKGROUND_PRIORITY_CLOCK)

# Helper code for toggling a reset pin on startup
class ResetHelper:
    def __init__(self, pin_desc, io_bus):
        self.mcu_reset = None
        if pin_desc is None:
            return
        self.mcu_reset = extras.bus.MCU_digital_out_queued(
            io_bus.get_mcu(), pin_desc, io_bus.get_command_queue())
    def init(self):
        if self.mcu_reset is None:
            return
        mcu = self.mcu_reset.get_mcu()
        curtime = mcu.get_printer().get_reactor().monotonic()
        print_time = mcu.estimated_print_time(curtime)
        # Toggle reset
        minclock = mcu.print_time_to_clock(print_time + .100)
        self.mcu_reset.update_digital_out(0, minclock=minclock)
        minclock = mcu.print_time_to_clock(print_time + .200)
        self.mcu_reset.update_digital_out(1, minclock=minclock)
        # Force a delay to any subsequent commands on the command queue
        minclock = mcu.print_time_to_clock(print_time + .300)
        self.mcu_reset.update_digital_out(1, minclock=minclock)

# The UC1701 is a "4-wire" SPI display device
class UC1701(DisplayBase):
    def __init__(self, config):
        io = SPI4wire(config, "a0_pin")
        DisplayBase.__init__(self, io)
        self.contrast = config.getint('contrast', 40, minval=0, maxval=63)
        self.reset = ResetHelper(config.get("rst_pin", None), io.spi)
    def init(self):
        self.reset.init()
        init_cmds = [0xE2, # System reset
                     0x40, # Set display to start at line 0
                     0xA0, # Set SEG direction
                     0xC8, # Set COM Direction
                     0xA2, # Set Bias = 1/9
                     0x2C, # Boost ON
                     0x2E, # Voltage regulator on
                     0x2F, # Voltage follower on
                     0xF8, # Set booster ratio
                     0x00, # Booster ratio value (4x)
                     0x23, # Set resistor ratio (3)
                     0x81, # Set Electronic Volume
                     self.contrast, # Electronic Volume value
                     0xAC, # Set static indicator off
                     0x00, # NOP
                     0xA6, # Disable Inverse
                     0xAF] # Set display enable
        self.send(init_cmds)
        self.send([0xA5]) # display all
        self.send([0xA4]) # normal display
        self.flush()

# The SSD1306 supports both i2c and "4-wire" spi
class SSD1306(DisplayBase):
    def __init__(self, config, columns=128):
        cs_pin = config.get("cs_pin", None)
        if cs_pin is None:
            io = I2C(config, 60)
            io_bus = io.i2c
        else:
            io = SPI4wire(config, "dc_pin")
            io_bus = io.spi
        self.reset = ResetHelper(config.get("reset_pin", None), io_bus)
        DisplayBase.__init__(self, io, columns)
    def init(self):
        self.reset.init()
        init_cmds = [
            0xAE,       # Display off
            0xD5, 0x80, # Set oscillator frequency
            0xA8, 0x3f, # Set multiplex ratio
            0xD3, 0x00, # Set display offset
            0x40,       # Set display start line
            0x8D, 0x14, # Charge pump setting
            0x20, 0x02, # Set Memory addressing mode
            0xA1,       # Set Segment re-map
            0xC8,       # Set COM output scan direction
            0xDA, 0x12, # Set COM pins hardware configuration
            0x81, 0xEF, # Set contrast control
            0xD9, 0xA1, # Set pre-charge period
            0xDB, 0x00, # Set VCOMH deselect level
            0x2E,       # Deactivate scroll
            0xA4,       # Output ram to display
            0xA6,       # Normal display
            0xAF,       # Display on
        ]
        self.send(init_cmds)
        self.flush()

# the SH1106 is SSD1306 compatible with up to 132 columns
class SH1106(SSD1306):
    def __init__(self, config):
        SSD1306.__init__(self, config, 132)
