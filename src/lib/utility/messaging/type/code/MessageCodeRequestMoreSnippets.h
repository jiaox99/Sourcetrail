#ifndef MESSAGE_CODE_REQUEST_MORE_SNIPPETS_H
#define MESSAGE_CODE_REQUEST_MORE_SNIPPETS_H


#include "Message.h"
#include "TabId.h"#pragma once

class MessageCodeRequestMoreSnippets: public Message<MessageCodeRequestMoreSnippets>
{
public:

	MessageCodeRequestMoreSnippets()
	{
		setSchedulerId(TabId::currentTab());
	}

	static const std::string getStaticType()
	{
		return "MessageCodeRequestMoreSnippets";
	}

	virtual void print(std::wostream& os) const
	{
		os << L"MessageCodeRequestMoreSnippets";
	}
};

#endif
