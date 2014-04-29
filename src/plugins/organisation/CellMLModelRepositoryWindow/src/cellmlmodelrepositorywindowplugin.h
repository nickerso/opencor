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
// CellMLModelRepositoryWindow plugin
//==============================================================================

#ifndef CELLMLMODELREPOSITORYWINDOWPLUGIN_H
#define CELLMLMODELREPOSITORYWINDOWPLUGIN_H

//==============================================================================

#include "guiinterface.h"
#include "i18ninterface.h"
#include "plugininterface.h"
#include "plugininfo.h"

//==============================================================================

namespace OpenCOR {
namespace CellMLModelRepositoryWindow {

//==============================================================================

PLUGININFO_FUNC CellMLModelRepositoryWindowPluginInfo();

//==============================================================================

class CellmlModelRepositoryWindowWindow;

//==============================================================================

class CellMLModelRepositoryWindowPlugin : public QObject, public GuiInterface,
                                          public I18nInterface,
                                          public PluginInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "OpenCOR.CellMLModelRepositoryWindowPlugin" FILE "cellmlmodelrepositorywindowplugin.json")

    Q_INTERFACES(OpenCOR::GuiInterface)
    Q_INTERFACES(OpenCOR::I18nInterface)
    Q_INTERFACES(OpenCOR::PluginInterface)

public:
#include "guiinterface.inl"
#include "i18ninterface.inl"
#include "plugininterface.inl"

private:
    QAction *mCellmlModelRepositoryAction;

    CellmlModelRepositoryWindowWindow *mCellmlModelRepositoryWindow;
};

//==============================================================================

}   // namespace CellMLModelRepositoryWindow
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
