#!/usr/bin/env python3

from adf435x.interfaces import FX2
from adf435x.core import freq_make_regs

intf = FX2()

regs = freq_make_regs(100)
intf.set_regs(regs[::-1])
