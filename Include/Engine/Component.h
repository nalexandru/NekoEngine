#ifndef _NE_ENGINE_COMPONENT_H_
#define _NE_ENGINE_COMPONENT_H_

#include <Engine/Types.h>

#define NE_COMPONENT_BASE		\
    uint64_t _valid : 1;		\
    uint64_t _enabled : 1;		\
    uint64_t _handleId : 30;	\
	uint64_t _typeId : 24;		\
	uint64_t _sceneId : 8;		\
	NeEntityHandle _owner

struct NeComponentCreationData
{
	NeCompTypeId type;
	NeCompHandle handle;
	NeEntityHandle owner;
	void *ptr;
};

struct NeCompBase
{
	NE_COMPONENT_BASE;
};

struct NeScene *Scn_GetScene(uint8_t);
ENGINE_API extern struct NeScene *Scn_activeScene;

NeCompHandle E_CreateComponentS(struct NeScene *s, const char *typeName, NeEntityHandle owner, const void **args);
static inline NeCompHandle
E_CreateComponent(const char *typeName, NeEntityHandle owner, const void **args) { return E_CreateComponentS(Scn_activeScene, typeName, owner, args); }

NeCompHandle E_CreateComponentIdS(struct NeScene *s, NeCompTypeId typeId, NeEntityHandle owner, const void **args);
static inline NeCompHandle
E_CreateComponentId(NeCompTypeId id, NeEntityHandle owner, const void **args) { return E_CreateComponentIdS(Scn_activeScene, id, owner, args); }

void E_DestroyComponentS(struct NeScene *s, NeCompHandle comp);
static inline void E_DestroyComponent(NeCompHandle comp) { E_DestroyComponentS(Scn_activeScene, comp); }

void *E_ComponentPtrS(struct NeScene *s, NeCompHandle comp);
static inline void *E_ComponentPtr(NeCompHandle comp) { return E_ComponentPtrS(Scn_activeScene, comp); }

NeCompTypeId E_ComponentTypeS(struct NeScene *s, NeCompHandle comp);
static inline NeCompTypeId E_ComponentType(NeCompHandle comp) { return E_ComponentTypeS(Scn_activeScene, comp); }

NeCompTypeId E_ComponentTypeId(const char *typeName);
const char *E_ComponentTypeName(NeCompTypeId typeId);
size_t E_ComponentTypeSize(NeCompTypeId typeId);

size_t E_ComponentCountS(struct NeScene *s, NeCompTypeId type);
static inline size_t E_ComponentCount(NeCompTypeId type) { return E_ComponentCountS(Scn_activeScene, type); }

NeEntityHandle E_ComponentOwnerS(struct NeScene *s, NeCompHandle comp);
static inline NeEntityHandle E_ComponentOwner(NeCompHandle comp) { return E_ComponentOwnerS(Scn_activeScene, comp); }

void E_SetComponentOwnerS(struct NeScene *s, NeCompHandle comp, NeEntityHandle owner);
static inline void E_SetComponentOwner(NeCompHandle comp, NeEntityHandle owner) { E_SetComponentOwnerS(Scn_activeScene, comp, owner); }

const struct NeArray *E_GetAllComponentsS(struct NeScene *s, NeCompTypeId type);
static inline const struct NeArray *E_GetAllComponents(NeCompTypeId type) { return E_GetAllComponentsS(Scn_activeScene, type); }

static inline NeCompHandle E_ComponentHandle(struct NeCompBase *comp) { return comp->_handleId | (uint64_t)comp->_typeId << 32; }
static inline NeEntityHandle E_ComponentOwnerHandle(struct NeCompBase *comp) { return comp->_owner; }
static inline struct NeScene *E_ComponentScene(struct NeCompBase *comp) { return Scn_GetScene((uint8_t)comp->_sceneId); }

bool E_RegisterComponent(const char *name, size_t size, size_t alignment, NeCompInitProc init, NeCompTermProc release);

#define E_REGISTER_COMPONENT(name, type, alignment, init, release) \
	E_INITIALIZER(_NeCompRegister_ ## name) { E_RegisterComponent(name, sizeof(type), alignment, (NeCompInitProc)init, (NeCompTermProc)release); }

#endif /* _NE_ENGINE_COMPONENT_H_ */
