#ifndef MESSAGE_EXPORT_REFERENCES_H
#define MESSAGE_EXPORT_REFERENCES_H

#include "Message.h"
#include "TabId.h"

class MessageExportReferences : public Message<MessageExportReferences>
{
public:
	MessageExportReferences()
	{
		setSchedulerId( TabId::currentTab() );
	}

	static const std::string getStaticType()
	{
		return "MessageExportReferences";
	}
};

#endif