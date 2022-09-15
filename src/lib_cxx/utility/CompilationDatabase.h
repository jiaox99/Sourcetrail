#ifndef UTILITY_COMPILATION_DATABASE_H
#define UTILITY_COMPILATION_DATABASE_H

#include <string>
#include <vector>

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#include "FilePath.h"

namespace utility
{
class CompilationDatabase
{
public:
	CompilationDatabase(const FilePath& filePath);

	std::vector<FilePath> getAllHeaderPaths() const;
	std::vector<FilePath> getHeaderPaths() const;
	std::vector<FilePath> getSystemHeaderPaths() const;
	std::vector<FilePath> getFrameworkHeaderPaths() const;
	clang::tooling::JSONCompilationDatabase* getCDB() const;
	std::shared_ptr<clang::tooling::JSONCompilationDatabase> getSharedCDB() const;

private:
	void init();

	FilePath m_filePath;
	std::shared_ptr<clang::tooling::JSONCompilationDatabase> m_cdb;
	std::vector<FilePath> m_headers;
	std::vector<FilePath> m_systemHeaders;
	std::vector<FilePath> m_frameworkHeaders;
};

}	 // namespace utility

#endif	  // UTILITY_COMPILATION_DATABASE_H
