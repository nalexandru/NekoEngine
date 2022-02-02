#ifndef _NE_ENGINE_ENTITY_H_
#define _NE_ENGINE_ENTITY_H_

#include <stdarg.h>

#include <Engine/Types.h>

#define MAX_ENTITY_NAME				64
#define MAX_ENTITY_COMPONENTS		30
#define ES_INVALID_COMPONENT		-1
#define ES_INVALID_COMPONENT_TYPE	-1
#define ES_INVALID_ENTITY			NULL
#define ES_INVALID_ECSYS			-1

struct NeEntityCompInfo
{
	const char *type;
	const void **args;
};

struct NeEntityComp
{
	NeCompTypeId type;
	NeCompHandle handle;
};

ENGINE_API extern struct NeScene *Scn_activeScene;

NeEntityHandle E_CreateEntityS(struct NeScene *s, const char *type);
static inline NeEntityHandle E_CreateEntity(const char *type) { return E_CreateEntityS(Scn_activeScene, type); }

NeEntityHandle E_CreateEntityWithArgsS(struct NeScene *s, const NeCompTypeId *compTypes, const void ***compArgs, uint8_t typeCount);
static inline NeEntityHandle E_CreateEntityWithArgs(const NeCompTypeId *compTypes, const void ***compArgs, uint8_t typeCount)
{ return E_CreateEntityWithArgsS(Scn_activeScene, compTypes, compArgs, typeCount); }

NeEntityHandle E_CreateEntityVS(struct NeScene *s, int count, const struct NeEntityCompInfo *info);
static inline NeEntityHandle E_CreateEntityV(int count, const struct NeEntityCompInfo *info) { return E_CreateEntityVS(Scn_activeScene, count, info); }

NeEntityHandle E_CreateEntityWithComponentsS(struct NeScene *s, int count, ...);

bool E_AddComponentS(struct NeScene *s, NeEntityHandle ent, NeCompTypeId type, NeCompHandle comp);
static inline bool E_AddComponent(NeEntityHandle ent, NeCompTypeId type, NeCompHandle comp) { return E_AddComponentS(Scn_activeScene, ent, type, comp); }

bool E_AddNewComponentS(struct NeScene *s, NeEntityHandle ent, NeCompTypeId type, const void **args);
static inline bool E_AddNewComponent(NeEntityHandle ent, NeCompTypeId type, const void **args) { return E_AddNewComponentS(Scn_activeScene, ent, type, args); }

void *E_GetComponentS(struct NeScene *s, NeEntityHandle ent, NeCompTypeId type);
static inline void *E_GetComponent(NeEntityHandle ent, NeCompTypeId type) { return Scn_activeScene ? E_GetComponentS(Scn_activeScene, ent, type) : NULL; }

//
// Retrieve the list of components; will initialize `comp` with MH_Frame memory.
//
void E_GetComponentsS(struct NeScene *s, NeEntityHandle ent, struct NeArray *comp);
static inline void E_GetComponents(NeEntityHandle ent, struct NeArray *comp) { E_GetComponentsS(Scn_activeScene, ent, comp); }

void E_RemoveComponentS(struct NeScene *s, NeEntityHandle ent, NeCompTypeId type);
static inline void E_RemoveComponent(NeEntityHandle ent, NeCompTypeId type) { E_RemoveComponentS(Scn_activeScene, ent, type); }

void E_DestroyEntityS(struct NeScene *s, NeEntityHandle ent);
static inline void E_DestroyEntity(NeEntityHandle ent) { E_DestroyEntityS(Scn_activeScene, ent); }

void *E_EntityPtrS(struct NeScene *s, NeEntityHandle ent);
static inline void *E_EntityPtr(NeEntityHandle ent) { return E_EntityPtrS(Scn_activeScene, ent); }

bool E_RegisterEntityType(const char *name, const NeCompTypeId *compTypes, uint8_t type_count);

uint32_t E_EntityCountS(struct NeScene *s);
static inline uint32_t E_EntityCount(void) { return E_EntityCountS(Scn_activeScene); }

const char *E_EntityName(NeEntityHandle ent);
void E_RenameEntity(NeEntityHandle ent, const char *name);

NeEntityHandle E_FindEntityS(struct NeScene *s, const char *name);
static inline NeEntityHandle E_FindEntity(const char *name) { return E_FindEntityS(Scn_activeScene, name); }

#endif /* _NE_ENGINE_ENTITY_H_ */
