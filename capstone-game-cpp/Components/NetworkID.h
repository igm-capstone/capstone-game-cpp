#pragma once
#include "Components\BaseComponent.h"

class NetworkID : public BaseComponent {
public:
	int				mUUID;
	bool			mHasAuthority;

	NetworkID(): mUUID(0), mHasAuthority(false) {}
	~NetworkID() {}

	EXPOSE_CALLBACK_1(NetAuthorityChange, bool)
	EXPOSE_CALLBACK_2(NetSyncTransform, vec3f, quatf)
	EXPOSE_CALLBACK_3(NetHealthChange, float, float, float)
	EXPOSE_CALLBACK_2(NetSyncAnimation, byte, byte)
	EXPOSE_CALLBACK_0(Interact)
};