/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

//==============================================================================
// Core plugin
//==============================================================================

#include "centralwidget.h"
#include "corecliutils.h"
#include "coreguiutils.h"
#include "coreplugin.h"
#include "filetypeinterface.h"
#include "filemanager.h"
#include "organisationwidget.h"
#include "plugin.h"

//==============================================================================

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPalette>
#include <QSettings>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

PLUGININFO_FUNC CorePluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("the core plugin."));
    descriptions.insert("fr", QString::fromUtf8("l'extension de base."));

    return new PluginInfo(PluginInfo::Miscellaneous, false, false,
                          QStringList(),
                          descriptions);
}

//==============================================================================

CorePlugin::CorePlugin() :
    mRecentFileNamesOrUrls(QStringList())
{
}

//==============================================================================
// Core interface
//==============================================================================

QWidget * CorePlugin::centralWidget()
{
    // Return our central widget

    return mCentralWidget;
}

//==============================================================================

void CorePlugin::settingsLoaded(const Plugins &pLoadedPlugins)
{
    // Let our central widget know that all the other plugins have now loaded
    // their settings

    mCentralWidget->settingsLoaded(pLoadedPlugins);
}

//==============================================================================

void CorePlugin::handleArguments(const QStringList &pArguments)
{
    // All the arguments are currently assumed to be files to open
    // Note: we open all valid (i.e. non empty) arguments as if they were remote
    //       files since if they are not then CentralWidget::openRemoteFile()
    //       will open them as normal files...

    foreach (const QString &argument, pArguments)
        if (!argument.isEmpty())
            mCentralWidget->openRemoteFile(argument);
}

//==============================================================================

bool CorePlugin::canClose()
{
    // To determine whether we can close, we must ask our central widget

    return mCentralWidget->canClose();
}

//==============================================================================
// File handling interface
//==============================================================================

bool CorePlugin::saveFile(const QString &pOldFileName,
                          const QString &pNewFileName)
{
    Q_UNUSED(pOldFileName);
    Q_UNUSED(pNewFileName);

    // We don't handle this interface...

    return false;
}

//==============================================================================

void CorePlugin::fileOpened(const QString &pFileName)
{
    // Remove the file from our list of recent files and update our Reopen
    // sub-menu

    FileManager *fileManagerInstance = FileManager::instance();

    if (fileManagerInstance->isRemote(pFileName))
        mRecentFileNamesOrUrls.removeOne(fileManagerInstance->url(pFileName));
    else
        mRecentFileNamesOrUrls.removeOne(pFileName);

    updateFileReopenMenu();
}

//==============================================================================

void CorePlugin::filePermissionsChanged(const QString &pFileName)
{
    // Update the checked state of our Locked menu, if needed

    if (!pFileName.compare(mCentralWidget->currentFileName()))
        mFileLockedAction->setChecked(!FileManager::instance()->isReadableAndWritable(pFileName));
        // Note: we really want to call isReadableAndWritable() rather than
        //       isLocked() since from the GUI perspective a file should only be
        //       considered unlocked if it can be both readable and writable
        //       (see CentralWidget::updateFileTab())...
}

//==============================================================================

void CorePlugin::fileModified(const QString &pFileName)
{
    // Update our new/modified sensitive actions, if needed

    if (!pFileName.compare(mCentralWidget->currentFileName()))
        updateNewModifiedSensitiveActions();
}

//==============================================================================

void CorePlugin::fileReloaded(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

void CorePlugin::fileRenamed(const QString &pOldFileName,
                             const QString &pNewFileName)
{
    // Remove the new file from our list of recent files, should it be there
    // Note: it's fine if the new file isn't in our list since nothing will be
    //       done in that case (thus avoiding us having to test for its
    //       presence)...

    mRecentFileNamesOrUrls.removeOne(pNewFileName);

    // A file has been created or saved under a new name, so we want the old
    // file name to be added to our list of recent files, i.e. as if it had been
    // closed

    fileClosed(pOldFileName);
}

//==============================================================================

void CorePlugin::fileSaved(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

void CorePlugin::fileClosed(const QString &pFileName)
{
    // Add, if isn't new and it still exists (i.e. we are not here because the
    // file has been deleted), the file to our list of recent files (making sure
    // that we don't end up with more than 10 recent file names) and update our
    // Reopen sub-menu
    // Note: the most recent file is to be shown first...

    FileManager *fileManagerInstance = FileManager::instance();

    if (   !fileManagerInstance->isNew(pFileName)
        &&  QFile::exists(pFileName)) {
        if (fileManagerInstance->isRemote(pFileName))
            mRecentFileNamesOrUrls.prepend(fileManagerInstance->url(pFileName));
        else
            mRecentFileNamesOrUrls.prepend(pFileName);

        while (mRecentFileNamesOrUrls.count() > 10)
            mRecentFileNamesOrUrls.removeLast();

        updateFileReopenMenu();
    }
}

//==============================================================================
// GUI interface
//==============================================================================

void CorePlugin::updateGui(Plugin *pViewPlugin, const QString &pFileName)
{
    Q_UNUSED(pViewPlugin);
    Q_UNUSED(pFileName);

    // Update our new/modified sensitive actions

    updateNewModifiedSensitiveActions();
}

//==============================================================================

Gui::Menus CorePlugin::guiMenus() const
{
    // Return our menus

    return Gui::Menus() << Gui::Menu(Gui::Menu::File, mOpenReloadSeparator, mFileReopenSubMenu);
}

//==============================================================================

Gui::MenuActions CorePlugin::guiMenuActions() const
{
    // Return our menu actions

    return Gui::MenuActions() << Gui::MenuAction(Gui::MenuAction::File, mFileOpenAction)
                              << Gui::MenuAction(Gui::MenuAction::File, mFileOpenRemoteAction)
                              << Gui::MenuAction(Gui::MenuAction::File, mOpenReloadSeparator)
                              << Gui::MenuAction(Gui::MenuAction::File, mFileReloadAction)
                              << Gui::MenuAction(Gui::MenuAction::File)
                              << Gui::MenuAction(Gui::MenuAction::File, mFileDuplicateAction)
                              << Gui::MenuAction(Gui::MenuAction::File)
                              << Gui::MenuAction(Gui::MenuAction::File, mFileLockedAction)
                              << Gui::MenuAction(Gui::MenuAction::File)
                              << Gui::MenuAction(Gui::MenuAction::File, mFileSaveAction)
                              << Gui::MenuAction(Gui::MenuAction::File, mFileSaveAsAction)
                              << Gui::MenuAction(Gui::MenuAction::File, mFileSaveAllAction)
                              << Gui::MenuAction(Gui::MenuAction::File)
                              << Gui::MenuAction(Gui::MenuAction::File, mFilePreviousAction)
                              << Gui::MenuAction(Gui::MenuAction::File, mFileNextAction)
                              << Gui::MenuAction(Gui::MenuAction::File)
                              << Gui::MenuAction(Gui::MenuAction::File, mFileCloseAction)
                              << Gui::MenuAction(Gui::MenuAction::File, mFileCloseAllAction)
                              << Gui::MenuAction(Gui::MenuAction::File);
}

//==============================================================================
// I18n interface
//==============================================================================

void CorePlugin::retranslateUi()
{
    // Retranslate our different File actions

    retranslateAction(mFileOpenAction, tr("Open..."),
                      tr("Open a file"));
    retranslateAction(mFileOpenRemoteAction, tr("Open Remote..."),
                      tr("Open a remote file"));

    retranslateAction(mFileReloadAction, tr("Reload"),
                      tr("Reload the current file"));

    retranslateAction(mFileDuplicateAction, tr("Duplicate"),
                      tr("Duplicate the current file"));

    retranslateAction(mFileLockedAction, tr("Locked"),
                      tr("Toggle the locked state of the current file"));

    retranslateAction(mFileSaveAction, tr("Save"),
                      tr("Save the current file"));
    retranslateAction(mFileSaveAsAction, tr("Save As..."),
                      tr("Save the current file under a different name"));
    retranslateAction(mFileSaveAllAction, tr("Save All"),
                      tr("Save all the files"));

    retranslateAction(mFilePreviousAction, tr("Previous"),
                      tr("Select the previous file"));
    retranslateAction(mFileNextAction, tr("Next"),
                      tr("Select the next file"));

    retranslateAction(mFileCloseAction, tr("Close"),
                      tr("Close the current file"));
    retranslateAction(mFileCloseAllAction, tr("Close All"),
                      tr("Close all the files"));

    // Retranslate our File sub-menu and its action

    retranslateMenu(mFileReopenSubMenu, tr("Reopen"));

    retranslateAction(mFileReopenMostRecentFileAction, tr("Most Recent"),
                      tr("Reopen the most recently closed file"));
    retranslateAction(mFileClearReopenSubMenuAction, tr("Clear Menu"),
                      tr("Clear the menu"));

    // Retranslate our central widget

    mCentralWidget->retranslateUi();
}

//==============================================================================
// Plugin interface
//==============================================================================

void CorePlugin::initializePlugin(QMainWindow *pMainWindow)
{
    // Keep track of our main window

    mMainWindow = pMainWindow;

    // Create our central widget

    mCentralWidget = new CentralWidget(pMainWindow);

    // Create our different File actions

    mFileOpenAction = newAction(QIcon(":/oxygen/actions/document-open.png"),
                                QKeySequence::Open, pMainWindow);
    mFileOpenRemoteAction = newAction(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_O),
                                      pMainWindow);

    mFileReloadAction = new QAction(pMainWindow);

    mFileDuplicateAction = newAction(QKeySequence(Qt::CTRL|Qt::Key_D),
                                     pMainWindow);

    mFileLockedAction = newAction(true, QKeySequence(Qt::CTRL|Qt::Key_L),
                                  pMainWindow);

    mFileSaveAction    = newAction(QIcon(":/oxygen/actions/document-save.png"),
                                   QKeySequence::Save, pMainWindow);
    mFileSaveAsAction  = newAction(QIcon(":/oxygen/actions/document-save-as.png"),
                                   QKeySequence::SaveAs, pMainWindow);
    mFileSaveAllAction = newAction(QIcon(":/oxygen/actions/document-save-all.png"),
                                   pMainWindow);

    // Note: for mFilePreviousAction and mFileNextAction, we would normally use
    //       QKeySequence::PreviousChild and QKeySequence::NextChild,
    //       respectively, but for Qt this means using Ctrl+Shift+BackTab and
    //       Ctrl+Tab, respectively, on Windows/Linux, and Ctrl+{ and Ctrl+},
    //       respectively, on OS X. On Windows, Ctrl+Shift+BackTab just doesn't
    //       work, on OS X those key sequences are not the most natural ones.
    //       So, instead, it would be more natural to use Ctrl+Shift+Tab and
    //       Ctr+Tab, respectively, on Windows/Linux, and Meta+Shift+Tab and
    //       Meta+Tab, respectively, on OS X. The original plan was therefore to
    //       use QKeySequence::PreviousChild and QKeySequence::NextChild, as
    //       well as our preferred key sequences, but Qt ended up 'allowing'
    //       only using one of them. So, in the end, we only use our preferred
    //       key sequences...

    mFilePreviousAction = newAction(QIcon(":/oxygen/actions/go-previous.png"),
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
                                    QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_Tab),
#elif defined(Q_OS_MAC)
                                    QKeySequence(Qt::META|Qt::SHIFT|Qt::Key_Tab),
#else
    #error Unsupported platform
#endif
                                    pMainWindow);
    mFileNextAction     = newAction(QIcon(":/oxygen/actions/go-next.png"),
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
                                    QKeySequence(Qt::CTRL|Qt::Key_Tab),
#elif defined(Q_OS_MAC)
                                    QKeySequence(Qt::META|Qt::Key_Tab),
#else
    #error Unsupported platform
#endif
                                    pMainWindow);

    mFileCloseAction    = newAction(QIcon(":/oxygen/actions/document-close.png"),
#if defined(Q_OS_WIN)
                                    QList<QKeySequence>() << QKeySequence::Close << QKeySequence(Qt::CTRL|Qt::Key_W),
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
                                    QKeySequence::Close,
#else
    #error Unsupported platform
#endif
                                    pMainWindow);
    mFileCloseAllAction = new QAction(pMainWindow);

    // Create the separator before which we will insert our Reopen sub-menu

    mOpenReloadSeparator = new QAction(pMainWindow);

    mOpenReloadSeparator->setSeparator(true);

    // Create our Reopen sub-menu

    mFileReopenSubMenu = newMenu(QIcon(":/oxygen/actions/document-open-recent.png"),
                                 pMainWindow);

    mFileReopenMostRecentFileAction = newAction(QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_T),
                                                pMainWindow);
    mFileReopenSubMenuSeparator1 = new QAction(pMainWindow);
    mFileReopenSubMenuSeparator2 = new QAction(pMainWindow);
    mFileClearReopenSubMenuAction = new QAction(pMainWindow);

    mFileReopenSubMenuSeparator1->setSeparator(true);
    mFileReopenSubMenuSeparator2->setSeparator(true);

    mFileReopenSubMenu->addAction(mFileReopenMostRecentFileAction);
    mFileReopenSubMenu->addAction(mFileReopenSubMenuSeparator1);
    mFileReopenSubMenu->addAction(mFileReopenSubMenuSeparator2);
    mFileReopenSubMenu->addAction(mFileClearReopenSubMenuAction);

    // Some connections to handle our different File actions

    connect(mFileOpenAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(openFile()));
    connect(mFileOpenRemoteAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(openRemoteFile()));

    connect(mFileReloadAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(reloadFile()));

    connect(mFileDuplicateAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(duplicateFile()));

    connect(mFileLockedAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(toggleLockedFile()));

    connect(mFileSaveAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(saveFile()));
    connect(mFileSaveAsAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(saveFileAs()));
    connect(mFileSaveAllAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(saveAllFiles()));

    connect(mFilePreviousAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(previousFile()));
    connect(mFileNextAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(nextFile()));

    connect(mFileCloseAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(closeFile()));
    connect(mFileCloseAllAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(closeAllFiles()));

    // Some connections to update the enabled state of our various actions

    connect(mCentralWidget, SIGNAL(atLeastOneView(const bool &)),
            mFileOpenAction, SLOT(setEnabled(bool)));
    connect(mCentralWidget, SIGNAL(atLeastOneView(const bool &)),
            mFileOpenRemoteAction, SLOT(setEnabled(bool)));
    connect(mCentralWidget, SIGNAL(atLeastOneView(const bool &)),
            this, SLOT(updateFileReopenMenu(const bool &)));

    connect(mCentralWidget, SIGNAL(canSave(const bool &)),
            mFileSaveAction, SLOT(setEnabled(bool)));
    connect(mCentralWidget, SIGNAL(canSaveAs(const bool &)),
            mFileSaveAsAction, SLOT(setEnabled(bool)));
    connect(mCentralWidget, SIGNAL(canSaveAll(const bool &)),
            mFileSaveAllAction, SLOT(setEnabled(bool)));

    connect(mCentralWidget, SIGNAL(atLeastTwoFiles(const bool &)),
            mFilePreviousAction, SLOT(setEnabled(bool)));
    connect(mCentralWidget, SIGNAL(atLeastTwoFiles(const bool &)),
            mFileNextAction, SLOT(setEnabled(bool)));

    connect(mCentralWidget, SIGNAL(atLeastOneFile(const bool &)),
            mFileCloseAction, SLOT(setEnabled(bool)));
    connect(mCentralWidget, SIGNAL(atLeastOneFile(const bool &)),
            mFileCloseAllAction, SLOT(setEnabled(bool)));

    // A connection related to our Reopen sub-menu

    connect(mFileReopenMostRecentFileAction, SIGNAL(triggered(bool)),
            this, SLOT(reopenMostRecentFile()));
    connect(mFileClearReopenSubMenuAction, SIGNAL(triggered(bool)),
            this, SLOT(clearReopenSubMenu()));
}

//==============================================================================

void CorePlugin::finalizePlugin()
{
    // We don't handle this interface...
}

//==============================================================================

void CorePlugin::pluginsInitialized(const Plugins &pLoadedPlugins)
{
    // Retrieve the different file types supported by our various plugins and
    // make our central widget aware of them

    FileTypes supportedFileTypes = FileTypes();

    foreach (Plugin *plugin, pLoadedPlugins) {
        FileTypeInterface *fileTypeInterface = qobject_cast<FileTypeInterface *>(plugin->instance());

        if (fileTypeInterface)
            // The plugin implements our file type interface, so add the
            // supported file types, but only if they are not already in our
            // list

            foreach (FileType *fileType, fileTypeInterface->fileTypes())
                if (!supportedFileTypes.contains(fileType))
                    supportedFileTypes << fileType;
    }

    mCentralWidget->setSupportedFileTypes(supportedFileTypes);

    // Check, based on the loaded plugins, which views, if any, our central
    // widget should support

    foreach (Plugin *plugin, pLoadedPlugins) {
        ViewInterface *viewInterface = qobject_cast<ViewInterface *>(plugin->instance());

        if (viewInterface)
            // The plugin implements our View interface, so add it to our
            // central widget

            mCentralWidget->addView(plugin);
    }
}

//==============================================================================

static const auto SettingsRecentFiles = QStringLiteral("RecentFiles");

//==============================================================================

void CorePlugin::loadSettings(QSettings *pSettings)
{
    // Retrieve the recent files
    // Note: it's important to retrieve the recent files before retrieving our
    //       central widget settings since mRecentFileNamesOrUrls gets updated
    //       as a result of opening/closing a file...

    mRecentFileNamesOrUrls = pSettings->value(SettingsRecentFiles).toStringList();

    // Update our Reopen sub-menu

    updateFileReopenMenu();

    // Retrieve the central widget settings

    pSettings->beginGroup(mCentralWidget->objectName());
        mCentralWidget->loadSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void CorePlugin::saveSettings(QSettings *pSettings) const
{
    // Keep track of the recent files

    pSettings->setValue(SettingsRecentFiles, mRecentFileNamesOrUrls);

    // Keep track of the central widget settings

    pSettings->beginGroup(mCentralWidget->objectName());
        mCentralWidget->saveSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void CorePlugin::handleAction(const QUrl &pUrl)
{
    Q_UNUSED(pUrl);

    // We don't handle this interface...
}

//==============================================================================
// Plugin specific
//==============================================================================

void CorePlugin::updateFileReopenMenu(const bool &pEnabled)
{
    // Update the contents of our Reopen sub-menu by first cleaning it

    foreach (QAction *action, mFileReopenSubMenu->actions()) {
        if (   (action != mFileReopenMostRecentFileAction)
            && (action != mFileReopenSubMenuSeparator1)
            && (action != mFileReopenSubMenuSeparator2)) {
            disconnect(action, SIGNAL(triggered(bool)),
                       this, SLOT(reopenRecentFile()));

            mFileReopenSubMenu->removeAction(action);

            delete action;
        } else if (action == mFileReopenSubMenuSeparator2) {
            break;
        }
    }

    // Add the recent files to our Reopen sub-menu

    foreach (const QString &recentFile, mRecentFileNamesOrUrls) {
        QAction *action = new QAction(mMainWindow);

        action->setEnabled(pEnabled);
        action->setText(recentFile);

        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(reopenRecentFile()));

        mFileReopenSubMenu->insertAction(mFileReopenSubMenuSeparator2, action);
    }

    // Enable/disable our reopen sub-menu actions depending on whether we have
    // recent file names

    mFileReopenMostRecentFileAction->setEnabled(!mRecentFileNamesOrUrls.isEmpty());
    mFileClearReopenSubMenuAction->setEnabled(!mRecentFileNamesOrUrls.isEmpty());
}

//==============================================================================

void CorePlugin::updateNewModifiedSensitiveActions()
{
    // Update our actions keeping in mind the fact that a file may be modified
    // and even possibly new

    QString fileName = mCentralWidget->currentFileName();

    if (!fileName.isEmpty()) {
        FileManager *fileManagerInstance = FileManager::instance();
        bool fileIsNew = fileManagerInstance->isNew(fileName);
        bool fileIsNewRemoteOrModified =    fileIsNew
                                         || fileManagerInstance->isRemote(fileName)
                                         || fileManagerInstance->isModified(fileName);

        mFileReloadAction->setEnabled(!fileIsNew);
        mFileDuplicateAction->setEnabled(!fileIsNewRemoteOrModified);
        mFileLockedAction->setEnabled(   !fileIsNewRemoteOrModified
                                      &&  fileManagerInstance->isReadable(fileName));
    } else {
        mFileReloadAction->setEnabled(false);
        mFileDuplicateAction->setEnabled(false);
        mFileLockedAction->setEnabled(false);
    }
}

//==============================================================================

void CorePlugin::reopenFile(const QString &pFileName)
{
    // Check that the recent file still exists

    bool isLocalFile;
    QString fileNameOrUrl;

    checkFileNameOrUrl(pFileName, isLocalFile, fileNameOrUrl);

    if (isLocalFile) {
        if (QFile::exists(fileNameOrUrl))
            // Open the recent file

            mCentralWidget->openFile(fileNameOrUrl);
        else
            // The file doesn't exist anymore, so let the user know about it

            QMessageBox::warning(mMainWindow, tr("Reopen File"),
                                 tr("<strong>%1</strong> does not exist anymore.").arg(fileNameOrUrl));
    } else {
        // Open the recent remote file

        mCentralWidget->openRemoteFile(fileNameOrUrl);
    }

    // Try to remove the file from our list of recent files and update our
    // Reopen sub-menu, if needed
    // Note: if the file was successfully opened, then it will have already been
    //       removed from our list of recent files...

    if (mRecentFileNamesOrUrls.removeOne(pFileName))
        updateFileReopenMenu();
}

//==============================================================================

void CorePlugin::reopenRecentFile()
{
    // Reopen a recent file

    reopenFile(qobject_cast<QAction *>(sender())->text());
}

//==============================================================================

void CorePlugin::reopenMostRecentFile()
{
    // Reopen the most recently closed file

    reopenFile(mRecentFileNamesOrUrls.first());
}

//==============================================================================

void CorePlugin::clearReopenSubMenu()
{
    // Indirectly clear our Reopen sub-menu

    mRecentFileNamesOrUrls.clear();

    updateFileReopenMenu();
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
