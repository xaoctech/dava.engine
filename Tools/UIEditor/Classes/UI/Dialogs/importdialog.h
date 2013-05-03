#ifndef __UIEDITOR__IMPORTDIALOG__
#define __UIEDITOR__IMPORTDIALOG__

#include <QDialog>
#include "DAVAEngine.h"
#include "HierarchyTreeNode.h"

using namespace DAVA;

namespace Ui {
	class ImportDialog;
}

class ImportDialogFileWidget;
class ImportDialog : public QDialog
{
	Q_OBJECT

public:
	enum eImportType
	{
		IMPORT_PLATFORM = 0,
		IMPORT_SCREEN,
		IMPORT_TYPE_COUNT
	};

	enum eAction
	{
		ACTION_IGNORE = 0,
		ACTION_SCREEN,
		ACTION_AGGREGATOR,
		ACTIONS_COUNT
	};

	struct FileItem
	{
		QString fileName;
		eAction action;
		QSize size;
	};

	explicit ImportDialog(eImportType type, QWidget *parent = 0, const FilePath& platformPath = FilePath(""));

	~ImportDialog();

	HierarchyTreeNode::HIERARCHYTREENODEID GetPlatformId();
	Vector<FileItem> GetFiles();
	QSize GetPlatformSize();

private slots:
	void OnActionChanged(uint32 id);
	void UpdateOkButtonState();

private:
	static const eAction DEFAULT_ACTION = ACTION_IGNORE;
	static const uint32 CONTROL_WIDGET_ID = (uint32)-1;

	Ui::ImportDialog *ui;

	eImportType importType;
	ImportDialogFileWidget* controlWidget;
	Vector<ImportDialogFileWidget*> fileWidgets;
	HierarchyTreeNode::HIERARCHYTREENODEID platformId;
	FilePath platformPath;

	void ClearCurrentList();
	bool CheckAggregatorsSize();
	void InitWithFileList(const Vector<String>& fileList);
	void InitScreensImport();
	void InitPlatformImport();

	Vector<String> GetDirectoryContent(const FilePath& path, bool getFiles = false,
									   const String& fileMask = "",
									   bool removeExtension = true);
	Vector<String> GetChildrenNames(HierarchyTreeNode* node);
	Vector<String> GetStringNotInList(const Vector<String>& strings,
									  const Vector<String>& listToSearchIn);
};

#endif /* defined(__UIEDITOR__IMPORTDIALOG__) */
