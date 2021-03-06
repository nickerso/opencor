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
// CLI utilities
//==============================================================================

#ifndef CLIUTILS_H
#define CLIUTILS_H

//==============================================================================

#include <QSslError>
#include <QString>

//==============================================================================

class QCoreApplication;
class QNetworkReply;

//==============================================================================

namespace OpenCOR {

//==============================================================================

#include "coreglobal.h"

//==============================================================================

#include "corecliutils.h.inl"

//==============================================================================
// Note: both cliutils.h and corecliutils.h must specifically define
//       SynchronousTextFileDownloader. To have it in cliutils.h.inl is NOT good
//       enough since the MOC won't pick it up...

class SynchronousTextFileDownloader : public QObject
{
    Q_OBJECT

public:
    bool readTextFromUrl(const QString &pUrl, QString &pText,
                         QString *pErrorMessage) const;

private Q_SLOTS:
    void networkAccessManagerSslErrors(QNetworkReply *pNetworkReply,
                                       const QList<QSslError> &pSslErrors);
};

//==============================================================================

void initQtMessagePattern();

void initPluginsPath(const QString &pAppFileName);

void initApplication(QCoreApplication *pApp, QString *pAppDate = 0);

bool cliApplication(QCoreApplication *pApp, int *pRes);

void removeGlobalInstances();

QString shortVersion(QCoreApplication *pApp);
QString version(QCoreApplication *pApp);

//==============================================================================

}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
