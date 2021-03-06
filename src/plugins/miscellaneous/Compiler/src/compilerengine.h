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
// Compiler engine class
//==============================================================================

#ifndef COMPILERENGINE_H
#define COMPILERENGINE_H

//==============================================================================

#include "compilerglobal.h"

//==============================================================================

#include <QObject>
#include <QString>

//==============================================================================

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

#include "llvm/ExecutionEngine/ExecutionEngine.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    #pragma GCC diagnostic error "-Wunused-parameter"
    #pragma GCC diagnostic error "-Wstrict-aliasing"
#endif

//==============================================================================

namespace OpenCOR {
namespace Compiler {

//==============================================================================

class COMPILER_EXPORT CompilerEngine : public QObject
{
    Q_OBJECT

public:
    explicit CompilerEngine();
    ~CompilerEngine();

    bool hasError() const;
    QString error() const;

    bool compileCode(const QString &pCode);

    void * getFunction(const QString &pFunctionName);

private:
    std::unique_ptr<llvm::ExecutionEngine> mExecutionEngine;

    QString mError;

    void reset(const bool &pResetError = true);
};

//==============================================================================

}   // namespace Compiler
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
