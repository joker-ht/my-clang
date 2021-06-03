//===--- DependencyDumper.h - Dumping dependency of source file -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"

namespace clang {

class DependencyDumperVisitor
  : public RecursiveASTVisitor<DependencyDumperVisitor> {
public:
  explicit DependencyDumperVisitor(ASTContext *Context)
    : Context(Context) {}
  // OVERRIDE visit func to record sourcefile to <set>
  bool VisitDecl(Decl *D) ;
  bool VisitStmt(Stmt *S) ;
  bool VisitTypeLoc(TypeLoc TL) ;
  bool VisitAttr(Attr *A) ;

private:
  ASTContext *Context;
};

class DependencyDumperConsumer : public clang::ASTConsumer {
public:
  explicit DependencyDumperConsumer(ASTContext *Context)
    : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  DependencyDumperVisitor Visitor;
};

class DependencyDumperAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<clang::ASTConsumer>(
        new DependencyDumperConsumer(&Compiler.getASTContext()));
  }
};

} // namespace clang