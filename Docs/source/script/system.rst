System Module
=============

This module contains

Usage
-----

    require 'NeSystem'


Constants
---------


Functions
---------
.. lua:module:: System

.. lua:function:: System.Time()

    Retrieve the time elapsed since the engine was started.

    :return: Time elapsed.
    :rtype: integer

.. lua:function:: System.ScreenVisible()

    Returns true if the engine's window is visible on the screen

    :return: Window visible.
    :rtype: boolean

.. lua:function:: System.Machine()

    Retrieve the CPU architecture.

    :return: CPU architecture.
    :rtype: string

.. lua:function:: System.Hostname()

    Retrieve the hostname.

    :return: Hostname.
    :rtype: string

.. lua:function:: System.CpuName()

    Retrieve the CPU name

    :return: CPU name.
    :rtype: string

.. lua:function:: System.CpuFreq()

    Retrieve the CPU frequency.

    :return: CPU frequency in MHz.
    :rtype: integer

.. lua:function:: System.CpuCount()

    Retrieve the physical cpu count.

    :return: Physical CPU count.
    :rtype: integer

.. lua:function:: System.CpuThreadCount()

    Retrieve the logical cpu count.

    :return: Logical CPU count.
    :rtype: integer

.. lua:function:: System.TotalMemory()

    Retrieve amount of system memory installed in the machine.

    :return: RAM size in MB.
    :rtype: integer

.. lua:function:: System.FreeMemory()

    Retrieve amount of available memory.

    :return: Available memory in MB.
    :rtype: integer

.. lua:function:: System.OS()

    Retrieve the operating system name.

    :return: Operating system name.
    :rtype: string

.. lua:function:: System.OSVersion()

    Retrieve the operating system version.

    :return: Operating system version.
    :rtype: string

.. lua:function:: System.MachineType()

    Retrieve the machine type.

    :return: Machine type.
    :rtype: string

.. lua:function:: System.Capabilities()

    Retrieve the machine capabilities.

    :return: Machine capabilities flags.
    :rtype: string

.. lua:function:: System.Sleep(seconds)

    Sleep for a specified number of seconds

    :param seconds: The number of real-time seconds to sleep.
    :type seconds: integer

.. WARNING::
    This function will suspend execution.

.. lua:function:: System.MSleep(miliseconds)

    Sleep for a specified number of miliseconds

    :param seconds: The number of real-time miliseconds to sleep.
    :type seconds: integer

.. WARNING::
    This function will suspend execution.

.. lua:function:: System.USleep(microseconds)

    Sleep for a specified number of microseconds

    :param seconds: The number of real-time microseconds to sleep.
    :type seconds: integer

.. WARNING::
    This function will suspend execution.

.. lua:function:: System.Rand()

    Generate a random number

    :return: A random number between 0 and 1
    :rtype: number

Message Box
-----------

.. c:enum:: MessageBoxIcon

    .. c:enumerator::
        NoIcon
        Information
        Warning
        Error

.. lua:module:: MessageBox

.. lua:function:: MessageBox.Show(title, message, icon=MessageBox.NoIcon)

    Display a modal message box

    :param title: Message box title.
    :type title: string
    :param message: Message box text.
    :type message: string
    :param icon: Message box icon.
    :type icon: :c:enum:`MessageBoxIcon` or nil

.. WARNING::
    This will suspend the engine until the user dismisses the message box.

