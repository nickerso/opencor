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
// SampleTools plugin
//==============================================================================

#ifndef SAMPLETOOLSPLUGIN_H
#define SAMPLETOOLSPLUGIN_H

//==============================================================================

#include "cliinterface.h"
#include "guiinterface.h"
#include "i18ninterface.h"
#include "plugininfo.h"
#include "plugininterface.h"

//==============================================================================

namespace OpenCOR {
namespace SampleTools {

//==============================================================================

PLUGININFO_FUNC SampleToolsPluginInfo();

//==============================================================================

class SampleToolsPlugin : public QObject, public CliInterface,
                          public GuiInterface, public I18nInterface,
                          public PluginInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "OpenCOR.SampleToolsPlugin" FILE "sampletoolsplugin.json")

    Q_INTERFACES(OpenCOR::CliInterface)
    Q_INTERFACES(OpenCOR::GuiInterface)
    Q_INTERFACES(OpenCOR::I18nInterface)
    Q_INTERFACES(OpenCOR::PluginInterface)

public:
#include "cliinterface.inl"
#include "guiinterface.inl"
#include "i18ninterface.inl"
#include "plugininterface.inl"

private:
    QMainWindow *mMainWindow;

    QAction *mAddTwoNumbersAction;

    void runHelpCommand();
    int runAddCommand(const QStringList &pArguments);

private Q_SLOTS:
    void addTwoNumbers();
};

//==============================================================================

}   // namespace SampleTools
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
