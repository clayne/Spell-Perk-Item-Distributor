#pragma once

// Record = FormOrEditorID|StringFilters|RawFormFilters|LevelFilters|Traits|IdxOrCount|Chance

using FormModPair = std::pair<
	std::optional<RE::FormID>,    // formID
	std::optional<std::string>>;  // modName

using FormOrEditorID = std::variant<
	FormModPair,   // formID~modName
	std::string>;  // editorID

template <class T>
struct Filters
{
	std::vector<T> ALL{};
	std::vector<T> NOT{};
	std::vector<T> MATCH{};
};

using StringVec = std::vector<std::string>;
struct StringFilters : Filters<std::string>
{
	StringVec ANY{};
};

using RawFormVec = std::vector<FormOrEditorID>;
using RawFormFilters = Filters<FormOrEditorID>;

using FormOrMod = std::variant<RE::TESForm*,  // form
	const RE::TESFile*>;                      // mod
using FormVec = std::vector<FormOrMod>;
using FormFilters = Filters<FormOrMod>;

template <typename T>
struct Range
{
	Range() = default;

	explicit Range(T a_min) :
		min(a_min)
	{}
	Range(T a_min, T a_max) :
		min(a_min),
		max(a_max)
	{}

	[[nodiscard]] bool IsValid() const
	{
		return min > std::numeric_limits<T>::min();  // min must always be valid, max is optional
	}
	[[nodiscard]] bool IsInRange(T value) const
	{
		return value >= min && value <= max;
	}

	[[nodiscard]] bool IsExact() const
	{
		return min == max;
	}

	[[nodiscard]] T GetRandom() const
	{
		return IsExact() ? min : RNG().generate<T>(min, max);
	}

	// members
	T min{ std::numeric_limits<T>::min() };
	T max{ std::numeric_limits<T>::max() };
};

// skill type, skill Level
struct SkillLevel
{
	std::uint32_t       type;
	Range<std::uint8_t> range;
};

struct LevelFilters
{
	Range<std::uint16_t>    actorLevel{};
	std::vector<SkillLevel> skillLevels{};   // skill levels
	std::vector<SkillLevel> skillWeights{};  // skill weights (from Class)
};

struct Traits
{
	std::optional<RE::SEX> sex{};
	std::optional<bool>    unique{};
	std::optional<bool>    summonable{};
	std::optional<bool>    child{};
	std::optional<bool>    leveled{};
	std::optional<bool>    teammate{};
	std::optional<bool>    startsDead{};
};

using Path = std::string;

using Index = std::int32_t;
using Count = std::int32_t;
using RandomCount = Range<Count>;
using IndexOrCount = std::variant<Index, RandomCount>;

/// <summary>
/// A chance that is represented as a decimal value between 0 and 1.
/// For example, 0.5 would be 50%.
///
/// This one is used in a processed Data for filtering.
/// </summary>
using DecimalChance = double;

/// <summary>
/// A chance that is represented as a percent value between 0 and 100.
/// It also can be decimal, but would describe fraction of a percent.
/// So that 0.5 would be 0.5%.
///
/// This is used during parsing of INI files.
/// </summary>
using PercentChance = double;

/// A standardized way of converting any object to string.
///
///	<p>
///	Overload `operator<<` to provide custom formatting for your value.
///	Alternatively, specialize this method and provide your own implementation.
///	</p>
template <typename Value>
std::string describe(Value value)
{
	std::ostringstream os;
	os << value;
	return os.str();
}

inline std::ostream& operator<<(std::ostream& os, RE::TESFile* file)
{
	os << file->fileName;
	return os;
}

inline std::ostream& operator<<(std::ostream& os, RE::TESForm* form)
{
	if (const auto& edid = editorID::get_editorID(form); !edid.empty()) {
		os << edid << " ";
	}
	os << "["
	   << std::to_string(form->GetFormType())
	   << ":"
	   << std::setfill('0')
	   << std::setw(sizeof(RE::FormID) * 2)
	   << std::uppercase
	   << std::hex
	   << form->GetFormID()
	   << "]";

	return os;
}

namespace fmt
{
	// Produces formatted strings for all Forms like this:
	// DefaultAshPile2 "Ash Pile" [ACTI:00000022]
	// defaultBlankTrigger [ACTI:00000F55]
	template <typename Form>
	struct formatter<Form, std::enable_if_t<std::is_base_of<RE::TESForm, Form>::value, char>> : fmt::formatter<std::string>
	{
		template <class ParseContext>
		constexpr auto parse(ParseContext& a_ctx)
		{
			return a_ctx.begin();
		}

		template <class FormatContext>
		constexpr auto format(const Form& form, FormatContext& a_ctx) const
		{
			const auto name = std::string(form.GetName());
			const auto edid = editorID::get_editorID(&form);
			if (name.empty() && edid.empty()) {
				return fmt::format_to(a_ctx.out(), "[{}:{:08X}]", form.GetFormType(), form.formID);
			} else if (name.empty()) {
				return fmt::format_to(a_ctx.out(), "{} [{}:{:08X}]", edid, form.GetFormType(), form.formID);
			} else if (edid.empty()) {
				return fmt::format_to(a_ctx.out(), "{:?} [{}:{:08X}]", name, form.GetFormType(), form.formID);
			}
			return fmt::format_to(a_ctx.out(), "{} {:?} [{}:{:08X}]", edid, name, form.GetFormType(), form.formID);
		}
	};

	// Does the same as generic formatter, but also includes a Base NPC info.
	template <>
	struct formatter<RE::Actor>
	{
		template <class ParseContext>
		constexpr auto parse(ParseContext& a_ctx)
		{
			return a_ctx.begin();
		}

		template <class FormatContext>
		constexpr auto format(const RE::Actor& actor, FormatContext& a_ctx) const
		{
			const auto& form = static_cast<const RE::TESForm&>(actor);
			if (auto npc = actor.GetActorBase(); npc) {
				return fmt::format_to(a_ctx.out(), "{} (Base: {})", form, *npc);
			}
			return fmt::format_to(a_ctx.out(), "{}", form);
		}
	};

}
