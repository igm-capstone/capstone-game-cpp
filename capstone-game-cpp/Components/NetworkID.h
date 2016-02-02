#pragma once
#include "Components\BaseComponent.h"

class NetworkID : public BaseComponent {
public:
	int				mUUID;
	bool			mHasAuthority;

	typedef void(*NetSyncCallback)(BaseSceneObject*, vec3f);
	typedef void(*NetAuthorityCallback)(BaseSceneObject*, bool);

private:
	//callbacks
	NetSyncCallback mOnNetSyncTransform;
	NetAuthorityCallback mOnNetAuthorityChange;

public:
	void RegisterCallback(NetSyncCallback handler)
	{
		mOnNetSyncTransform = handler;
	}
	void RegisterCallback(NetAuthorityCallback handler)
	{
		mOnNetAuthorityChange = handler;
	}

	void OnNetSyncTransform(vec3f newPos)
	{
		if (mOnNetSyncTransform)
		{
			return mOnNetSyncTransform(mSceneObject, newPos);
		}
	}
	void OnNetAuthorityChange(bool newAuth)
	{
		if (mOnNetAuthorityChange)
		{
			return mOnNetAuthorityChange(mSceneObject, newAuth);
		}
	}
};