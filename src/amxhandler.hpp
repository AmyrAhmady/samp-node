#pragma once

namespace sampnode
{
	class amx
	{
	public:
		static std::unordered_map<AMX*, amx*> amx_list;

		static void load(AMX* _amx);
		static void unload(AMX* _amx);
		static amx* get(AMX* _amx);

		amx(AMX* _amx);
		~amx();

		AMX* get() { return internal_amx; }

	private:
		AMX* internal_amx;
	};
}