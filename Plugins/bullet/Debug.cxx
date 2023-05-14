#include "Plugin.h"

void
NeDebugDraw::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{

}

void
NeDebugDraw::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
{

}

void
NeDebugDraw::reportErrorWarning(const char *warningString)
{

}

void
NeDebugDraw::draw3dText(const btVector3 &location, const char *textString)
{

}

void
NeDebugDraw::setDebugMode(int debugMode)
{
	_debugMode = debugMode;
}

int
NeDebugDraw::getDebugMode() const
{
	return _debugMode;
}
