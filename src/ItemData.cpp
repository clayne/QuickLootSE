
#include "ItemData.h"
#include "LootMenu.h"
#include "skse64/GameRTTI.h"
#include "skse64/GameForms.h"
#include "skse64/GameObjects.h"
#include "skse64/GameReferences.h"
#include "skse64/GameExtraData.h"

static ItemData::Type GetItemType(TESForm *form);

static const char * strIcons[] = {
	"none",					// 00
	"default_weapon",
	"weapon_sword",
	"weapon_greatsword",
	"weapon_daedra",
	"weapon_dagger",
	"weapon_waraxe",
	"weapon_battleaxe",
	"weapon_mace",
	"weapon_hammer",
	"weapon_staff",			// 10
	"weapon_bow",
	"weapon_arrow",
	"weapon_pickaxe",
	"weapon_woodaxe",
	"weapon_crossbow",
	"weapon_bolt",
	"default_armor",
	"lightarmor_body",
	"lightarmor_head",
	"lightarmor_hands",		// 20
	"lightarmor_forearms",
	"lightarmor_feet",
	"lightarmor_calves",
	"lightarmor_shield",
	"lightarmor_mask",
	"armor_body",
	"armor_head",
	"armor_hands",
	"armor_forearms",
	"armor_feet",			// 30
	"armor_calves",
	"armor_shield",
	"armor_mask",
	"armor_bracer",
	"armor_daedra",
	"clothing_body",
	"clothing_robe",
	"clothing_head",
	"clothing_pants",
	"clothing_hands",		// 40
	"clothing_forearms",
	"clothing_feet",
	"clothing_calves",
	"clothing_shoes",
	"clothing_shield",
	"clothing_mask",
	"armor_amulet",
	"armor_ring",
	"armor_circlet",
	"default_scroll",		// 50
	"default_book",
	"default_book_read",
	"book_tome",
	"book_tome_read",
	"book_journal",
	"book_note",
	"book_map",
	"default_food",
	"food_wine",
	"food_beer",			// 60
	"default_ingredient",
	"default_key",
	"key_house",
	"default_potion",
	"potion_health",
	"potion_stam",
	"potion_magic",
	"potion_poison",
	"potion_frost",
	"potion_fire",			// 70
	"potion_shock",
	"default_misc",
	"misc_artifact",
	"misc_clutter",
	"misc_lockpick",
	"misc_soulgem",
	"soulgem_empty",
	"soulgem_partial",
	"soulgem_full",
	"soulgem_grandempty",	// 80
	"soulgem_grandpartial",
	"soulgem_grandfull",
	"soulgem_azura",
	"misc_gem",
	"misc_ore",
	"misc_ingot",
	"misc_hide",
	"misc_strips",
	"misc_leather",
	"misc_wood",			// 90
	"misc_remains",
	"misc_trollskull",
	"misc_torch",
	"misc_goldsack",
	"misc_gold",
	"misc_dragonclaw"
};

ItemData::ItemData(InventoryEntryData *a_pEntry, TESForm *owner) : pEntry(a_pEntry), type(Type::kType_None), priority(0), name(),
isStolen(false), isEnchanted(false), isQuestItem(false)
{
	if (!pEntry || !pEntry->type)
		return;

	TESForm *form = pEntry->type;

	// set type
	type = GetItemType(form);


	// set name
	name = CALL_MEMBER_FN(pEntry, GenerateName)();


	// set isEnchanted
	if (form->IsArmor() || form->IsWeapon())
	{
		if (pEntry->extendDataList)
		{
			for (ExtendDataList::Iterator it = pEntry->extendDataList->Begin(); !it.End(); ++it)
			{
				BaseExtraList * pExtraDataList = it.Get();

				if (pExtraDataList)
				{
					ExtraEnchantment* extraEnchant = static_cast<ExtraEnchantment*>(pExtraDataList->GetByType(kExtraData_Enchantment));
					if (extraEnchant && extraEnchant->enchant)
					{
						isEnchanted = true;
						break;
					}
				}
			}
		}

		TESEnchantableForm *enchantForm = DYNAMIC_CAST(pEntry->type, TESForm, TESEnchantableForm);
		if (enchantForm && enchantForm->enchantment)
			isEnchanted = true;
	}

	// set isStolen
	TESForm *itemOwner = CALL_MEMBER_FN(pEntry, GetOwner)();
	if (!itemOwner)
		itemOwner = owner; 
	if (itemOwner)
		isStolen = !CALL_MEMBER_FN(pEntry, IsOwnedBy2)(*g_thePlayer, itemOwner, true);
	else
		isStolen = !CALL_MEMBER_FN(pEntry, IsOwnedBy)(*g_thePlayer, true);


	// set isQuestItem
	isQuestItem = CALL_MEMBER_FN(pEntry, IsQuestItem)();

	// set priority
	enum Priority
	{
		Key,
		Gold,
		LockPick,
		Ammo,
		SoulGem,
		Potion,
		Poison,
		EnchantedWeapon,
		EnchantedArmor,
		Gem,
		Amulet,
		Ring,
		Weapon,
		Armor,
		Other,
		Food = Other
	};

	switch (form->formType)
	{
	case FormType::kFormType_Ammo:
		priority = Ammo;
		break;
	case FormType::kFormType_SoulGem:
		priority = SoulGem;
		break;
	case FormType::kFormType_Potion:
	{
									   AlchemyItem *alchemyItem = static_cast<AlchemyItem *>(form);
									   if (alchemyItem->IsFood())
										   priority = Food;
									   else if (alchemyItem->IsPoison())
										   priority = Poison;
									   else
										   priority = Potion;
	}
		break;
	case FormType::kFormType_Weapon:
		priority = (IsEnchanted()) ? EnchantedWeapon : Weapon;
		break;
	case FormType::kFormType_Armor:
		if (IsEnchanted())
			priority = EnchantedArmor;
		else if (type == kType_ArmorAmulet)
			priority = Amulet;
		else if (type == kType_ArmorRing)
			priority = Ring;
		else
			priority = Armor;
		break;
	case FormType::kFormType_Key:
		priority = Key;
		break;
	case FormType::kFormType_Misc:
		switch (type)
		{
		case kType_MiscGold:
			priority = Gold;
			break;
		case kType_MiscLockPick:
			priority = LockPick;
			break;
		case kType_MiscGem:
			priority = Gem;
			break;
		default:
			priority = Other;
		}
		break;
	default:
		priority = Other;
	}
}


ItemData::ItemData(const ItemData &rhs) : pEntry(rhs.pEntry), type(rhs.type), priority(rhs.priority), name(rhs.name),
isStolen(rhs.isStolen), isEnchanted(rhs.isEnchanted), isQuestItem(rhs.isQuestItem)
{
}

ItemData::ItemData(ItemData &&rhs) : pEntry(rhs.pEntry), type(rhs.type), priority(rhs.priority), name(rhs.name),
isStolen(rhs.isStolen), isEnchanted(rhs.isEnchanted), isQuestItem(rhs.isQuestItem)
{
	rhs.pEntry = nullptr;
}


ItemData::~ItemData()
{
	if (pEntry)
	{
		pEntry->Delete();
	}	
}



const char * ItemData::GetName() const
{
	return CALL_MEMBER_FN(pEntry, GenerateName)();
}

UInt32 ItemData::GetCount() const
{
	return CALL_MEMBER_FN(pEntry, GetCount)();
}

SInt32 ItemData::GetValue() const
{
	return CALL_MEMBER_FN(pEntry, GetValue)();
}

float ItemData::GetWeight() const
{
	return GetFormWeight(pEntry->type);
}


UInt32 ItemData::GetPickpocketChance() const
{
	if (containerRef->baseForm->formType == kFormType_NPC)
	{
		if (!containerRef->IsDead(true) && ((*g_thePlayer)->actorState.flags04 & ActorState::kState_Sneaking) != 0)
		{
			Actor * targetActor = (Actor*)containerRef;

			SInt32 itemCount = 1;
			float itemWeight = GetFormWeight(pEntry->type);
			if (itemWeight <= 0)
				itemCount = pEntry->countDelta;

			typedef UInt32 UnkPickpocketFunc_t(Actor * targetActor, InventoryEntryData * entryData, UInt32 numItems, bool unk);
			RelocAddr<UnkPickpocketFunc_t*> UnkPickpocketFunc(0x001D9270);  // 1_5_50

			UInt32 unkResult = UnkPickpocketFunc(targetActor, pEntry, itemCount, true);

			typedef SInt32 GetDetectionLevel_t(Actor * target, Actor * player, UInt32 flag);
			RelocAddr<GetDetectionLevel_t*> GetDetectionLevel(0x005FCB90);  // 1_5_50

			bool isDetected = false;
			SInt32 detectionValue = GetDetectionLevel(targetActor, (*g_thePlayer), 3);
			if (detectionValue > 0)
				isDetected = true;

			typedef float GetPlayerPickpocketSkill_t(ActorValueOwner * actorValue, UInt32 actorValueID);
			RelocAddr<GetPlayerPickpocketSkill_t*> GetPlayerPickpocketSkill(0x003E5440);  // 1_5_50

			float playerSkill = GetPlayerPickpocketSkill(&(*g_thePlayer)->actorValueOwner, 13);
			float targetSkill = targetActor->actorValueOwner.GetCurrent(13);

			typedef UInt32 GetPickpocketChance_t(float playerSkill, float targetSkill, UInt32 resultUnkPickpocketFunc, float totalWeight, Actor * player, Actor * target, bool isDetected, TESForm * item);
			RelocAddr<GetPickpocketChance_t*> GetPickpocketChance(0x003BD130);  // 1_5_50

			UInt32 finalResult = GetPickpocketChance(playerSkill, targetSkill, unkResult, itemWeight, (*g_thePlayer), targetActor, isDetected, pEntry->type);
			if (finalResult > 100)
				finalResult = 100;

			return finalResult;
		}
		else
			return 0;
	}
	else
		return 0;
}


const char * ItemData::GetIcon() const
{
	return strIcons[type];
}

//===================================================================================
// compare
//===================================================================================

typedef int(*FnCompare)(const ItemData &a, const ItemData &b);

static int CompareByType(const ItemData &a, const ItemData &b)
{
	return a.priority - b.priority;
}

static int CompareByStolen(const ItemData &a, const ItemData &b)
{
	SInt32 valueA = a.IsStolen() ? 1 : 0;
	SInt32 valueB = b.IsStolen() ? 1 : 0;

	return valueA - valueB;
}

static int CompareByQuestItem(const ItemData &a, const ItemData &b)
{
	SInt32 valueA = a.IsQuestItem() ? 0 : 1;
	SInt32 valueB = b.IsQuestItem() ? 0 : 1;

	return valueA - valueB;
}

static int CompareByValue(const ItemData &a, const ItemData &b)
{
	SInt32 valueA = a.GetValue();
	SInt32 valueB = b.GetValue();
	return valueA - valueB;
}

static int CompareByCount(const ItemData &a, const ItemData &b)
{
	SInt32 valueA = a.GetCount();
	SInt32 valueB = b.GetCount();
	return valueA - valueB;
}

static int CompareByName(const ItemData &a, const ItemData &b)
{
	//return a.name.compare(b.name);
	return strcmp(a.name, b.name);
}

bool operator<(const ItemData &a, const ItemData &b)
{
	static FnCompare compares[] = {
		&CompareByQuestItem,
		&CompareByStolen,
		&CompareByType,
		&CompareByName,
		&CompareByValue,
		&CompareByCount
	};

	for (FnCompare compare : compares)
	{
		int cmp = compare(a, b);
		if (cmp == 0)
			continue;

		return cmp < 0;
	}

	return a.pEntry < b.pEntry;
}

//===================================================================================
// get type
//===================================================================================

static ItemData::Type GetItemTypeWeapon(TESObjectWEAP *weap)
{
	ItemData::Type type = ItemData::kType_DefaultWeapon;

	switch (weap->type())
	{
	case TESObjectWEAP::GameData::kType_OneHandSword:
		type = ItemData::kType_WeaponSword;
		break;
	case TESObjectWEAP::GameData::kType_OneHandDagger:
		type = ItemData::kType_WeaponDagger;
		break;
	case TESObjectWEAP::GameData::kType_OneHandAxe:
		type = ItemData::kType_WeaponWarAxe;
		break;
	case TESObjectWEAP::GameData::kType_OneHandMace:
		type = ItemData::kType_WeaponMace;
		break;
	case TESObjectWEAP::GameData::kType_TwoHandSword:
		type = ItemData::kType_WeaponGreatSword;
		break;
	case TESObjectWEAP::GameData::kType_TwoHandAxe:
	{
													  TESForm * kwdWarHammerForm = LookupFormByID(0x06D930);
													  static BGSKeyword *keywordWarHammer = (BGSKeyword*)kwdWarHammerForm;		// WeapTypeWarhammer
													  if (weap->keyword.HasKeyword(keywordWarHammer))
														  type = ItemData::kType_WeaponHammer;
													  else
														  type = ItemData::kType_WeaponBattleAxe;
													  break;
	}
	case TESObjectWEAP::GameData::kType_Bow:
		type = ItemData::kType_WeaponBow;
		break;
	case TESObjectWEAP::GameData::kType_Staff:
		type = ItemData::kType_WeaponStaff;
		break;
	case TESObjectWEAP::GameData::kType_CrossBow:
		type = ItemData::kType_WeaponCrossbow;
		break;
	}

	return type;
}


static ItemData::Type GetItemTypeArmor(TESObjectARMO *armor)
{
	static ItemData::Type types[] = {
		ItemData::kType_LightArmorBody,		// 0
		ItemData::kType_LightArmorHead,
		ItemData::kType_LightArmorHands,
		ItemData::kType_LightArmorForearms,
		ItemData::kType_LightArmorFeet,
		ItemData::kType_LightArmorCalves,
		ItemData::kType_LightArmorShield,
		ItemData::kType_LightArmorMask,

		ItemData::kType_ArmorBody,			// 8
		ItemData::kType_ArmorHead,
		ItemData::kType_ArmorHands,
		ItemData::kType_ArmorForearms,
		ItemData::kType_ArmorFeet,
		ItemData::kType_ArmorCalves,
		ItemData::kType_ArmorShield,
		ItemData::kType_ArmorMask,

		ItemData::kType_ClothingBody,		// 16
		ItemData::kType_ClothingHead,
		ItemData::kType_ClothingHands,
		ItemData::kType_ClothingForearms,
		ItemData::kType_ClothingFeet,
		ItemData::kType_ClothingCalves,
		ItemData::kType_ClothingShield,
		ItemData::kType_ClothingMask,

		ItemData::kType_ArmorAmulet,		// 24
		ItemData::kType_ArmorRing,
		ItemData::kType_Circlet,

		ItemData::kType_DefaultArmor		// 27
	};

	UInt32 index = 0;

	if (armor->bipedObject.IsLightArmor())
	{
		index = 0;
	}
	else if (armor->bipedObject.IsHeavyArmor())
	{
		index = 8;
	}
	else
	{
		static BGSKeyword *keywordJewelry = DYNAMIC_CAST(LookupFormByID(0x08F95A), TESForm, BGSKeyword);		// VendorItemJewelry
		static BGSKeyword *keywordClothing = DYNAMIC_CAST(LookupFormByID(0x08F95B), TESForm, BGSKeyword);	// VendorItemClothing

		if (armor->keyword.HasKeyword(keywordClothing))
		{
			index = 16;
		}
		else if (armor->keyword.HasKeyword(keywordJewelry))
		{
			if (armor->bipedObject.HasPartOf(BGSBipedObjectForm::kPart_Amulet))
				index = 24;
			else if (armor->bipedObject.HasPartOf(BGSBipedObjectForm::kPart_Ring))
				index = 25;
			else if (armor->bipedObject.HasPartOf(BGSBipedObjectForm::kPart_Circlet))
				index = 26;
			else
				index = 27;
		}
		else
		{
			index = 27;
		}
	}

	if (index >= 24)
		return types[index];

	if (armor->bipedObject.HasPartOf(BGSBipedObjectForm::kPart_Body | BGSBipedObjectForm::kPart_Unnamed10))
		index += 0;			// body
	else if (armor->bipedObject.HasPartOf(BGSBipedObjectForm::kPart_Head | BGSBipedObjectForm::kPart_Hair | BGSBipedObjectForm::kPart_LongHair))
	{
		index += 1;			// head
		if (armor->formID >= 0x061C8B && armor->formID < 0x061CD7)
			index += 6;		// mask
	}
	else if (armor->bipedObject.HasPartOf(BGSBipedObjectForm::kPart_Hands))
		index += 2;			// hands
	else if (armor->bipedObject.HasPartOf(BGSBipedObjectForm::kPart_Forearms))
		index += 3;			// forarms
	else if (armor->bipedObject.HasPartOf(BGSBipedObjectForm::kPart_Feet))
		index += 4;			// forarms
	else if (armor->bipedObject.HasPartOf(BGSBipedObjectForm::kPart_Calves))
		index += 5;			// calves
	else if (armor->bipedObject.HasPartOf(BGSBipedObjectForm::kPart_Shield))
		index += 6;			// shield
	else
		index = 27;

	return types[index];
}


static ItemData::Type GetItemTypePotion(AlchemyItem *potion)
{
	ItemData::Type type = ItemData::kType_DefaultPotion;

	if (potion->IsFood())
	{
		type = ItemData::kType_DefaultFood;

		const static UInt32 ITMPosionUse = 0x000B6435;
		if (potion->itemData.useSound && potion->itemData.useSound->formID == ITMPosionUse)
			type = ItemData::kType_FoodWine;
	}
	else if (potion->IsPoison())
	{
		type = ItemData::kType_PotionPoison;
	}
	else
	{
		type = ItemData::kType_DefaultPotion;

		MagicItem::EffectItem *pEffect = CALL_MEMBER_FN(potion, GetCostliestEffectItem)(5, false);
		if (pEffect && pEffect->mgef)
		{
			UInt32 primaryValue = pEffect->mgef->properties.primaryValue;
			switch (primaryValue)
			{
			case EffectSetting::Properties::kActorValue_Health:
				type = ItemData::kType_PotionHealth;
				break;
			case EffectSetting::Properties::kActorValue_Magicka:
				type = ItemData::kType_PotionMagic;
				break;
			case EffectSetting::Properties::kActorValue_Stamina:
				type = ItemData::kType_PotionStam;
				break;
			case EffectSetting::Properties::kActorValue_FireResist:
				type = ItemData::kType_PotionFire;
				break;
			case EffectSetting::Properties::kActorValue_ElectricResist:
				type = ItemData::kType_PotionShock;
				break;
			case EffectSetting::Properties::kActorValue_FrostResist:
				type = ItemData::kType_PotionFrost;
				break;
			}
		}
	}

	return type;
}

static ItemData::Type GetItemTypeMisc(TESObjectMISC *misc)
{
	ItemData::Type type = ItemData::kType_DefaultMisc;

	static const UInt32 LockPick = 0x00000A;
	static const UInt32 Gold = 0x00000F;
	static const UInt32 Leather01 = 0x000DB5D2;
	static const UInt32 LeatherStrips = 0x000800E4;

	static const UInt32 VendorItemAnimalHideFormId = 0x0914EA;
	static const UInt32 VendorItemDaedricArtifactFormId = 0x000917E8;
	static const UInt32 VendorItemGemFormId = 0x000914ED;
	static const UInt32 VendorItemToolFormId = 0x000914EE;
	static const UInt32 VendorItemAnimalPartFormId = 0x000914EB;
	static const UInt32 VendorItemOreIngotFormId = 0x000914EC;
	static const UInt32 VendorItemClutterFormId = 0x000914E9;
	static const UInt32 VendorItemFirewoodFormId = 0x000BECD7;

	BGSKeyword * VendorItemAnimalHide = DYNAMIC_CAST(LookupFormByID(VendorItemAnimalHideFormId), TESForm, BGSKeyword);
	BGSKeyword * VendorItemDaedricArtifact = DYNAMIC_CAST(LookupFormByID(VendorItemDaedricArtifactFormId), TESForm, BGSKeyword);
	BGSKeyword * VendorItemGem = DYNAMIC_CAST(LookupFormByID(VendorItemGemFormId), TESForm, BGSKeyword);
	BGSKeyword * VendorItemTool = DYNAMIC_CAST(LookupFormByID(VendorItemToolFormId), TESForm, BGSKeyword);
	BGSKeyword * VendorItemOreIngot = DYNAMIC_CAST(LookupFormByID(VendorItemOreIngotFormId), TESForm, BGSKeyword);
	BGSKeyword * VendorItemAnimalPart = DYNAMIC_CAST(LookupFormByID(VendorItemAnimalPartFormId), TESForm, BGSKeyword);
	BGSKeyword * VendorItemClutter = DYNAMIC_CAST(LookupFormByID(VendorItemClutterFormId), TESForm, BGSKeyword);
	BGSKeyword * VendorItemFirewood = DYNAMIC_CAST(LookupFormByID(VendorItemFirewoodFormId), TESForm, BGSKeyword);

	static const UInt32 RubyDragonClaw = 0x04B56C;
	static const UInt32 IvoryDragonClaw = 0x0AB7BB;
	static const UInt32 GlassCraw = 0x07C260;
	static const UInt32 EbonyCraw = 0x05AF48;
	static const UInt32 EmeraldDragonClaw = 0x0ED417;
	static const UInt32 DiamondClaw = 0x0AB375;
	static const UInt32 IronClaw = 0x08CDFA;
	static const UInt32 CoralDragonClaw = 0x0B634C;
	static const UInt32 E3GoldenClaw = 0x0999E7;
	static const UInt32 SapphireDragonClaw = 0x0663D7;
	static const UInt32 MS13GoldenClaw = 0x039647;

	if (misc->formID == LockPick)
		type = ItemData::kType_MiscLockPick;
	else if (misc->formID == Gold)
		type = ItemData::kType_MiscGold;
	else if (misc->formID == Leather01)
		type = ItemData::kType_MiscLeather;
	else if (misc->formID == LeatherStrips)
		type = ItemData::kType_MiscStrips;
	else if (misc->keyword.HasKeyword(VendorItemAnimalHide))
		type = ItemData::kType_MiscHide;
	else if (misc->keyword.HasKeyword(VendorItemDaedricArtifact))
		type = ItemData::kType_MiscArtifact;
	else if (misc->keyword.HasKeyword(VendorItemGem))
		type = ItemData::kType_MiscGem;
	else if (misc->keyword.HasKeyword(VendorItemAnimalPart))
		type = ItemData::kType_MiscRemains;
	else if (misc->keyword.HasKeyword(VendorItemOreIngot))
		type = ItemData::kType_MiscIngot;
	else if (misc->keyword.HasKeyword(VendorItemClutter))
		type = ItemData::kType_MiscClutter;
	else if (misc->keyword.HasKeyword(VendorItemFirewood))
		type = ItemData::kType_MiscWood;
	else if (misc->formID == RubyDragonClaw
		|| misc->formID == IvoryDragonClaw
		|| misc->formID == GlassCraw
		|| misc->formID == EbonyCraw
		|| misc->formID == EmeraldDragonClaw
		|| misc->formID == DiamondClaw
		|| misc->formID == IronClaw
		|| misc->formID == CoralDragonClaw
		|| misc->formID == E3GoldenClaw
		|| misc->formID == SapphireDragonClaw
		|| misc->formID == MS13GoldenClaw
		)
		type = ItemData::kType_MiscDragonClaw;

	return type;
}


static ItemData::Type GetItemTypeSoulGem(TESSoulGem *gem)
{
	ItemData::Type type = ItemData::kType_MiscSoulGem;

	const static UInt32 DA01SoulGemAzurasStar = 0x063B27;
	const static UInt32 DA01SoulGemBlackStar = 0x063B29;

	if (gem->formID == DA01SoulGemBlackStar || gem->formID == DA01SoulGemAzurasStar)
	{
		type = ItemData::kType_SoulGemAzura;
	}
	else
	{
		if (gem->gemSize < 4)
		{
			if (gem->soulSize == 0)
				type = ItemData::kType_SoulGemEmpty;
			else if (gem->soulSize >= gem->gemSize)
				type = ItemData::kType_SoulGemFull;
			else
				type = ItemData::kType_SoulGemPartial;
		}
		else
		{
			if (gem->soulSize == 0)
				type = ItemData::kType_SoulGemGrandEmpty;
			else if (gem->soulSize >= gem->gemSize)
				type = ItemData::kType_SoulGemGrandFull;
			else
				type = ItemData::kType_SoulGemGrandPartial;
		}
	}

	return type;
}


const ItemData::Type GetItemTypeBook(TESObjectBOOK *book)
{
	ItemData::Type type = ItemData::kType_DefaultBook;

	const static UInt32 VendorItemRecipeFormID = 0x000F5CB0;
	const static UInt32 VendorItemSpellTomeFormID = 0x000937A5;
	BGSKeyword * VendorItemRecipe = DYNAMIC_CAST(LookupFormByID(VendorItemRecipeFormID), TESForm, BGSKeyword);
	BGSKeyword * VendorItemSpellTome = DYNAMIC_CAST(LookupFormByID(VendorItemSpellTomeFormID), TESForm, BGSKeyword);

	if (book->data.type == 0xFF || book->keyword.HasKeyword(VendorItemRecipe))
	{
		type = ItemData::kType_BookNote;
	}
	else if (book->keyword.HasKeyword(VendorItemSpellTome))
	{
		type = ItemData::kType_BookTome;
	}

	return type;
}

static ItemData::Type GetItemType(TESForm *form)
{
	ItemData::Type type = ItemData::kType_None;

	switch (form->formType)
	{
	case FormType::kFormType_ScrollItem:
		type = ItemData::kType_DefaultScroll;
		break;
	case FormType::kFormType_Armor:
		type = GetItemTypeArmor(static_cast<TESObjectARMO*>(form));
		break;
	case FormType::kFormType_Book:
		type = GetItemTypeBook(static_cast<TESObjectBOOK*>(form));
		break;
	case FormType::kFormType_Ingredient:
		type = ItemData::kType_DefaultIngredient;
		break;
	case FormType::kFormType_Light:
		type = ItemData::kType_MiscTorch;
		break;
	case FormType::kFormType_Misc:
		type = GetItemTypeMisc(static_cast<TESObjectMISC*>(form));
		break;
	case FormType::kFormType_Weapon:
		type = GetItemTypeWeapon(static_cast<TESObjectWEAP*>(form));
		break;
	case FormType::kFormType_Ammo:
		type = (static_cast<TESAmmo*>(form)->isBolt()) ? ItemData::kType_WeaponBolt : ItemData::kType_WeaponArrow;
		break;
	case FormType::kFormType_Key:
		type = ItemData::kType_DefaultKey;
		break;
	case FormType::kFormType_Potion:
		type = GetItemTypePotion(static_cast<AlchemyItem*>(form));
		break;
	case FormType::kFormType_SoulGem:
		type = GetItemTypeSoulGem(static_cast<TESSoulGem*>(form));
		break;
	}

	return type;
}
