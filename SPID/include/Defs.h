#pragma once

namespace RECORD
{
	enum TYPE
	{
		kSpell = 0,
		kPerk,
		kItem,
		kShout,
		kLevSpell,
		kPackage,
		kOutfit,
		kKeyword,
		kDeathItem,
		kFaction,
		kSleepOutfit,
		kSkin,

		kTotal
	};

	inline constexpr std::array add{ "Spell"sv, "Perk"sv, "Item"sv, "Shout"sv, "LevSpell"sv, "Package"sv, "Outfit"sv, "Keyword"sv, "DeathItem"sv, "Faction"sv, "SleepOutfit"sv, "Skin"sv };
	inline constexpr std::array remove{ "-Spell"sv, "-Perk"sv, "-Item"sv, "-Shout"sv, "-LevSpell"sv, "-Package"sv, "-Outfit"sv, "-Keyword"sv, "-DeathItem"sv, "-Faction"sv, "-SleepOutfit"sv, "-Skin"sv };
}

namespace TRAITS
{
	enum : std::uint32_t
	{
		kSex,
		kUnique,
		kSummonable,
		kChild
	};
}

namespace INI
{
	enum TYPE : std::uint32_t
	{
		kFormIDPair = 0,
		kFormID = kFormIDPair,
		kStrings,
		kESP = kStrings,
		kFilterIDs,
		kLevel,
		kTraits,
		kIdxOrCount,
		kChance
	};

	using Values = std::vector<std::pair<std::string, std::string>>;

	inline bool is_mod_name(const std::string& a_str)
	{
		return a_str.rfind(".esp") != std::string::npos || a_str.rfind(".esl") != std::string::npos || a_str.rfind(".esm") != std::string::npos;
	}
}

namespace DATA
{
	enum TYPE : std::uint32_t
	{
		kForm = 0,
		kIdxOrCount,
		kStrings = kIdxOrCount,
		kFilterForms,
		kLevel,
		kTraits,
		kChance,
		kNPCCount
	};
}

/// Trait that is used to infer default sorter for Forms.
template <class TForm>
struct form_sorter
{
	using Sorter = std::less<TForm*>;
};

/// Custom ordering for keywords that ensures that dependent keywords are disitrbuted after the keywords that they depend on.
struct KeywordDependencySorter
{
	static bool sort(RE::BGSKeyword* a, RE::BGSKeyword* b);
};

template <>
struct form_sorter<RE::BGSKeyword>
{
	using Sorter = KeywordDependencySorter;
};

using EventResult = RE::BSEventNotifyControl;

using FormIDPair = std::pair<std::optional<RE::FormID>,std::optional<std::string>>;
using FormOrEditorID = std::variant<FormIDPair,std::string>;
using FormIDVec = std::vector<FormOrEditorID>;
using StringVec = std::vector<std::string>;
using FormVec = std::vector<std::variant<RE::TESForm*, const RE::TESFile*>>;

using ActorLevel = std::pair<std::uint16_t, std::uint16_t>;
using SkillLevel = std::pair<
	std::uint32_t,
	std::pair<std::uint8_t, std::uint8_t>>;
using IdxOrCount = std::int32_t;
using Traits = std::tuple<
	std::optional<RE::SEX>,
	std::optional<bool>,
	std::optional<bool>,
	std::optional<bool>>;
using Chance = float;
using NPCCount = std::uint32_t;

using INIData = std::tuple<
	FormOrEditorID,
	std::array<StringVec, 4>,
	std::array<FormIDVec, 3>,
	std::pair<ActorLevel, std::vector<SkillLevel>>,
	Traits,
	IdxOrCount,
	Chance,
	std::string>;
using INIDataVec = std::vector<INIData>;

using StringFilters = std::array<StringVec, 4>;
using FormFilters = std::array<FormVec, 3>;
using LevelFilters = std::pair<ActorLevel, std::vector<SkillLevel>>;

template <class Form>
struct FormData
{
	Form* form{ nullptr };
	IdxOrCount idxOrCount{ 1 };
	StringFilters stringFilters{};
	FormFilters formFilters{};
	LevelFilters levelFilters{};
	Traits traits{};
	Chance chance{ 100 };
	NPCCount npcCount{ 0 };

	bool operator<(const FormData& a_rhs) const
	{
		if constexpr (std::is_same_v<RE::BGSKeyword, Form>) {
			return KeywordDependencySorter::sort(form, a_rhs.form);
		} else {
			return true;
		}
	}
};

template <class Form>
using FormDataVec = std::vector<FormData<Form>>;
