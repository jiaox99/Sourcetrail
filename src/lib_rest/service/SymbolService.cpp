#include "SymbolService.h"
#include "StorageAccess.h"
#include "NodeTypeSet.h"
#include "NameHierarchy.h"
#include "Graph.h"
#include "rest_util.h"
#include <set>
#include <sstream>

namespace sourcetrail {
namespace service {

// Helper functions (moved from sourcetrail_v0_Symbols.cc)
namespace {

void symbolNameToIds(const std::vector<std::string>& symbolFullNames, std::vector<Id>& outSymbolIds, StorageAccess* storage)
{
	for (const auto& symbolFullName: symbolFullNames)
	{
		NameHierarchy symbolHierarchy = NameHierarchy::deserialize(to_wstring(symbolFullName));
		auto symbolId = storage->getNodeIdForNameHierarchy(symbolHierarchy);
		if (symbolId != 0)
			outSymbolIds.push_back(symbolId);
	}
}

void printClassNode(std::wostream& wos, const Node& curNode, bool simpleMode=true, const Node* pFieldNode=nullptr)
{
	wos << L"class " << curNode.getName() << L" {" << std::endl;
	if (!simpleMode)
	{
		curNode.forEachEdgeOfType(Edge::EDGE_MEMBER,
			[&wos](Edge* edge)
			{
				auto fieldNode = edge->getTo();
				if (fieldNode->getType().getKind() & NODE_FIELD)
				{
					wos << fieldNode->getNameHierarchy().getRawNameWithSignature() << std::endl;
				}

			});

		wos << L"---" << std::endl;

		curNode.forEachEdgeOfType(Edge::EDGE_MEMBER,
			[&wos](Edge* edge)
			{
				auto fieldNode = edge->getTo();
				if (fieldNode->getType().getKind() & NODE_METHOD)
				{
					wos << fieldNode->getNameHierarchy().getRawNameWithSignature() << std::endl;
				}

			});
	}

	if (pFieldNode != nullptr)
	{
		wos << pFieldNode->getNameHierarchy().getRawNameWithSignature() << std::endl;
	}
	wos << L"}" << std::endl;
}

void printSuperClassNode(std::wostream& wos, const Node& curNode, std::set<Id>& printedNodeIds)
{
	curNode.forEachEdgeOfType(
		Edge::EDGE_INHERITANCE,
		[&wos, &printedNodeIds, &curNode](Edge* edge)
		{
			auto superClassNode = edge->getTo();
			if (printedNodeIds.find(superClassNode->getId()) == printedNodeIds.end())
			{
				printedNodeIds.insert(superClassNode->getId());
				printClassNode(wos, *superClassNode);
				wos << curNode.getName() << L" --> " << superClassNode->getName()
					<< std::endl;
				printSuperClassNode(wos, *superClassNode, printedNodeIds);
			}
		});
}

void printSubClassNodes(std::wostream& wos, const Node& curNode, std::set<Id>& printedNodeIds)
{
	curNode.forEachEdgeOfType(Edge::EDGE_INHERITANCE,
		[&wos, &printedNodeIds, &curNode](Edge* edge)
		{
			auto subClassNode = edge->getFrom();
			if (printedNodeIds.find(subClassNode->getId()) == printedNodeIds.end())
			{
				printedNodeIds.insert(subClassNode->getId());
				printClassNode(wos, *subClassNode);
				wos << subClassNode->getName() << L" --> " << curNode.getName() << std::endl;
				printSubClassNodes(wos, *subClassNode, printedNodeIds);
			}
		});
}

void printGraphAsClass(const Graph& graph, std::wostream& wos, const Node& coreNode)
{
	wos << L"@startuml" << std::endl;

	std::set<Id> printedNodeIds;

	printSuperClassNode(wos, coreNode, printedNodeIds);
	printClassNode(wos, coreNode, false);
	printSubClassNodes(wos, coreNode, printedNodeIds);

	wos << L"@enduml";
}

void printGraphAsFunction(const Graph& graph, std::wostream& wos, const Node& coreNode)
{
	wos << L"@startuml" << std::endl;

	std::set<Id> printedNodeIds;
	std::wstring functionNodeStr;
	if (coreNode.getParentNode() != nullptr)
	{
		printClassNode(wos, *coreNode.getParentNode(), true, &coreNode);
		functionNodeStr = coreNode.getParentNode()->getName() + L"::" + coreNode.getName();
	}
	else
	{
		functionNodeStr = coreNode.getName();
		wos << L"object " << functionNodeStr << std::endl;
	}

	printedNodeIds.insert(coreNode.getId());

	coreNode.forEachEdgeOfType(Edge::EDGE_CALL,
		[&wos, &printedNodeIds, &functionNodeStr](Edge* edge)
		{
			auto calleeNode = edge->getFrom();
			if (printedNodeIds.find(calleeNode->getId()) == printedNodeIds.end())
			{
				printedNodeIds.insert(calleeNode->getId());
				if (calleeNode->getParentNode() != nullptr)
				{
					printClassNode(wos, *calleeNode->getParentNode(), true, calleeNode);
					wos << functionNodeStr << L" <-- " << calleeNode->getParentNode()->getName()
						<< L"::" << calleeNode->getName() << std::endl;
				}
				else
				{
					wos << L"object " << calleeNode->getName() << std::endl;
					wos << functionNodeStr << L" <-- " << calleeNode->getName() << std::endl;
				}
			}
		});

	wos << L"@enduml" << std::endl;
}

void printGraphAsFile(const Graph& graph, std::wostream& wos, const Node& coreNode)
{
	wos << L"@startuml" << std::endl;

	std::set<Id> printedNodeIds;
	std::wstring fileNodeStr;
	fileNodeStr = coreNode.getName();
	wos << L"object " << fileNodeStr << std::endl;

	printedNodeIds.insert(coreNode.getId());

	coreNode.forEachEdgeOfType(
		Edge::EDGE_INCLUDE,
		[&wos, &printedNodeIds, &fileNodeStr](Edge* edge)
		{
			auto calleeNode = edge->getFrom();
			if (printedNodeIds.find(calleeNode->getId()) == printedNodeIds.end())
			{
				wos << L"object " << calleeNode->getName() << std::endl;
				wos << fileNodeStr << L" <-- " << calleeNode->getName() << std::endl;
			}
		});

	wos << L"@enduml" << std::endl;
}

void printGraphAsField(const Graph& graph, std::wostream& wos, const Node& coreNode)
{
	wos << L"@startuml" << std::endl;

	std::set<Id> printedNodeIds;
	std::wstring fieldNodeStr;
	if (coreNode.getParentNode() != nullptr)
	{
		printClassNode(wos, *coreNode.getParentNode(), true, &coreNode);
		fieldNodeStr = coreNode.getParentNode()->getName() + L"::" + coreNode.getName();
	}
	else
	{
		fieldNodeStr = coreNode.getName();
		wos << L"object " << fieldNodeStr << std::endl;
	}

	printedNodeIds.insert(coreNode.getId());

	coreNode.forEachEdgeOfType(Edge::EDGE_USAGE,
		[&wos, &printedNodeIds, &fieldNodeStr](Edge* edge)
		{
			auto calleeNode = edge->getFrom();
			if (printedNodeIds.find(calleeNode->getId()) == printedNodeIds.end())
			{
				printedNodeIds.insert(calleeNode->getId());
				if (calleeNode->getParentNode() != nullptr)
				{
					printClassNode(wos, *calleeNode->getParentNode(), true, calleeNode);
					wos << fieldNodeStr << L" <-- "
						<< calleeNode->getParentNode()->getName() << L"::" << calleeNode->getName() << std::endl;
				}
				else
				{
					wos << L"object " << calleeNode->getName() << std::endl;
					wos << fieldNodeStr << L" <-- " << calleeNode->getName() << std::endl;
				}
			}
		});

	wos << L"@enduml" << std::endl;
}

NodeKindMask fromNodeKindStrings(const std::vector<std::string>& nodeKinds)
{
	NodeKindMask mask = 0;
	for (const auto& nodeKindStr: nodeKinds)
	{
		mask |= getNodeKindForReadableNodeKindString(to_wstring(nodeKindStr));
	}
	mask = mask == 0 ? ~0 : mask;
	return mask;
}

Edge::TypeMask fromEdgeTypeStrings(const std::vector<std::string>& edgeTypes)
{
	Edge::TypeMask mask = 0;
	for (const auto& edgeTypeStr: edgeTypes)
	{
		mask |= Edge::getTypeForReadableTypeString(to_wstring(edgeTypeStr));
	}

	mask = mask == 0 ? ~0 : mask;
	return mask;
}

} // anonymous namespace

// Public service functions

Json::Value fuzzySearchSymbols(const FuzzySearchParams& params, StorageAccess* storage) {
	auto matches = storage->getAutocompletionMatches(
		params.query,
		NodeTypeSet::all(),
		false  // acceptCommands
	);

	Json::Value result(Json::arrayValue);
	int count = 0;
	for (const auto& entry : matches) {
		for (const auto& tokenName : entry.tokenNames) {
			Json::Value item;
			item["fullName"] = to_string(tokenName.getQualifiedName());
			item["serializedName"] = to_string(NameHierarchy::serialize(tokenName));
			result.append(item);

			if (++count >= params.maxResults) break;
		}
		if (count >= params.maxResults) break;
	}

	return result;
}

std::wstring generateSymbolGraph(const GraphQueryParams& params, StorageAccess* storage) {
	if (params.symbolFullNames.empty()) {
		return L"Error: No symbolFullNames provided";
	}

	std::vector<Id> symbolIds;
	symbolNameToIds(params.symbolFullNames, symbolIds, storage);
	if (symbolIds.empty()) {
		return L"Error: No valid Symbol Names found";
	}

	std::vector<Id> expandedNodeIds;
	bool isNameSpace = false;
	auto graph = storage->getGraphForActiveTokenIds(symbolIds, expandedNodeIds, &isNameSpace);

	auto coreNode = graph->getNodeById(symbolIds[0]);
	std::wstringstream wss;

	switch (coreNode->getType().getKind())
	{
	case NODE_CLASS:
	case NODE_STRUCT:
		printGraphAsClass(*graph, wss, *coreNode);
		break;
	case NODE_FUNCTION:
	case NODE_METHOD:
		printGraphAsFunction(*graph, wss, *coreNode);
		break;
	case NODE_GLOBAL_VARIABLE:
	case NODE_FIELD:
		printGraphAsField(*graph, wss, *coreNode);
		break;
	case NODE_FILE:
		printGraphAsFile(*graph, wss, *coreNode);
		break;
	default:
		graph->print(wss);
		break;
	}

	return wss.str();
}

std::wstring generateCustomGraph(const GraphQueryParams& params, StorageAccess* storage) {
	if (params.symbolFullNames.empty()) {
		return L"Error: No symbolFullNames provided";
	}

	std::vector<Id> symbolIds;
	symbolNameToIds(params.symbolFullNames, symbolIds, storage);
	if (symbolIds.size() != 2) {
		return L"Error: Two Symbol Names needed";
	}

	auto graph = storage->getGraphForTrail(
		symbolIds[0],
		symbolIds[1],
		fromNodeKindStrings(params.nodeTypes),
		fromEdgeTypeStrings(params.edgeTypes),
		true,
		params.maxDepth,
		true
	);

	std::wstringstream wss;
	if (graph) {
		graph->print(wss);
	} else {
		wss << L"Error: Failed to generate graph";
	}

	return wss.str();
}

}} // namespace
