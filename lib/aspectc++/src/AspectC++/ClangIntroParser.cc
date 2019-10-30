
// FIXME: This is a hack to give ClangIntroParser access rights to the current scope
// This has to be included FIRST!
#define private protected
#include "clang/Parse/Parser.h"
#undef private

#include "version.h"

#include "ClangIntroParser.h"
#include "clang/Basic/Version.h"
#include "clang/Parse/ParseDiagnostic.h"
using namespace clang;

// FIXME: duplicated loop!
// TODO: try to make Clang developers integrate this refactoring
void ClangIntroParser::ParseCXXClassBody(Decl *TagDecl) {
  // ----------------AspectC++ modified
  unsigned TagType = DeclSpec::TST_struct;
  AccessSpecifier CurAS = AS_private; // doesn't really matter; introduced code contains private/...
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
  ParsedAttributesWithRange AccessAttrs(getAttrFactory());
#else
  ParsedAttributes AccessAttrs(getAttrFactory());
#endif
  // the following two reference definitions are needed, because
  // our trick (private->protected)does not make all attributes protected :-(
  Token &Tok = (Token&)getCurToken();
  Sema &Actions = getActions();
  // ----------------up to here

  while (Tok.isNot(tok::eof)) {
    if (Tok.is(tok::r_brace)) {
      break;
    }

    // Each iteration of this loop reads one member-declaration.

    if (getLangOpts().MicrosoftExt && (Tok.is(tok::kw___if_exists) ||
        Tok.is(tok::kw___if_not_exists))) {
      ParseMicrosoftIfExistsClassDeclaration((DeclSpec::TST)TagType, CurAS);
      continue;
    }

    // Check for extraneous top-level semicolon.
    if (Tok.is(tok::semi)) {
      ConsumeExtraSemi(InsideStruct, TagType);
      continue;
    }

    if (Tok.is(tok::annot_pragma_vis)) {
      HandlePragmaVisibility();
      continue;
    }

    if (Tok.is(tok::annot_pragma_pack)) {
      HandlePragmaPack();
      continue;
    }

    if (Tok.is(tok::annot_pragma_align)) {
      HandlePragmaAlign();
      continue;
    }

    if (Tok.is(tok::annot_pragma_openmp)) {
#if CLANG_VERSION_NUMBER >= VERSION_NUMBER_3_9_1
      ParseOpenMPDeclarativeDirectiveWithExtDecl(CurAS, AccessAttrs);
#else
      ParseOpenMPDeclarativeDirective();
#endif
      continue;
    }

    // If we see a namespace here, a close brace was missing somewhere.
    if (Tok.is(tok::kw_namespace)) {
      DiagnoseUnexpectedNamespace(cast<NamedDecl>(TagDecl));
      break;
    }

    AccessSpecifier AS = getAccessSpecifierIfPresent();
    if (AS != AS_none) {
      // Current token is a C++ access specifier.
      CurAS = AS;
      SourceLocation ASLoc = Tok.getLocation();
      unsigned TokLength = Tok.getLength();
      ConsumeToken();
      AccessAttrs.clear();
      MaybeParseGNUAttributes(AccessAttrs);

      SourceLocation EndLoc;
      if (Tok.is(tok::colon)) {
        EndLoc = Tok.getLocation();
        ConsumeToken();
      } else if (Tok.is(tok::semi)) {
        EndLoc = Tok.getLocation();
        ConsumeToken();
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
        Diag(EndLoc, diag::err_expected_colon)
          << FixItHint::CreateReplacement(EndLoc, ":");
#else // C++ 11 interface
        Diag(EndLoc, diag::err_expected)
            << tok::colon << FixItHint::CreateReplacement(EndLoc, ":");
#endif
      } else {
        EndLoc = ASLoc.getLocWithOffset(TokLength);
#if (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && !defined(CLANG_VERSION_PATCHLEVEL)) || \
  (CLANG_VERSION_MAJOR == 3 && CLANG_VERSION_MINOR == 4 && CLANG_VERSION_PATCHLEVEL == 2)
        Diag(EndLoc, diag::err_expected_colon)
          << FixItHint::CreateInsertion(EndLoc, ":");
#else // C++ 11 interface
        Diag(EndLoc, diag::err_expected)
            << tok::colon << FixItHint::CreateInsertion(EndLoc, ":");
#endif
      }

      // The Microsoft extension __interface does not permit non-public
      // access specifiers.
      if (TagType == DeclSpec::TST_interface && CurAS != AS_public) {
        Diag(ASLoc, diag::err_access_specifier_interface)
          << (CurAS == AS_protected);
      }

      if (Actions.ActOnAccessSpecifier(AS, ASLoc, EndLoc,
                                       AccessAttrs.getList())) {
        // found another attribute than only annotations
        AccessAttrs.clear();
      }

      continue;
    }

    // Parse all the comma separated declarators.
    ParseCXXClassMemberDeclaration(CurAS, AccessAttrs.getList());
  }
}

void ClangIntroParser::ParseBaseClause(clang::Decl *TagDecl) {
  clang::Parser::ParseBaseClause(TagDecl);
}
