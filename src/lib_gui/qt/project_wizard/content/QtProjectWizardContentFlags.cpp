#include "QtProjectWizardContentFlags.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QMessageBox>

#include "QtStringListBox.h"
#include "SourceGroupSettingsWithCxxPathsAndFlags.h"

QtProjectWizardContentFlags::QtProjectWizardContentFlags(
	std::shared_ptr<SourceGroupSettingsWithCxxPathsAndFlags> settings,
	QtProjectWizardWindow* window,
	bool indicateAsAdditional)
	: QtProjectWizardContent(window)
	, m_settings(settings)
	, m_indicateAsAdditional(indicateAsAdditional)
{
}

void QtProjectWizardContentFlags::populate(QGridLayout* layout, int& row)
{
	const QString labelText(
		(std::string(m_indicateAsAdditional ? "Additional " : "") + "Compiler Flags").c_str());
	QLabel* label = createFormLabel(labelText);
	layout->addWidget(label, row, QtProjectWizardWindow::FRONT_COL, Qt::AlignTop);

	addHelpButton(
		labelText,
		QStringLiteral(
			"<p>Define additional Clang compiler flags used during indexing. Here are some "
			"examples:</p>"
			"<ul style=\"-qt-list-indent:0;\">"
			"<li style=\"margin-left:1em\">use '-DRELEASE' to add a preprocessor #define for "
			"'RELEASE'</li>"
			"<li style=\"margin-left:1em\">use '-U__clang__' to remove the preprocessor #define "
			"for "
			"'__clang__'</li>"
			"<li style=\"margin-left:1em\">use '-DFOO=900' to add an integer preprocessor "
			"define</li>"
			"<li style=\"margin-left:1em\">use '-DFOO=\"bar\"' to add a string preprocessor "
			"define</li>"
			"</ul>"),
		layout,
		row);

	m_list = new QtStringListBox(this, label->text());
	layout->addWidget(m_list, row, QtProjectWizardWindow::BACK_COL);
	row++;

	const QString useToolCxxHeadersText(QStringLiteral("Use Sourcetrail's bundled Clang header files"));
	addHelpButton(
		useToolCxxHeadersText,
		QStringLiteral(
			"<p>Uncheck <b>") +
			useToolCxxHeadersText +
			QStringLiteral(
				"</b> to exclude the C/C++ header files that Sourcetrail bundles for its own Clang "
				"indexer from the include search path.</p>"
				"<p>These headers (e.g. xmmintrin.h, immintrin.h and similar compiler intrinsics "
				"headers) are usually helpful because they match Sourcetrail's Clang version "
				"exactly. But for some projects, especially those that mix Clang with MSVC (via "
				"'-fms-compatibility') and rely on their real compiler's own intrinsics headers, "
				"Sourcetrail's bundled headers can conflict with the ones already provided by the "
				"project's own search paths, causing indexing errors. Uncheck this option to make "
				"the indexer fall back to whatever headers are visible through your project's own "
				"'Include Paths' / compilation database entries.</p>"),
		layout,
		row);

	m_useToolCxxHeaders = new QCheckBox(useToolCxxHeadersText);
	layout->addWidget(m_useToolCxxHeaders, row, QtProjectWizardWindow::BACK_COL);
	row++;
}

void QtProjectWizardContentFlags::load()
{
	m_list->setStrings(m_settings->getCompilerFlags());
	m_useToolCxxHeaders->setChecked(m_settings->getUseToolCxxHeaders());
}

void QtProjectWizardContentFlags::save()
{
	m_settings->setCompilerFlags(m_list->getStrings());
	m_settings->setUseToolCxxHeaders(m_useToolCxxHeaders->isChecked());
}

bool QtProjectWizardContentFlags::check()
{
	std::wstring error;

	for (const std::wstring& flag: m_list->getStrings())
	{
		if (utility::isPrefix<std::wstring>(L"-include ", flag) ||
			utility::isPrefix<std::wstring>(L"--include ", flag))
		{
			error = L"The entered compiler flag \"" + flag +
				L"\" contains an error. Please remove the intermediate space character.\n";
		}
	}

	if (!error.empty())
	{
		QMessageBox msgBox(m_window);
		msgBox.setText(QString::fromStdWString(error));
		msgBox.exec();
		return false;
	}

	return true;
}
