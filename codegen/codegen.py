"""The outer interface for the codegen module"""

from .msp430.msp430gen import MSP430Gen
def msp430gen():
    """The codegen interface for msp430"""
    return MSP430Gen
