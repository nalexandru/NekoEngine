#include "SceneHierarchy.h"

#include <System/Thread.h>
#include <Engine/Events.h>
#include <Engine/Entity.h>
#include <Engine/Component.h>
#include <Scene/Components.h>
#include <Scene/Transform.h>
#include <Scene/Scene.h>

#include "Inspector.h"

static NeSceneHierarchy *f_dlg;

static NeFutex f_ftx;
static uint64_t f_sceneActivatedHandler, f_entityCreateHandler, f_entityDestroyHandler, f_componentCreateHandler;

bool
GUI_CreateSceneHierarchy(void)
{
	f_dlg = new NeSceneHierarchy();
	return f_dlg != nullptr;
}

bool
GUI_InitSceneHierarchy(void)
{
	f_sceneActivatedHandler = E_RegisterHandler(EVT_SCENE_ACTIVATED,
												(NeEventHandlerProc) NeSceneHierarchy::SceneActivated, f_dlg);
	f_entityCreateHandler = E_RegisterHandler(EVT_ENTITY_CREATED, (NeEventHandlerProc) NeSceneHierarchy::EntityCreated, f_dlg);
	f_entityDestroyHandler = E_RegisterHandler(EVT_ENTITY_DESTROYED,
											   (NeEventHandlerProc) NeSceneHierarchy::EntityDestroyed, f_dlg);
	f_componentCreateHandler = E_RegisterHandler(EVT_COMPONENT_CREATED,
												 (NeEventHandlerProc) NeSceneHierarchy::ComponentCreated, f_dlg);

	Sys_InitFutex(&f_ftx);

	f_dlg->show();

	return true;
}

void
GUI_TermSceneHierarchy(void)
{
	E_UnregisterHandler(f_sceneActivatedHandler);
	E_UnregisterHandler(f_entityCreateHandler);
	E_UnregisterHandler(f_entityDestroyHandler);
	E_UnregisterHandler(f_componentCreateHandler);

	Sys_TermFutex(f_ftx);

	f_dlg->close();
	delete f_dlg;
}

NeSceneHierarchy::NeSceneHierarchy(QWidget *parent) : QDialog(parent)
{
	setMinimumSize(250, 600);
	setWindowTitle("Scene Hierarchy");
	setWindowIcon(QIcon(":/EdIcon.png"));

	_lyt = new QVBoxLayout(this);

	_tree = new QTreeWidget();
	_tree->setColumnCount(1);
	_tree->setHeaderLabel("Scene Hierarchy");

	_lyt->addWidget(_tree);

	connect(_tree, SIGNAL(itemSelectionChanged()), this, SLOT(TreeSelectionChanged()));

	//_tree->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void
NeSceneHierarchy::TreeSelectionChanged()
{
	NeFutexLock lock(f_ftx);

	if (_tree->selectedItems().count())
		GUI_InspectEntity(_tree->selectedItems().first()->data(1, Qt::UserRole).value<NeEntityHandle>());
	else
		GUI_InspectEntity(NULL);
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
NeSceneHierarchy::EntityCreated(NeSceneHierarchy *shd, NeEntityHandle eh)
{
	NeFutexLock lock(f_ftx);

	const struct NeTransform *xform = (struct NeTransform *)E_GetComponent(eh, NE_TRANSFORM_ID);
	if (!xform)
		return;

	QList<QTreeWidgetItem *> items = shd->_tree->findItems(E_EntityName(eh), Qt::MatchExactly);
	if (!items.empty())
		return;

	QTreeWidgetItem *parentItem = nullptr;
	if (xform->parent != NE_INVALID_HANDLE) {
		struct NeTransform *parent = (struct NeTransform *)E_ComponentPtrS(Scn_GetScene((uint8_t)xform->_sceneId), xform->parent);
		QList<QTreeWidgetItem *> parentItems = shd->_tree->findItems(E_EntityName(parent->_owner), Qt::MatchExactly);
		if (!parentItems.empty())
			parentItem = parentItems.first();
	}

	f_dlg->_AddTransform(xform, parentItem);
}

void
NeSceneHierarchy::EntityDestroyed(NeSceneHierarchy *shd, NeEntityHandle eh)
{
	NeFutexLock lock(f_ftx);

	QTreeWidgetItem *item = shd->_tree->findItems(E_EntityName(eh), Qt::MatchExactly).first();
	if (!item)
		return;

	QTreeWidgetItem *parent = item->parent();
	if (parent)
		parent->removeChild(item);

	delete item;
}

void
NeSceneHierarchy::ComponentCreated(NeSceneHierarchy *shd, const struct NeComponentCreationData *ccd)
{
	if (ccd->type != NE_TRANSFORM_ID)
		return;

	EntityCreated(shd, ccd->owner);
}

void
NeSceneHierarchy::SceneActivated(NeSceneHierarchy *shd, struct NeScene *scn)
{
	NeFutexLock lock(f_ftx);

	const struct NeTransform *xform = NULL;
	const struct NeArray *transforms = E_GetAllComponentsS(scn, NE_TRANSFORM_ID);

	Rt_ArrayForEach(xform, transforms, const struct NeTransform *)
		shd->_AddTransform(xform, nullptr);
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