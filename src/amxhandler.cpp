#include <unordered_map>
#include "amx//amx.h"
#include "amxhandler.hpp"


namespace sampnode
{
		std::unordered_map<AMX*, amx*> amx::amx_list = std::unordered_map<AMX*, amx*>();

		void amx::load(AMX* _amx)
		{
			amx_list[_amx] = new amx(_amx);
		}

		void amx::unload(AMX* amx)
		{
			amx_list.erase(amx);
		}

		amx* amx::get(AMX* _amx)
		{
			return amx_list.at(_amx);
		}

		amx::amx(AMX* _amx)
		{
			internal_amx = _amx;
		}

		amx::~amx()
		{

		}
}