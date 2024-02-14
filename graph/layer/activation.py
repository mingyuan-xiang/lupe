"""Activation layers"""

from .layer import LupeLayer

class Activation(LupeLayer):
    """Activation layer"""

class Relu(Activation):
    """Relu layer"""

class Tanh(Activation):
    """Tanh layer"""
