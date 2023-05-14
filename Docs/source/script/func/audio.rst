Transform
=========


Functions
---------

.. function:: Audio.DistanceModel()

    Retrieve the time elapsed since the engine was started.

    :return: Time elapsed.
    :rtype: integer


ProjectionType Perspective, Orphographics

	lua_newtable(vm);
	{
		lua_pushinteger(vm, DM_InverseDistance);
		lua_setfield(vm, -2, "Inverse");

		lua_pushinteger(vm, DM_InverseDistanceClamped);
		lua_setfield(vm, -2, "InverseClamped");

		lua_pushinteger(vm, DM_LinearDistance);
		lua_setfield(vm, -2, "Linear");

		lua_pushinteger(vm, DM_LinearDistanceClamped);
		lua_setfield(vm, -2, "LinearClamped");

		lua_pushinteger(vm, DM_ExponentDistance);
		lua_setfield(vm, -2, "Exponent");

		lua_pushinteger(vm, DM_ExponentDistanceClamped);
		lua_setfield(vm, -2, "ExponentClamped");
	}
	lua_setglobal(vm, "DistanceModel");
