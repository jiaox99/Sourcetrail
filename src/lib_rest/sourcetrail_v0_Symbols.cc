#include "sourcetrail_v0_Symbols.h"
#include "rest_api_main.h"
#include "StorageAccess.h"
#include "boost/locale.hpp"
#include "NodeTypeSet.h"
#include "Graph.h"
#include "NameHierarchy.h"
#include "rest_util.h"
#include <set>

using namespace sourcetrail::v0;
using namespace drogon;

// Add definition of your processing function here
void Symbols::fuzzyQuery(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, const std::string& query) const
{
	int limit = req->getOptionalParameter<int>("max").value_or(50);
	auto *storage = getStorageInstance();
	auto result = storage->getAutocompletionMatches(to_wstring(query), NodeTypeSet::all(), false);
	Json::Value jsonResult;
	int count = 0;
	for (const auto& entry: result)
	{
		for (const auto& tokenName : entry.tokenNames)
		{
			Json::Value tokenValue;
			tokenValue["fullName"] = to_string(tokenName.getQualifiedName());
			tokenValue["serializedName"] = to_string(NameHierarchy::serialize(tokenName));
			jsonResult.append(tokenValue);
			if (++count >= limit)
				break;
		}

		if (count >= limit)
			break;
	}
	auto resp = HttpResponse::newHttpJsonResponse(jsonResult);
	callback(resp);
}

bool validateQueryRequest(const SymbolQueryRequest& query, std::function<void(const drogon::HttpResponsePtr&)>& callback)
{
	if (query.symbolFullNames.empty())
	{
		message_response("No symbolFullNames provided", callback);
		return false;
	}

	return true;
}

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

void Symbols::graphQuery(const SymbolQueryRequest&& query, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
	if (!validateQueryRequest(query, callback))
	{
		return;
	}

	auto *storage = getStorageInstance();
	std::vector<Id> symbolIds;
	symbolNameToIds(query.symbolFullNames, symbolIds, storage);
	if (symbolIds.empty())
	{
		message_response("No valid Symbol Names found", callback);
		return;
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

	message_response(to_string(wss.str()), callback);
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

void Symbols::customGraphQuery(const SymbolQueryRequest&& query, std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
	if (!validateQueryRequest(query, callback))
	{
		return;
	}

	auto* storage = getStorageInstance();
	std::vector<Id> symbolIds;
	symbolNameToIds(query.symbolFullNames, symbolIds, storage);
	if (symbolIds.size() != 2)
	{
		message_response("Two Symbol Names needed", callback);
		return;
	}
	auto graph = storage->getGraphForTrail(symbolIds[0], symbolIds[1], fromNodeKindStrings(query.nodeTypes), fromEdgeTypeStrings(query.edgeTypes), true, query.maxDepth, true);
}