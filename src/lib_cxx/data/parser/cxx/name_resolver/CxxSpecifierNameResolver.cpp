#include "CxxSpecifierNameResolver.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/PrettyPrinter.h>

#include "CxxDeclNameResolver.h"
#include "CxxTypeNameResolver.h"
#include "utilityString.h"

CxxSpecifierNameResolver::CxxSpecifierNameResolver(CanonicalFilePathCache* canonicalFilePathCache)
	: CxxNameResolver(canonicalFilePathCache)
{
}

CxxSpecifierNameResolver::CxxSpecifierNameResolver(const CxxNameResolver* other)
	: CxxNameResolver(other)
{
}

std::unique_ptr<CxxName> CxxSpecifierNameResolver::getName(
	clang::NestedNameSpecifier nestedNameSpecifier)
{
	if (nestedNameSpecifier)
	{
		clang::NestedNameSpecifier::Kind nnsKind = nestedNameSpecifier.getKind();
		switch (nnsKind)
		{
		case clang::NestedNameSpecifier::Kind::Namespace:
		{
			const clang::NamespaceBaseDecl* nsBase =
				nestedNameSpecifier.getAsNamespaceAndPrefix().Namespace;
			if (const clang::NamespaceAliasDecl* aliasDecl =
					clang::dyn_cast_or_null<clang::NamespaceAliasDecl>(nsBase))
			{
				return CxxDeclNameResolver(this).getName(aliasDecl);
			}
			if (const clang::NamespaceDecl* nsDecl =
					clang::dyn_cast_or_null<clang::NamespaceDecl>(nsBase))
			{
				return CxxDeclNameResolver(this).getName(nsDecl);
			}
			break;
		}

		case clang::NestedNameSpecifier::Kind::Type:
			return CxxTypeName::makeUnsolvedIfNull(
				CxxTypeNameResolver(this).getName(nestedNameSpecifier.getAsType()));

		case clang::NestedNameSpecifier::Kind::Global:
			// no context name hierarchy needed.
			break;

		case clang::NestedNameSpecifier::Kind::MicrosoftSuper:
			return CxxDeclNameResolver(this).getName(nestedNameSpecifier.getAsRecordDecl());

		case clang::NestedNameSpecifier::Kind::Null:
			break;
		}
	}

	return nullptr;
}
