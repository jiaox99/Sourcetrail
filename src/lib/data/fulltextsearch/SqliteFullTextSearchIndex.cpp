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
		std::vector<std::vector<long>> offsets = m_sqliteIndexStorage->queryFTSFileContentOffsets(
			_term);
		for (int i = 0; i < offsets.size(); i++)
		{
			FullTextSearchResult hit;
			std::vector<long> offset = offsets[i];

			hit.fileId = offset[0];

			for (int j = 1; j < offset.size(); j++)
			{
				hit.positions.push_back(offset[j]);
			}

			ret.push_back(hit);
		}
	}

	return ret;
}

void SqliteFullTextSearchIndex::clear()
{
	m_sqliteIndexStorage->clearFTSFileContent();
}