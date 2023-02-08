#!/usr/bin/env python

# clear default register setup - power-on uninitialized

from adf435x.interfaces import FX2

FX2().clear_default()
