# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2025 Mingyuan Xiang

from .msp430.msp430gen import MSP430Gen
def msp430gen():
    """The codegen interface for msp430"""
    return MSP430Gen
