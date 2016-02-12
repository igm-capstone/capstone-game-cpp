#pragma once
#include "Components\BaseComponent.h"

class NetworkID : public BaseComponent {
public:
	int				mUUID;
	bool			mHasAuthority;

	EXPOSE_CALLBACK(NetAuthorityChange, bool)
	EXPOSE_CALLBACK(NetSyncTransform, vec3f)
	EXPOSE_CALLBACK(NetHealthChange, float)
};