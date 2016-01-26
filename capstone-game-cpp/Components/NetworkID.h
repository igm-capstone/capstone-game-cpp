#pragma once
#include "Components\BaseComponent.h"

class NetworkID : public BaseComponent {
public:
	int				mUUID;
	bool			mHasAuthority;
};