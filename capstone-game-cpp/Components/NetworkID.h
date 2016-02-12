#pragma once
#include "Components\BaseComponent.h"

class NetworkID : public BaseComponent {
public:
	int				mUUID;
	bool			mHasAuthority;

	EXPOSE_CALLBACK_1(NetAuthorityChange, bool)
	EXPOSE_CALLBACK_1(NetSyncTransform, vec3f)
};