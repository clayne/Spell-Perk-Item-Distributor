#include "LookupForms.h"
#include "KeywordDependencies.h"

bool Lookup::GetForms()
{
	using namespace Forms;

	if (const auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
		const auto lookup_forms = [&]<class Form>(const RECORD::TYPE a_recordType, Distributables<Form>& a_map) {
			const auto& recordName = RECORD::add[a_recordType];

			a_map.LookupForms(dataHandler, recordName, INI::configs[recordName]);
			if constexpr (std::is_same_v<RE::BGSKeyword, Form>) {
				Dependencies::ResolveKeywords();
			}
			a_map.FinishLookupForms();
		};

		lookup_forms(RECORD::kKeyword, keywords);
		lookup_forms(RECORD::kSpell, spells);
		lookup_forms(RECORD::kPerk, perks);
		lookup_forms(RECORD::kItem, items);
		lookup_forms(RECORD::kShout, shouts);
		lookup_forms(RECORD::kLevSpell, levSpells);
		lookup_forms(RECORD::kPackage, packages);
		lookup_forms(RECORD::kOutfit, outfits);
		lookup_forms(RECORD::kDeathItem, deathItems);
		lookup_forms(RECORD::kFaction, factions);
		lookup_forms(RECORD::kSleepOutfit, sleepOutfits);
		lookup_forms(RECORD::kSkin, skins);
	}

	return spells || perks || items || shouts || levSpells || packages || outfits || keywords || deathItems || factions || sleepOutfits || skins;
}

void Lookup::LogFormLookup()
{
	using namespace Forms;

	logger::info("{:*^50}", "PROCESSING");

	const auto list_lookup_result = [&]<class Form>(const RECORD::TYPE a_recordType, Distributables<Form>& a_map) {
		const auto& recordName = RECORD::add[a_recordType];

		const auto all = INI::configs[recordName].size();
		const auto added = a_map.forms.size();

		// Only log entries that are actually present in INIs.
		if (all > 0) {
			logger::info("\tAdding {}/{} {}s", added, all, recordName);
		}
	};

	list_lookup_result(RECORD::kKeyword, keywords);
	list_lookup_result(RECORD::kSpell, spells);
	list_lookup_result(RECORD::kPerk, perks);
	list_lookup_result(RECORD::kItem, items);
	list_lookup_result(RECORD::kShout, shouts);
	list_lookup_result(RECORD::kLevSpell, levSpells);
	list_lookup_result(RECORD::kPackage, packages);
	list_lookup_result(RECORD::kOutfit, outfits);
	list_lookup_result(RECORD::kDeathItem, deathItems);
	list_lookup_result(RECORD::kFaction, factions);
	list_lookup_result(RECORD::kSleepOutfit, sleepOutfits);
	list_lookup_result(RECORD::kSkin, skins);

	// Clear logger's buffer to free some memory :)
	buffered_logger::clear();
}

std::size_t Forms::GetTotalEntries()
{
	return keywords.GetSize() +
	       spells.GetSize() +
	       perks.GetSize() +
	       items.GetSize() +
	       shouts.GetSize() +
	       levSpells.GetSize() +
	       packages.GetSize() +
	       outfits.GetSize() +
	       deathItems.GetSize() +
	       factions.GetSize() +
	       sleepOutfits.GetSize() +
	       skins.GetSize();
}

std::size_t Forms::GetTotalLeveledEntries()
{
	return keywords.GetLeveledSize() +
	       spells.GetLeveledSize() +
	       perks.GetLeveledSize() +
	       items.GetLeveledSize() +
	       shouts.GetLeveledSize() +
	       levSpells.GetLeveledSize() +
	       packages.GetLeveledSize() +
	       outfits.GetLeveledSize() +
	       deathItems.GetLeveledSize() +
	       factions.GetLeveledSize() +
	       sleepOutfits.GetLeveledSize() +
	       skins.GetLeveledSize();
}
