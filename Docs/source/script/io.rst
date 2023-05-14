I/O Module
==========

This module contains I/O functions.

Usage
-----

    require 'NeIO'

Constants
---------

	lua_newtable(vm);
	{
		lua_pushinteger(vm, IO_READ);
		lua_setglobal(vm, "Read");
		lua_pushinteger(vm, IO_WRITE);
		lua_setglobal(vm, "Write");
		lua_pushinteger(vm, IO_APPEND);
		lua_setglobal(vm, "Append");
	}
	lua_setglobal(vm, "OpenMode");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, IO_SEEK_SET);
		lua_setglobal(vm, "Set");

		lua_pushinteger(vm, IO_SEEK_CUR);
		lua_setglobal(vm, "Cur");

		lua_pushinteger(vm, IO_SEEK_END);
		lua_setglobal(vm, "End");
	}
	lua_setglobal(vm, "Seek");

	lua_newtable(vm);
	{
		lua_pushinteger(vm, WD_Data);
		lua_setglobal(vm, "Data");

		lua_pushinteger(vm, WD_Save);
		lua_setglobal(vm, "Save");

		lua_pushinteger(vm, WD_Temp);
		lua_setglobal(vm, "Temp");

		lua_pushinteger(vm, WD_Config);
		lua_setglobal(vm, "Config");
	}
	lua_setglobal(vm, "WriteDirectory");

Methods
-------
.. lua:class:: NeStream

    .. note:: Binary streams are not supported.

    .. lua:method:: Read(charCount)

        Read a number of characters from the stream

        :param charCount: Number of characters to read.
        :type charCount: integer
        :return: Characters read.
        :rtype: string

    .. lua:method:: ReadLine

        Read a line from the stream

        .. note:: The maximum line length is set by the ``Script::ReadBufferSize`` cvar. Implicitly, it has a value of 1k.

        :return: Line read.
        :rtype: string

    .. lua:method:: Write(data)

        Write to the stream

        :param name: Data to write.
        :type name: string
        :return: True if successful.
        :rtype: boolean

    .. lua:method:: Tell

        Current position in the stream.

        :return: Current position in bytes.
        :rtype: integer

    .. lua:method:: Seek(offset, whence)

        Set the stream position

        :param offset: Variable name
        :type offset: string
        :param whence: Variable name
        :type whence: string
        :return: Length of the stream in bytes.
        :rtype: integer

    .. lua:method:: Length

        Stream length.

        :return: Length of the stream in bytes.
        :rtype: integer

    .. lua:method:: End

        Test if the stream is at the end.

        :return: True if the stream is at the end.
        :rtype: boolean

    .. lua:method:: Close

        Close the stream

Functions
---------
.. lua:module:: IO

.. lua:function:: IO.FileStream(path, mode)

    Open a file stream.

    :param path: Absolute path.
    :type path: string
    :param mode: Open mode.
    :type mode: integer
    :return: File stream.
    :rtype: :lua:class:`NeStream`

.. lua:function:: IO.FileExists(path)

    Check if a file exists.

    :param path: Absolute path.
    :type path: string
    :return: True if the file exists.
    :rtype: boolean

.. lua:function:: IO.ListFiles()

    List files in a directory.

    :param path: Absolute path.
    :type path: string
    :return: Array of strings with the file names.
    :rtype: array

.. lua:function:: IO.IsDirectory()

    Check if a path is a directory.

    :param path: Absolute path.
    :type path: string
    :return: True if the path exists and is a directory.
    :rtype: boolean

.. lua:function:: IO.EnableWrite(writeDirectory)

    Enable writing to a directory.

    :param writeDirectory: Writable directory id.
    :type writeDirectory: integer
    :return: True if writing was enabled.
    :rtype: boolean

.. lua:function:: IO.DisableWrite()

    Disable writing.

.. lua:function:: IO.CreateDirectory(path)

    Create a directory.

    .. note:: Writing must be enabled to the parent directory.

    :param path: Absolute path.
    :type path: string
    :return: True if the directory was created.
    :rtype: boolean
