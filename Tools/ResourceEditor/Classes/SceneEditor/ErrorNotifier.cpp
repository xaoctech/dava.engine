#include "ErrorNotifier.h"
#include "ErrorDialog.h"

ErrorNotifier::ErrorNotifier()
{
    errorDialog = new ErrorDialog();
}
    
ErrorNotifier::~ErrorNotifier()
{
    SafeRelease(errorDialog);
}

void ErrorNotifier::ShowError(const String &errorMessage)
{
    Set<String> errors;
    errors.insert(errorMessage);
    ShowError(errors);
}

void ErrorNotifier::ShowError(const Set<String> &errorMessages)
{
    if(errorDialog && !errorDialog->GetParent())
    {
        errorDialog->Show(errorMessages);
    }
}
