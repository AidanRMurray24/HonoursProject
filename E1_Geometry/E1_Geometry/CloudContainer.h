#pragma once
#include "SceneObject.h"

class CloudContainer : public SceneObject
{
public:
	CloudContainer();
	~CloudContainer();

	void Render(XMMATRIX view, XMMATRIX proj, ID3D11ShaderResourceView* texture);

private:

};

