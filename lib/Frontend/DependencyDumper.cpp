//===--- DependencyDumper.cpp - Dumping dependency of source file ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the AST dump methods, which dump out the
// AST in a form that exposes type details and other fields.
//
//===----------------------------------------------------------------------===//

#include "clang/Frontend/DependencyDumper.h"
#include <string>
#include <set>

using namespace clang;

// use a <set> to record sorcefile of a node while traversing AST
static std::set<std::string> dep;

// for a SourceRange, insert all file it contains to <set>
static PresumedLoc AddDifference(const SourceManager &SM, 
                                  SourceLocation Loc, PresumedLoc Previous) {
  if (Loc.isFileID()) {
     PresumedLoc PLoc = SM.getPresumedLoc(Loc);
  
     if (PLoc.isInvalid()) {
       return Previous;
     }
  
     if (Previous.isInvalid() ||
         strcmp(PLoc.getFilename(), Previous.getFilename()) != 0) {
       dep.insert(PLoc.getFilename()) ;
     } 
     return PLoc;
   }

   auto PrintedLoc = AddDifference(SM, SM.getExpansionLoc(Loc), Previous);
   PrintedLoc = AddDifference(SM, SM.getSpellingLoc(Loc), PrintedLoc);

   return PrintedLoc;
}

static void AddToSet(const SourceManager &SM, const SourceRange &SR) {
  auto AddedLoc = AddDifference(SM, SR.getBegin(), {});
  if (SR.getBegin() != SR.getEnd()) {
    AddDifference(SM, SR.getEnd(), AddedLoc);
  }
}


// OVERRIDE visit func to record sourcefile to <set>
bool DependencyDumperVisitor::VisitDecl(Decl *D) {
    FullSourceLoc BeginLocation = Context->getFullLoc(D->getBeginLoc());
    const SourceManager &SM = BeginLocation.getManager();
    SM.dump();
    SourceRange SR = D->getSourceRange();
    AddToSet(SM, SR);
        
    return true;
}

bool DependencyDumperVisitor::VisitStmt(Stmt *S) {
    FullSourceLoc BeginLocation = Context->getFullLoc(S->getBeginLoc());
    const SourceManager &SM = BeginLocation.getManager();
    SourceRange SR = S->getSourceRange();
    AddToSet(SM, SR);
        
    return true;
}

bool DependencyDumperVisitor::VisitTypeLoc(TypeLoc TL) {
    FullSourceLoc BeginLocation = Context->getFullLoc(TL.getBeginLoc());
    const SourceManager &SM = BeginLocation.getManager();
    SourceRange SR = TL.getSourceRange();
    AddToSet(SM, SR);
        
    return true;
}

bool DependencyDumperVisitor::VisitAttr(Attr *A) {
    FullSourceLoc FullLocation = Context->getFullLoc(A->getLocation());

    if (FullLocation.isValid()) {
        PresumedLoc PLoc = FullLocation.getManager().getPresumedLoc(FullLocation);
        if (PLoc.isValid())
        dep.insert(PLoc.getFilename());

        FullSourceLoc ESpellLocation = FullLocation.getSpellingLoc();
        if (ESpellLocation.isValid()) {
        PresumedLoc EPLoc = ESpellLocation.getManager().getPresumedLoc(ESpellLocation);
        if (EPLoc.isValid()) {
            dep.insert(EPLoc.getFilename());
        }
        } 
    }
    return true;
}

