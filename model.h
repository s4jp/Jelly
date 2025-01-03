#pragma once
#include "figure.h"
#include <string>

class Model : public Figure
{
public:
	std::vector<float> minMaxValues;
	float color[4] = { 1, 1, 0, 1 };

	Model(std::string path);

	void Render(int colorLoc) override;

private:
	void FillMinMaxValues(std::string path);
};
