/* ============================================================
* QupZilla - WebKit based browser
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
* ============================================================ */
#pragma once

#include <QUrl>

#include "qzcommon.h"
#include "animatedwidget.h"
#include "passwordmanager.h"
#include "autofill.h"

namespace Ui
{
class AutoFillNotification;
}

class AnimatedWidget;

class QUPZILLA_EXPORT AutoFillNotification : public AnimatedWidget
{
    Q_OBJECT

public:
    explicit AutoFillNotification(const QUrl &url,
                                  const PageFormData &formData,
                                  const PasswordEntry &updateData);
    ~AutoFillNotification();

private slots:
    void update();
    void remember();
    void never();

private:
    Ui::AutoFillNotification* ui;

    QUrl m_url;
    PageFormData m_formData;
    PasswordEntry m_updateData;
};

