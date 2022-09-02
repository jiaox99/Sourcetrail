#include "SqliteFullTextSearchIndex.h"

#include "logging.h"
#include "tracing.h"

void SqliteFullTextSearchIndex::setStorage(SqliteIndexStorage* p)
{
	m_sqliteIndexStorage = p;
}

void SqliteFullTextSearchIndex::addFile(const Id id, const std::string& data)
{
	m_sqliteIndexStorage->addFTSFileContent(id, data);
}

std::vector<FullTextSearchResult> SqliteFullTextSearchIndex::searchForTerm(const std::wstring& term) const
{
	TRACE();

	std::vector<FullTextSearchResult> ret;
	{
		const std::string _term(term.begin(), term.end());
		std::vector<std::vector<int>> offsets = m_sqliteIndexStorage->queryFTSFileContentOffsets(_term);
		std::vector<int> ids = m_sqliteIndexStorage->queryFTSFileContentIds(_term);
		for (int i = 0; i < ids.size(); i++)
		{
			FullTextSearchResult hit;
			hit.fileId = ids[i];
			hit.positions = offsets[i];

			ret.push_back(hit);
		}
	}

	return ret;
}

void SqliteFullTextSearchIndex::clear()
{
	m_sqliteIndexStorage->clearFTSFileContent();
}