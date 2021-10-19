#pragma once

class IHaveId
{
protected:
	unsigned int id_ = 0;

public:
	unsigned int getId() const;
};

