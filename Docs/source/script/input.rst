Input Module
============

This module contains functions for debugging.

.. note:: This module is available only in Debug builds of the Engine.

Usage
-----

    require 'NeInput'

Aliases
-------

.. lua:alias:: ButtonMap = table<button:KeyCode, controller:integer>

    The ``controller`` field is optional.

.. lua:alias:: ButtonMapping = table<primary:ButtonMap, secondary:ButtonMap>

    The ``secondary`` field is optional.

.. lua:alias:: AxisMap = table<axis:AxisId, controller:integer>

    The ``controller`` field is optional.

.. lua:alias:: AxisMapping = table<primary:AxisMap, secondary:AxisMap>

    The ``secondary`` field is optional.

Functions
---------
.. lua:module:: Input

.. lua:function:: Input.Button(map)

    Test if a mapped button is pressed.

    :param map: Button map
    :type map: integer
    :return: Button state.
    :rtype: boolean

.. lua:function:: Input.ButtonDown(map)

    Test if a mapped button was pressed during the current frame.

    :param map: Button map
    :type map: integer
    :return: Button state.
    :rtype: boolean

.. lua:function:: Input.ButtonUp(map)

    Test if a mapped button was released durig the current frame.

    :param map: Button map
    :type map: integer
    :return: Button state.
    :rtype: boolean

.. lua:function:: Input.Axis(map)

    Read a mapped axis value.

    :param map: Axis map
    :type map: integer
    :return: Axis value between -1.0 and 1.0.
    :rtype: number

.. lua:function:: Input.PointerPosition([x, y])

    Get or set the pointer position.

    :param x: x coordinate.
    :type x: integer
    :param y: y coordinate.
    :type y: integer
    :return: Pointer position as x, y if called without parameters.
    :rtype: x, y numbers

.. lua:function:: Input.CapturePointer(capture)

    Capture or release the mouse pointer.

    :param capture: true to capture, false to release.
    :type capture: boolean

.. lua:function:: Input.ShowPointer(show)

    Show or hide the mouse pointer.

    :param show: true to show, false to hide.
    :type show: boolean

.. lua:function:: Input.UnmappedButton(keycode)

    Test if a button is pressed.

    :param keycode: Button key code
    :type keycode: KeyCode
    :return: Button state.
    :rtype: boolean

.. lua:function:: Input.UnmappedButtonDown(keycode)

    Test if a button was pressed during the current frame.

    :param keycode: Button key code
    :type keycode: KeyCode
    :return: Button state.
    :rtype: boolean

.. lua:function:: Input.UnmappedButtonUp(keycode)

    Test if a button was released durig the current frame.

    :param keycode: Button key code
    :type keycode: KeyCode
    :return: Button state.
    :rtype: boolean

.. lua:function:: Input.UnmappedAxis(axis)

    Read an axis value.

    :param axis: Axis code
    :type axis: integer
    :return: Axis value between -1.0 and 1.0.
    :rtype: number

.. lua:function:: Input.EnableMouseAxis(enable)

    Enable or disable the mouse axis. When enabled, the engine will set mouse movement deltas in the MouseX and MouseY axis. In order to do this, the pointer will be moved to the center of the screen at the start of every frame.

    :param enable: true to show, false to hide.
    :type enable: boolean

    .. note:: When enabling the mouse axis the pointer will be captured and hidden. When it is disabled, it will be released and shown.

.. lua:function:: Input.CreateVirtualAxis(name, min, max)

    Create a virtual axis using two buttons. While a button is pressed, the axis will have the value of -1.0 or 1.0. If both buttons are not pressed, the value will be 0.0.

    :param name: Axis name.
    :type name: string
    :param min: Key code for setting the axis to -1.0.
    :type min: KeyCode
    :param min: Key code for setting the axis to 1.0.
    :type min: KeyCode

.. lua:function:: Input.GetVirtualAxis()

    Get a virtual axis id.

    :param name: Axis name.
    :type name: string
    :return: Axis code.
    :rtype: integer

.. lua:function:: Input.CreateMap(name)

    Create an input map.

    :param name: Map name.
    :type name: string
    :return: Map id.
    :rtype: integer

.. lua:function:: Input.MapButton(map, mapping)

    Assign buttons to an input map.

    :param map: Map id.
    :type map: integer
    :param mapping: Mapping table.
    :type mapping: ButtonMapping

.. lua:function:: Input.MapAxis(map, mapping)

    Assign axis to an input map.

    :param map: Map id.
    :type map: integer
    :param mapping: Mapping table.
    :type mapping: AxisMapping

Key Codes
---------

    * KeyCode.0
    * KeyCode.1
    * KeyCode.2
    * KeyCode.3
    * KeyCode.4
    * KeyCode.5
    * KeyCode.6
    * KeyCode.7
    * KeyCode.8
    * KeyCode.9
    * KeyCode.A
    * KeyCode.B
    * KeyCode.C
    * KeyCode.D
    * KeyCode.E
    * KeyCode.F
    * KeyCode.G
    * KeyCode.H
    * KeyCode.I
    * KeyCode.J
    * KeyCode.K
    * KeyCode.L
    * KeyCode.M
    * KeyCode.N
    * KeyCode.O
    * KeyCode.P
    * KeyCode.Q
    * KeyCode.R
    * KeyCode.S
    * KeyCode.T
    * KeyCode.U
    * KeyCode.V
    * KeyCode.W
    * KeyCode.X
    * KeyCode.Y
    * KeyCode.Z
    * KeyCode.Up
    * KeyCode.Down
    * KeyCode.Left
    * KeyCode.Right
    * KeyCode.Space
    * KeyCode.Tilde
    * KeyCode.Tab
    * KeyCode.CapsLock
    * KeyCode.Return
    * KeyCode.LeftShift
    * KeyCode.RightShift
    * KeyCode.LeftAlt
    * KeyCode.RightAlt
    * KeyCode.LeftSuper
    * KeyCode.RightSuper
    * KeyCode.LeftControl
    * KeyCode.RightControl
    * KeyCode.Slash
    * KeyCode.Backslash
    * KeyCode.Comma
    * KeyCode.Period
    * KeyCode.Semicolon
    * KeyCode.Quote
    * KeyCode.Delete
    * KeyCode.Equal
    * KeyCode.Minus
    * KeyCode.Backspace
    * KeyCode.LeftBracket
    * KeyCode.RightBracket
    * KeyCode.Escape
    * KeyCode.PageUp
    * KeyCode.PageDown
    * KeyCode.Home
    * KeyCode.End
    * KeyCode.Insert
    * KeyCode.Scroll
    * KeyCode.PrintScreen
    * KeyCode.Pause
    * KeyCode.F1
    * KeyCode.F2
    * KeyCode.F3
    * KeyCode.F4
    * KeyCode.F5
    * KeyCode.F6
    * KeyCode.F7
    * KeyCode.F8
    * KeyCode.F9
    * KeyCode.F10
    * KeyCode.F11
    * KeyCode.F12
    * KeyCode.F13
    * KeyCode.F14
    * KeyCode.F15
    * KeyCode.F16
    * KeyCode.F17
    * KeyCode.F18
    * KeyCode.F19
    * KeyCode.F20
    * KeyCode.F21
    * KeyCode.F22
    * KeyCode.F23
    * KeyCode.F24
    * KeyCode.NumLock
    * KeyCode.Numpad0
    * KeyCode.Numpad1
    * KeyCode.Numpad2
    * KeyCode.Numpad3
    * KeyCode.Numpad4
    * KeyCode.Numpad5
    * KeyCode.Numpad6
    * KeyCode.Numpad7
    * KeyCode.Numpad8
    * KeyCode.Numpad9
    * KeyCode.NumpadPlus
    * KeyCode.NumpadMinus
    * KeyCode.NumpadDecimal
    * KeyCode.NumpadDivide
    * KeyCode.NumpadMultiply
    * KeyCode.NumpadReturn
    * KeyCode.NumpadClear
    * KeyCode.LeftMouse
    * KeyCode.RightMouse
    * KeyCode.MiddleMouse
    * KeyCode.Mouse4
    * KeyCode.Mouse5
    * KeyCode.Gpad_Up
    * KeyCode.Gpad_Down
    * KeyCode.Gpad_Left
    * KeyCode.Gpad_Right
    * KeyCode.Gpad_Start
    * KeyCode.Gpad_Back
    * KeyCode.Gpad_LeftThumb
    * KeyCode.Gpad_RightThumb
    * KeyCode.Gpad_LeftBumper
    * KeyCode.Gpad_RightBumper
    * KeyCode.Gpad_A
    * KeyCode.Gpad_B
    * KeyCode.Gpad_X
    * KeyCode.Gpad_Y

Axis Codes
----------

    * Axis.LeftStickX
    * Axis.LeftStickY
    * Axis.RightStickX
    * Axis.RightStickY
    * Axis.LeftTrigger
    * Axis.RightTrigger
    * Axis.GyroscopeX
    * Axis.GyroscopeY
    * Axis.GyroscopeZ
    * Axis.MouseX
    * Axis.MouseY
    * Axis.MouseWheel
    * Axis.Virtual0
    * Axis.Virtual1
    * Axis.Virtual2
    * Axis.Virtual3
    * Axis.Virtual4
    * Axis.Virtual5
    * Axis.Virtual6
    * Axis.Virtual7
    * Axis.Virtual8
    * Axis.Virtual9
