#pragma once

#include "FullTextSearchIndex.h"
#include "SqliteIndexStorage.h"

class SqliteFullTextSearchIndex
{
public:
	void setStorage(SqliteIndexStorage* p);
	void addFile(const Id id, const std::string& data);
	std::vector<FullTextSearchResult> searchForTerm(const std::wstring& term) const;
	void clear();

private:
	SqliteIndexStorage* m_sqliteIndexStorage;
};
