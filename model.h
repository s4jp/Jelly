#pragma once
#include "figure.h"
#include <string>

class Model : public Figure
{
public:
	float color[4] = { 1, 1, 0, 1 };

	Model(std::string path);

	void Render(int colorLoc) override;
};
