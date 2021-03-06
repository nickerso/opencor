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

#include "compilerengine.h"
#include "compilermath.h"
#include "corecliutils.h"

//==============================================================================

#include <QApplication>
#include <QFile>
#include <QIODevice>
#include <QTemporaryFile>
#include <QTextStream>

//==============================================================================

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Host.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    #pragma GCC diagnostic error "-Wunused-parameter"
    #pragma GCC diagnostic error "-Wstrict-aliasing"
#endif

//==============================================================================

namespace OpenCOR {
namespace Compiler {

//==============================================================================

CompilerEngine::CompilerEngine() :
    mExecutionEngine(std::unique_ptr<llvm::ExecutionEngine>()),
    mError(QString())
{
}

//==============================================================================

CompilerEngine::~CompilerEngine()
{
    // Reset ourselves

    reset();
}

//==============================================================================

void CompilerEngine::reset(const bool &pResetError)
{
    // Reset some internal objects

    delete mExecutionEngine.release();

    mExecutionEngine = std::unique_ptr<llvm::ExecutionEngine>();

    if (pResetError)
        mError = QString();
}

//==============================================================================

bool CompilerEngine::hasError() const
{
    // Return whether an error occurred

    return mError.size();
}

//==============================================================================

QString CompilerEngine::error() const
{
    // Return the compiler engine's error

    return mError;
}

//==============================================================================

bool CompilerEngine::compileCode(const QString &pCode)
{
    // Reset our compiler engine

    reset();

    // Create a temporary file with the given code as its contents

    QString tempFileName = Core::temporaryFileName(".c");

    if (!Core::writeTextToFile(tempFileName, pCode)) {
        mError = tr("<strong>%1</strong> could not be created").arg(tempFileName);

        QFile::remove(tempFileName);

        return false;
    }

    // Get a driver to compile our code

#ifdef QT_DEBUG
    llvm::raw_ostream &outputStream = llvm::outs();
#else
    llvm::raw_ostream &outputStream = llvm::nulls();
#endif
    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> diagnosticOptions = new clang::DiagnosticOptions();

    clang::DiagnosticsEngine diagnosticsEngine(llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs>(new clang::DiagnosticIDs()),
                                               &*diagnosticOptions,
                                               new clang::TextDiagnosticPrinter(outputStream, &*diagnosticOptions));
    std::string targetTriple = llvm::sys::getProcessTriple();

#ifdef Q_OS_WIN
    // For now, on Windows, MCJIT only works through the ELF object format

    targetTriple += "-elf";
#endif

    clang::driver::Driver driver("clang", targetTriple, diagnosticsEngine);

    // Get a compilation object to which we pass some arguments

    QByteArray tempFileByteArray = tempFileName.toUtf8();

    llvm::SmallVector<const char *, 16> compilationArguments;

    compilationArguments.push_back("clang");
    compilationArguments.push_back("-fsyntax-only");
    compilationArguments.push_back("-O3");
    compilationArguments.push_back("-ffast-math");
    compilationArguments.push_back("-Werror");
    compilationArguments.push_back(tempFileByteArray.constData());

    std::unique_ptr<clang::driver::Compilation> compilation(driver.BuildCompilation(compilationArguments));

    if (!compilation) {
        mError = tr("the compilation object could not be created");

        QFile::remove(tempFileName);

        return false;
    }

    // The compilation object should have only one command, so if it doesn't
    // then something went wrong

    const clang::driver::JobList &jobList = compilation->getJobs();

    if (    (jobList.size() != 1)
        || !llvm::isa<clang::driver::Command>(*jobList.begin())) {
        mError = tr("the compilation object must contain only one command");

        QFile::remove(tempFileName);

        return false;
    }

    // Retrieve the command job

    const clang::driver::Command &command = llvm::cast<clang::driver::Command>(*jobList.begin());
    QString commandName = command.getCreator().getName();

    if (commandName.compare("clang")) {
        mError = tr("a <strong>clang</strong> command was expected, but a <strong>%1</strong> command was found instead").arg(commandName);

        QFile::remove(tempFileName);

        return false;
    }

    // Create a compiler invocation using our command's arguments

    const clang::driver::ArgStringList &commandArguments = command.getArguments();
    std::unique_ptr<clang::CompilerInvocation> compilerInvocation(new clang::CompilerInvocation());

    clang::CompilerInvocation::CreateFromArgs(*compilerInvocation,
                                              const_cast<const char **>(commandArguments.data()),
                                              const_cast<const char **>(commandArguments.data())+commandArguments.size(),
                                              diagnosticsEngine);

    // Create a compiler instance to handle the actual work

    clang::CompilerInstance compilerInstance;

    compilerInstance.setInvocation(compilerInvocation.release());

    // Create the compiler instance's diagnostics engine

    compilerInstance.createDiagnostics();

    if (!compilerInstance.hasDiagnostics()) {
        mError = tr("the diagnostics engine could not be created");

        QFile::remove(tempFileName);

        return false;
    }

    // Create and execute the frontend to generate an LLVM bitcode module
    // Note: the LLVM team has been meaning to modify
    //       CompilerInstance::ExecuteAction() so that we could specify the
    //       output stream we want to use (rather than always use llvm::errs()),
    //       but they have yet to actually do it, so we modified it ourselves...

    std::unique_ptr<clang::CodeGenAction> codeGenerationAction(new clang::EmitLLVMOnlyAction(&llvm::getGlobalContext()));

    if (!compilerInstance.ExecuteAction(*codeGenerationAction, outputStream)) {
        mError = tr("the code could not be compiled");

        QFile::remove(tempFileName);

        reset(false);

        return false;
    }

    // We are done with the temporary file, so we can remove it

    QFile::remove(tempFileName);

    // Retrieve the LLVM bitcode module

    std::unique_ptr<llvm::Module> module = codeGenerationAction->takeModule();

    // Initialise the native target (and its ASM printer), so not only can we
    // then create an execution engine, but more importantly its data layout
    // will match that of our target platform

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    // Create and keep track of an execution engine

    mExecutionEngine = std::unique_ptr<llvm::ExecutionEngine>(llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create());

    if (!mExecutionEngine) {
        mError = tr("the execution engine could not be created");

        delete module.release();

        return false;
    }

    return true;
}

//==============================================================================

void * CompilerEngine::getFunction(const QString &pFunctionName)
{
    // Return the address of the requested function

    if (mExecutionEngine)
        return (void *) mExecutionEngine->getFunctionAddress(qPrintable(pFunctionName));
    else
        return 0;
}

//==============================================================================

}   // namespace Compiler
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
