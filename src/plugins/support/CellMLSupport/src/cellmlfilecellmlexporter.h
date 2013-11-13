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
// CellML file CellML exporter
//==============================================================================

#ifndef CELLMLFILECELLMLEXPORTER_H
#define CELLMLFILECELLMLEXPORTER_H

//==============================================================================

#include "cellmlfileexporter.h"

//==============================================================================

#include <QString>

//==============================================================================

#include "cellml-api-cxx-support.hpp"

//==============================================================================

namespace OpenCOR {
namespace CellMLSupport {

//==============================================================================

static const std::wstring MATHML_NAMESPACE = L"http://www.w3.org/1998/Math/MathML";

static const std::wstring CELLML_1_0_NAMESPACE = L"http://www.cellml.org/cellml/1.0#";
static const std::wstring CELLML_1_1_NAMESPACE = L"http://www.cellml.org/cellml/1.1#";

//==============================================================================

class CellmlFileCellmlExporter : public CellmlFileExporter
{
public:
    explicit CellmlFileCellmlExporter(iface::cellml_api::Model *pModel,
                                      const std::wstring &pVersion);

protected:
    iface::cellml_api::Model *mModel;
    ObjRef<iface::cellml_api::Model> mExportedModel;
};

//==============================================================================

}   // namespace CellMLSupport
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================