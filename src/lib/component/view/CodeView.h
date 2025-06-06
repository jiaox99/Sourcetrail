#ifndef CODE_VIEW_H
#define CODE_VIEW_H

#include <memory>

#include "CodeScrollParams.h"
#include "CodeSnippetParams.h"
#include "ErrorInfo.h"
#include "LocationType.h"
#include "ScreenSearchInterfaces.h"
#include "View.h"

class CodeController;
class FilePath;
class SourceLocationCollection;

class CodeView
	: public View
	, public ScreenSearchResponder
{
public:
	static const char* VIEW_NAME;

	enum FileState
	{
		FILE_MINIMIZED,
		FILE_SNIPPETS,
		FILE_MAXIMIZED
	};

	struct CodeParams
	{
		bool clearSnippets = false;
		bool useSingleFileCache = true;
		Id locationIdToFocus = 0;

		size_t referenceCount = 0;
		size_t referenceIndex = 0;
		size_t localReferenceCount = 0;
		size_t localReferenceIndex = 0;

		std::vector<Id> activeTokenIds;
		std::vector<Id> activeLocationIds;
		std::vector<Id> activeLocalSymbolIds;
		LocationType activeLocalSymbolType = LOCATION_TOKEN;
		std::vector<Id> currentActiveLocalLocationIds;

		std::vector<ErrorInfo> errorInfos;
	};

	CodeView(ViewLayout* viewLayout);
	virtual ~CodeView();

	virtual std::string getName() const;

	virtual void clear() = 0;

	virtual void showSnippets(
		const std::vector<CodeFileParams>::const_iterator begin,
		const std::vector<CodeFileParams>::const_iterator end,
		const CodeParams& params,
		const CodeScrollParams& scrollParams) = 0;

	virtual void showSnippets(
		const std::vector<CodeFileParams>::const_iterator begin,
		const std::vector<CodeFileParams>::const_iterator end) = 0;

	virtual void showSingleFile(
		const CodeFileParams& file, const CodeParams& params, const CodeScrollParams& scrollParams) = 0;

	virtual void updateSourceLocations(const std::vector<CodeFileParams>& files) = 0;

	virtual void scrollTo(const CodeScrollParams& params, bool animated) = 0;

	virtual bool showsErrors() const = 0;

	virtual void coFocusTokenIds(const std::vector<Id>& coFocusedTokenIds) = 0;
	virtual void deCoFocusTokenIds() = 0;

	virtual bool isInListMode() const = 0;
	virtual void setMode(bool listMode) = 0;

	virtual bool hasSingleFileCached(const FilePath& filePath) const = 0;

	virtual void setNavigationFocus(bool focus) = 0;
	virtual bool hasNavigationFocus() const = 0;
	virtual void exportReferences( const std::vector<CodeFileParams>& files ) = 0;

protected:
	CodeController* getController();
};

#endif	  // CODE_VIEW_H
