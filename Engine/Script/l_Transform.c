#include <Script/Script.h>
#include <Scene/Transform.h>

#include "Interface.h"

SIF_FUNC(GetForward)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeTransform *xform = lua_touserdata(vm, 1);

	lua_pushnumber(vm, xform->forward.x);
	lua_pushnumber(vm, xform->forward.y);
	lua_pushnumber(vm, xform->forward.z);

	return 3;
}

SIF_FUNC(GetRight)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeTransform *xform = lua_touserdata(vm, 1);

	lua_pushnumber(vm, xform->right.x);
	lua_pushnumber(vm, xform->right.y);
	lua_pushnumber(vm, xform->right.z);

	return 3;
}

SIF_FUNC(GetUp)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeTransform *xform = lua_touserdata(vm, 1);

	lua_pushnumber(vm, xform->up.x);
	lua_pushnumber(vm, xform->up.y);
	lua_pushnumber(vm, xform->up.z);

	return 3;
}

SIF_FUNC(IsDirty)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeTransform *xform = lua_touserdata(vm, 1);

	lua_pushboolean(vm, xform->dirty);
	return 1;
}

SIF_FUNC(Move)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeVec3 movement =
	{
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	};

	xform_move(lua_touserdata(vm, 1), &movement);
	return 0;
}

SIF_FUNC(Rotate)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	const float angle = (float)luaL_checknumber(vm, 2);
	struct NeVec3 axis =
	{
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4),
		(float)luaL_checknumber(vm, 5)
	};

	xform_rotate(lua_touserdata(vm, 1), angle, &axis);
	return 0;
}

SIF_FUNC(Scale)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeVec3 scale =
	{
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	};

	xform_scale(lua_touserdata(vm, 1), &scale);
	return 0;
}

SIF_FUNC(SetPosition)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeVec3 pos =
	{
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	};

	xform_set_pos(lua_touserdata(vm, 1), &pos);
	return 0;
}

SIF_FUNC(SetScale)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeVec3 scale =
	{
		(float)luaL_checknumber(vm, 2),
		(float)luaL_checknumber(vm, 3),
		(float)luaL_checknumber(vm, 4)
	};

	xform_set_scale(lua_touserdata(vm, 1), &scale);
	return 0;
}

SIF_FUNC(UpdateOrientation)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	xform_update_orientation(lua_touserdata(vm, 1));
	return 0;
}

SIF_FUNC(GetPosition)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeTransform *xform = lua_touserdata(vm, 1);

	lua_pushnumber(vm, xform->position.x);
	lua_pushnumber(vm, xform->position.y);
	lua_pushnumber(vm, xform->position.z);

	return 3;
}

SIF_FUNC(GetRotation)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeTransform *xform = lua_touserdata(vm, 1);

	lua_pushnumber(vm, xform->rotation.x);
	lua_pushnumber(vm, xform->rotation.y);
	lua_pushnumber(vm, xform->rotation.z);
	lua_pushnumber(vm, xform->rotation.w);

	return 4;
}

SIF_FUNC(GetRotationAngles)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	float x, y, z;
	struct NeTransform *xform = lua_touserdata(vm, 1);
	xform_rotation_angles_f(xform, &x, &y, &z);

	lua_pushnumber(vm, x);
	lua_pushnumber(vm, y);
	lua_pushnumber(vm, z);

	return 3;
}

SIF_FUNC(GetScale)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	struct NeTransform *xform = lua_touserdata(vm, 1);

	lua_pushnumber(vm, xform->scale.x);
	lua_pushnumber(vm, xform->scale.y);
	lua_pushnumber(vm, xform->scale.z);

	return 3;
}

SIF_FUNC(MoveForward)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	xform_move_forward(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(MoveBackward)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	xform_move_backward(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(MoveRight)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	xform_move_right(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(MoveLeft)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	xform_move_left(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(MoveUp)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	xform_move_up(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2));
	return 0;
}

SIF_FUNC(MoveDown)
{
	if (!lua_islightuserdata(vm, 1))
		luaL_argerror(vm, 1, "Must be light user data");

	xform_move_down(lua_touserdata(vm, 1), (float)luaL_checknumber(vm, 2));
	return 0;
}

void
SIface_OpenTransform(lua_State *vm)
{
	luaL_Reg reg[] =
	{
		SIF_REG(GetForward),
		SIF_REG(GetRight),
		SIF_REG(GetUp),
		SIF_REG(IsDirty),
		SIF_REG(UpdateOrientation),

		SIF_REG(Move),
		SIF_REG(Rotate),
		SIF_REG(Scale),

		SIF_REG(SetPosition),
		SIF_REG(SetScale),

		SIF_REG(GetPosition),
		SIF_REG(GetRotation),
		SIF_REG(GetRotationAngles),
		SIF_REG(GetScale),

		SIF_REG(MoveForward),
		SIF_REG(MoveBackward),
		SIF_REG(MoveRight),
		SIF_REG(MoveLeft),
		SIF_REG(MoveUp),
		SIF_REG(MoveDown),
		SIF_ENDREG()
	};

	luaL_newlib(vm, reg);
	lua_setglobal(vm, "Xform");
}
