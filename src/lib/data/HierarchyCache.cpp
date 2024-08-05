#include "HierarchyCache.h"

#include "utility.h"

HierarchyCache::HierarchyNode::HierarchyNode(Id nodeId)
	: m_nodeId(nodeId), m_edgeId(0), m_parent(-1), m_isVisible(true), m_isImplicit(false)
{
}

HierarchyCache::HierarchyNode::HierarchyNode(const HierarchyNode& other)
{
	m_nodeId = other.m_nodeId;
	m_edgeId = other.m_edgeId;
	m_parent = other.m_parent;
	m_bases = other.m_bases;
	m_baseEdgeIds = other.m_baseEdgeIds;
	m_children = other.m_children;
	m_isVisible = other.m_isVisible;
	m_isImplicit = other.m_isImplicit;
}

HierarchyCache::HierarchyNode& HierarchyCache::HierarchyNode::operator=(const HierarchyNode& other)
{
	if (this != &other)
	{
		m_nodeId = other.m_nodeId;
		m_edgeId = other.m_edgeId;
		m_parent = other.m_parent;
		m_bases = other.m_bases;
		m_baseEdgeIds = other.m_baseEdgeIds;
		m_children = other.m_children;
		m_isVisible = other.m_isVisible;
		m_isImplicit = other.m_isImplicit;
	}

	return *this;
}

Id HierarchyCache::HierarchyNode::getNodeId() const
{
	return m_nodeId;
}

flashmapper::Address HierarchyCache::HierarchyNode::writeData(
	flashmapper::Mapper& mapper, flashmapper::DataBlock& block)
{
	mapper.writeData(m_nodeId, block);
	mapper.writeData(m_edgeId, block);
	mapper.writeData(m_parent, block);
	mapper.writeData(m_bases, block);
	mapper.writeData(m_baseEdgeIds, block);
	mapper.writeData(m_children, block);
	mapper.writeData(m_isVisible, block);
	mapper.writeData(m_isImplicit, block);

	return block.baseOffset + block.cursor;
}

void HierarchyCache::HierarchyNode::resolveData(flashmapper::DataBlock& block)
{
	m_bases.resolveData(block);
	m_baseEdgeIds.resolveData(block);
	m_children.resolveData(block);
}

Id HierarchyCache::HierarchyNode::getEdgeId() const
{
	return m_edgeId;
}

void HierarchyCache::HierarchyNode::setEdgeId(Id edgeId)
{
	m_edgeId = edgeId;
}

size_t HierarchyCache::HierarchyNode::getParent() const
{
	return m_parent;
}

void HierarchyCache::HierarchyNode::setParent(size_t parent)
{
	m_parent = parent;
}

void HierarchyCache::HierarchyNode::addBase(size_t base, Id edgeId)
{
	m_bases.push_back(base);
	m_baseEdgeIds.push_back(edgeId);
}

void HierarchyCache::HierarchyNode::addChild(size_t child)
{
	m_children.push_back(child);
}

size_t HierarchyCache::HierarchyNode::getChildrenCount() const
{
	return m_children.size();
}

size_t HierarchyCache::getNonImplicitChildrenCount(HierarchyNode* node) const
{
	size_t count = 0;
	flashmapper::vector<size_t>& children = *node->getChildren();
	for (size_t i=0; i<children.size(); i++)
	{
		HierarchyNode* child = m_nodes.getByIndex(children[i]);
		if (!child->isImplicit())
		{
			count++;
		}
	}
	return count;
}

void HierarchyCache::addChildIds(HierarchyNode* node, std::vector<Id>* nodeIds, std::vector<Id>* edgeIds) const
{
	flashmapper::vector<size_t>& children = *node->getChildren();
	for (size_t i = 0; i < children.size(); i++)
	{
		HierarchyNode* child = m_nodes.getByIndex(children[i]);
		nodeIds->push_back(child->getNodeId());
		edgeIds->push_back(child->getEdgeId());
	}
}

void HierarchyCache::addNonImplicitChildIds(
	HierarchyNode* node,
	std::vector<Id>* nodeIds, std::vector<Id>* edgeIds) const
{
	flashmapper::vector<size_t>& children = *node->getChildren();
	for (size_t i = 0; i < children.size(); i++)
	{
		HierarchyNode* child = m_nodes.getByIndex(children[i]);
		if (!child->isImplicit())
		{
			nodeIds->push_back(child->getNodeId());
			edgeIds->push_back(child->getEdgeId());
		}
	}
}

void HierarchyCache::addChildIdsRecursive(
	HierarchyNode* node, std::set<Id>* nodeIds, std::set<Id>* edgeIds) const
{
	flashmapper::vector<size_t>& children = *node->getChildren();
	for (size_t i = 0; i < children.size(); i++)
	{
		HierarchyNode* child = m_nodes.getByIndex(children[i]);
		nodeIds->insert(child->getNodeId());
		edgeIds->insert(child->getEdgeId());

		addChildIdsRecursive(child, nodeIds, edgeIds);
	}
}

bool HierarchyCache::HierarchyNode::isVisible() const
{
	return m_isVisible;
}

void HierarchyCache::HierarchyNode::setIsVisible(bool isVisible)
{
	m_isVisible = isVisible;
}

bool HierarchyCache::HierarchyNode::isImplicit() const
{
	return m_isImplicit;
}

void HierarchyCache::HierarchyNode::setIsImplicit(bool isImplicit)
{
	m_isImplicit = isImplicit;
}

HierarchyCache::HierarchyNode::~HierarchyNode()
{
	std::cout << "HierarchyNode::~HierarchyNode()" << std::endl;
}

std::map</*target*/ Id, std::vector<std::pair</*source*/ Id, /*edge*/ Id>>>
HierarchyCache::getReverseReachableInheritanceSubgraph(HierarchyNode* sourceNode) const
{
	std::map<Id, std::vector<std::pair<Id, Id>>> reverseGraph;
	reverseGraph.try_emplace(sourceNode->getNodeId());	// mark start node as visited
	getReverseReachableInheritanceSubgraphHelper(sourceNode, reverseGraph);
	return reverseGraph;
}

void HierarchyCache::getReverseReachableInheritanceSubgraphHelper(
	HierarchyNode* sourceNode,
	std::map</*target*/ Id, std::vector<std::pair</*source*/ Id, /*edge*/ Id>>>& reverseGraph) const
{
	flashmapper::vector<size_t>& m_bases = *sourceNode->getBases();
	for (size_t i = 0; i < sourceNode->getBases()->size(); ++i)
	{
		size_t base = m_bases[i];
		HierarchyNode* baseNode = m_nodes.getByIndex(base);
		auto emplacedBase = reverseGraph.try_emplace(baseNode->getNodeId());
		const Id nodeId = baseNode->getNodeId();
		const Id edgeId = (*baseNode->getBaseEdgeIds())[i];
		emplacedBase.first->second.push_back({nodeId, edgeId});
		if (emplacedBase.second)
		{
			getReverseReachableInheritanceSubgraphHelper(baseNode, reverseGraph);
		}
	}
}

flashmapper::Address HierarchyCache::writeData(flashmapper::Mapper& mapper, flashmapper::DataBlock& block) const
{
	mapper.writeData(m_nodes, block);

	return block.baseOffset + block.cursor;
}

void HierarchyCache::resolveData(flashmapper::DataBlock& block)
{
	m_nodes.resolveData(block);
}

void HierarchyCache::load(std::string filePath, flashmapper::Mapper& mapper)
{
	m_nodes.clear();

	mapper.readFromFile(filePath.c_str());
	HierarchyCache* hierarchy = mapper.readData<HierarchyCache>();
	m_nodes = std::move(hierarchy->m_nodes);
}

void HierarchyCache::save(std::string filePath, flashmapper::Mapper& mapper)
{
	mapper.reset();
	flashmapper::DataBlock block = mapper.requestBlock(sizeof(HierarchyCache));
	mapper.writeData(*this, block);
	block.align();
	assert(block.postValidate());
	mapper.writeToFile(filePath.c_str());
}

void HierarchyCache::clear()
{
	m_nodes.clear();
}

void HierarchyCache::createConnection(
	Id edgeId, Id fromId, Id toId, bool sourceVisible, bool sourceImplicit, bool targetImplicit)
{
	if (fromId == toId)
	{
		return;
	}

	createNode(fromId);
	HierarchyNode* to = createNode(toId);
	HierarchyNode* from = getNode(fromId);

	from->addChild(m_nodes.findIndex(toId));
	to->setParent(m_nodes.findIndex(fromId));

	from->setIsVisible(sourceVisible);
	from->setIsImplicit(sourceImplicit);

	to->setEdgeId(edgeId);
	to->setIsImplicit(targetImplicit);
}

void HierarchyCache::createInheritance(Id edgeId, Id fromId, Id toId)
{
	if (fromId == toId)
	{
		return;
	}

	createNode(fromId);
	HierarchyNode* to = createNode(toId);
	HierarchyNode* from = getNode(fromId);

	from->addBase(m_nodes.findIndex(toId), edgeId);
}

Id HierarchyCache::getLastVisibleParentNodeId(Id nodeId) const
{
	HierarchyNode* node = nullptr;
	HierarchyNode* parentNode = getNode(nodeId);

	while (parentNode && parentNode->isVisible())
	{
		node = parentNode;
		size_t parent = node->getParent();
		if (parent == INVALID_INDEX)
		{
			break;
		}
		parentNode = m_nodes.getByIndex(parent);

		nodeId = node->getNodeId();
	}

	return nodeId;
}

size_t HierarchyCache::getIndexOfLastVisibleParentNode(Id nodeId) const
{
	HierarchyNode* node = nullptr;
	HierarchyNode* parentNode = getNode(nodeId);

	size_t idx = 0;
	bool visible = false;

	while (parentNode)
	{
		node = parentNode;
		size_t parent = node->getParent();
		if (parent == INVALID_INDEX)
		{
			break;
		}
		parentNode = m_nodes.getByIndex(parent);

		if (node->isVisible() && !idx)
		{
			visible = true;
		}
		else if (visible)
		{
			idx++;
		}
	}

	return idx;
}

void HierarchyCache::addAllVisibleParentIdsForNodeId(
	Id nodeId, std::set<Id>* nodeIds, std::set<Id>* edgeIds) const
{
	HierarchyNode* node = getNode(nodeId);
	Id edgeId = 0;
	while (node && node->isVisible())
	{
		if (edgeId)
		{
			edgeIds->insert(edgeId);
		}

		nodeIds->insert(node->getNodeId());
		edgeId = node->getEdgeId();

		size_t parent = node->getParent();
		if (parent == INVALID_INDEX)
		{
			break;
		}
		node = m_nodes.getByIndex(parent);
	}
}

void HierarchyCache::addAllChildIdsForNodeId(Id nodeId, std::set<Id>* nodeIds, std::set<Id>* edgeIds) const
{
	HierarchyNode* node = getNode(nodeId);
	if (node && node->isVisible())
	{
		addChildIdsRecursive(node, nodeIds, edgeIds);
	}
}

void HierarchyCache::addFirstChildIdsForNodeId(
	Id nodeId, std::vector<Id>* nodeIds, std::vector<Id>* edgeIds) const
{
	HierarchyNode* node = getNode(nodeId);
	if (node)
	{
		if (node->isImplicit())
		{
			addChildIds(node, nodeIds, edgeIds);
		}
		else
		{
			addNonImplicitChildIds(node, nodeIds, edgeIds);
		}
	}
}

size_t HierarchyCache::getFirstChildIdsCountForNodeId(Id nodeId) const
{
	HierarchyNode* node = getNode(nodeId);
	if (node)
	{
		if (node->isImplicit())
		{
			return node->getChildrenCount();
		}
		else
		{
			return getNonImplicitChildrenCount(node);
		}
	}
	return 0;
}

bool HierarchyCache::isChildOfVisibleNodeOrInvisible(Id nodeId) const
{
	HierarchyNode* node = getNode(nodeId);
	if (!node)
	{
		return false;
	}

	if (!node->isVisible())
	{
		return true;
	}

	size_t parent = node->getParent();
	if (parent != INVALID_INDEX && m_nodes.getByIndex(parent)->isVisible())
	{
		return true;
	}

	return false;
}

bool HierarchyCache::nodeHasChildren(Id nodeId) const
{
	HierarchyNode* node = getNode(nodeId);
	if (node)
	{
		return node->getChildrenCount();
	}

	return false;
}

bool HierarchyCache::nodeIsVisible(Id nodeId) const
{
	HierarchyNode* node = getNode(nodeId);
	if (node)
	{
		return node->isVisible();
	}

	return false;
}

bool HierarchyCache::nodeIsImplicit(Id nodeId) const
{
	HierarchyNode* node = getNode(nodeId);
	if (node)
	{
		return node->isImplicit();
	}

	return false;
}

std::vector<std::tuple</*source*/ Id, /*target*/ Id, std::vector</*edge*/ Id>>>
HierarchyCache::getInheritanceEdgesForNodeId(
	Id sourceId, const std::set<Id>& targetIds) const
{
	// For two nodes s and t of a graph g0, this function determines the subgraph g2 that consists
	// of all nodes and edges that are reachable by going from s forwards and from t backwards as
	// follows: First the subgraph g1 that consists of all nodes and edges that are reachable from s
	// is determined. Afterwards g2 is determined by keeping only those nodes and edges of g1 that
	// are reachable by going from t backwards. If t is not in g1, then the g2 is empty.
	//
	// For example (edges are pointing upwards):
	//
	//     g0  *            g1  *            g2
	//         |                |
	//         t                t                t
	//         |                |                |
	//     *   *            *   *                *
	//      \ / \            \ / \              / \
	//       *   *            *   *            *   *
	//        \ / \            \ /              \ /
	//         *   *            *                *
	//         |                |                |
	//         s                s                s
	//         |
	//         *

	std::vector<std::tuple<Id, Id, std::vector<Id>>> inheritanceEdges;

	if (targetIds.empty())
	{
		return inheritanceEdges;
	}

	HierarchyNode* sourceNode = getNode(sourceId);
	if (!sourceNode)
	{
		return inheritanceEdges;
	}

	std::map<Id, std::vector<std::pair<Id, Id>>> reverseGraph
		= getReverseReachableInheritanceSubgraph(sourceNode);

	for (Id targetId : targetIds)
	{
		std::set<Id> nodes;
		std::vector<Id> edges;
		getReverseReachable(targetId, reverseGraph, nodes, edges);

		if (!edges.empty())
		{
			inheritanceEdges.push_back({sourceId, targetId, std::move(edges)});
		}
	}

	return inheritanceEdges;
}

void HierarchyCache::getReverseReachable(
	Id nodeId,
	const std::map</*target*/ Id, std::vector<std::pair</*source*/ Id, /*edge*/ Id>>>& reverseGraph,
	std::set<Id>& nodes,
	std::vector<Id>& edges)
{
	if (!nodes.insert(nodeId).second)
	{
		return;
	}

	auto search = reverseGraph.find(nodeId);
	if (search == reverseGraph.end())
	{
		return;
	}

	for (const std::pair<Id, Id>& nodeAndEdge : search->second)
	{
		Id node = nodeAndEdge.first;
		Id edge = nodeAndEdge.second;
		edges.push_back(edge);
		getReverseReachable(node, reverseGraph, nodes, edges);
	}
}

HierarchyCache::HierarchyNode* HierarchyCache::getNode(Id nodeId) const
{
	auto it = m_nodes.find(nodeId);

	if (it != m_nodes.end())
	{
		return it->second;
	}

	return nullptr;
}

HierarchyCache::HierarchyNode* HierarchyCache::createNode(Id nodeId)
{
	auto it = m_nodes.find(nodeId);

	if (it == m_nodes.end())
	{
		return &m_nodes.emplace(nodeId, HierarchyNode(nodeId));
	}

	return it->second;
}
