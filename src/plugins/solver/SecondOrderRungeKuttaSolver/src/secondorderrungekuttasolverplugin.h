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
// SecondOrderRungeKuttaSolver plugin
//==============================================================================

#ifndef SECONDORDERRUNGEKUTTASOLVERPLUGIN_H
#define SECONDORDERRUNGEKUTTASOLVERPLUGIN_H

//==============================================================================

#include "i18ninterface.h"
#include "plugininfo.h"
#include "solverinterface.h"

//==============================================================================

namespace OpenCOR {
namespace SecondOrderRungeKuttaSolver {

//==============================================================================

PLUGININFO_FUNC SecondOrderRungeKuttaSolverPluginInfo();

//==============================================================================

class SecondOrderRungeKuttaSolverPlugin : public QObject,
                                          public I18nInterface,
                                          public SolverInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "OpenCOR.SecondOrderRungeKuttaSolverPlugin" FILE "secondorderrungekuttasolverplugin.json")

    Q_INTERFACES(OpenCOR::I18nInterface)
    Q_INTERFACES(OpenCOR::SolverInterface)

public:
#include "i18ninterface.inl"
#include "solverinterface.inl"
};

//==============================================================================

}   // namespace SecondOrderRungeKuttaSolver
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
