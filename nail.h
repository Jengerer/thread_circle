#pragma once

#include "vector.h"

// Nail around which a bit of thread is wrapped
class Nail
{
public:
	Nail();

	Vector getExitPosition(const Vector& incoming) const
	{

	}

private:
	Vector position;
};