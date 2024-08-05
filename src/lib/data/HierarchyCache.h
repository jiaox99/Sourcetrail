#ifndef HIERARCHY_CACHE_H
#define HIERARCHY_CACHE_H

#include <map>
#include <memory>
#include <set>
#include <vector>
#include "flashmapper.h"
#include "types.h"

const size_t INVALID_INDEX = -1;

class HierarchyCache : public flashmapper::ComplexMapper
{
private:
	class HierarchyNode;
public:
	void clear();

	void load(std::string filePath, flashmapper::Mapper& mapper);
	void save(std::string filePath, flashmapper::Mapper& mapper);
	flashmapper::Address writeData(flashmapper::Mapper& mapper, flashmapper::DataBlock& block) const;
	void resolveData(flashmapper::DataBlock& block);

	void createConnection(
		Id edgeId, Id fromId, Id toId, bool sourceVisible, bool sourceImplicit, bool targetImplicit);
	void createInheritance(Id edgeId, Id fromId, Id toId);

	Id getLastVisibleParentNodeId(Id nodeId) const;
	size_t getIndexOfLastVisibleParentNode(Id nodeId) const;

	void addAllVisibleParentIdsForNodeId(Id nodeId, std::set<Id>* nodeIds, std::set<Id>* edgeIds) const;

	void addAllChildIdsForNodeId(Id nodeId, std::set<Id>* nodeIds, std::set<Id>* edgeIds) const;
	void addFirstChildIdsForNodeId(Id nodeId, std::vector<Id>* nodeIds, std::vector<Id>* edgeIds) const;

	size_t getFirstChildIdsCountForNodeId(Id nodeId) const;

	bool isChildOfVisibleNodeOrInvisible(Id nodeId) const;

	bool nodeHasChildren(Id nodeId) const;
	bool nodeIsVisible(Id nodeId) const;
	bool nodeIsImplicit(Id nodeId) const;

	std::vector<std::tuple</*source*/ Id, /*target*/ Id, std::vector</*edge*/ Id>>>
		getInheritanceEdgesForNodeId(Id sourceId, const std::set<Id>& targetIds) const;

	/**
	 * Determine the reversed subgraph of all nodes and edges that are reachable from this node.
	 *
	 * The subgraph is represented by a map that maps a node ID *t* to a set of pairs where each
	 * pair consists of a node ID *s* and and edge ID *e* such that *e* refers to an edge from
	 * *s* to *t*. Note that the mapping is reversed compared to the edges.
	 */
	std::map</*target*/ Id, std::vector<std::pair</*source*/ Id, /*edge*/ Id>>> getReverseReachableInheritanceSubgraph( HierarchyNode* sourceNode) const;

private:
	/**
	 * Helper for getReverseReachableInheritanceSubgraph().
	 */
	void getReverseReachableInheritanceSubgraphHelper(
		HierarchyNode* sourceNode,
		std::map</*target*/ Id, std::vector<std::pair</*source*/ Id, /*edge*/ Id>>>&) const;

	/**
	 * Determine nodes and edges from which a specific node can be reached in a reversed graph.
	 *
	 * A reversed graph can be produced by HierarchyNode::getReverseReachableInheritanceSubgraph().
	 *
	 * @param[in]  nodeId        ID of the target node.
	 * @param[in]  reverseGraph  The reversed graph.
	 * @param[out] nodes         The nodes from which the node @p nodeId can be reached.
	 * @param[out] edges         The edges from which the node @p nodeId can be reached.
	 *
	 * @pre The arguments for @p nodes and @p edges must be provided empty.
	 */
	static void getReverseReachable(
		Id nodeId,
		const std::map</*target*/ Id, std::vector<std::pair</*source*/ Id, /*edge*/ Id>>>&
			reverseGraph,
		std::set<Id>& nodes,
		std::vector<Id>& edges);

	size_t getNonImplicitChildrenCount(HierarchyNode* node) const;

	void addChildIds(HierarchyNode* node, std::vector<Id>* nodeIds, std::vector<Id>* edgeIds) const;
	void addNonImplicitChildIds(HierarchyNode* node, std::vector<Id>* nodeIds, std::vector<Id>* edgeIds) const;
	void addChildIdsRecursive(HierarchyNode* node, std::set<Id>* nodeIds, std::set<Id>* edgeIds) const;

	class HierarchyNode : public flashmapper::ComplexMapper
	{
	public:
		HierarchyNode() : HierarchyNode(INVALID_INDEX){}
		HierarchyNode(Id nodeId);
		HierarchyNode(const HierarchyNode& other);
		HierarchyNode& operator=(const HierarchyNode& other);
		~HierarchyNode();

		flashmapper::Address writeData(flashmapper::Mapper& mapper, flashmapper::DataBlock& block);
		void resolveData(flashmapper::DataBlock& block);

		Id getNodeId() const;
		void setNodeId(Id nodeId) { m_nodeId = nodeId;}

		Id getEdgeId() const;
		void setEdgeId(Id edgeId);

		size_t getParent() const;
		void setParent(size_t parent);

		void addBase(size_t base, Id edgeId);

		void addChild(size_t child);

		flashmapper::vector<size_t>* getBases()
		{
			return &m_bases;
		}

		flashmapper::vector<Id>* getBaseEdgeIds()
		{
			return &m_baseEdgeIds;
		}

		flashmapper::vector<size_t>* getChildren()
		{
			return &m_children;
		}

		size_t getChildrenCount() const;

		bool isVisible() const;
		void setIsVisible(bool isVisible);

		bool isImplicit() const;
		void setIsImplicit(bool isImplicit);

	private:

		Id m_nodeId;
		Id m_edgeId;

		size_t m_parent;

		flashmapper::vector<size_t> m_bases;
		flashmapper::vector<Id> m_baseEdgeIds;

		flashmapper::vector<size_t> m_children;

		bool m_isVisible;
		bool m_isImplicit;
	};

	HierarchyNode* getNode(Id nodeId) const;
	HierarchyNode* createNode(Id nodeId);

	flashmapper::map<Id, HierarchyNode> m_nodes;
};

#endif	  // HIERARCHY_CACHE_H
