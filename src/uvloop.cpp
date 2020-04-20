#include "uvloop.hpp"

UvLoop::UvLoop(const std::string& loopTag)
	: m_loopName(loopTag)
{
	uv_loop_init(&m_loop);
	m_loop.data = this;
	uv_run(&m_loop, UV_RUN_NOWAIT);
}

UvLoop::~UvLoop()
{
	uv_async_t async;
	uv_stop(&m_loop);

	uv_async_init(&m_loop, &async, [](uv_async_t*)
		{

		});

	uv_async_send(&async);
	uv_close(reinterpret_cast<uv_handle_t*>(&async), nullptr);
}