/// MIT License, Copyright Burak Kara, burak@burak.io, https://en.wikipedia.org/wiki/MIT_License

#pragma once

#include "CoreMinimal.h"
#include "BHttpServerHandler.h"

class BGENERICAUTHFILESRVCLIENT_API BKLoginBrowserListener : public BKHttpServerHandler
{

public:
	/** Override to implement your custom logic of request processing */
	virtual void ProcessRequest(const FGuid& RequestUniqueId, const FString& RequestData) override;

	void OpenBrowser(const FString& Url);

	~BKLoginBrowserListener();

private:
	class FDelegateHandle ProcessCompletedDelegateHandle;
};