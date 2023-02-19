#include "SceneHierarchy.h"

#include <System/Thread.h>
#include <Engine/Events.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Scene/Components.h>
#include <Scene/Transform.h>
#include <Scene/Scene.h>

#include "Inspector.h"

static NeSceneHierarchy *_dlg;

static NeFutex _ftx;
static uint64_t _sceneActivatedHandler, _entityCreateHandler, _entityDestroyHandler, _componentCreateHandler;

bool
GUI_CreateSceneHierarchy(void)
{
	_dlg = new NeSceneHierarchy();
	return _dlg != nullptr;
}

bool
GUI_InitSceneHierarchy(void)
{
	_sceneActivatedHandler = E_RegisterHandler(EVT_SCENE_ACTIVATED,
											   (NeEventHandlerProc)NeSceneHierarchy::_SceneActivated, _dlg);
	_entityCreateHandler = E_RegisterHandler(EVT_ENTITY_CREATED,
											 (NeEventHandlerProc)NeSceneHierarchy::_EntityCreate, _dlg);
	_entityDestroyHandler = E_RegisterHandler(EVT_ENTITY_DESTROYED,
											  (NeEventHandlerProc)NeSceneHierarchy::_EntityDestroy, _dlg);
	_componentCreateHandler = E_RegisterHandler(EVT_COMPONENT_CREATED,
												(NeEventHandlerProc)NeSceneHierarchy::_ComponentCreate, _dlg);

	Sys_InitFutex(&_ftx);

	_dlg->show();

	return true;
}

void
GUI_TermSceneHierarchy(void)
{
	E_UnregisterHandler(_sceneActivatedHandler);
	E_UnregisterHandler(_entityCreateHandler);
	E_UnregisterHandler(_entityDestroyHandler);
	E_UnregisterHandler(_componentCreateHandler);

	Sys_TermFutex(_ftx);

	_dlg->close();
	delete _dlg;
}

NeSceneHierarchy::NeSceneHierarchy(QWidget *parent) : QDialog(parent)
{
	setWindowTitle("Scene Hierarchy");
	setMinimumSize(250, 600);

	_lyt = new QVBoxLayout(this);

	_tree = new QTreeWidget();
	_tree->setColumnCount(1);
	_tree->setHeaderLabel("Scene Hierarchy");

	_lyt->addWidget(_tree);

	connect(_tree, SIGNAL(itemSelectionChanged()), this, SLOT(TreeSelectionChanged()));
}

void
NeSceneHierarchy::TreeSelectionChanged()
{
	Sys_LockFutex(_ftx);

	if (_tree->selectedItems().count())
		GUI_InspectEntity(_tree->selectedItems().first()->data(1, Qt::UserRole).value<NeEntityHandle>());
	else
		GUI_InspectEntity(NULL);

	Sys_UnlockFutex(_ftx);
}

void
NeSceneHierarchy::_AddTransform(const struct NeTransform *xform, QTreeWidgetItem *parent)
{
	QTreeWidgetItem *item = new QTreeWidgetItem();

	item->setText(0, E_EntityName(xform->_owner));
	item->setData(1, Qt::UserRole, QVariant::fromValue(xform->_owner));

	if (parent)
		parent->addChild(item);
	else
		_tree->addTopLevelItem(item);

	Rt_ArrayForEach(xform, &xform->children, const struct NeTransform *)
		_AddTransform(xform, item);
}

NeSceneHierarchy::~NeSceneHierarchy()
{
	delete _tree;
}

void
NeSceneHierarchy::_EntityCreate(NeSceneHierarchy *shd, NeEntityHandle eh)
{
	Sys_LockFutex(_ftx);

	const struct NeTransform *xform = (struct NeTransform *)E_GetComponent(eh, E_ComponentTypeId(TRANSFORM_COMP));
	if (!xform)
		return;

	QList<QTreeWidgetItem *> items = shd->_tree->findItems(E_EntityName(eh), Qt::MatchExactly);
	if (!items.empty())
		return;

	QTreeWidgetItem *parentItem = nullptr;
	if (xform->parent != E_INVALID_HANDLE) {
		struct NeTransform *parent = (struct NeTransform *)E_ComponentPtrS(Scn_GetScene((uint8_t)xform->_sceneId), xform->parent);
		QList<QTreeWidgetItem *> parentItems = shd->_tree->findItems(E_EntityName(parent->_owner), Qt::MatchExactly);
		if (!parentItems.empty())
			parentItem = parentItems.first();
	}

	_dlg->_AddTransform(xform, parentItem);

	Sys_UnlockFutex(_ftx);
}

void
NeSceneHierarchy::_EntityDestroy(NeSceneHierarchy *shd, NeEntityHandle eh)
{
	Sys_LockFutex(_ftx);

	QTreeWidgetItem *item = shd->_tree->findItems(E_EntityName(eh), Qt::MatchExactly).first();
	if (!item)
		return;

	QTreeWidgetItem *parent = item->parent();
	if (parent)
		parent->removeChild(item);

	delete item;

	Sys_UnlockFutex(_ftx);
}

void
NeSceneHierarchy::_ComponentCreate(NeSceneHierarchy *shd, const struct NeComponentCreationData *ccd)
{
	if (ccd->type != E_ComponentTypeId(TRANSFORM_COMP))
		return;

	_EntityCreate(shd, ccd->owner);
}

void
NeSceneHierarchy::_SceneActivated(NeSceneHierarchy *shd, struct NeScene *scn)
{
	Sys_LockFutex(_ftx);

	const struct NeTransform *xform = NULL;
	const struct NeArray *transforms = E_GetAllComponentsS(scn, E_ComponentTypeId(TRANSFORM_COMP));

	Rt_ArrayForEach(xform, transforms, const struct NeTransform *)
		shd->_AddTransform(xform, nullptr);

	Sys_UnlockFutex(_ftx);
}

/* NekoEditor
 *
 * SceneHierarchy.cxx
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