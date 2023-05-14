#ifndef NE_ENGINE_COMPONENT_H
#define NE_ENGINE_COMPONENT_H

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

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

NeCompTypeId E_ComponentTypeId(const char *typeName);
const char *E_ComponentTypeName(NeCompTypeId typeId);
size_t E_ComponentTypeSize(NeCompTypeId typeId);

NeCompHandle E_CreateComponentIdS(struct NeScene *s, NeCompTypeId typeId, NeEntityHandle owner, const void **args);
static inline NeCompHandle
E_CreateComponentId(NeCompTypeId id, NeEntityHandle owner, const void **args) { return E_CreateComponentIdS(Scn_activeScene, id, owner, args); }

static inline NeCompHandle
E_CreateComponentS(struct NeScene *s, const char *typeName, NeEntityHandle owner, const void **args)
{ return E_CreateComponentIdS(s, E_ComponentTypeId(typeName), owner, args); }
static inline NeCompHandle
E_CreateComponent(const char *typeName, NeEntityHandle owner, const void **args) { return E_CreateComponentS(Scn_activeScene, typeName, owner, args); }

void E_DestroyComponentS(struct NeScene *s, NeCompHandle comp);
static inline void E_DestroyComponent(NeCompHandle comp) { E_DestroyComponentS(Scn_activeScene, comp); }

void *E_ComponentPtrS(struct NeScene *s, NeCompHandle comp);
static inline void *E_ComponentPtr(NeCompHandle comp) { return E_ComponentPtrS(Scn_activeScene, comp); }

NeCompTypeId E_ComponentTypeS(struct NeScene *s, NeCompHandle comp);
static inline NeCompTypeId E_ComponentType(NeCompHandle comp) { return E_ComponentTypeS(Scn_activeScene, comp); }

size_t E_ComponentCountS(struct NeScene *s, NeCompTypeId type);
static inline size_t E_ComponentCount(NeCompTypeId type) { return E_ComponentCountS(Scn_activeScene, type); }

NeEntityHandle E_ComponentOwnerS(struct NeScene *s, NeCompHandle comp);
static inline NeEntityHandle E_ComponentOwner(NeCompHandle comp) { return E_ComponentOwnerS(Scn_activeScene, comp); }

void E_SetComponentOwnerS(struct NeScene *s, NeCompHandle comp, NeEntityHandle owner);
static inline void E_SetComponentOwner(NeCompHandle comp, NeEntityHandle owner) { E_SetComponentOwnerS(Scn_activeScene, comp, owner); }

const struct NeArray *E_GetAllComponentsS(struct NeScene *s, NeCompTypeId type);
static inline const struct NeArray *E_GetAllComponents(NeCompTypeId type) { return E_GetAllComponentsS(Scn_activeScene, type); }

#define E_ComponentHandle(comp) ((comp)->_handleId | (uint64_t)(comp)->_typeId << 32)
static inline NeEntityHandle E_ComponentOwnerHandle(struct NeCompBase *comp) { return comp->_owner; }
static inline struct NeScene *E_ComponentScene(struct NeCompBase *comp) { return Scn_GetScene((uint8_t)comp->_sceneId); }

bool E_RegisterComponent(const char *name, size_t size, size_t alignment, NeCompInitProc init, NeCompMessageHandlerProc handler, NeCompTermProc release, NeCompTypeId *id);

#define NE_REGISTER_COMPONENT(name, type, alignment, init, handler, release) \
	NeCompTypeId name ## _ID;\
	NE_INITIALIZER(NeCompRegister_ ## name) { E_RegisterComponent(name, sizeof(type), alignment, (NeCompInitProc)init, (NeCompMessageHandlerProc)handler, (NeCompTermProc)release, &name ## _ID); }

#ifdef __cplusplus
}
#endif

#endif /* NE_ENGINE_COMPONENT_H */

/* NekoEngine
 *
 * Component.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
