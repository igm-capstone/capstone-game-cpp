#include "SceneObjects/BaseSceneObject.h"

class Door : public BaseSceneObject
{
	friend class Factory<Door>;

public:
	bool mCanOpen;

private:
	Door() : mCanOpen(false)
	{
	}

	~Door()
	{
	}
};