#!/usr/bin/env python

# requires the new fx2 firmware (based on libfx2)
# clear default register setup - power-on uninitialized

from adf435x.interfaces import FX2

FX2().clear_default()
