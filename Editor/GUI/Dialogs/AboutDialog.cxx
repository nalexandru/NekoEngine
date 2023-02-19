#include "AboutDialog.h"

#include <QLabel>
#include <QPainter>
#include <QPushButton>

#include <Engine/Version.h>

NeAboutDialog::NeAboutDialog(QWidget *parent) : QDialog(parent)
{
	setFixedSize(500, 138);
	setWindowIcon(QIcon(":/EdIcon.png"));

	QLabel *lbl = new QLabel(this);
	lbl->setPixmap(QPixmap(":/EdIcon.png"));
	lbl->setGeometry(5, 5, 128, 128);
	lbl->setScaledContents(true);

	lbl = new QLabel(this);
	lbl->setText("NekoEditor");
	lbl->setGeometry(138, 35, 357, 15);

	lbl = new QLabel(this);
	lbl->setText(QString::asprintf("Version: %s %s", E_VER_STR, E_CODENAME));
	lbl->setGeometry(138, 50, 357, 15);

	lbl = new QLabel(this);
	lbl->setText(QString::asprintf("Copyright (c) %s", E_CPY_STR));
	lbl->setGeometry(138, 65, 357, 15);

	lbl = new QLabel(this);
	lbl->setText("Licensed under BSD 3-clause.");
	lbl->setGeometry(138, 80, 357, 15);

	QPushButton *btn = new QPushButton(this);
	btn->setText("OK");
	btn->setGeometry(420, 108, 75, 25);
	connect(btn, SIGNAL(clicked(bool)), this, SLOT(accept()));
}

NeAboutDialog::~NeAboutDialog() noexcept
{
}

/* NekoEditor
 *
 * AboutDialog.cxx
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