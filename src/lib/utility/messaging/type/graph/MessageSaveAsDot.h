#ifndef MESSAGE_SAVE_AS_DOT_H
#define MESSAGE_SAVE_AS_DOT_H

#include "Message.h"

class MessageSaveAsDot : public Message<MessageSaveAsDot>
{
public:
	MessageSaveAsDot(std::wstring* _pContent, Id _scheduleId)
		: pContent(_pContent), scheduleId(_scheduleId)
	{}

	static const std::string getStaticType()
	{
		return "MessageSaveAsDot";
	}

	void setContent(std::wstring& content)
	{
		if (pContent != nullptr)
		{
			*pContent = content;
		}
	}

	Id scheduleId;

private:
	std::wstring* pContent;
};

#endif /* MESSAGE_SAVE_AS_DOT_H */
