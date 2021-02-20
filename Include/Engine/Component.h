#ifndef _E_COMPONENT_H_
#define _E_COMPONENT_H_

#include <Engine/Types.h>

#define COMPONENT_BASE		\
	void *_self;			\
	EntityHandle _owner

ENGINE_API extern struct Scene *Scn_activeScene;

CompHandle E_CreateComponentS(struct Scene *s, const wchar_t *typeName, EntityHandle owner, const void **args);
static inline CompHandle
E_CreateComponent(const wchar_t *typeName, EntityHandle owner, const void **args) { return E_CreateComponentS(Scn_activeScene, typeName, owner, args); }

CompHandle E_CreateComponentIdS(struct Scene *s, CompTypeId id, EntityHandle owner, const void **args);
static inline CompHandle
E_CreateComponentId(CompTypeId id, EntityHandle owner, const void **args) { return E_CreateComponentIdS(Scn_activeScene, id, owner, args); }

void E_DestroyComponentS(struct Scene *s, CompHandle comp);
static inline void E_DestroyComponent(CompHandle comp) { E_DestroyComponentS(Scn_activeScene, comp); }

void *E_ComponentPtrS(struct Scene *s, CompHandle comp);
static inline void *E_ComponentPtr(CompHandle comp) { return E_ComponentPtrS(Scn_activeScene, comp); }

CompHandle E_ComponentHandle(void *ptr);

CompTypeId E_ComponentTypeS(struct Scene *s, CompHandle comp);
static inline CompTypeId E_ComponentType(CompHandle comp) { return E_ComponentTypeS(Scn_activeScene, comp); }

CompTypeId E_ComponentTypeId(const wchar_t *type_name);

size_t E_ComponentCountS(struct Scene *s, CompTypeId type);
static inline size_t E_ComponentCount(CompTypeId type) { return E_ComponentCountS(Scn_activeScene, type); }

EntityHandle E_ComponentOwnerS(struct Scene *s, CompHandle comp);
static inline EntityHandle E_ComponentOwner(CompHandle comp) { return E_ComponentOwnerS(Scn_activeScene, comp); }

void E_SetComponentOwnerS(struct Scene *s, CompHandle comp, EntityHandle owner);
static inline void E_SetComponentOwner(CompHandle comp, EntityHandle owner) { E_SetComponentOwnerS(Scn_activeScene, comp, owner); }

const struct Array *E_GetAllComponentsS(struct Scene *s, CompTypeId type);
static inline const struct Array *E_GetAllComponents(CompTypeId type) { return E_GetAllComponentsS(Scn_activeScene, type); }

bool E_RegisterComponent(const wchar_t *name, size_t size, size_t alignment, CompInitProc init, CompTermProc release);

#define REGISTER_COMPONENT(name, type, init_proc, release_proc)

#endif /* _E_COMPONENT_H_ */
