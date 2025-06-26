#pragma once

namespace Distribute
{
	inline RE::BGSKeyword* processed{ nullptr };

	namespace detail
	{
		bool should_process_NPC(RE::TESNPC* a_npc, RE::BGSKeyword* a_keyword = processed);
		void distribute_on_load(RE::Actor* a_actor, RE::TESNPC* a_npc);
	}

	namespace Actor
	{
		void Install();
	}

	namespace Event
	{
		class Manager :
			public ISingleton<Manager>,
			public RE::BSTEventSink<RE::TESFormDeleteEvent>
		{
		public:
			static void Register();

		protected:
			RE::BSEventNotifyControl ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>*) override;
		};
	}

	void Setup();
	void DoInitialDistribution();
	void LogResults(std::uint32_t a_actorCount);
}
