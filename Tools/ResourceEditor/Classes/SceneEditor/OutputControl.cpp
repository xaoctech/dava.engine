#include "OutputControl.h"

#include "ControlsFactory.h"

OutputControl::OutputControl(const Rect & rect)
    :   UIControl(rect)
{
    messageList = new UIList(Rect(5, 0, rect.dx - 10, rect.dy), UIList::ORIENTATION_VERTICAL);
    messageList->SetDelegate(this);
    ControlsFactory::SetScrollbar(messageList);
    AddControl(messageList);
}


OutputControl::~OutputControl()
{
    SafeRelease(messageList);

    ReleaseList();
}

void OutputControl::WillAppear()
{
    messageList->Refresh();
}

int32 OutputControl::ElementsCount(UIList * list)
{
    return messages.size();
}

UIListCell *OutputControl::CellAtIndex(UIList *list, int32 index)
{
    int32 width = list->GetRect().dx;
    
	UIListCell *c = (UIListCell *)list->GetReusableCell("OutputCell");
	if(!c)
	{ 
		c = new UIListCell(Rect(0, 0, width, 20), "OutputCell");
	}
    
    LogMessage *lm = messages[index];
	c->SetStateText(UIControl::STATE_NORMAL, lm->text);
	c->SetStateText(UIControl::STATE_SELECTED, lm->text);

    Font *font = ControlsFactory::GetFontLight()->Clone();
    switch (lm->type) 
    {
        case EMT_LOG:
            font->SetColor(Color(0.f, 0.f, 0.f, 1.0f));
            break;

        case EMT_WARNING:
            font->SetColor(Color(1.f, 1.f, 0.f, 1.0f));
            break;

        case EMT_ERROR:
            font->SetColor(Color(1.f, 0.f, 0.f, 1.0f));
            break;
    
        default:
            break;
    }
    
	c->SetStateFont(UIControl::STATE_NORMAL, font);
	c->SetStateFont(UIControl::STATE_SELECTED, font);

	c->SetStateAlign(UIControl::STATE_NORMAL, ALIGN_LEFT | ALIGN_VCENTER);
	c->SetStateAlign(UIControl::STATE_SELECTED, ALIGN_LEFT | ALIGN_VCENTER);
	c->GetStateTextControl(UIControl::STATE_NORMAL)->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
	c->GetStateTextControl(UIControl::STATE_SELECTED)->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    
    SafeRelease(font);
	return c;
}

int32 OutputControl::CellHeight(UIList * list, int32 index)
{
    return CELL_HEIGHT;
}

void OutputControl::Log(const WideString &message)
{
    AddMessageToList(EMT_LOG, message);
}

void OutputControl::Warning(const WideString &message)
{
    AddMessageToList(EMT_WARNING, message);
}

void OutputControl::Error(const WideString &message)
{
    AddMessageToList(EMT_ERROR, message);
}

void OutputControl::AddMessageToList(eMessageType type, const WideString &message)
{
    LogMessage *lm = new LogMessage();
    lm->type = type;
    lm->text = message;
    
    messages.push_back(lm);
    
    if(HISTORY_SIZE < messages.size())
    {
        Vector<LogMessage *>::iterator it = messages.begin();
        LogMessage *lmToRelease = (*it);
        SafeDelete(lmToRelease);
        
        messages.erase(it);
    }
    
    messageList->Refresh();
}

void OutputControl::Clear()
{
    ReleaseList();
    messageList->Refresh();
}

void OutputControl::ReleaseList()
{
    for(Vector<LogMessage *>::iterator it = messages.begin(); it != messages.end(); ++it)
    {
        LogMessage *lmToRelease = (*it);
        SafeDelete(lmToRelease);
    }
    messages.clear();
}