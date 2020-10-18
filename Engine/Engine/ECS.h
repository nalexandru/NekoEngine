#ifndef _ECS_H_
#define _ECS_H_

#include <Runtime/Array.h>

#include <Engine/Types.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>

#ifdef __cplusplus
extern "C" {
#endif

struct EntityComp
{
	CompTypeId type;
	CompHandle handle;
};

struct Entity
{
	size_t id;
	uint8_t comp_count;
	struct EntityComp comp[MAX_ENTITY_COMPONENTS];
};

struct EntityType
{
	uint64_t hash;
	uint8_t comp_count;
	CompTypeId comp_types[MAX_ENTITY_COMPONENTS];
};

struct CompHandleData
{
	CompHandle handle;
	CompTypeId type;
	size_t index;
};

struct CompBase
{
	COMPONENT_BASE;
};

struct CompType
{
	size_t size, alignment;
	uint64_t hash;
	CompInitProc create;
	CompTermProc destroy;
};

struct ECSystem
{
	ECSysExecProc exec;
	CompTypeId *comp_types;
	uint64_t name_hash;
	uint64_t group_hash;
	int32_t priority;
	size_t type_count;
};

typedef bool(*CompSysRegisterAllProc)(void);

bool E_InitComponents(void);
void E_TermComponents(void);

bool E_InitEntities(void);
void E_TermEntities(void);

bool E_InitSceneComponents(struct Scene *s);
void E_TermSceneComponents(struct Scene *s);

bool E_InitSceneEntities(struct Scene *s);
void E_TermSceneEntities(struct Scene *s);

bool E_InitECSystems(void);
void E_TermECSystems(void);

#ifdef __cplusplus
}
#endif

#endif /* _ECS_H_ */
