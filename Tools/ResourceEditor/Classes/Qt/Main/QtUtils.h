#ifndef __QT_UTILS_H__
#define __QT_UTILS_H__

#include "DAVAEngine.h"
#include <QString>
#include <QColor>
#include <QMessageBox>


#define QSTRING_TO_DAVASTRING(str)   (str).toStdString().data()

DAVA::FilePath PathnameToDAVAStyle(const QString &convertedPathname);

DAVA::FilePath GetOpenFileName(const DAVA::String &title, const DAVA::FilePath &pathname, const DAVA::String &filter);


DAVA::WideString SizeInBytesToWideString(DAVA::float32 size);
DAVA::String SizeInBytesToString(DAVA::float32 size);

DAVA::Image * CreateTopLevelImage(const DAVA::FilePath &imagePathname);

void ShowErrorDialog(const DAVA::Set<DAVA::String> &errors);
void ShowErrorDialog(const DAVA::String &errorMessage);

bool IsKeyModificatorPressed(DAVA::int32 key);
bool IsKeyModificatorsPressed();

QColor ColorToQColor(const DAVA::Color& color);
DAVA::Color QColorToColor(const QColor& qcolor);

enum eMessageBoxFlags
{
    MB_FLAG_YES = QMessageBox::Yes,
    MB_FLAG_NO = QMessageBox::No,
    MB_FLAG_CANCEL = QMessageBox::Cancel
};

int ShowQuestion(const DAVA::String &header, const DAVA::String &question, int buttons, int defaultButton);
int ShowSaveSceneQuestion(DAVA::Scene *scene);



#endif // __QT_UTILS_H__
