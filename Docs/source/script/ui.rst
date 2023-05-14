UI Module
=========

This module contains functions for debugging.

.. note:: This module is available only in Debug builds of the Engine.

Usage
-----

    require 'NeDebug'

Functions
---------
.. function:: Debug.LogAddress(name, userdata)

    Log the address of an userdata or lightuserdata variable. The entry will be in the form <name>: <address>.

    :param name: Variable name
    :type name: string
    :param userdata: The variable to print the address of
    :type userdata: userdata or lightuserdata

.. function:: Debug.Break()

    Trigger a break point in native code.

    .. WARNING:: This function will terminate the engine if there is no debugger attached.
