/**************************************************************************
**
** This file is part of Qt SDK**
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).*
**
** Contact:  Nokia Corporation qt-info@nokia.com**
**
** No Commercial Usage
**
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception version
** 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you are unsure which license is appropriate for your use, please contact
** (qt-info@nokia.com).
**
**************************************************************************/
#ifndef PACKAGEMANAGERCORE_P_H
#define PACKAGEMANAGERCORE_P_H

#include "getrepositoriesmetainfojob.h"
#include "settings.h"
#include "packagemanagercore.h"

#include <KDToolsCore/KDSysInfo>
#include <KDUpdater/Application>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QPair>
#include <QtCore/QPointer>

class FSEngineClientHandler;
QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(QFileInfo)

class KDJob;
namespace KDUpdater {
    class UpdateFinder;
}

namespace QInstaller {

struct BinaryLayout;

class Component;
class TempDirDeleter;

class PackageManagerCorePrivate : public QObject
{
    Q_OBJECT
    friend class PackageManagerCore;

public:
    enum OperationType {
        Backup,
        Perform,
        Undo
    };
    enum AppendToInstallState {
        StartAppendToInstallState,
        WithoutDependenciesAppendToInstallState,
        WithResolvedDependenciesAppendToInstallState

    };

    explicit PackageManagerCorePrivate(PackageManagerCore *core);
    explicit PackageManagerCorePrivate(PackageManagerCore *core, qint64 magicInstallerMaker,
        const OperationList &performedOperations);
    ~PackageManagerCorePrivate();

    static bool isProcessRunning(const QString &name, const QList<KDSysInfo::ProcessInfo> &processes);

    static bool performOperationThreaded(Operation *op, PackageManagerCorePrivate::OperationType type
        = PackageManagerCorePrivate::Perform);

    void initialize();

    bool statusCanceledOrFailed() const;
    void setStatus(int status, const QString &error = QString());

    QString targetDir() const;
    QString registerPath() const;

    QString uninstallerName() const;
    QString installerBinaryPath() const;
    void readUninstallerIniFile(const QString &targetDir);
    void writeUninstaller(OperationList performedOperations);

    QString componentsXmlPath() const;
    QString configurationFileName() const;

    void clearAllComponentLists();
    void clearUpdaterComponentLists();
    QHash<QString, QPair<Component*, Component*> > &componentsToReplace(RunMode mode);

    void clearComponentsToInstall();
    void appendComponentsToInstall(const QList<Component*> &components,
        const AppendToInstallState state = StartAppendToInstallState);
    QString installReason(Component* component);

    void runInstaller();
    bool isInstaller() const;

    void runUninstaller();
    bool isUninstaller() const;

    void runUpdater();
    bool isUpdater() const;

    void runPackageUpdater();
    bool isPackageManager() const;

    QString replaceVariables(const QString &str) const;
    QByteArray replaceVariables(const QByteArray &str) const;

    void stopProcessesForUpdates(const QList<Component*> &components);
    int countProgressOperations(const QList<Component*> &components);
    int countProgressOperations(const OperationList &operations);
    void connectOperationToInstaller(Operation* const operation, double progressOperationPartSize);

    Operation* createOwnedOperation(const QString &type);
    Operation* takeOwnedOperation(Operation *operation);

    Operation* createPathOperation(const QFileInfo &fileInfo,
        const QString &componentName);
    void registerPathesForUninstallation(const QList<QPair<QString, bool> > &pathesForUninstallation,
        const QString &componentName);

    void addPerformed(Operation* op) {
        m_performedOperationsCurrentSession.append(op);
    }

    void commitSessionOperations() {
        m_performedOperationsOld += m_performedOperationsCurrentSession;
        m_performedOperationsCurrentSession.clear();
    }

    void installComponent(Component *component, double progressOperationSize,
        bool adminRightsGained = false);

signals:
    void installationStarted();
    void installationFinished();
    void uninstallationStarted();
    void uninstallationFinished();

public:
    KDUpdater::UpdateFinder *m_updateFinder;
    KDUpdater::Application m_updaterApplication;
    FSEngineClientHandler *m_FSEngineClientHandler;

    int m_status;
    QString m_error;

    Settings m_settings;
    bool m_forceRestart;
    bool m_testChecksum;
    bool m_launchedAsRoot;
    bool m_completeUninstall;
    bool m_needToWriteUninstaller;
    QHash<QString, QString> m_vars;
    QHash<QString, bool> m_sharedFlags;
    QString m_installerBaseBinaryUnreplaced;

    QList<Component*> m_rootComponents;
    QList<Component*> m_updaterComponents;
    QList<Component*> m_updaterComponentsDeps;

    OperationList m_ownedOperations;
    OperationList m_performedOperationsOld;
    OperationList m_performedOperationsCurrentSession;

    bool isInstallComponentsOrderCalculated;

private slots:
    void infoMessage(KDJob *, const QString &message) {
        emit m_core->metaJobInfoMessage(message);
    }

private:
    void deleteUninstaller();
    void registerUninstaller();
    void unregisterUninstaller();

    void writeUninstallerBinary(QFile *const input, qint64 size, bool writeBinaryLayout);
    void writeUninstallerBinaryData(QIODevice *output, QFile *const input, const OperationList &performed,
        const BinaryLayout &layout, bool compressOperations, bool forceUncompressedResources);

    void runUndoOperations(const OperationList &undoOperations, double undoOperationProgressSize,
        bool adminRightsGained, bool deleteOperation);

    PackagesList remotePackages();
    LocalPackagesHash localInstalledPackages();
    bool fetchMetaInformationFromRepositories();
    bool addUpdateResourcesFromRepositories(bool parseChecksum);
    void realAppendToInstallComponents(Component *component);
    void insertInstallReason(Component *component, const QString &reason);

private:
    PackageManagerCore *m_core;
    GetRepositoriesMetaInfoJob *m_repoMetaInfoJob;

    bool m_updates;
    bool m_repoFetched;
    bool m_updateSourcesAdded;
    qint64 m_magicBinaryMarker;

    // < name (component to replace), < replacement component, component to replace > >
    QHash<QString, QPair<Component*, Component*> > m_componentsToReplaceAllMode;
    QHash<QString, QPair<Component*, Component*> > m_componentsToReplaceUpdaterMode;

    //calculate installation order variables
    QList<Component*> m_orderedToInstallComponents;
    QHash<QString, QString> m_toInstallComponentIdReasonHash;

    //in some cases we have the reason a while before we can it really,
    //so we can't use ReasonHash as a quick is allready added check
    QSet<QString> m_toInstallComponentIds;

    QString debugComponent;

};

}   // QInstaller

#endif  // PACKAGEMANAGERCORE_P_H
