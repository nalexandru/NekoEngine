#ifndef _NE_ENGINE_ENTITY_H_
#define _NE_ENGINE_ENTITY_H_

#include <stdarg.h>

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ENTITY_NAME				64
#define MAX_ENTITY_COMPONENTS		30
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

NeEntityHandle E_CreateEntityS(struct NeScene *s, const char *name, const char *type);
static inline NeEntityHandle E_CreateEntity(const char *name, const char *type) { return E_CreateEntityS(Scn_activeScene, name, type); }

NeEntityHandle E_CreateEntityWithArgsS(struct NeScene *s, const char *name, const NeCompTypeId *compTypes, const void ***compArgs, uint8_t typeCount);
static inline NeEntityHandle E_CreateEntityWithArgs(const char *name, const NeCompTypeId *compTypes, const void ***compArgs, uint8_t typeCount)
{ return E_CreateEntityWithArgsS(Scn_activeScene, name, compTypes, compArgs, typeCount); }

NeEntityHandle E_CreateEntityWithArgArrayS(struct NeScene *s, const char *name, const NeCompTypeId *compTypes, const struct NeArray *compArgs, uint8_t typeCount);
static inline NeEntityHandle E_CreateEntityWithArgArray(const char *name, const NeCompTypeId *compTypes, const struct NeArray *compArgs, uint8_t typeCount)
{ return E_CreateEntityWithArgArrayS(Scn_activeScene, name, compTypes, compArgs, typeCount); }

NeEntityHandle E_CreateEntityVS(struct NeScene *s, const char *name, int count, const struct NeEntityCompInfo *info);
static inline NeEntityHandle E_CreateEntityV(const char *name, int count, const struct NeEntityCompInfo *info) { return E_CreateEntityVS(Scn_activeScene, name, count, info); }

NeEntityHandle E_CreateEntityWithComponentsS(struct NeScene *s, const char *name, int count, ...);

bool E_AddComponentS(struct NeScene *s, NeEntityHandle ent, NeCompTypeId type, NeCompHandle comp);
static inline bool E_AddComponent(NeEntityHandle ent, NeCompTypeId type, NeCompHandle comp) { return E_AddComponentS(Scn_activeScene, ent, type, comp); }

bool E_AddNewComponentS(struct NeScene *s, NeEntityHandle ent, NeCompTypeId type, const void **args);
static inline bool E_AddNewComponent(NeEntityHandle ent, NeCompTypeId type, const void **args) { return E_AddNewComponentS(Scn_activeScene, ent, type, args); }

void *E_GetComponentS(struct NeScene *s, NeEntityHandle ent, NeCompTypeId type);
static inline void *E_GetComponent(NeEntityHandle ent, NeCompTypeId type) { return Scn_activeScene ? E_GetComponentS(Scn_activeScene, ent, type) : NULL; }

NeCompHandle E_GetComponentHandleS(struct NeScene *s, NeEntityHandle ent, NeCompTypeId type);
static inline NeCompHandle E_GetComponentHandle(NeEntityHandle ent, NeCompTypeId type) { return Scn_activeScene ? E_GetComponentHandleS(Scn_activeScene, ent, type) : E_INVALID_HANDLE; }

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

#ifdef __cplusplus
}
#endif

#endif /* _NE_ENGINE_ENTITY_H_ */

/* NekoEngine
 *
 * Entity.h
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
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
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
