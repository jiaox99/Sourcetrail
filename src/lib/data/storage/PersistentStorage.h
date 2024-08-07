#ifndef PERSISTENT_STORAGE_H
#define PERSISTENT_STORAGE_H

#include <memory>
#include <vector>

#include "FullTextSearchIndex.h"
#include "SqliteFullTextSearchIndex.h"
#include "HierarchyCache.h"
#include "SearchIndex.h"
#include "SqliteBookmarkStorage.h"
#include "SqliteIndexStorage.h"
#include "Storage.h"
#include "StorageAccess.h"
#include "flashmapper.h"

class PersistentStorage
	: public Storage
	, public StorageAccess
{
public:
	PersistentStorage(const FilePath& dbPath, const FilePath& bookmarkPath);

	std::pair<Id, bool> addNode(const StorageNodeData& data) override;
	std::vector<Id> addNodes(const std::vector<StorageNode>& nodes) override;
	void addSymbol(const StorageSymbol& data) override;
	void addSymbols(const std::vector<StorageSymbol>& symbols) override;
	void addFile(const StorageFile& data) override;
	Id addEdge(const StorageEdgeData& data) override;
	std::vector<Id> addEdges(const std::vector<StorageEdge>& edges) override;
	Id addLocalSymbol(const StorageLocalSymbolData& data) override;
	std::vector<Id> addLocalSymbols(const std::set<StorageLocalSymbol>& symbols) override;
	Id addSourceLocation(const StorageSourceLocationData& data) override;
	std::vector<Id> addSourceLocations(const std::vector<StorageSourceLocation>& locations) override;
	void addOccurrence(const StorageOccurrence& data) override;
	void addOccurrences(const std::vector<StorageOccurrence>& occurrences) override;
	void addComponentAccess(const StorageComponentAccess& componentAccess) override;
	void addComponentAccesses(const std::vector<StorageComponentAccess>& componentAccesses) override;
	void addElementComponent(const StorageElementComponent& component) override;
	void addElementComponents(const std::vector<StorageElementComponent>& components) override;
	Id addError(const StorageErrorData& data) override;

	void removeElement(const Id id);
	void removeElements(const std::vector<Id>& ids);
	void removeOccurrence(const StorageOccurrence& occurrence);
	void removeOccurrences(const std::vector<StorageOccurrence>& occurrences);
	void removeElementsWithoutOccurrences(const std::vector<Id>& elementIds);

	const std::vector<StorageNode>& getStorageNodes() const override;
	const std::vector<StorageFile>& getStorageFiles() const override;
	const std::vector<StorageSymbol>& getStorageSymbols() const override;
	const std::vector<StorageEdge>& getStorageEdges() const override;
	const std::set<StorageLocalSymbol>& getStorageLocalSymbols() const override;
	const std::set<StorageSourceLocation>& getStorageSourceLocations() const override;
	const std::set<StorageOccurrence>& getStorageOccurrences() const override;
	const std::set<StorageComponentAccess>& getComponentAccesses() const override;
	const std::set<StorageElementComponent>& getElementComponents() const override;
	const std::vector<StorageError>& getErrors() const override;

	void startInjection() override;
	void finishInjection() override;
	void rollbackInjection();

	const std::vector<ErrorInfo> getErrorInfos() const;

	void beforeErrorRecording();
	void afterErrorRecording();

	void setMode(const SqliteIndexStorage::StorageModeType mode);

	FilePath getIndexDbFilePath() const;
	FilePath getBookmarkDbFilePath() const;

	bool isEmpty() const;
	bool isIncompatible() const;
	std::string getProjectSettingsText() const;
	void setProjectSettingsText(std::string text);

	void setup();
	void updateVersion();
	void clear();
	void clearCaches();
	void updateOverview();

	std::set<FilePath> getReferenced(const std::set<FilePath>& filePaths) const;
	std::set<FilePath> getReferencing(const std::set<FilePath>& filePaths) const;

	void clearAllErrors();
	void clearFileElements(
		const std::vector<FilePath>& filePaths, std::function<void(int)> updateStatusCallback);

	std::vector<FileInfo> getFileInfoForAllFiles() const;
	std::set<FilePath> getIncompleteFiles() const;
	bool getFilePathIndexed(const FilePath& path) const;

	void buildCaches();

	void optimizeMemory();

	// StorageAccess implementation
	Id getNodeIdForFileNode(const FilePath& filePath) const override;
	Id getNodeIdForNameHierarchy(const NameHierarchy& nameHierarchy) const override;
	std::vector<Id> getNodeIdsForNameHierarchies(
		const std::vector<NameHierarchy> nameHierarchies) const override;

	NameHierarchy getNameHierarchyForNodeId(Id nodeId) const override;
	std::vector<NameHierarchy> getNameHierarchiesForNodeIds(const std::vector<Id>& nodeIds) const override;
	std::map<Id, std::pair<Id, NameHierarchy>> getNodeIdToParentFileMap(
		const std::vector<Id>& nodeIds) const override;

	NodeType getNodeTypeForNodeWithId(Id nodeId) const override;

	StorageEdge getEdgeById(Id edgeId) const override;

	std::shared_ptr<SourceLocationCollection> getFullTextSearchLocations(
		const std::wstring& searchTerm, bool caseSensitive) const override;

	std::vector<SearchMatch> getAutocompletionMatches(
		const std::wstring& query, NodeTypeSet acceptedNodeTypes, bool acceptCommands) const override;
	std::vector<SearchMatch> getAutocompletionSymbolMatches(
		const std::wstring& query,
		const NodeTypeSet& acceptedNodeTypes,
		size_t maxResultsCount,
		size_t maxBestScoredResultsLength) const;
	std::vector<SearchMatch> getAutocompletionFileMatches(
		const std::wstring& query, size_t maxResultsCount) const;
	std::vector<SearchMatch> getAutocompletionCommandMatches(
		const std::wstring& query, NodeTypeSet acceptedNodeTypes) const;
	std::vector<SearchMatch> getSearchMatchesForTokenIds(const std::vector<Id>& elementIds) const override;

	std::shared_ptr<Graph> getGraphForAll() const override;
	std::shared_ptr<Graph> getGraphForNodeTypes(NodeTypeSet nodeTypes) const override;
	std::shared_ptr<Graph> getGraphForActiveTokenIds(
		const std::vector<Id>& tokenIds,
		const std::vector<Id>& expandedNodeIds,
		bool* isActiveNamespace = nullptr) const override;
	std::shared_ptr<Graph> getGraphForChildrenOfNodeId(Id nodeId) const override;
	std::shared_ptr<Graph> getGraphForTrail(
		Id originId,
		Id targetId,
		NodeKindMask nodeTypes,
		Edge::TypeMask trailType,
		bool nodeNonIndexed,
		size_t depth,
		bool directed) const override;

	NodeKindMask getAvailableNodeTypes() const override;
	Edge::TypeMask getAvailableEdgeTypes() const override;

	std::vector<Id> getActiveTokenIdsForId(Id tokenId, Id* declarationId) const override;
	std::vector<Id> getNodeIdsForLocationIds(const std::vector<Id>& locationIds) const override;

	std::shared_ptr<SourceLocationCollection> getSourceLocationsForTokenIds(
		const std::vector<Id>& tokenIds) const override;
	std::shared_ptr<SourceLocationCollection> getSourceLocationsForLocationIds(
		const std::vector<Id>& locationIds) const override;

	std::shared_ptr<SourceLocationFile> getSourceLocationsForFile(const FilePath& filePath) const override;
	std::shared_ptr<SourceLocationFile> getSourceLocationsForLinesInFile(
		const FilePath& filePath, size_t startLine, size_t endLine) const override;
	std::shared_ptr<SourceLocationFile> getSourceLocationsOfTypeInFile(
		const FilePath& filePath, LocationType type) const override;

	std::shared_ptr<TextAccess> getFileContent(const FilePath& filePath, bool showsErrors) const override;
	bool hasContentForFile(const FilePath& filePath) const;

	FileInfo getFileInfoForFileId(Id id) const override;

	FileInfo getFileInfoForFilePath(const FilePath& filePath) const override;
	std::vector<FileInfo> getFileInfosForFilePaths(const std::vector<FilePath>& filePaths) const override;

	StorageStats getStorageStats() const override;

	ErrorCountInfo getErrorCount() const override;
	std::vector<ErrorInfo> getErrorsLimited(const ErrorFilter& filter) const override;
	std::vector<ErrorInfo> getErrorsForFileLimited(
		const ErrorFilter& filter, const FilePath& filePath) const override;
	std::shared_ptr<SourceLocationCollection> getErrorSourceLocations(
		const std::vector<ErrorInfo>& errors) const override;

	Id addNodeBookmark(const NodeBookmark& bookmark) override;
	Id addEdgeBookmark(const EdgeBookmark& bookmark) override;
	Id addBookmarkCategory(const std::wstring& categoryName) override;

	void updateBookmark(
		const Id bookmarkId,
		const std::wstring& name,
		const std::wstring& comment,
		const std::wstring& categoryName) override;
	void removeBookmark(const Id id) override;
	void removeBookmarkCategory(const Id id) override;

	std::vector<NodeBookmark> getAllNodeBookmarks() const override;
	std::vector<EdgeBookmark> getAllEdgeBookmarks() const override;
	std::vector<BookmarkCategory> getAllBookmarkCategories() const override;

	TooltipInfo getTooltipInfoForTokenIds(
		const std::vector<Id>& tokenIds, TooltipOrigin origin) const override;
	TooltipSnippet getTooltipSnippetForNode(const StorageNode& node) const;
	TooltipInfo getTooltipInfoForSourceLocationIdsAndLocalSymbolIds(
		const std::vector<Id>& locationIds, const std::vector<Id>& localSymbolIds) const override;

private:
	mutable struct
	{
		std::vector<StorageNode> nodes;
		std::vector<StorageFile> files;
		std::vector<StorageSymbol> symbols;
		std::vector<StorageEdge> edges;
		std::set<StorageLocalSymbol> locals;
		std::set<StorageSourceLocation> locations;
		std::set<StorageOccurrence> occurrences;
		std::set<StorageComponentAccess> accesses;
		std::set<StorageElementComponent> components;
		std::vector<StorageError> errors;
	} m_storageData;

	struct FilePathMapCache : public flashmapper::ComplexMapper
	{
		void clear()
		{
			m_fileNodeIds.clear();
			m_lowerCasefileNodeIds.clear();
			m_fileNodePaths.clear();
			m_fileNodeComplete.clear();
			m_fileNodeIndexed.clear();
			m_fileNodeLanguage.clear();
			m_symbolDefinitionKinds.clear();
		}

		flashmapper::Address writeData(flashmapper::Mapper& mapper, flashmapper::DataBlock& block)
		{
			mapper.writeData(m_fileNodeIds, block);
			mapper.writeData(m_lowerCasefileNodeIds, block);
			mapper.writeData(m_fileNodePaths, block);
			mapper.writeData(m_fileNodeComplete, block);
			mapper.writeData(m_fileNodeIndexed, block);
			mapper.writeData(m_fileNodeLanguage, block);
			mapper.writeData(m_symbolDefinitionKinds, block);
			mapper.writeData(m_hasJavaFiles, block);
			return block.baseOffset + block.cursor;
		}

		void resolveData(flashmapper::DataBlock& block)
		{
			m_fileNodeIds.resolveData(block);
			m_lowerCasefileNodeIds.resolveData(block);
			m_fileNodePaths.resolveData(block);
			m_fileNodeComplete.resolveData(block);
			m_fileNodeIndexed.resolveData(block);
			m_fileNodeLanguage.resolveData(block);
			m_symbolDefinitionKinds.resolveData(block);
		}

		void load(std::string filePath, flashmapper::Mapper& mapper)
		{
			clear();

			mapper.readFromFile(filePath.c_str());
			FilePathMapCache* cache = mapper.readData<FilePathMapCache>();
			m_fileNodeIds = std::move(cache->m_fileNodeIds);
			m_lowerCasefileNodeIds = std::move(cache->m_lowerCasefileNodeIds);
			m_fileNodePaths = std::move(cache->m_fileNodePaths);
			m_fileNodeComplete = std::move(cache->m_fileNodeComplete);
			m_fileNodeIndexed = std::move(cache->m_fileNodeIndexed);
			m_fileNodeLanguage = std::move(cache->m_fileNodeLanguage);
			m_symbolDefinitionKinds = std::move(cache->m_symbolDefinitionKinds);
		}

		void save(std::string filePath, flashmapper::Mapper& mapper)
		{
			mapper.reset();
			flashmapper::DataBlock block = mapper.requestBlock(sizeof(FilePathMapCache));
			mapper.writeData(*this, block);
			block.align();
			assert(block.postValidate());
			mapper.writeToFile(filePath.c_str());
		}

		flashmapper::hashmap<wchar_t, Id> m_fileNodeIds;
		flashmapper::hashmap<wchar_t, Id> m_lowerCasefileNodeIds;
		flashmapper::map<Id, flashmapper::wstring> m_fileNodePaths;
		flashmapper::map<Id, bool> m_fileNodeComplete;
		flashmapper::map<Id, bool> m_fileNodeIndexed;
		flashmapper::map<Id, flashmapper::wstring> m_fileNodeLanguage;
		flashmapper::map<Id, DefinitionKind> m_symbolDefinitionKinds;
		bool m_hasJavaFiles = false;
	};

	Id getFileNodeId(const FilePath& filePath) const;
	std::vector<Id> getFileNodeIds(const std::vector<FilePath>& filePaths) const;
	std::set<Id> getFileNodeIds(const std::set<FilePath>& filePaths) const;
	FilePath getFileNodePath(Id fileId) const;
	bool getFileNodeComplete(Id fileId) const;
	bool getFileNodeIndexed(Id fileId) const;
	std::wstring getFileNodeLanguage(Id fileId) const;

	std::unordered_map<Id, std::set<Id>> getFileIdToIncludingFileIdMap() const;
	std::unordered_map<Id, std::set<Id>> getFileIdToIncludedFileIdMap() const;
	std::unordered_map<Id, std::set<Id>> getFileIdToImportingFileIdMap() const;
	std::set<Id> getReferenced(
		const std::set<Id>& filePaths,
		std::unordered_map<Id, std::set<Id>> idToReferencingIdMap) const;
	std::set<Id> getReferencing(
		const std::set<Id>& filePaths,
		std::unordered_map<Id, std::set<Id>> idToReferencingIdMap) const;

	std::set<FilePath> getReferencedByIncludes(const std::set<FilePath>& filePaths) const;
	std::set<FilePath> getReferencedByImports(const std::set<FilePath>& filePaths) const;

	std::set<FilePath> getReferencingByIncludes(const std::set<FilePath>& filePaths) const;
	std::set<FilePath> getReferencingByImports(const std::set<FilePath>& filePaths) const;

	void addNodesToGraph(const std::vector<Id>& nodeIds, Graph* graph, bool addChildCount) const;
	void addEdgesToGraph(const std::vector<Id>& edgeIds, Graph* graph) const;
	void addNodesWithParentsAndEdgesToGraph(
		const std::vector<Id>& nodeIds,
		const std::vector<Id>& edgeIds,
		Graph* graph,
		bool addChildCount) const;
	inline void addFileNodeToGraph(const StorageNode& storageNode, Graph* const graph) const;
	void addNodeToGraph(
		const StorageNode& newNode, const NodeType& type, Graph* graph, bool addChildCount) const;
	void addBundledEdgesToGraph(
		Id nodeId, const std::vector<StorageEdge>& edgesToBundle, Graph* graph) const;
	void addFileContentsToGraph(Id fileId, Graph* graph) const;
	void addComponentAccessToGraph(Graph* graph) const;
	void addComponentIsAmbiguousToGraph(Graph* graph) const;

	void addCompleteFlagsToSourceLocationCollection(SourceLocationCollection* collection) const;
	void addInheritanceChainsToGraph(const std::vector<Id>& nodeIds, Graph* graph) const;

	void buildFilePathMaps();
	void buildSearchIndex();
	void buildFullTextSearchIndex() const;
	void buildMemberEdgeIdOrderMap();
	void buildHierarchyCache();

	bool m_preIndexingErrorCountSet = false;
	size_t m_preIndexingErrorCount = 0;
	size_t m_preInjectionErrorCount = 0;

	SearchIndex m_commandIndex;
	SearchIndex m_symbolIndex;
	flashmapper::Mapper m_symbolIndexMapper;
	SearchIndex m_fileIndex;
	flashmapper::Mapper m_fileIndexMapper;

	mutable FullTextSearchIndex m_fullTextSearchIndex;
	mutable SqliteFullTextSearchIndex m_sqliteFullTextSearchIndex;
	mutable std::string m_fullTextSearchCodec;
	mutable std::mutex m_fullTextSearchMutex;

	SqliteIndexStorage m_sqliteIndexStorage;
	SqliteBookmarkStorage m_sqliteBookmarkStorage;

	std::map<Id, Id> m_memberEdgeIdOrderMap;

	HierarchyCache m_hierarchyCache;
	flashmapper::Mapper m_hierarchyCacheMapper;

	mutable FilePathMapCache m_filePathMapCache;
	flashmapper::Mapper m_filePathMapCacheMapper;
};

#endif	  // PERSISTENT_STORAGE_H
