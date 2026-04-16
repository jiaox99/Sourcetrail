#ifndef CXX_DIAGNOSTIC_CONSUMER
#define CXX_DIAGNOSTIC_CONSUMER

#include "FilePath.h"
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <memory>

class CanonicalFilePathCache;
class ParserClient;

class CxxDiagnosticConsumer: public clang::TextDiagnosticPrinter
{
public:
	/// \p diagOptions must outlive this consumer. The caller retains ownership.
	CxxDiagnosticConsumer(
		clang::raw_ostream& os,
		clang::DiagnosticOptions& diagOptions,
		std::shared_ptr<ParserClient> client,
		std::shared_ptr<CanonicalFilePathCache> canonicalFilePathCache,
		const FilePath& sourceFilePath,
		bool useLogging = true);

	void BeginSourceFile(
		const clang::LangOptions& langOptions, const clang::Preprocessor* preProcessor) override;
	void EndSourceFile() override;

	void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic& info) override;

private:
	std::shared_ptr<ParserClient> m_client;
	std::shared_ptr<CanonicalFilePathCache> m_canonicalFilePathCache;

	const FilePath m_sourceFilePath;
	bool m_useLogging;
};

#endif	  // CXX_DIAGNOSTIC_CONSUMER
