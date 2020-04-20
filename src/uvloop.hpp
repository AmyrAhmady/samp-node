#pragma once
#include <string>
#include "uv.h"

class UvLoop
{
private:
	uv_loop_t m_loop;
	std::string m_loopName;

public:
	UvLoop(const std::string& name);
	~UvLoop();

	inline uv_loop_t* GetLoop()
	{
		return &m_loop;
	}

	inline const std::string& GetName() const
	{
		return m_loopName;
	}
};