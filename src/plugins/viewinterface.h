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
// View interface
//==============================================================================

#ifndef VIEWINTERFACE_H
#define VIEWINTERFACE_H

//==============================================================================

#include <QString>
#include <QWidget>

//==============================================================================

namespace OpenCOR {

//==============================================================================

class ViewInterface
{
public:
    enum Mode {
        Unknown,
#ifdef ENABLE_SAMPLES
        Sample,
#endif
        Editing,
        Simulation,
        Analysis
    };

#define INTERFACE_DEFINITION
    #include "viewinterface.inl"
#undef INTERFACE_DEFINITION

    static QString viewModeAsString(const Mode &pMode);
    static Mode viewModeFromString(const QString &pMode);
};

//==============================================================================

}   // namespace OpenCOR

//==============================================================================

Q_DECLARE_INTERFACE(OpenCOR::ViewInterface, "OpenCOR::ViewInterface")

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
