#ifndef _NE_ENGINE_ECS_H_
#define _NE_ENGINE_ECS_H_

#include <Runtime/Array.h>

#include <Engine/Types.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Script/Script.h>

typedef bool (*NeScriptCompInitProc)(void *, const void **, const char *);
typedef void (*NeScriptCompTermProc)(void *, const char *);

struct NeEntity
{
	size_t id;
	uint32_t compCount;
	struct NeEntityComp comp[MAX_ENTITY_COMPONENTS];
	uint64_t hash;
	char name[MAX_ENTITY_NAME];
};

struct NeEntityType
{
	uint64_t hash;
	uint32_t compCount;
	NeCompTypeId compTypes[MAX_ENTITY_COMPONENTS];
	struct NeArray initialArguments[MAX_ENTITY_COMPONENTS];
};

struct NeCompType
{
	size_t size, alignment;
	uint64_t hash;
	union {
		NeCompInitProc init;
		NeScriptCompInitProc initScript;
	};
	union {
		NeCompTermProc term;
		NeScriptCompTermProc termScript;
	};
	char *script, name[MAX_ENTITY_NAME];
};

struct NeECSystem
{
	NeECSysExecProc exec;
	lua_State *vm;
	NeCompTypeId *compTypes;
	uint64_t nameHash;
	uint64_t groupHash;
	int32_t priority;
	size_t typeCount;
	bool singleThread;
};

typedef bool (*NeCompSysRegisterAllProc)(void);

size_t E_ComponentSize(struct NeScene *s, const struct NeCompBase *comp);

bool E_InitComponents(void);
void E_TermComponents(void);

bool E_InitEntities(void);
void E_TermEntities(void);

bool E_InitSceneComponents(struct NeScene *s);
void E_TermSceneComponents(struct NeScene *s);

bool E_InitSceneEntities(struct NeScene *s);
void E_TermSceneEntities(struct NeScene *s);

bool E_InitECSystems(void);
void E_TermECSystems(void);

#endif /* _NE_ENGINE_ECS_H_ */
