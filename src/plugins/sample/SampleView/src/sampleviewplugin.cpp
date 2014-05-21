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
// SampleView plugin
//==============================================================================

#include "sampleviewplugin.h"
#include "sampleviewwidget.h"

//==============================================================================

#include <QMainWindow>

//==============================================================================

#include <QIcon>

//==============================================================================

namespace OpenCOR {
namespace SampleView {

//==============================================================================

PLUGININFO_FUNC SampleViewPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("a plugin that provides a test view."));
    descriptions.insert("fr", QString::fromUtf8("une extension qui fournit une vue de test."));

    return new PluginInfo(PluginInfo::Sample, true, false,
                          QStringList() << "Core",
                          descriptions);
}

//==============================================================================

SampleViewPlugin::SampleViewPlugin() :
    mFileName(QString())
{
}

//==============================================================================
// File handling interface
//==============================================================================

bool SampleViewPlugin::saveFile(const QString &pOldFileName,
                                const QString &pNewFileName)
{
    Q_UNUSED(pOldFileName);
    Q_UNUSED(pNewFileName);

    // We don't handle this interface...

    return false;
}

//==============================================================================

void SampleViewPlugin::fileOpened(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

void SampleViewPlugin::filePermissionsChanged(const QString &pFileName)
{
    // The given file has had its permissions changed, so re-initialise our
    // view widget, if needed

    if (!pFileName.compare(mFileName))
        mViewWidget->initialize(pFileName);
}

//==============================================================================

void SampleViewPlugin::fileModified(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

void SampleViewPlugin::fileReloaded(const QString &pFileName)
{
    // The given file has been reloaded, so re-initialise our view widget, if
    // needed

    if (!pFileName.compare(mFileName))
        mViewWidget->initialize(pFileName);
}

//==============================================================================

void SampleViewPlugin::fileRenamed(const QString &pOldFileName,
                                   const QString &pNewFileName)
{
    Q_UNUSED(pOldFileName);

    // The given file has been renamed, so re-initialise our view widget

    if (!pOldFileName.compare(mFileName)) {
        mFileName = pNewFileName;

        mViewWidget->initialize(pNewFileName);
    }
}

//==============================================================================

void SampleViewPlugin::fileClosed(const QString &pFileName)
{
    // The given file has been closed, so update our internals, if needed

    if (!pFileName.compare(mFileName))
        mFileName = QString();
}

//==============================================================================
// I18n interface
//==============================================================================

void SampleViewPlugin::retranslateUi()
{
    // Retranslate our view widget, if needed

    if (!mFileName.isEmpty())
        mViewWidget->retranslateUi();
}

//==============================================================================
// Plugin interface
//==============================================================================

void SampleViewPlugin::initializePlugin(QMainWindow *pMainWindow)
{
    // Create our sample view widget

    mViewWidget = new SampleViewWidget(pMainWindow);

    // Hide our sample view widget since it may not initially be shown in our
    // central widget

    mViewWidget->setVisible(false);
}

//==============================================================================

void SampleViewPlugin::finalizePlugin()
{
    // We don't handle this interface...
}

//==============================================================================

void SampleViewPlugin::pluginInitialized(const Plugins &pLoadedPlugins)
{
    Q_UNUSED(pLoadedPlugins);

    // We don't handle this interface...
}

//==============================================================================

void SampleViewPlugin::loadSettings(QSettings *pSettings)
{
    Q_UNUSED(pSettings);

    // We don't handle this interface...
}

//==============================================================================

void SampleViewPlugin::saveSettings(QSettings *pSettings) const
{
    Q_UNUSED(pSettings);

    // We don't handle this interface...
}

//==============================================================================

void SampleViewPlugin::handleAction(const QUrl &pUrl)
{
    Q_UNUSED(pUrl);

    // We don't handle this interface...
}

//==============================================================================
// View interface
//==============================================================================

ViewInterface::Mode SampleViewPlugin::viewMode() const
{
    // Return our mode

    return ViewInterface::Sample;
}

//==============================================================================

QStringList SampleViewPlugin::viewMimeTypes() const
{
    // Return the MIME types we support
    // Note: we allow any kind of file, hence our empty string list...

    return QStringList();
}

//==============================================================================

void SampleViewPlugin::initializeView()
{
    // We don't handle this interface...
}

//==============================================================================

void SampleViewPlugin::finalizeView()
{
    // We don't handle this interface...
}

//==============================================================================

QWidget * SampleViewPlugin::viewWidget(const QString &pFileName,
                                       const bool &pCreate)
{
    // Update our sample view widget using the given file

    if (pCreate) {
        mFileName = pFileName;

        mViewWidget->initialize(pFileName);

        return mViewWidget;
    } else {
        mFileName = QString();

        return 0;
    }
}

//==============================================================================

void SampleViewPlugin::removeViewWidget(const QString &pFileName)
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...
}

//==============================================================================

QString SampleViewPlugin::viewName() const
{
    // Return our sample view's name

    return tr("Sample");
}

//==============================================================================

QIcon SampleViewPlugin::fileTabIcon(const QString &pFileName) const
{
    Q_UNUSED(pFileName);

    // We don't handle this interface...

    return QIcon();
}

//==============================================================================

}   // namespace SampleView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
