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
// CSV data store exporter class
//==============================================================================

#include "corecliutils.h"
#include "coreguiutils.h"
#include "coredatastore.h"
#include "csvdatastoreexporter.h"
#include "datastorevariable.h"

//==============================================================================

#include <QApplication>
#include <QFile>
#include <QTextStream>

//==============================================================================

namespace OpenCOR {
namespace CSVDataStore {

//==============================================================================

void CsvDataStoreExporter::execute(CoreDataStore::CoreDataStore *pDataStore) const
{
    // Export the given data store to a CSV file

    QString fileName = Core::getSaveFileName(QObject::tr("Export to a CSV file"),
                                             QString(),
                                             QObject::tr("CSV File")+" (*.csv)");

    if (!fileName.isEmpty()) {
        // Header

        static const QString Header = "%1 (%2)";

        CoreDataStore::DataStoreVariable *voi = pDataStore->voi();
        CoreDataStore::DataStoreVariables variables = pDataStore->variables();

        QString data = QString();

        data += Header.arg(voi->uri().replace("/prime", "'").replace("/", " | "),
                           voi->unit());

        auto variableBegin = variables.constBegin();
        auto variableEnd = variables.constEnd();

        for (auto variable = variableBegin; variable != variableEnd; ++variable) {
            data += ","+Header.arg((*variable)->uri().replace("/prime", "'").replace("/", " | "),
                                   (*variable)->unit());
        }

        data += "\n";

        // Data itself

        for (qulonglong i = 0; i < pDataStore->size(); ++i) {
            data += QString::number(voi->value(i));

            for (auto variable = variableBegin; variable != variableEnd; ++variable)
                data += ","+QString::number((*variable)->value(i));

            data += "\n";

            qApp->processEvents();
//---GRY--- THE CALL TO qApp->processEvents() SHOULD BE REMOVED AND THE EXPORTER
//          BE SUCH THAT IT DOESN'T BLOCK THE MAIN THREAD (E.G. WHEN EXPORTING
//          LONG SIMULATIONS). MAYBE THIS COULD BE DONE BY MAKING THE EXPORTER
//          WORK IN ITS OWN THREAD?...
        }

        // The data is ready, so write it to the file

        Core::writeTextToFile(fileName, data);
    }
}

//==============================================================================

}   // namespace CSVDataStore
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
