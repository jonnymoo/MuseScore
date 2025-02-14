/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "editshortcutmodel.h"

#include <QKeySequence>

#include "translation.h"
#include "shortcutstypes.h"
#include "log.h"

using namespace mu::shortcuts;

EditShortcutModel::EditShortcutModel(QObject* parent)
    : QObject(parent)
{
}

void EditShortcutModel::load(const QVariant& shortcut, const QVariantList& allShortcuts)
{
    clear();

    m_allShortcuts = allShortcuts;
    m_originShortcut = shortcut.toMap();
    emit originSequenceChanged(originSequence());
}

void EditShortcutModel::clear()
{
    m_inputedSequence = QKeySequence();
    m_errorMessage.clear();

    emit inputedSequenceChanged(QString());
}

void EditShortcutModel::inputKey(int key, Qt::KeyboardModifiers modifiers)
{
    std::pair<int, Qt::KeyboardModifiers> correctedKeyInput = correctKeyInput(key, modifiers);
    int newKey = correctedKeyInput.first;
    int newModifiers = correctedKeyInput.second;

    if (needIgnoreKey(newKey)) {
        return;
    }

    newKey += newModifiers;

    for (int i = 0; i < m_inputedSequence.count(); i++) {
        if (m_inputedSequence[i] == key) {
            return;
        }
    }

    switch (m_inputedSequence.count()) {
    case 0:
        m_inputedSequence = QKeySequence(newKey);
        break;
    case 1:
        m_inputedSequence = QKeySequence(m_inputedSequence[0], newKey);
        break;
    case 2:
        m_inputedSequence = QKeySequence(m_inputedSequence[0], m_inputedSequence[1], newKey);
        break;
    case 3:
        m_inputedSequence = QKeySequence(m_inputedSequence[0], m_inputedSequence[1], m_inputedSequence[2], newKey);
        break;
    }

    validateInputedSequence();

    emit inputedSequenceChanged(inputedSequence());
}

void EditShortcutModel::validateInputedSequence()
{
    m_errorMessage.clear();

    QString input = inputedSequence();
    QVariant context = m_originShortcut.value("context");
    for (const QVariant& shortcut : m_allShortcuts) {
        QVariantMap sc = shortcut.toMap();
        if (sc.value("context") != context) {
            continue;
        }

        if (sc.value("sequence").toString() == input) {
            QString title = sc.value("title").toString();
            m_errorMessage = qtrc("shortcuts", "Shortcut conflicts with %1").arg(title);
            return;
        }
    }
}

QString EditShortcutModel::originSequence() const
{
    return m_originShortcut.value("sequence").toString();
}

QString EditShortcutModel::inputedSequence() const
{
    return m_inputedSequence.toString();
}

QString EditShortcutModel::errorMessage() const
{
    return m_errorMessage;
}

bool EditShortcutModel::canApplySequence() const
{
    return m_errorMessage.isEmpty() && !m_inputedSequence.isEmpty();
}

QString EditShortcutModel::unitedSequence() const
{
    QString united = originSequence();
    if (!united.isEmpty()) {
        united += "; ";
    }
    united += inputedSequence();
    return united;
}
