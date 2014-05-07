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
// Central widget
//==============================================================================

#include "centralwidget.h"
#include "cliutils.h"
#include "filehandlinginterface.h"
#include "filemanager.h"
#include "guiinterface.h"
#include "guiutils.h"
#include "plugin.h"
#include "usermessagewidget.h"
#include "viewinterface.h"
#include "viewwidget.h"

//==============================================================================

#include "ui_centralwidget.h"

//==============================================================================

#include <Qt>

//==============================================================================

#include <QApplication>
#include <QDesktopWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDragEnterEvent>
#include <QFile>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QMimeData>
#include <QRect>
#include <QSettings>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStatusBar>
#include <QUrl>
#include <QVariant>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

CentralWidgetMode::CentralWidgetMode(CentralWidget *pOwner) :
    mEnabled(false)
{
    // Initialise a few internal objects

    mViewTabs = pOwner->newTabBar(QTabBar::RoundedEast);
    mViewPlugins = new CentralWidgetViewPlugins();
}

//==============================================================================

CentralWidgetMode::~CentralWidgetMode()
{
    // Delete some internal objects

    delete mViewPlugins;
    delete mViewTabs;
}

//==============================================================================

bool CentralWidgetMode::isEnabled() const
{
    // Return whether the mode is enabled

    return mEnabled;
}

//==============================================================================

void CentralWidgetMode::setEnabled(const bool &pEnabled)
{
    // Set whether a mode is enabled

    mEnabled = pEnabled;
}

//==============================================================================

QTabBar * CentralWidgetMode::viewTabs() const
{
    // Return the mode's view tabs

    return mViewTabs;
}

//==============================================================================

CentralWidgetViewPlugins * CentralWidgetMode::viewPlugins() const
{
    // Return the mode's view plugins

    return mViewPlugins;
}

//==============================================================================

CentralWidget::CentralWidget(QMainWindow *pMainWindow) :
    Widget(pMainWindow),
    mMainWindow(pMainWindow),
    mGui(new Ui::CentralWidget),
    mState(Starting),
    mLoadedFileHandlingPlugins(Plugins()),
    mLoadedGuiPlugins(Plugins()),
    mLoadedViewPlugins(Plugins()),
    mModeTabIndexModes(QMap<int, ViewInterface::Mode>()),
    mModeModeTabIndexes(QMap<ViewInterface::Mode, int>()),
    mFileModeTabIndexes(QMap<QString, int>()),
    mFileModeViewTabIndexes(QMap<QString, QMap<int, int>>()),
    mSupportedFileTypes(FileTypes()),
    mFileNames(QStringList()),
    mModes(QMap<ViewInterface::Mode, CentralWidgetMode *>()),
    mRemoteLocalFileNames(QMap<QString, QString>())
{
    // Set up the GUI

    mGui->setupUi(this);

    // Allow for things to be dropped on us

    setAcceptDrops(true);

    // Create our modes tab bar with no tabs by default

    mModeTabs = newTabBar(QTabBar::RoundedWest);

    // Create our modes

    mModes.insert(ViewInterface::Editing, new CentralWidgetMode(this));
    mModes.insert(ViewInterface::Simulation, new CentralWidgetMode(this));
    mModes.insert(ViewInterface::Analysis, new CentralWidgetMode(this));
    // Note: these will be deleted in CentralWidget's destructor...

    // Create our files tab bar widget

    mFileTabs = newTabBar(QTabBar::RoundedNorth, true, true);

    // Create our contents widget

    mContents = new QStackedWidget(this);

    mContents->setFrameShape(QFrame::StyledPanel);

    // Create our logo view which simply display OpenCOR's logo

    mLogoView = new QWidget(this);

    mLogoView->setStyleSheet("QWidget {"
                             "    background-color: white;"
                             "    background-image: url(:Core_logo);"
                             "    background-position: center;"
                             "    background-repeat: no-repeat;"
                             "}");

    mContents->addWidget(mLogoView);

    // Create our no view message widget which will display a customised message
    // to let the user know that there view doesn't support the type of the
    // current file

    mNoViewMsg = new UserMessageWidget(":/oxygen/actions/help-about.png", this);

    mNoViewMsg->setVisible(false);
    // Note: we don't initially want to see our no view message widget, so...

    // Create and set up our central widget

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *centralWidgetVBoxLayout = new QVBoxLayout(centralWidget);

    centralWidgetVBoxLayout->setMargin(0);
    centralWidgetVBoxLayout->setSpacing(0);

    centralWidget->setLayout(centralWidgetVBoxLayout);

    centralWidgetVBoxLayout->addWidget(mFileTabs);
    centralWidgetVBoxLayout->addWidget(mContents);

    // Add the widgets to our layout

    mGui->layout->addWidget(mModeTabs);
    mGui->layout->addWidget(centralWidget);

    foreach (CentralWidgetMode *mode, mModes)
        mGui->layout->addWidget(mode->viewTabs());

    // A connection to handle the case where a file was created or duplicated

    FileManager *fileManagerInstance = FileManager::instance();

    connect(fileManagerInstance, SIGNAL(fileCreated(const QString &, const QString &)),
            this, SLOT(fileCreated(const QString &, const QString &)));
    connect(fileManagerInstance, SIGNAL(fileDuplicated(const QString &)),
            this, SLOT(fileDuplicated(const QString &)));

    // Some connections to handle changes to a file

    connect(fileManagerInstance, SIGNAL(filePermissionsChanged(const QString &)),
            this, SLOT(filePermissionsChanged(const QString &)));
    connect(fileManagerInstance, SIGNAL(fileModified(const QString &)),
            this, SLOT(fileModified(const QString &)));

    // Some connections to handle our files tab bar

    connect(mFileTabs, SIGNAL(currentChanged(int)),
            this, SLOT(updateGui()));
    connect(mFileTabs, SIGNAL(tabMoved(int, int)),
            this, SLOT(moveFile(const int &, const int &)));
    connect(mFileTabs, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeFile(const int &)));

    // A connection to handle our modes tab bar

    connect(mModeTabs, SIGNAL(currentChanged(int)),
            this, SLOT(updateGui()));
    connect(mModeTabs, SIGNAL(currentChanged(int)),
            this, SLOT(updateFileTabIcons()));

    // Some connections to handle our mode views tab bar

    foreach (CentralWidgetMode *mode, mModes) {
        connect(mode->viewTabs(), SIGNAL(currentChanged(int)),
                this, SLOT(updateGui()));
        connect(mode->viewTabs(), SIGNAL(currentChanged(int)),
                this, SLOT(updateFileTabIcons()));
    }

    // Create our remote file dialog box
//---GRY--- THE ORIGINAL PLAN WAS TO HAVE A REGULAR EXPRESSION TO VALIDATE A
//          URL, BUT IT LOOKS LIKE THERE MIGHT BE AN ISSUE WITH
//          QRegularExpressionValidator, SO WE SIMPLY ALLOW FREE TEXT FOR NOW
//          (SEE https://bugreports.qt-project.org/browse/QTBUG-38034)

    mRemoteFileDialog = new QDialog(this);
    QGridLayout *dialogLayout = new QGridLayout(mRemoteFileDialog);

    mRemoteFileDialog->setLayout(dialogLayout);

    mRemoteFileDialogUrlLabel = new QLabel(mRemoteFileDialog);
    mRemoteFileDialogUrlValue = new QLineEdit(mRemoteFileDialog);

    mRemoteFileDialogUrlValue->setMinimumWidth(qApp->desktop()->availableGeometry().width()/5);

    dialogLayout->addWidget(mRemoteFileDialogUrlLabel, 0, 0);
    dialogLayout->addWidget(mRemoteFileDialogUrlValue, 0, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Open|QDialogButtonBox::Cancel, this);

    dialogLayout->addWidget(buttonBox, 1, 0, 1, 2);

    dialogLayout->setSizeConstraint(QLayout::SetFixedSize);

    // Some connections to handle our remote file dialog box

    connect(buttonBox, SIGNAL(accepted()),
            this, SLOT(doOpenRemoteFile()));
    connect(buttonBox, SIGNAL(rejected()),
            this, SLOT(cancelOpenRemoteFile()));
}

//==============================================================================

CentralWidget::~CentralWidget()
{
    // We are shutting down, so we must let updateGui() know about it otherwise
    // we may get a segmentation fault (should there be a need to switch from
    // one view to another)

    mState = Stopping;

    // Close all the files
    // Note: we force the closing of all the files since canClose() will have
    //       been called before to decide what needs to be done with modified
    //       files, so...

    closeAllFiles(true);

    // Delete our various modes

    foreach (CentralWidgetMode *mode, mModes)
        delete mode;

    // Delete the GUI

    delete mGui;
}

//==============================================================================

static const auto SettingsFileNamesOrUrls      = QStringLiteral("FileNamesOrUrls");
static const auto SettingsCurrentFileNameOrUrl = QStringLiteral("CurrentFileNameOrUrl");
static const auto SettingsFileIsRemote         = QStringLiteral("FileIsRemote%1");
static const auto SettingsFileMode             = QStringLiteral("FileMode%1");
static const auto SettingsFileModeView         = QStringLiteral("FileModeView%1%2");

//==============================================================================

void CentralWidget::loadSettings(QSettings *pSettings)
{
    // Some connections to handle an external change in the state of a file
    // Note: we do it here because we want other plugins to get a chance to
    //       handle our file manager's signals before us. Indeed, in the case of
    //       the CellML file manager for example, we will want the CellML file
    //       manager to reload a CellML file before our different views get
    //       asked (by us, the central widget) to reload the file (i.e. update
    //       their GUIs)...

    FileManager *fileManagerInstance = FileManager::instance();

    connect(fileManagerInstance, SIGNAL(fileChanged(const QString &)),
            this, SLOT(fileChanged(const QString &)));
    connect(fileManagerInstance, SIGNAL(fileDeleted(const QString &)),
            this, SLOT(fileDeleted(const QString &)));

    // Some connections to handle an internal change in the state of a file
    // Note: we do it here for the same reason as above (see the note for the
    //       connections to handle an external change in the state of a file)...

    connect(fileManagerInstance, SIGNAL(fileModified(const QString &)),
            this, SLOT(updateModifiedSettings()));
    connect(fileManagerInstance, SIGNAL(fileReloaded(const QString &)),
            this, SLOT(fileReloaded(const QString &)));
    connect(fileManagerInstance, SIGNAL(fileRenamed(const QString &, const QString &)),
            this, SLOT(fileRenamed(const QString &, const QString &)));

    // Let the user know of a few default things about ourselves by emitting a
    // few signals

    emit canSave(false);
    emit canSaveAs(false);
    emit canSaveAll(false);

    emit atLeastOneFile(false);
    emit atLeastTwoFiles(false);

    // Retrieve and open the files that were previously opened

    QStringList fileNamesOrUrls = pSettings->value(SettingsFileNamesOrUrls).toStringList();

    foreach (const QString &fileNameOrUrl, fileNamesOrUrls)
        if (pSettings->value(SettingsFileIsRemote.arg(fileNameOrUrl)).toBool())
            openRemoteFile(fileNameOrUrl, false);
        else
            openFile(fileNameOrUrl);

    // Retrieve the modes and views of our different files

    foreach (const QString &fileName, mFileNames) {
        QString fileNameOrUrl = fileManagerInstance->isRemote(fileName)?
                                    fileManagerInstance->url(fileName):
                                    fileName;
        ViewInterface::Mode fileMode = ViewInterface::viewModeFromString(pSettings->value(SettingsFileMode.arg(fileNameOrUrl)).toString());

        if (fileMode != ViewInterface::Unknown)
            mFileModeTabIndexes.insert(fileName, mModeModeTabIndexes.value(fileMode));

        QMap<int, int> modeViewTabIndexes = QMap<int, int>();

        for (int i = 0, iMax = mModeTabs->count(); i < iMax; ++i) {
            fileMode = mModeTabIndexModes.value(i);

            QString viewPluginName = pSettings->value(SettingsFileModeView.arg(fileNameOrUrl, ViewInterface::viewModeAsString(fileMode))).toString();
            CentralWidgetViewPlugins *viewPlugins = mModes.value(fileMode)->viewPlugins();

            for (int j = 0, jMax = viewPlugins->count(); j < jMax; ++j)
                if (!viewPluginName.compare(viewPlugins->value(j)->name())) {
                    modeViewTabIndexes.insert(i, j);

                    break;
                }
        }

        mFileModeViewTabIndexes.insert(fileName, modeViewTabIndexes);
    }

    // Select the previously selected file, if it still exists, by pretending to
    // open it (which, in turn, will select the file)

    QString fileNameOrUrl = pSettings->value(SettingsCurrentFileNameOrUrl).toString();
    QString fileName = pSettings->value(SettingsFileIsRemote.arg(fileNameOrUrl)).toBool()?
                           mRemoteLocalFileNames.value(fileNameOrUrl):
                           fileNameOrUrl;

    if (mFileNames.contains(fileName))
        openFile(fileName);
    else
        // The previously selected file doesn't exist anymore, so select the
        // first file (otherwise the last file will be selected)

        mFileTabs->setCurrentIndex(0);

    // Retrieve the active directory

    setActiveDirectory(pSettings->value(SettingsActiveDirectory,
                                        QDir::homePath()).toString());
}

//==============================================================================

void CentralWidget::saveSettings(QSettings *pSettings) const
{
    // Remove all the settings related to previously opened files

    static const QString settingsFileIsRemote = SettingsFileIsRemote.arg(QString());
    static const QString settingsFileMode = SettingsFileMode.arg(QString());
    static const QString settingsFileModeView = SettingsFileModeView.arg(QString());

    foreach (const QString &key, pSettings->allKeys())
        if (   key.startsWith(settingsFileIsRemote)
            || key.startsWith(settingsFileMode)
            || key.startsWith(settingsFileModeView))
            pSettings->remove(key);

    // Keep track of the files that are opened, skipping new files

    FileManager *fileManagerInstance = FileManager::instance();
    QStringList fileNames = QStringList();
    QStringList fileNamesOrUrls = QStringList();

    foreach (const QString &fileName, mFileNames)
        if (!fileManagerInstance->isNew(fileName)) {
            // The file is not new, so keep track of it, as well as of whether
            // it's a remote file

            bool fileIsRemote = fileManagerInstance->isRemote(fileName);

            if (fileIsRemote)
                fileNamesOrUrls << fileManagerInstance->url(fileName);
            else
                fileNamesOrUrls << fileName;

            fileNames << fileName;

            pSettings->setValue(SettingsFileIsRemote.arg(fileNamesOrUrls.last()), fileIsRemote);
        }

    pSettings->setValue(SettingsFileNamesOrUrls, fileNamesOrUrls);

    // Keep track of the modes and views of our different files

    foreach (const QString &fileName, fileNames) {
        QString fileNameOrUrl = fileManagerInstance->isRemote(fileName)?
                                    fileManagerInstance->url(fileName):
                                    fileName;

        pSettings->setValue(SettingsFileMode.arg(fileNameOrUrl),
                            ViewInterface::viewModeAsString(mModeTabIndexModes.value(mFileModeTabIndexes.value(fileName))));

        QMap<int, int> modeViewTabIndexes = mFileModeViewTabIndexes.value(fileName);

        for (int i = 0, iMax = mModeTabs->count(); i < iMax; ++i) {
            ViewInterface::Mode fileMode = mModeTabIndexModes.value(i);

            pSettings->setValue(SettingsFileModeView.arg(fileNameOrUrl, ViewInterface::viewModeAsString(fileMode)),
                                mModes.value(fileMode)->viewPlugins()->value(modeViewTabIndexes.value(i))->name());
        }
    }

    // Keep track of the currently selected file
    // Note: we don't rely on mFileTabs->currentIndex() since it may refer to a
    //       new file, which we will have skipped above...

    bool hasCurrentFile = false;

    if (fileNames.count()) {
        QString fileName = mFileNames[mFileTabs->currentIndex()];
        QString fileNameOrUrl = fileManagerInstance->isRemote(fileName)?
                                    fileManagerInstance->url(fileName):
                                    fileName;

        if (fileNames.contains(fileName)) {
            pSettings->setValue(SettingsCurrentFileNameOrUrl, fileNameOrUrl);

            hasCurrentFile = true;
        }
    }

    if (!hasCurrentFile)
        pSettings->setValue(SettingsCurrentFileNameOrUrl, QString());

    // Keep track of the active directory

    pSettings->setValue(SettingsActiveDirectory, activeDirectory());
}

//==============================================================================

void CentralWidget::settingsLoaded(const Plugins &pLoadedPlugins)
{
    // Determine which loaded plugins support the file handling interface, and
    // which ones support the view interface

    foreach (Plugin *plugin, pLoadedPlugins) {
        if (qobject_cast<FileHandlingInterface *>(plugin->instance()))
            mLoadedFileHandlingPlugins << plugin;

        if (qobject_cast<GuiInterface *>(plugin->instance()))
            mLoadedGuiPlugins << plugin;

        if (qobject_cast<ViewInterface *>(plugin->instance()))
            mLoadedViewPlugins << plugin;
    }

    // Update our state now that our plugins  are fully ready

    mState = Idling;

    // Update the GUI

    updateGui();

    // Let our plugins know that our files have been opened
    // Note: the below is normally done as part of openFile(), but
    //       mLoadedFileHandlingPlugins is not yet set when openFile() gets
    //       called as part of OpenCOR's loading its settings, so we do it here
    //       instead...

    foreach (Plugin *plugin, mLoadedFileHandlingPlugins)
        foreach (const QString &fileName, mFileNames)
            qobject_cast<FileHandlingInterface *>(plugin->instance())->fileOpened(fileName);
}

//==============================================================================

void CentralWidget::retranslateUi()
{
    // Retranslate our files tab bar, in case some files are new

    for (int i = 0, iMax = mFileTabs->count(); i < iMax; ++i)
        updateFileTab(i);

    // Retranslate our modes tab bar

    mModeTabs->setTabText(mModeModeTabIndexes.value(ViewInterface::Editing, -1),
                          tr("Editing"));
    mModeTabs->setTabText(mModeModeTabIndexes.value(ViewInterface::Simulation, -1),
                          tr("Simulation"));
    mModeTabs->setTabText(mModeModeTabIndexes.value(ViewInterface::Analysis, -1),
                          tr("Analysis"));

    // Retranslate our mode views tab bar

    foreach (CentralWidgetMode *mode, mModes) {
        QTabBar *viewTabs = mode->viewTabs();

        for (int i = 0, iMax = viewTabs->count(); i < iMax; ++i)
            viewTabs->setTabText(i, qobject_cast<ViewInterface *>(mode->viewPlugins()->value(i)->instance())->viewName());
    }

    // Retranslate our modified settings, if needed

    updateModifiedSettings();

    // Retranslate our no view widget message

    updateNoViewMsg();

    // Retranslate our remote file dialog

    mRemoteFileDialog->setWindowTitle(tr("Open Remote File"));
    mRemoteFileDialogUrlLabel->setText(tr("URL:"));
}

//==============================================================================

void CentralWidget::setSupportedFileTypes(const FileTypes &pSupportedFileTypes)
{
    // Set the supported file types

    mSupportedFileTypes = pSupportedFileTypes;
}

//==============================================================================

QString CentralWidget::currentFileName() const
{
    // Return the current file name, if any

    int fileTabIndex = mFileTabs->currentIndex();

    if (fileTabIndex != -1)
        return mFileNames[fileTabIndex];
    else
        return QString();
}

//==============================================================================

void CentralWidget::updateFileTab(const int &pIndex)
{
    // Update the text, tool tip and icon to be used for the given file tab

    FileManager *fileManagerInstance = FileManager::instance();
    QString fileName = mFileNames[pIndex];
    bool fileIsNew = fileManagerInstance->isNew(fileName);
    bool fileIsRemote = fileManagerInstance->isRemote(fileName);
    QString fileUrl = fileManagerInstance->url(fileName);
    QString tabText = fileIsNew?
                          tr("File")+" #"+QString::number(fileManagerInstance->newIndex(fileName)):
                          fileIsRemote?
                              QUrl(fileUrl).fileName():
                              QFileInfo(fileName).fileName();

    mFileTabs->setTabText(pIndex, tabText+(fileManagerInstance->isNewOrModified(fileName)?"*":QString()));
    mFileTabs->setTabToolTip(pIndex, fileIsNew?
                                         tabText:
                                         fileIsRemote?
                                             fileUrl:
                                             fileName);
    mFileTabs->setTabIcon(pIndex, fileIsRemote?
                                      QIcon(":/oxygen/categories/applications-internet.png"):
                                      fileManagerInstance->isReadableAndWritable(fileName)?
                                          QIcon():
                                          QIcon(":/oxygen/status/object-locked.png"));
    // Note: we really want to call isReadableAndWritable() rather than
    //       isLocked() since no icon should be shown only if the file can be
    //       both readable and writable (see
    //       CorePlugin::filePermissionsChanged())...
}

//==============================================================================

void CentralWidget::openFile(const QString &pFileName, const File::Type &pType,
                             const QString &pUrl)
{
    if (!mModeTabs->count() || !QFile::exists(pFileName))
        // No modes are available or the file doesn't exist, so...

        return;

    // Check whether the file is already opened and, if so, select it and leave

    QString nativeFileName = nativeCanonicalFileName(pFileName);

    for (int i = 0, iMax = mFileNames.count(); i < iMax; ++i)
        if (!mFileNames[i].compare(nativeFileName)) {
            mFileTabs->setCurrentIndex(i);

            return;
        }

    // Register the file with our file manager

    FileManager::instance()->manage(nativeFileName, pType, pUrl);

    // Keep track of the mapping between the remote file and its local version,
    // if needed

    if (!pUrl.isEmpty())
        mRemoteLocalFileNames.insert(pUrl, nativeFileName);

    // Create a new tab, insert it just after the current tab, set the full name
    // of the file as the tool tip for the new tab, and make the new tab the
    // current one
    // Note #1: mFileNames is, for example, used to retrieve the name of a file,
    //          which we want to close (see CentralWidget::closeFile()), so we
    //          must make sure that the order of its contents matches that of
    //          the tabs...
    // Note #2: rather than using mFileNames, we could have used a tab's tool
    //          tip, but this makes it a bit tricky to handle with regards to
    //          connections and therefore some events triggering updateGui() to
    //          be called when the tool tip has not yet been assigned, so...

    int fileTabIndex = mFileTabs->currentIndex()+1;

    mFileNames.insert(fileTabIndex, nativeFileName);
    mFileTabs->insertTab(fileTabIndex, QString());

    updateFileTab(fileTabIndex);

    mFileTabs->setCurrentIndex(fileTabIndex);

    // Everything went fine, so let our plugins know that our file has been
    // opened
    // Note: this requires using mLoadedFileHandlingPlugins, but it will not be
    //       set when we come here following OpenCOR's loading of settings,
    //       hence we do something similar to what is done in
    //       settingsLoaded()...

    foreach (Plugin *plugin, mLoadedFileHandlingPlugins)
        qobject_cast<FileHandlingInterface *>(plugin->instance())->fileOpened(nativeFileName);
}

//==============================================================================

void CentralWidget::openFiles(const QStringList &pFileNames)
{
    // Open the various files

    for (int i = 0, iMax = pFileNames.count(); i < iMax; ++i)
        openFile(pFileNames[i]);
}

//==============================================================================

void CentralWidget::openFile()
{
    // Ask for the file(s) to be opened

    QString supportedFileTypes;

    foreach (const FileType &supportedFileType, mSupportedFileTypes)
        supportedFileTypes +=  ";;"
                              +supportedFileType.description()
                              +" (*."+supportedFileType.fileExtension()+")";

    QStringList files = getOpenFileNames(tr("Open File"),
                                         tr("All Files")
                                         +" (*"
#ifdef Q_OS_WIN
                                         +".*"
#endif
                                         +")"+supportedFileTypes);

    // Open the file(s)

    openFiles(files);
}

//==============================================================================

void CentralWidget::openRemoteFile(const QString &pUrl,
                                   const bool &pShowWarning)
{
    // Check whether the remote file is already opened and if so select it,
    // otherwise retrieve its contents

    QString url = QUrl(pUrl).toString(QUrl::NormalizePathSegments);
    QString fileName = mRemoteLocalFileNames.value(url);

    if (fileName.isEmpty()) {
        // The remote file isn't already opened, so download its contents

        QString fileContents;
        QString errorMessage;

        if (readTextFromUrl(url, fileContents, errorMessage)) {
            // We were able to retrieve the contents of the remote file, so ask
            // our file manager to create a new remote file

            Core::FileManager *fileManagerInstance = Core::FileManager::instance();

#ifdef QT_DEBUG
            Core::FileManager::Status createStatus =
#endif
            fileManagerInstance->create(url, fileContents);

#ifdef QT_DEBUG
            // Make sure that the file has indeed been created

            if (createStatus != Core::FileManager::Created)
                qFatal("FATAL ERROR | %s:%d: the remote file was not created", __FILE__, __LINE__);
#endif
        } else {
            // We were not able to retrieve the contents of the remote file, so
            // let the user know about it

            if (pShowWarning)
                QMessageBox::warning(this, tr("Open Remote File"),
                                     tr("Sorry, but <strong>%1</strong> could not be opened (%2).").arg(url, Core::formatErrorMessage(errorMessage, false)));
        }
    } else {
        openFile(fileName);
    }
}

//==============================================================================

void CentralWidget::doOpenRemoteFile()
{
    // Open the remote file

    openRemoteFile(mRemoteFileDialogUrlValue->text());

    mRemoteFileDialog->accept();
}

//==============================================================================

void CentralWidget::cancelOpenRemoteFile()
{
    // Cancel the opening of a remote file

    mRemoteFileDialog->reject();
}

//==============================================================================

void CentralWidget::openRemoteFile()
{
    // Ask for the URL of the remote file that is to be opened

    mRemoteFileDialogUrlValue->setText(QString());

    mRemoteFileDialog->exec();
}

//==============================================================================

void CentralWidget::reloadFile(const int &pIndex)
{
    // Ask our file manager to reload the file, but only if it isn't new and if
    // the user wants (in case the file has been modified)

    int realIndex = (pIndex == -1)?mFileTabs->currentIndex():pIndex;

    if (realIndex != -1) {
        FileManager *fileManagerInstance = FileManager::instance();
        QString fileName = mFileNames[realIndex];

        if (!fileManagerInstance->isNew(fileName)) {
            bool doReloadFile = true;

            if (fileManagerInstance->isModified(fileName))
                // The current file is modified, so ask the user whether s/he
                // still wants to reload it

                doReloadFile = QMessageBox::question(mMainWindow, qApp->applicationName(),
                                                     tr("<strong>%1</strong> has been modified. Do you still want to reload it?").arg(fileName),
                                                     QMessageBox::Yes|QMessageBox::No,
                                                     QMessageBox::Yes) == QMessageBox::Yes;

            // Reload the file, if needed, and consider it as non-modified
            // anymore (in case it was before)
            // Note: by making the file non-modified anymore, we clearly assume
            //       that all view plugins do their job properly and update
            //       their GUI...

            if (doReloadFile) {
                // Actually redownload the file, if it is a remote one

                if (fileManagerInstance->isRemote(fileName)) {
                    QString url = fileManagerInstance->url(fileName);
                    QString fileContents;
                    QString errorMessage;

                    if (readTextFromUrl(url, fileContents, errorMessage)) {
                        Core::writeTextToFile(fileName, fileContents);

                        fileManagerInstance->reload(fileName);
                    } else {
                        QMessageBox::warning(this, tr("Reload Remote File"),
                                             tr("Sorry, but <strong>%1</strong> could not be reloaded (%2).").arg(url, Core::formatErrorMessage(errorMessage, false)));
                    }
                } else {
                    fileManagerInstance->reload(fileName);

                    fileManagerInstance->setModified(fileName, false);
                }
            }
        }
    }
}

//==============================================================================

void CentralWidget::duplicateFile()
{
    // Make sure that the file is neither new nor modified

    QString fileName = currentFileName();
    FileManager *fileManagerInstance = FileManager::instance();

    if (fileManagerInstance->isNewOrModified(fileName))
        return;

    // Ask our file manager to duplicate the current file

#ifdef QT_DEBUG
    FileManager::Status duplicateStatus =
#endif
    fileManagerInstance->duplicate(fileName);

#ifdef QT_DEBUG
    // Make sure that the file has indeed been duplicated

    if (duplicateStatus != FileManager::Duplicated)
        qFatal("FATAL ERROR | %s:%d: '%s' did not get duplicated", __FILE__, __LINE__, qPrintable(fileName));
#endif
}

//==============================================================================

void CentralWidget::toggleLockedFile()
{
    // Make sure that the file is neither new nor modified

    QString fileName = currentFileName();
    FileManager *fileManagerInstance = FileManager::instance();

    if (fileManagerInstance->isNewOrModified(fileName))
        return;

    // Ask our file manager to toggle the locked state of the current file

    bool fileLocked = fileManagerInstance->isLocked(fileName);

    if (fileManagerInstance->setLocked(fileName, !fileLocked) == FileManager::LockedNotSet)
        QMessageBox::warning(mMainWindow, fileLocked?tr("Unlock File"):tr("Lock File"),
                             tr("Sorry, but <strong>%1</strong> could not be %2.").arg(fileName, fileLocked?tr("unlocked"):tr("locked")));
}

//==============================================================================

bool CentralWidget::saveFile(const int &pIndex, const bool &pNeedNewFileName)
{
    // Make sure that we have a valid index

    if ((pIndex < 0) || (pIndex >= mFileNames.count()))
        return false;

    // Make sure that the view plugin supports saving

    Plugin *fileViewPlugin = viewPlugin(pIndex);
    FileHandlingInterface *fileHandlingInterface = qobject_cast<FileHandlingInterface *>(fileViewPlugin->instance());
    ViewInterface *viewInterface = qobject_cast<ViewInterface *>(fileViewPlugin->instance());

    if (!fileHandlingInterface) {
        QMessageBox::warning(mMainWindow, tr("Save File"),
                             tr("Sorry, but the <strong>%1</strong> view does not support saving files.").arg(viewInterface->viewName()));

        return false;
    }

    // Make sure that we have a file name

    FileManager *fileManagerInstance = FileManager::instance();
    QString oldFileName = mFileNames[pIndex];
    QString newFileName = oldFileName;
    bool fileIsNew = fileManagerInstance->isNew(oldFileName);
    bool hasNewFileName = false;

    if (pNeedNewFileName || fileIsNew) {
        // Either we want to save the file under a new name or we are dealing
        // with a new file, so we ask the user for a file name based on the MIME
        // types supported by our current view

        QStringList mimeTypes = viewInterface->viewMimeTypes();

        QString supportedFileTypes = QString();

        foreach (const FileType &supportedFileType, mSupportedFileTypes)
            if (mimeTypes.contains(supportedFileType.mimeType())) {
                if (!supportedFileTypes.isEmpty())
                    supportedFileTypes += ";;";

                supportedFileTypes +=  supportedFileType.description()
                                      +" (*."+supportedFileType.fileExtension()+")";
            }

        newFileName = Core::getSaveFileName(tr("Save File"),
                                            fileIsNew?mFileTabs->tabToolTip(pIndex):newFileName,
                                            supportedFileTypes);

        // Make sure that a new file name was retrieved

        hasNewFileName = !newFileName.isEmpty();

        if (!hasNewFileName)
            return false;
    }

    // Try to save the file in case it has been modified or it needs a new file
    // name (either as a result of a save as or because the file was new)

    bool fileIsModified = fileManagerInstance->isModified(oldFileName);

    if (fileIsModified || hasNewFileName) {
        // Inactivate our file manager so that it doesn't check for changes in
        // files
        // Note: indeed, otherwise we will get told that the current file has
        //       changed and we will be asked whether we want to reload it...

        fileManagerInstance->setActive(false);

        if (fileIsNew || fileIsModified) {
            // The file is or has been modified, so ask the current view to save
            // it

            bool fileSaved = fileHandlingInterface->saveFile(oldFileName, newFileName);

            if (fileSaved) {
                // The file has been saved, so ask our file manager to reset its
                // settings

                fileManagerInstance->reset(oldFileName);
            } else {
                // The file couldn't be saved, so...

                QMessageBox::warning(mMainWindow, tr("Save File"),
                                     tr("Sorry, but the <strong>%1</strong> view could not save <strong>%2</strong>.").arg(viewInterface->viewName(), newFileName));

                return false;
            }
        } else {
            // The file hasn't been modified, so we just need to make a physical
            // copy of it
            // Note: there may already be a file, which name is that of the one
            //       we want to use, so remove it (if no such file exists, then
            //       nothing will happen, so we are fine)...

            QFile::remove(newFileName);

            if (!QFile::copy(oldFileName, newFileName)) {
                QMessageBox::warning(mMainWindow, tr("Save File"),
                                     tr("Sorry, but <strong>%1</strong> could not be saved.").arg(newFileName));

                return false;
            }
        }

        // Update its file name, if needed

        if (hasNewFileName) {
            // Ask our file manager to rename the file

#ifdef QT_DEBUG
            FileManager::Status renameStatus =
#endif
            fileManagerInstance->rename(oldFileName, newFileName);

#ifdef QT_DEBUG
            // Make sure that the file has indeed been renamed

            if (renameStatus != FileManager::Renamed)
                qFatal("FATAL ERROR | %s:%d: '%s' did not get renamed to '%s'", __FILE__, __LINE__, qPrintable(oldFileName), qPrintable(newFileName));
#endif
        }

        // Delete the 'old' file, if it was a new one that got saved

        if (fileIsNew)
            QFile::remove(oldFileName);

        // Let people know that the file has been saved, if needed, by reloading
        // it

        reloadFile(pIndex);

        // Update our modified settings

        updateModifiedSettings();

        // Re-activate our file manager

        fileManagerInstance->setActive(true);

        // Everything went fine, so...

        return true;
    } else {
        // Nothing to save, so...

        return false;
    }
}

//==============================================================================

void CentralWidget::saveFile()
{
    // Save the current file

    saveFile(mFileTabs->currentIndex());
}

//==============================================================================

void CentralWidget::saveFileAs()
{
    // Save the current file under a new name

    saveFile(mFileTabs->currentIndex(), true);
}

//==============================================================================

void CentralWidget::saveAllFiles()
{
    // Go through the different files and ask the current view to save them

    for (int i = 0, iMax = mFileNames.count(); i < iMax; ++i)
        saveFile(i);
}

//==============================================================================

void CentralWidget::previousFile()
{
    // Select the previous file

    if (mFileTabs->count())
        mFileTabs->setCurrentIndex(mFileTabs->currentIndex()?
                                       mFileTabs->currentIndex()-1:
                                       mFileTabs->count()-1);
}

//==============================================================================

void CentralWidget::nextFile()
{
    // Select the next file

    if (mFileTabs->count())
        mFileTabs->setCurrentIndex((mFileTabs->currentIndex() == mFileTabs->count()-1)?
                                       0:
                                       mFileTabs->currentIndex()+1);
}

//==============================================================================

bool CentralWidget::canCloseFile(const int &pIndex)
{
    FileManager *fileManagerInstance = FileManager::instance();
    QString fileName = mFileNames[pIndex];

    if (fileManagerInstance->isNewOrModified(fileName))
        // The current file is modified, so ask the user whether to save it or
        // ignore it

        switch (QMessageBox::question(mMainWindow, qApp->applicationName(),
                                      fileManagerInstance->isNew(fileName)?
                                          tr("<strong>%1</strong> is new. Do you want to save it before closing it?").arg(mFileTabs->tabToolTip(pIndex)):
                                          tr("<strong>%1</strong> has been modified. Do you want to save it before closing it?").arg(fileName),
                                      QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
                                      QMessageBox::Yes)) {
        case QMessageBox::Yes:
            // The user wants to save the file, so...

            return saveFile(pIndex);
        case QMessageBox::No:
            // The user doesn't want to save the file, so...

            return true;
        default:   // QMessageBox::Cancel
            // The user wants to cancel, so...

            return false;
        }
    else
        // The current file is not modified, so...

        return true;
}

//==============================================================================

bool CentralWidget::closeFile(const int &pIndex, const bool &pForceClosing)
{
    if (mState == UpdatingGui)
        // We are updating the GUI, so we can't close the file for now

        return false;

    // Close the file at the given tab index or the current tab index, if no tab
    // index is provided, and then return the name of the file that was closed,
    // if any
    // Note: pIndex can take one of the following values:
    //        - 0+: the index of the file to close;
    //        - -1: the current file is to be closed; and
    //        - -2: same as -1, except that we come here with the intention of
    //              closing all the files (see closeAllFiles()).

    bool closingAllFiles = pIndex == -2;
    int realIndex = (pIndex < 0)?mFileTabs->currentIndex():pIndex;

    if (realIndex != -1) {
        // There is a file currently opened, so check whether the file can be
        // closed
        // Note: this may involve saving the file (possibly under a new name),
        //       hence we must retrieve the file name after making sure that the
        //       file can be closed...

        if (!closingAllFiles && !pForceClosing && !canCloseFile(realIndex))
            // The file cannot be closed, so...

            return false;

        // Retrieve the file name

        QString fileName = mFileNames[realIndex];

        // Update some of our internals

        mFileNames.removeAt(realIndex);

        mFileModeTabIndexes.remove(fileName);
        mFileModeViewTabIndexes.remove(fileName);

        FileManager *fileManagerInstance = FileManager::instance();

        if (fileManagerInstance->isRemote(fileName))
            mRemoteLocalFileNames.remove(fileManagerInstance->url(fileName));

        // Remove the file tab

        mFileTabs->removeTab(realIndex);

        // Ask our view plugins to remove the corresponding view for the file

        foreach (Plugin *plugin, mLoadedViewPlugins) {
            ViewInterface *viewInterface = qobject_cast<ViewInterface *>(plugin->instance());

            mContents->removeWidget(viewInterface->viewWidget(fileName, false));

            viewInterface->removeViewWidget(fileName);
        }

        // Let our plugins know about the file having just been closed

        foreach (Plugin *plugin, mLoadedFileHandlingPlugins)
            qobject_cast<FileHandlingInterface *>(plugin->instance())->fileClosed(fileName);

        // Unregister the file from our file manager

        FileManager::instance()->unmanage(fileName);

        // Update our modified settings

        updateModifiedSettings();

        // Everything went fine, so...

        return true;
    } else {
        // The file is not currently opened, so...

        return false;
    }
}

//==============================================================================

void CentralWidget::closeAllFiles(const bool &pForceClosing)
{
    // Check whether we can close all the files

    if (!pForceClosing && !canClose())
        // We can't close all the files, so...

        return;

    // Close all the files

    while (closeFile(-2, pForceClosing)) {}
}

//==============================================================================

void CentralWidget::moveFile(const int &pFromIndex, const int &pToIndex)
{
    // Update our list of file names to reflect the fact that a tab has been
    // moved

    mFileNames.move(pFromIndex, pToIndex);
}

//==============================================================================

bool CentralWidget::canClose()
{
    // We can close only if none of the opened files is modified

    for (int i = 0, iMax = mFileTabs->count(); i < iMax; ++i)
        if (!canCloseFile(i))
            return false;

    // We checked all the opened files, so...

    return true;
}

//==============================================================================

void CentralWidget::addView(Plugin *pPlugin)
{
    // Make sure that our list of required modes is up to date

    ViewInterface *viewInterface = qobject_cast<ViewInterface *>(pPlugin->instance());
    ViewInterface::Mode viewMode = viewInterface->viewMode();

    if (!mModes.value(viewMode)->isEnabled()) {
        // There is no tab for the mode, so add one and enable it

        int tabIndex = mModeTabs->addTab(QString());

        mModes.value(viewMode)->setEnabled(true);

        // Keep track of the correspondance mode type/index

        mModeTabIndexModes.insert(tabIndex, viewMode);
        mModeModeTabIndexes.insert(viewMode, tabIndex);
    }

    // Add the requested view to the mode's views tab bar

    CentralWidgetMode *mode = mModes.value(viewMode);
    int modeViewTabIndex = mode->viewTabs()->addTab(QString());

    mode->viewPlugins()->insert(modeViewTabIndex, pPlugin);
}

//==============================================================================

void CentralWidget::dragEnterEvent(QDragEnterEvent *pEvent)
{
    // Accept the proposed action for the event, but only if at least one mode
    // is available and if we are dropping URIs or items from our file organiser

    if (   mModeTabs->count()
        && (pEvent->mimeData()->hasFormat(FileSystemMimeType))
        && (!pEvent->mimeData()->urls().isEmpty())) {
        // Note: we test the list of URLs in case we are trying to drop one or
        //       several folders (and no file) from the file organiser, in which
        //       case the list of URLs will be empty...

        pEvent->acceptProposedAction();
    } else {
        pEvent->ignore();
    }
}

//==============================================================================

void CentralWidget::dragMoveEvent(QDragMoveEvent *pEvent)
{
    // Accept the proposed action for the event
    // Note: this is required when dragging external objects...

    pEvent->acceptProposedAction();
}

//==============================================================================

void CentralWidget::dropEvent(QDropEvent *pEvent)
{
    // Retrieve the name of the various files that have been dropped

    QList<QUrl> urlList = pEvent->mimeData()->urls();
    QStringList fileNames;

    for (int i = 0, iMax = urlList.count(); i < iMax; ++i)
    {
        QString fileName = urlList[i].toLocalFile();
        QFileInfo fileInfo = fileName;

        if (fileInfo.isFile()) {
            if (fileInfo.isSymLink()) {
                // We are dropping a symbolic link, so retrieve its target and
                // check that it exists, and if it does then add it

                fileName = fileInfo.symLinkTarget();

                if (QFile::exists(fileName))
                    fileNames << fileName;
            } else {
                // We are dropping a file, so we can just add it

                fileNames << fileName;
            }
        }
    }

    // fileNames may contain duplicates (in case we dropped some symbolic
    // links), so remove them

    fileNames.removeDuplicates();

    // Open the various files

    openFiles(fileNames);

    // Accept the proposed action for the event

    pEvent->acceptProposedAction();
}

//==============================================================================

Plugin * CentralWidget::viewPlugin(const int &pIndex) const
{
    // Return the view plugin associated with the file, which index is given

    if (pIndex != -1) {
        CentralWidgetMode *mode = mModes.value(mModeTabIndexModes.value(mFileModeTabIndexes.value(mFileNames[pIndex])));

        return mode->viewPlugins()->value(mode->viewTabs()->currentIndex());
    } else {
        return 0;
    }
}

//==============================================================================

Plugin * CentralWidget::viewPlugin(const QString &pFileName) const
{
    // Return the view plugin associated with the file, which file name is given

    for (int i = 0, iMax = mFileNames.count(); i < iMax; ++i)
        if (!pFileName.compare(mFileNames[i]))
            return viewPlugin(i);

    return 0;
}

//==============================================================================

void CentralWidget::updateGui()
{
    if (mState != Idling)
        // We are doing something, so too risky to update the GUI during that
        // time (e.g. things may not be fully initialised)

        return;

    // Update our state to reflect the fact that we are updating the GUI

    mState = UpdatingGui;

    // Determine whether we are here as a result of changing files, modes or
    // views

    bool changedFiles = sender() == mFileTabs;
    bool changedModes = sender() == mModeTabs;
    bool changedViews = false;

    foreach (CentralWidgetMode *mode, mModes.values())
        if (sender() == mode->viewTabs()) {
            changedViews = true;

            break;
        }

    bool directCall = !changedFiles && !changedModes && !changedViews;

    // Set or keep track of the mode and view for the current file

    QString fileName = currentFileName();

    if (!fileName.isEmpty()) {
        int fileModeTabIndex = mFileModeTabIndexes.value(fileName, -1);

        // Set the mode and view for the current file, should we know them and
        // should we be changing files or coming here directly, or set the view
        // for the current file, should we be changing modes

        if (((fileModeTabIndex != -1) && (changedFiles || directCall)) || changedModes) {
            if (changedModes)
                fileModeTabIndex = mModeTabs->currentIndex();
            else
                mModeTabs->setCurrentIndex(fileModeTabIndex);

            CentralWidgetMode *mode = mModes.value(mModeTabIndexModes.value(fileModeTabIndex));
            QMap<int, int> modeViewTabIndexes = mFileModeViewTabIndexes.value(fileName);

            mode->viewTabs()->setCurrentIndex(modeViewTabIndexes.value(fileModeTabIndex));
        }

        // Keep track of the mode and view for the current file, should we not
        // have any track of them or should we be changing modes or views

        if ((fileModeTabIndex == -1) || changedModes || changedViews) {
            fileModeTabIndex = mModeTabs->currentIndex();

            mFileModeTabIndexes.insert(fileName, fileModeTabIndex);

            QMap<int, int> modeViewTabIndexes = mFileModeViewTabIndexes.value(fileName);
            ViewInterface::Mode fileMode = mModeTabIndexModes.value(fileModeTabIndex);

            modeViewTabIndexes.insert(fileModeTabIndex, mModes.value(fileMode)->viewTabs()->currentIndex());
            mFileModeViewTabIndexes.insert(fileName, modeViewTabIndexes);
        }
    }

    // Show/hide the editing, simulation and analysis modes' corresponding views
    // tab, as needed, and retrieve the corresponding view plugin

    int fileModeTabIndex = mModeTabs->currentIndex();

    mModes.value(ViewInterface::Editing)->viewTabs()->setVisible(fileModeTabIndex == mModeModeTabIndexes.value(ViewInterface::Editing));
    mModes.value(ViewInterface::Simulation)->viewTabs()->setVisible(fileModeTabIndex == mModeModeTabIndexes.value(ViewInterface::Simulation));
    mModes.value(ViewInterface::Analysis)->viewTabs()->setVisible(fileModeTabIndex == mModeModeTabIndexes.value(ViewInterface::Analysis));

    // Ask the GUI interface for the widget to use for the current file (should
    // there be one)

    Plugin *fileViewPlugin = viewPlugin(mFileTabs->currentIndex());
    ViewInterface *viewInterface = fileViewPlugin?qobject_cast<ViewInterface *>(fileViewPlugin->instance()):0;
    QWidget *newView;

    if (fileName.isEmpty()) {
        newView = mLogoView;
    } else {
        // There is a current file, so retrieve its view

        newView = viewInterface?viewInterface->viewWidget(fileName):0;

        if (!newView) {
            // The interface doesn't have a view for the current file, so use
            // our no-view widget instead and update its message

            newView = mNoViewMsg;

            updateNoViewMsg();
        }
    }

    // Create a connection to update the tab icon for the current file and
    // update our status bar widgets, but all of this only if we have a proper
    // view for our current file
    // Note: we pass Qt::UniqueConnection in our call to connect() so that we
    //       don't end up with several identical connections (something that
    //       might happen if we were to switch views and back)...

    ViewWidget *newViewWidget = dynamic_cast<ViewWidget *>(newView);

    if (newViewWidget) {
        connect(newViewWidget, SIGNAL(updateFileTabIcon(const QString &, const QString &, const QIcon &)),
                this, SLOT(updateFileTabIcon(const QString &, const QString &, const QIcon &)),
                Qt::UniqueConnection);

        updateStatusBarWidgets(newViewWidget->statusBarWidgets());
    } else {
        updateStatusBarWidgets(QList<QWidget *>());
    }

    // and also create a connection for it, should it be be of
    // the right type
    // Note: we pass Qt::UniqueConnection in all our calls to connect()
    //       to ensure that we don't have several identical connections
    //       (something that might happen if we were to switch views and
    //       back)...

    // Let people know that we are about to update the GUI

    emit guiUpdated(fileViewPlugin, fileName);

    // Replace the current view with the new one
    // Note: the order in which the adding and removing (as well as the
    //       showing/hiding) of view is done to ensures that the replacement
    //       looks as good as possible...

    mContents->removeWidget(mContents->currentWidget());
    mContents->addWidget(newView);

    // Give the focus to the new view after first checking whether it has a
    // focused widget

    if (newView->focusWidget())
        // The new view has a focused widget, so just focus it (indeed, say that
        // we are using the CellML Annotation view and that we are editing some
        // metadata, then we don't want the CellML element list to get the focus
        // back, so...)

        newView->focusWidget()->setFocus();
    else
        // The new view doesn't have a focused widget, so simply give the focus
        // to our new view

        newView->setFocus();

    // Update our modified settings

    updateModifiedSettings();

    // Let people know whether we can save as, as well as whether there is/are
    // at least one/two file/s

    emit canSaveAs(   mFileTabs->count() && (newView != mNoViewMsg)
                   && !FileManager::instance()->isRemote(fileName)
                   && viewInterface);

    emit atLeastOneFile(mFileTabs->count());
    emit atLeastTwoFiles(mFileTabs->count() > 1);

    // We are done with updating the GUI, so...

    mState = Idling;
}

//==============================================================================

QTabBar * CentralWidget::newTabBar(const QTabBar::Shape &pShape,
                                   const bool &pMovable,
                                   const bool &pTabsClosable)
{
    // Create and return a tab bar

    QTabBar *res = new QTabBar(this);

    res->setExpanding(false);
    // Note: if the above property is not enabled and many files are opened,
    //       then the central widget will widen reducing the width of any docked
    //       window which is clearly not what we want, so...
    res->setFocusPolicy(Qt::NoFocus);
    res->setMovable(pMovable);
    res->setShape(pShape);
    res->setTabsClosable(pTabsClosable);
    res->setUsesScrollButtons(true);
    // Note: the above property is style dependent and it happens that it's not
    //       enabled on OS X, so... set it in all cases, even though it's
    //       already set on Windows and Linux, but one can never know what the
    //       future holds, so...

    return res;
}

//==============================================================================

void CentralWidget::updateNoViewMsg()
{
    // Customise, if possible, our no view widget so that it shows a relevant
    // warning message

    int fileModeTabIndex = mModeTabs->currentIndex();

    if (fileModeTabIndex == -1) {
        // There are no modes (and therefore no views) available, so leave

        return;
    } else {
        CentralWidgetMode *mode = mModes.value(mModeTabIndexModes.value(fileModeTabIndex));

        mNoViewMsg->setMessage(tr("Sorry, but the <strong>%1</strong> view does not support this type of file...").arg(qobject_cast<ViewInterface *>(mode->viewPlugins()->value(mode->viewTabs()->currentIndex())->instance())->viewName()));
    }
}

//==============================================================================

void CentralWidget::fileChanged(const QString &pFileName)
{
    // The given file has been changed, so ask the user whether to reload it

    if (QMessageBox::question(mMainWindow, qApp->applicationName(),
                              tr("<strong>%1</strong> has been modified. Do you want to reload it?").arg(pFileName),
                              QMessageBox::Yes|QMessageBox::No,
                              QMessageBox::Yes) == QMessageBox::Yes) {
        // The user wants to reload the file

        for (int i = 0, iMax = mFileNames.count(); i < iMax; ++i)
            if (!mFileNames[i].compare(pFileName)) {
                // We have found the file to reload

                reloadFile(i);

                break;
            }
    } else {
        // The user doesn't want to reload the file, so consider it as modified

        FileManager::instance()->setModified(pFileName, true);
    }
}

//==============================================================================

void CentralWidget::fileDeleted(const QString &pFileName)
{
    // The given file doesn't exist anymore, so ask the user whether to close it

    if (QMessageBox::question(mMainWindow, qApp->applicationName(),
                              tr("<strong>%1</strong> does not exist anymore. Do you want to close it?").arg(pFileName),
                              QMessageBox::Yes|QMessageBox::No,
                              QMessageBox::Yes) == QMessageBox::Yes) {
        // The user wants to close the file

        for (int i = 0, iMax = mFileNames.count(); i < iMax; ++i)
            if (!mFileNames[i].compare(pFileName)) {
                // We have found the file to close

                closeFile(i, true);

                break;
            }
    } else {
        // The user wants to keep the file, so consider it as modified

        FileManager::instance()->setModified(pFileName, true);
    }
}

//==============================================================================

void CentralWidget::updateModifiedSettings()
{
    // Update all our file tabs and determine the number of modified files

    FileManager *fileManagerInstance = FileManager::instance();
    int nbOfNewOrModifiedFiles = 0;

    for (int i = 0, iMax = mFileTabs->count(); i < iMax; ++i) {
        updateFileTab(i);

        if (fileManagerInstance->isNewOrModified(mFileNames[i]))
            ++nbOfNewOrModifiedFiles;
    }

    // Reset the enabled state and tool tip of all our View tabs

    foreach (CentralWidgetMode *mode, mModes) {
        QTabBar *viewTabs = mode->viewTabs();

        viewTabs->setEnabled(true);
        viewTabs->setToolTip(QString());
    }

    // Enable/disable the current mode's View tabs, in case the current file has
    // been modified

    QString fileName = mFileTabs->count()?
                           mFileNames[mFileTabs->currentIndex()]:
                           QString();
    bool fileIsNewOrModified = fileManagerInstance->isNewOrModified(fileName);

    if (fileIsNewOrModified) {
        QTabBar *viewTabs = mModes.value(mModeTabIndexModes.value(mModeTabs->currentIndex()))->viewTabs();

        viewTabs->setEnabled(false);
        viewTabs->setToolTip(fileManagerInstance->isNew(fileName)?
                                 tr("The file is new, so switching views is not possible for now"):
                                 tr("The file is being edited, so switching views is not possible for now"));
    }

    // Let people know whether we can save the current file and/or all files

    emit canSave(fileIsNewOrModified);
    emit canSaveAll(nbOfNewOrModifiedFiles);
}

//==============================================================================

void CentralWidget::filePermissionsChanged(const QString &pFileName)
{
    // Update the tab text for the given file

    for (int i = 0, iMax = mFileNames.count(); i < iMax; ++i)
        if (!pFileName.compare(mFileNames[i])) {
            updateFileTab(i);

            break;
        }

    // Let our plugins know about the file having had its permissions changed

    foreach (Plugin *plugin, mLoadedFileHandlingPlugins)
        qobject_cast<FileHandlingInterface *>(plugin->instance())->filePermissionsChanged(pFileName);
}

//==============================================================================

void CentralWidget::fileModified(const QString &pFileName)
{
    // Let our plugins know about the file having been modified

    foreach (Plugin *plugin, mLoadedFileHandlingPlugins)
        qobject_cast<FileHandlingInterface *>(plugin->instance())->fileModified(pFileName);
}

//==============================================================================

void CentralWidget::fileReloaded(const QString &pFileName)
{
    // Let our plugins know about the file having been reloaded, but ignore the
    // current plugin if our file manager is inactive
    // Note: indeed, if our file manager is inactive, then it means that we are
    //       saving the file (see saveFile()), hence we don't need and don't
    //       want (since it may mess up our current view; e.g. the caret of a
    //       QScintilla-based view will get moved back to its original position)
    //       our current plugin to reload it...

    FileManager *fileManagerInstance = FileManager::instance();
    Plugin *fileViewPlugin = viewPlugin(pFileName);

    foreach (Plugin *plugin, mLoadedFileHandlingPlugins)
        if (fileManagerInstance->isActive() || (plugin != fileViewPlugin))
            qobject_cast<FileHandlingInterface *>(plugin->instance())->fileReloaded(pFileName);

    // Now, because of the way some our views may reload a file (see
    // CoreEditingPlugin::fileReloaded()), we need to tell them to update their
    // GUI

    foreach (Plugin *plugin, mLoadedGuiPlugins)
        if (fileManagerInstance->isActive() || (plugin != fileViewPlugin))
            qobject_cast<GuiInterface *>(plugin->instance())->updateGui(fileViewPlugin, pFileName);
}

//==============================================================================

void CentralWidget::fileCreated(const QString &pFileName, const QString &pUrl)
{
    // A file got created, so open it as new if no URL is provided or remote
    // otherwise

    openFile(pFileName, pUrl.isEmpty()?File::New:File::Remote, pUrl);
}

//==============================================================================

void CentralWidget::fileDuplicated(const QString &pFileName)
{
    // A file got duplicated, so open it as new

    openFile(pFileName, File::New);
}

//==============================================================================

void CentralWidget::fileRenamed(const QString &pOldFileName,
                                const QString &pNewFileName)
{
    // A file has been renamed, so update various things

    for (int i = 0, iMax = mFileNames.count(); i < iMax; ++i)
        if (!mFileNames[i].compare(pOldFileName)) {
            // Update our internal copy of the file name

            mFileNames[i] = pNewFileName;

            // Update the model and view raw names

            mFileModeTabIndexes.insert(pNewFileName, mFileModeTabIndexes.value(pOldFileName));
            mFileModeViewTabIndexes.insert(pNewFileName, mFileModeViewTabIndexes.value(pOldFileName));

            mFileModeTabIndexes.remove(pOldFileName);
            mFileModeViewTabIndexes.remove(pOldFileName);

            // Update the file tab

            mFileTabs->setTabText(i, QFileInfo(pNewFileName).fileName());
            mFileTabs->setTabToolTip(i, pNewFileName);

            // Let our plugins know about a file having been renamed

            foreach (Plugin *plugin, mLoadedFileHandlingPlugins)
                qobject_cast<FileHandlingInterface *>(plugin->instance())->fileRenamed(pOldFileName, pNewFileName);

            break;
        }
}

//==============================================================================

void CentralWidget::updateFileTabIcon(const QString &pViewName,
                                      const QString &pFileName,
                                      const QIcon &pIcon)
{
    // Update the requested file tab icon, but only if the view plugin (from
    // which the signal was emitted) is the one currently active

    if (!pViewName.compare(qobject_cast<ViewInterface *>(viewPlugin(pFileName)->instance())->viewName()))
        // The view from which the signal was emitted is the currently active
        // one, so we can try to handle its signal

        for (int i = 0, iMax = mFileTabs->count(); i < iMax; ++i)
            if (!pFileName.compare(mFileNames[i])) {
                // This is the file tab for which we want to update the icon, so
                // do it and leave

                if (pIcon.isNull())
                    updateFileTab(i);
                else
                    mFileTabs->setTabIcon(i, pIcon);

                return;
            }
}

//==============================================================================

void CentralWidget::updateFileTabIcons()
{
    // Update all the file tab icons

    for (int i = 0, iMax = mFileTabs->count(); i < iMax; ++i) {
        QIcon tabIcon = qobject_cast<ViewInterface *>(viewPlugin(i)->instance())->fileTabIcon(mFileNames[i]);

        if (tabIcon.isNull())
            updateFileTab(i);
        else
            mFileTabs->setTabIcon(i, tabIcon);
    }
}

//==============================================================================

void CentralWidget::updateStatusBarWidgets(QList<QWidget *> pWidgets)
{
    // Remove (hide) our existing status bar widgets

    static QList<QWidget *> statusBarWidgets = QList<QWidget *>();

    foreach (QWidget *statusBarWidget, statusBarWidgets)
        mMainWindow->statusBar()->removeWidget(statusBarWidget);

    // Add and show the given status bar widgets, and keep track of them

    statusBarWidgets.clear();

    foreach (QWidget *widget, pWidgets) {
        mMainWindow->statusBar()->addWidget(widget);

        widget->show();

        statusBarWidgets << widget;
    }
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================