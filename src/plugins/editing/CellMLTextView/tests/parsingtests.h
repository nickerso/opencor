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
// CellML Text view parsing tests
//==============================================================================

#ifndef PARSINGTESTS_H
#define PARSINGTESTS_H

//==============================================================================

#include <QObject>

//==============================================================================

class ParsingTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void basicTests();
    void fileTests();
    void importTests();
    void unitsTests();
    void componentTests();
    void groupTests();
    void mapTests();
};

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
