/*
 * Creates RegistersView and RegisterController based on current architecture.
 * Copyright 2013  Vlas Puhov <vlas.puhov@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef REGISTERSMANAGER_H
#define REGISTERSMANAGER_H

#include <QObject>
#include <QStringList>
#include <QScopedPointer>

namespace KDevMI {
namespace MI
{
struct ResultRecord;
}

class MIDebugSession;

class RegistersView;
class IRegisterController;
class ModelsManager;

enum Architecture {x86, x86_64, arm, other = 100, undefined};

/** @brief Determines current CPU architecture */
class ArchitectureParser : public QObject
{
    Q_OBJECT

public:

    explicit ArchitectureParser(QObject* parent);

    ///Asynchronously determines current architecture. emits @p architectureParsed when ready.
    void determineArchitecture(MIDebugSession* debugSession);

Q_SIGNALS:
    ///Emits current CPU architecture. @sa determineArchitecture
    void architectureParsed(Architecture arch);

private:

    void registerNamesHandler(const MI::ResultRecord& r);
    void parseArchitecture();

    QStringList m_registerNames;
};

class RegistersManager : public QObject
{
    Q_OBJECT

public:
    explicit RegistersManager(QWidget* parent);

public Q_SLOTS:
    void setSession(MIDebugSession* debugSession);
    ///Updates all registers.
    void updateRegisters();
    ///@sa ArchitectureParser::determineArchitecture
    void architectureParsedSlot(const Architecture arch);

private:
    void setController(IRegisterController* c);

    RegistersView* m_registersView;

    QScopedPointer<IRegisterController> m_registerController;

    ArchitectureParser* m_architectureParser;

    MIDebugSession* m_debugSession;

    ModelsManager* m_modelsManager;

    Architecture m_currentArchitecture;

    ///True if architecture could has changed(e.g. from x86 to arm)
    bool m_needToCheckArch;
};

} // end of namespace KDevMI
#endif // REGISTERSMANAGER_H
