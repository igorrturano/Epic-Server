// ============================================================================
// epicNPCInitialization.inc - cold-init functions for NPC spawn
// ============================================================================
// Extracted from epicai.inc to keep init code out of brain.src compile unit.
// Consumed only by pkg/mobiles/epicAI/scripts/initialize.src.
// All functions here run at NPC spawn (and only at spawn) - never per-tick.
// ============================================================================

use cfgfile;
use uo;
use os;

// Init code calls per-tick utilities defined in epicai.inc (e.g., ParseTemplateName).
// Re-include epicai.inc here so init scripts have access to those utilities.
include ":epicAI:include/epicai";

// Direct deps for init bodies (formerly leaky-transitive via lootParser/etc.):
include "makeRace";
include "makeReligion";
include ":merchants:include/item_template";
include ":epicAI:include/humanoidBrain";   // GetHabsByLevel, GetClassProgression, ParseAbilityMeta
include ":epicAI:include/npcDataCache";    // NPC_GetCachedData
include ":wrappers:Objects/Model/MobileWrapper"; // MobileWrapper.HasAbility
include ":commands:create";                // CreateItem*Container, GetItemInfo, GetItemDescriptor
include ":crafting:include/fabricacaoComplexos"; // GeraDescriptorComplexo (tiered items)
include "include/epicMageryUtils";         // ApplyElementalWeaponProperties (swordmage loadout)


// Color helpers used only by boss-init paths (ApplyBossColors / GetColorValue)
function GetRandomColorFromRanges(color_ranges)
    if (!color_ranges || color_ranges.size() == 0)
        return 0; // Default color
    endif
    var selected_range := color_ranges[RandomInt(color_ranges.size()) + 1];
    return GetColorValue(selected_range);
endfunction

function RandomColorFromRanges(colorRanges)
    return colorRanges[RandomInt(colorRanges.size()) + 1];
endfunction


function InitializeEpicNPC(mobile)
    var npc_already_initiliazed := GetObjProperty(mobile, "started");
    if (npc_already_initiliazed)
        return 0;
    endif

    // Build NPC cache FIRST - all subsequent calls will use O(1) lookups
    var cache := NPC_GetCachedData(mobile);
    var npc_elem := NPC_GetNPCConfig(mobile);

    var npccfgfile := ReadConfigFile( ":epicai:npcdesc" );
    var mytemplate := cache.template;  // Use cached template name
    var elem := FindConfigElem(npccfgfile, mytemplate);
    var sounds := GetConfigStringArray(elem, "IdleSound");

    // Use cached category_lower for darkness check
    var isDarknessCreature := GetObjProperty(mobile, "Mobnoturno") || cache.environment_lower == "undead";
    if ((isDarknessCreature) && !InDarkness(mobile))
        ApplyRawDamage(mobile, CInt(AP_GetVitalMaximumValue(mobile, "Hits")));
    endif

    setupAnchorNPC(mobile);
    setupChampspawnNPC(mobile);
    setupNPCBackPack(mobile);
    // GenerateRecipeLoot(mobile);  // Temporarily stubbed — :tn:recipes dropped to free brain data segment (2026-04-18). Restore via start_script(":tn:generate_recipe_loot", array{mobile}) if needed.
    SetupAmmo(mobile, npccfgfile, mytemplate);
    var loot := npccfgfile[mytemplate].Loot;
    if (loot)
        start_script(":epicAI:scripts/loot_init", array{mobile.backpack, loot, mobile});
    endif

    var boss_struct := GetObjProperty(mobile, "EpicBossStruct");
    if (boss_struct)
        HandleInitializationLoot(mobile, boss_struct);
    endif

    var throwItem := CInt(npc_elem.throwItem);
    if (throwItem != error)
        SetObjProperty(mobile, "throwItem", throwItem);
    endif

    var spawnScale := CInt(GetConfigString(npc_elem, "SpawnScale"));
    if (spawnScale > 0 && spawnScale != 100)
        var rawScale := spawnScale * 10;
        SetObjProperty(mobile, "entityScale", struct{ height := rawScale, width := rawScale });
    endif

    // Use cached brain_type
    var brain_type := cache.brain_type;
    // Type-specific initialization
    case (brain_type)
        "monster":  InitializeMonsterNPC(mobile, npc_elem, elem, sounds);
        "animal":   InitializeAnimalNPC(mobile, npc_elem, elem, sounds);
        "humanoid": InitializeHumanoidNPC(mobile, npc_elem, elem, sounds);
        default:    InitializeGenericNPC(mobile, npc_elem, elem, sounds);
    endcase

    SetObjProperty(mobile, "started", 1);

    // Se o NPC tem SpawnScale, o 0x78 inicial foi enviado antes do init
    // (antes do entityScale estar setado), então forçamos um re-render
    // teleportando para fora do range e voltando.
    if (spawnScale > 0 && spawnScale != 100)
        var ox := mobile.x;
        var oy := mobile.y;
        var oz := mobile.z;
        MoveObjectToLocation(mobile, 0, 0, 0, mobile.realm, MOVEOBJECT_FORCELOCATION);
        sleepms(100);
        MoveObjectToLocation(mobile, ox, oy, oz, mobile.realm, MOVEOBJECT_FORCELOCATION);
    endif
endfunction


function InitializeMonsterNPC(mobile, npc_elem, elem, sounds)
    var has_colors := npc_elem.colors;
    if (has_colors)
        setupNPCColor(mobile, has_colors);
    endif
    SetupNPCEquip(mobile, npc_elem);

    // Monstros falam uma língua específica
    var lang_str := npc_elem.Languages;
    var known_languages;
    if (lang_str)
        known_languages := SplitWords(lang_str);
    else
        known_languages := {"Comum"};
    endif
    var chosen_language := known_languages[RandomInt(Len(known_languages)) + 1];

    SetObjProperty(mobile, "linguas_conhecidas", known_languages);
    SetObjProperty(mobile, "lingua_escolhida", chosen_language);

    // Add any other monster-specific setup here
endfunction


function InitializeAnimalNPC(mobile, npc_elem, elem, sounds)
    var has_colors := npc_elem.colors;
    if (has_colors)
        setupNPCColor(mobile, has_colors);
    endif

    SetupAnimalGender(mobile, npc_elem);
    SetupAnimalCriticalHit(mobile, npc_elem);

    // Animal-specific initialization
endfunction


function InitializeHumanoidNPC(mobile, npc_elem, elem, sounds)
    // Clones already have appearance copied from their creator
    if (GetObjProperty(mobile, "MirrorImage"))
        return;
    endif

    var has_colors := npc_elem.colors;
    var isBoss := GetObjProperty(mobile, "EpicBossStruct");
    var race := npc_elem.epicRace;

    if (has_colors)
        setupNPCColor(mobile, has_colors);
    endif

    var cache := NPC_GetCachedData(mobile);
    if (cache.tier >= 2)
        SetupHumanoidHabs(mobile);
    endif
    SetupItemsForSpecificClasses(mobile, npc_elem);

    setupNPCGender(mobile, Cint(isBoss.gender));

    // makeRace BEFORE equip - equip config may override hair/beard from race
    if (isBoss)
        race := isBoss.race;
        makeRace(mobile, race);
    elseif (race)
        makeRace(mobile, race);
    else
        makeRace(mobile);
    endif

    // Religion — picks from habs-specific pool (cleric/arqueiro arcano).
    // No-op for NPCs without religion-gated abilities.
    makeReligion(mobile);

    SetupNPCEquip(mobile, npc_elem);

    SetupBaseLanguages(mobile);

    if (cache.tier >= 2 && mobile.isSage())
        SetupWizardClass(mobile, npc_elem);
    endif

    SetupNPCAppearence(mobile);
endfunction


function SetupItemsForSpecificClasses(mobile, npc_elem)
    var habs := GetObjProperty(mobile, "myhabs");
    if (!habs)
        return 0;
    endif

    // Setup backpack if needed
    if (!mobile.backpack)
        var backpack := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, "BackPack", 1);
        EquipItem(mobile, backpack);
    endif

    case (npc_elem.Classe)
        GUERREIRO:
            if (ABILITY_ID::MARTIAL_ARTIST in habs)
                var monk_weapons := array{0x0df0, 0x13f9, 0x0e8a, 0x0e81, 0x1F020};
                EquipRandomFromArray(mobile, monk_weapons);
            endif

            if (ABILITY_ID::TIRO_PODEROSO in habs)
                SetupArcherLoadout(mobile);
            endif

            if (ABILITY_ID::CAMUFLAGEM in habs)
                var tier := 0;
                var boss_struct := GetObjProperty(mobile, "EpicBossStruct");
                if (boss_struct && boss_struct.tier)
                    tier := boss_struct.tier;
                endif
                if (!tier)
                    tier := CInt(npc_elem.Tier);
                endif

                // var template := GetAnimalTemplateByTier(tier);
                // if (template)
                //     SetObjProperty(mobile, "#camuflagem_template", template);
                //     var pet := CreateNpcFromTemplate(":epicai:"+template, mobile.x, mobile.y, mobile.z, 0, mobile.realm);
                //     sleepms(250);  // This was the key - giving the NPC time to initialize
                //     pet.alignment := 1;
                //     SetObjProperty(pet, "owner", mobile.serial);
                //     GrantPrivilege(pet, "hiddenattack");
                //     pet.enable("hiddenattack");
                //     GrantPrivilege(mobile, "hiddenattack");
                //     mobile.enable("hiddenattack");
                //     mobile.stealthsteps := 30;
                //     pet.stealthsteps := 30;
                // endif
                var net := CreateItemInBackpack(mobile, "net", RandomInt(2)+1);
            endif

            if ("Nazg khalug" in habs)
                makeRace(mobile, ORC);
            endif

            // Defensor maneuvers (301-304) all gate on the "Postura" prop, which
            // players normally set via .postura. Pick a stance at spawn so
            // MANOBRA_DE_DEFESA / DESAFIADORA / RESILIENCIA / INVENCIBILIDADE
            // pass CanActivate. PROVOCAR (300) is the defensor base ability.
            if (ABILITY_ID::PROVOCAR in habs)
                var posturas := array{"Ofensiva", "Defensiva"};
                SetObjProperty(mobile, "Postura", posturas[RandomInt(posturas.size()) + 1]);
            endif
        LADINO:
            if ("Mestre dos Venenos" in habs)
                var bleed_weapons := array{0x1401, 0x13f6, 0x0ec2, 0x26C5, 11565, 0x27af, 0x27ab};
                EquipRandomFromArray(mobile, bleed_weapons);
            endif

            if ("Arsenal Sombrio" in habs)
                var assassin_weapons := array{0x1401, 0x1440, 0x0f51, 0x0f63, 0x143c, 0x13B2};
                EquipRandomFromArray(mobile, assassin_weapons);
            endif

            if ("Sensibilidade Arcana" in habs)
                SetupArcaneTricksterLoadout(mobile, npc_elem);
            endif

        SABIO:
            if ("Alquimista Experiente" in habs)
                SetupAlchemistLoadout(mobile);
            endif

            if (ABILITY_ID::EXPLOSIVE_FIST in habs)
                makeRace(mobile, ANAO);
                SetupEngineerLoadout(mobile);
            endif

            if (ABILITY_ID::GOLEM_ASSEMBLY in habs)
                makeRace(mobile, ANAO);
                // Store mobile's current position before hiding it
                SetObjProperty(mobile, "#original_pos", struct{
                    "x" := mobile.x,
                    "y" := mobile.y,
                    "z" := mobile.z,
                    "realm" := mobile.realm
                });
                
                // Create the golem slightly offset
                var golem := CreateNpcFromTemplate(":epicai:PowerArmor", mobile.x+1, mobile.y+1, mobile.z, 0, mobile.realm);

                SetObjProperty(golem, "golem_maker", mobile.serial);
                SetObjProperty(mobile, "pilotando", 1);

                // Make the mobile anchor in the golem
                if (golem)
                    SetObjProperty(mobile, "anchor", array{ 4230 , 25 , 1 });
                    // Hide the original NPC
                    MoveObjectToLocation(mobile, 4230, 25, 1, mobile.realm, MOVEOBJECT_FORCELOCATION);
                endif
            endif

            if (ABILITY_ID::VINCULO_ELEMENTAL in habs)
                makeRace(mobile, ELFO);
                SetupSwordmageLoadout(mobile);
            endif

            if ("Visao verdadeira" in habs)
                mobile.gender := 1;
                makeRace(mobile, DROW);
                EquipRandomFromArray(mobile, array{0x166E});
            endif

            if ("Campeao das Montanhas" in habs)
                makeRace(mobile, ANAO);
                EquipRandomFromArray(mobile, array{0xFA11});
            endif

            if (ABILITY_ID::BLOODTHIRST in habs)
                makeRace(mobile, ORC);
                var two_handed := array{0x1401, 0x1440, 0x0f51, 0x0f63, 0x143c, 0x13B2};
                EquipRandomFromArray(mobile, two_handed);
            endif

            if (ABILITY_ID::ARCANE_BOW  in habs)
                makeRace(mobile, ELFO);
            endif

            if (ABILITY_ID::ELO_NATURAL in habs)
                makeRace(mobile, ELFO);
            endif

            if (ABILITY_ID::FORMA_ADAPTAVEL in habs)
                var two_handed := array{0x1401, 0x1440, 0x0f51, 0x0f63, 0x143c, 0x13B2};
                EquipRandomFromArray(mobile, two_handed);
            endif

    endcase

    return 1;
endfunction


// Steam equipment loadout for Engineer-class NPCs (base hab: EXPLOSIVE_FIST).
// Equips the full steam armor set so the NPC passes CanActivate on engenheiro abilities:
//   - Luvas de Vapor (0xFFC1)   -> EXPLOSIVE_FIST
//   - Grevas de Vapor (0xFFC2)  -> STEAM_JUMP
//   - Bracos a Vapor (0xFFC3)   -> TACTICAL_SHOULDER (with random InstalledGadget)
//   - Capacete a Vapor (0xFFC4) -> TARGETING_VISOR
// Plus ammo/fuel in backpack so abilities have resources to consume.
//
// Uses the create.src flow (GeraDescriptorComplexo + CreateItem*InContainer) so armor
// pieces are proper tier-1 craftable items — usable and statted correctly when looted
// by players. Ammo/fuel spawn raw (no tier variants).
function SetupEngineerLoadout(mobile)
    _NpcEquipItem(mobile, 0xFFC1);                   // Luvas de Vapor
    _NpcEquipItem(mobile, 0xFFC2);                   // Grevas de Vapor
    var steam_arms := _NpcEquipItem(mobile, 0xFFC3); // Bracos a Vapor
    _NpcEquipItem(mobile, 0xFFC4);                   // Capacete a Vapor

    if (steam_arms)
        var gadgets := array{"handcannon", "harpoon", "powdertrap"};
        SetObjProperty(steam_arms, "InstalledGadget", gadgets[RandomInt(gadgets.size()) + 1]);
    endif

    // Ammo / fuel — stackable, no tier
    CreateItemStacksInContainer(mobile, 0x9092, 30, mobile.backpack);  // Gunpowder
    CreateItemStacksInContainer(mobile, 0x4224, 5, mobile.backpack);   // Cannonballs
    CreateItemStacksInContainer(mobile, 0x2808, 3, mobile.backpack);   // Powder traps
    CreateItemStacksInContainer(mobile, 0x1bde, 10, mobile.backpack);  // Coal (rocket jump fuel)
endfunction

// Ranged loadout for Arqueiro subclass (base hab: TIRO_PODEROSO).
// Equips a random bow so CanActivate on all arqueiro abilities passes:
// PowerShot/ChargedShot/FindOpening/MultiShot all require weapon.GetAttribute() == RANGED.
// Quiver with arrows populates the ammo pool.
function SetupArcherLoadout(mobile)
    var bows := array{0x13B2, 0x13B1, 0x0F4F, 0x0F50, 0x13FC, 0x13FD};
    EquipRandomFromArray(mobile, bows);

    var quiver := _NpcEquipItem(mobile, 0x2FB7);
    if (quiver)
        CreateItemInContainer(quiver, 0xF3F, RandomIntMinMax(20, 80));
    endif
endfunction


// Spiritual blade + elemental enchantment for Lamina Arcana (swordmage) NPCs.
// Unblocks Fluency/ArcaneArmor/PrismaticVortex, which all read who.weapon.#elementalweapon
// and silently fail when the NPC has a plain weapon.
//
// Spiritual blade = item 0x090C (swordmage weapon class) with a random weapon graphic
// for visual variety. Cooldown "spiritual_weapon" matches the player flow from
// SummonSpiritualBlade, so the NPC won't re-summon mid-combat.
// ApplyElementalWeaponProperties reads mobile.weapon and sets #elementalweapon with
// type + charges + decay (20min) — matches the player enchant flow from espada*.src.
function SetupSwordmageLoadout(mobile)
    var blade_graphics := array{0x13fe, 0x26C9, 0x26C5, 0x26CB, 0x26CF, 0xF9E6};
    var weapon := CreateItemInBackpack(mobile, 0x090C, 1);
    if (!weapon)
        return 0;
    endif

    weapon.graphic := blade_graphics[RandomInt(blade_graphics.size()) + 1];
    EquipItem(mobile, weapon);
    if (weapon.maxhp > 0)
        weapon.hp := weapon.maxhp;
    endif

    SetCooldown(mobile, "spiritual_weapon", 99999);

    var elements := array{"Fire", "Cold", "Energy"};
    ApplyElementalWeaponProperties(mobile, elements[RandomInt(elements.size()) + 1]);

    return weapon;
endfunction

// Creates a tier-1 item (armor or weapon) in mobile's backpack, equips it, and
// returns the objref. Falls back to raw objtype if no tier-1 material config exists.
// Sets hp := maxhp (guarded against non-durable items).
//
// Uses the create.src flow (GeraDescriptorComplexo + CreateItemUnstackableInContainer)
// so the item is a proper craftable — stats/material/PROPNAME_FABRICADO populated, behaves
// identically to a crafted item when looted by a player.
//
// Does NOT pre-clear the target layer. If replacing an existing weapon, use
// EquipRandomFromArray or DestroyItem(mobile.weapon) before calling.
function _NpcEquipItem(mobile, objtype)
    var finalObjtype := GeraDescriptorComplexo(objtype, 1);
    if (!finalObjtype)
        finalObjtype := objtype;
    endif

    var created := CreateItemUnstackableInContainer(mobile, finalObjtype, 1, mobile.backpack);
    if (!created || created.size() == 0)
        return 0;
    endif

    var item := SystemFindObjectBySerial(created[1].Serial);
    if (!item)
        return 0;
    endif

    if (item.maxhp > 0)
        item.hp := item.maxhp;
    endif
    EquipItem(mobile, item);
    return item;
endfunction


function SetupAlchemistLoadout(mobile)
    // Create explosive potions
    var explosive_potions := array{0xbf13, 0xbf14, 0xbf15, 0xef60, 0xef61, 0xef62};
    foreach potion in explosive_potions
        for i := 1 to 2
            CreateItemInBackpack(mobile, potion, 1);
            sleepms(2);
        endfor
    endforeach
    
    // Create healing potions
    var heal_potions := array{0xDC01, 0xDC02, 0xDC03};
    foreach potion in heal_potions
        for i := 1 to 2
            CreateItemInBackpack(mobile, potion, 1);
            sleepms(2);
        endfor
    endforeach
    
    // Speed potion
    CreateItemInBackpack(mobile, 0xbf0d, 1);
endfunction


function SetupArcaneTricksterLoadout(mobile, npc_elem)
    _NpcEquipItem(mobile, 0x13B2);           // Bow (tier 1)
    var quiver := _NpcEquipItem(mobile, 0x2FB7);  // Quiver

    if (quiver)
        CreateItemInContainer(quiver, 0xF3F, RandomIntMinMax(20, 80));  // Arrows
    endif

    var tier := npc_elem.Tier;
    for i := 1 to tier
        CreateItemInBackpack(mobile, "pergaminhospellcaster"+i, RandomInt(2)+1);
    endfor
endfunction


// Replaces mobile's current weapon with a random item from the array.
// Goes through _NpcEquipItem so the replacement is a proper tier-1 crafted item
// (stats, material, correct HP) instead of a raw spawn.
function EquipRandomFromArray(mobile, items)
    if (mobile.weapon)
        DestroyItem(mobile.weapon);
    endif
    return _NpcEquipItem(mobile, items[RandomInt(items.size()) + 1]);
endfunction


// Mirrors comprarHabilidade's data shape: dict keyed by slot level (meta.level)
// so MobileWrapper.HasAbility / GetAbilities and the combat passive dispatcher
// see NPCs and players the same way.
function BuildMyhabsDict(habs_array)
    var habcfg := ReadConfigFile(":abilities:configs/habilidades");
    var habs_dict := dictionary{};
    foreach hab in habs_array
        var elem := FindConfigElem(habcfg, CInt(hab));
        if (elem)
            var meta := ParseAbilityMeta(elem);
            if (meta != error)
                habs_dict[meta.level] := CInt(hab);
            endif
        endif
        sleepms(2);
    endforeach
    return habs_dict;
endfunction

function SetupHumanoidHabs(mobile)
    var wrestling := AP_GetSkill(mobile, WRESTLING);
    var tactics := AP_GetSkill(mobile, TACTICS);
    var calculate_level_hp := CInt(wrestling + tactics) - 80;

    // Add safety cap to prevent overflow
    if (calculate_level_hp > 100)
        calculate_level_hp := 100;
    endif

    var npc_level := (calculate_level_hp / 6);
    // Ensure npc_level is within reasonable bounds
    if (npc_level < 0)
        npc_level := 0;
    elseif (npc_level > 20)
        npc_level := 20;
    endif

    var habs_by_level := GetHabsByLevel(mobile, npc_level);
    SetObjProperty(mobile, "myhabs", BuildMyhabsDict(habs_by_level));
endfunction


function SetupWizardClass(mobile, npc_elem)
    // First try to get school from config
    var spellschool := GetConfigString(npc_elem, "SpellSchool") ? GetConfigString(npc_elem, "SpellSchool") : GetObjProperty(mobile, "SpellSchool");
    var base_ability := 0;

    // If no school defined in config, infer from existing habs.
    // myhabs is now dict-shape (slot -> id), so use HasAbility instead of `in`.
    if (!spellschool)
        var wrapper := MobileWrapper(mobile);
        if (wrapper.HasAbility(ABILITY_ID::VINCULO_ELEMENTAL))
            spellschool := "Ahtaleloreth";
        elseif (wrapper.HasAbility(ABILITY_ID::ELO_NATURAL))
            spellschool := "Aradalore";
        elseif (wrapper.HasAbility(ABILITY_ID::SACRIFICIO_DA_CARNE))
            spellschool := "Kaijin";
        elseif (wrapper.HasAbility(ABILITY_ID::DANCA_DAS_SOMBRAS))
            spellschool := "Velkyn";
        elseif (wrapper.HasAbility(ABILITY_ID::DETERMINACAO_ARCANA))
            spellschool := "Evocatium";
        endif
    endif

    // Map school to base ability ID
    case (spellschool)
        "Kaijin": base_ability := ABILITY_ID::SACRIFICIO_DA_CARNE;
        "Ahtaleloreth": base_ability := ABILITY_ID::VINCULO_ELEMENTAL;
        "Aradalore": base_ability := ABILITY_ID::ELO_NATURAL;
        "Evocatium": base_ability := ABILITY_ID::DETERMINACAO_ARCANA;
        "Velkyn": base_ability := ABILITY_ID::DANCA_DAS_SOMBRAS;
    endcase

    if (!base_ability)
        return 0;
    endif

    // Store the school
    SetObjProperty(mobile, "SpellSchool", spellschool);

    // Build progression based on base ability
    var habcfg := ReadConfigFile(":abilities:configs/habilidades");
    var progression := GetClassProgression(habcfg, base_ability, mobile);
    SetObjProperty(mobile, "myhabs", BuildMyhabsDict(progression));

    return 1;
endfunction


function SetupBaseLanguages(mobile)
    SetObjProperty(mobile, "linguas_conhecidas", {"Comum"});
    SetObjProperty(mobile, "lingua_escolhida", "Comum");
endfunction


function InitializeGenericNPC(mobile, npc_elem, elem, sounds)
   // Chamar o initializeNPC do ghaia para fazer uma migração não destrutiva, mesmo que
   // eu deva só adicionar mobs que eu tenha criado
endfunction


function SetupAmmo(mobile, npccfgfile, template := 0)
	if (!template)
		template := ParseTemplateName(mobile.npctemplate).template;
	endif

	var elem := npccfgfile[template];
	if ( elem.MissileWeapon )
		var weapon := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, elem.MissileWeapon, 1, mobile.realm);
		if ( !EquipItem(mobile, weapon) )
			DestroyItem(weapon);
		endif
	endif

	if ( elem.AmmoType )
		if ( CInt(elem.AmmoAmount) > 0 )
			var ammo := CreateItemInBackpack(mobile, elem.AmmoType, CInt(elem.AmmoAmount));
			if ( !ammo )
				return ammo;
			endif
			return 1;
		endif
	endif

	return 1;
endfunction


function SetupNPCEquip(who, npc_elem)
    var categ := GetConfigString(npc_elem, "Equip");
    categ := categ;
    if(isMaleGraphic(who))
        categ := categ + "M";
    elseif (isFemaleGraphic(who))
        categ := categ + "F";
    endif
    var template := ReadConfigFile(":ghaia:config/equip2");
    var group := FindConfigElem(template, categ);
    if(group)
        // Read config files ONCE, pass to all EquipNPCItemString calls
        var colors_cfg := ReadConfigFile(":ghaia:config/colors");
        var vestiments_cfg := ReadConfigFile(":ghaia:config/cloths");
        var str;
        var lastc := 0;
        foreach str in GetConfigStringArray(group, "Equip")
            lastc := EquipNPCItemString(who, str, lastc, colors_cfg, vestiments_cfg);
        endforeach
        foreach str in GetConfigStringArray(group, "Armor")
            lastc := EquipNPCItemString(who, str, lastc, colors_cfg, vestiments_cfg);
        endforeach
        foreach str in GetConfigStringArray(group, "Weapon")
            lastc := EquipNPCItemString(who, str, lastc, colors_cfg, vestiments_cfg);
        endforeach
    endif
    var boss := GetObjProperty(who, "EpicBossStruct");
    if (boss.specific_weapon)
        var weapon := boss.specific_weapon;
        if (weapon)
            var item := CreateItemAtLocation(who.x, who.y, who.z, weapon, 1);
            // First Unequip hands
            if (who.weapon)
                DestroyItem(who.weapon);
            endif
            if (who.shield)
                DestroyItem(who.shield);
            endif
            EquipItem(who, item);
        endif
    endif

    if (boss.equipments && boss.equipments.size() > 0)
        foreach equipment in (boss.equipments)
            var objtype := CInt(equipment.objtype);
            if (objtype)
                // Use amount if specified, otherwise default to 1
                var amount := equipment.amount ? CInt(equipment.amount) : 1;
                var item := CreateItemAtLocation(who.x, who.y, who.z, objtype, amount);
                if (item)
                    // Apply color if specified
                    if (equipment.color && equipment.color != 0)
                        item.color := CInt(equipment.color);
                    endif
                    
                    // Apply all other properties that were captured
                    if (equipment.properties && equipment.properties.size() > 0)
                        foreach prop_name in (equipment.properties.keys())
                            var prop_value := equipment.properties[prop_name];
                            // Skip objtype, color, and amount as they're handled separately
                            if (prop_name != "objtype" && prop_name != "color" && prop_name != "amount")
                                SetObjProperty(item, prop_name, prop_value);
                            endif
                            sleepms(2);
                        endforeach
                    endif
                    
                    // Try to equip the new item first
                    if (!EquipItem(who, item))
                        // Find what layer this item wants to use
                        var equipped_items := ListEquippedItems(who);
                        var equipment_removed := 0;
                        
                        foreach equipped_item in equipped_items
                            // Try removing each equipped item and see if our item can equip
                            var temp_item := equipped_item;
                            MoveItemToContainer(temp_item, who.backpack);
                            
                            // Try to equip our new item
                            if (EquipItem(who, item))
                                // Success! The temp_item was conflicting, so destroy it
                                DestroyItem(temp_item);
                                equipment_removed := 1;
                                break;
                            else
                                // Not the conflicting item, put it back
                                EquipItem(who, temp_item);
                            endif
                            sleepms(2);
                        endforeach
                        
                        // If we still couldn't equip after removing items, put it in backpack or destroy
                        if (!equipment_removed)
                            if (!MoveItemToContainer(item, who.backpack))
                                DestroyItem(item);
                            endif
                        endif
                    endif
                endif
            endif
            sleepms(2);
        endforeach
    endif

    // Apply boss colors to clothing AFTER all equipment is set up
    if (boss && boss.color_ranges && boss.color_ranges.size() > 0)
        ApplyBossColors(who, boss.color_ranges);
    endif
    
    var montaria := Getobjproperty(who, "Montaria");
    if (boss.mount)
        montaria := boss.mount;
    endif
    if (montaria)
        start_script(":epicAI:scripts/setup_mount", array{who, montaria});
    endif
endfunction


function SetupAnimalCriticalHit(mobile, npc_elem)
    var gender_set := npc_elem.Gender;
    var taxonomy := GetConfigString(npc_elem, "Taxonomy");
    var evil := (mobile.alignment == 1);
    if (!evil)
        return;
    endif

    if (taxonomy == "bovidae" || taxonomy == "cervidae")
        case (RandomInt(1))
            0: SetObjProperty(mobile, "Critic", "Coice" );
            1: SetObjProperty(mobile, "Critic", "Chifrada");
        endcase
    elseif (taxonomy == "suidae" || taxonomy == "camelidae" || taxonomy == "equus")
        SetObjProperty(mobile, "Critic", "Coice");
    elseif (taxonomy == "anura" || taxonomy == "serpentes")
        case (RandomInt(1))
            0: SetObjProperty(mobile, "Critic", "Bote" );
            1: SetObjProperty(mobile, "Critic", "Glandulas_concentradas" );
        endcase
    elseif (taxonomy == "primatomorpha" || taxonomy == "ursidae")
        case (RandomInt(2))
            0: SetObjProperty(mobile, "Critic", "Frenesi" );
            1: SetObjProperty(mobile, "Critic", "Rugir" );
        endcase
    elseif (taxonomy == "canidae" || taxonomy == "felidae" || taxonomy == "chiroptera" || taxonomy == "reptilia" || taxonomy == "crustacean" || taxonomy == "scarabaeidae")
        case (RandomInt(2))
            0: SetObjProperty(mobile, "Critic", "Presas_profundas" );
            1: SetObjProperty(mobile, "Critic", "Garras_afiadas" );
        endcase
    elseif (taxonomy == "immortuos" || taxonomy == "draco")
        SetObjProperty(mobile, "Critic", "Baforada" );
    elseif (taxonomy == "aves" || taxonomy == "phasianidae")
        SetObjProperty(mobile, "Critic", "Garras_afiadas");
    endif

    SetObjProperty(mobile, "Selvagem", 1);

endfunction


function SetupAnimalGender(mobile, npc_elem)
  var gender_set := npc_elem.Gender;

  if ( gender_set == 0)
    //var gender_new := Cint(randomint(2) + 1);
    case(randomint(2))
          0: Setobjproperty(mobile, "Sexo", 1);
          1: Setobjproperty(mobile, "Sexo", 2);
        endcase
  else
    Setobjproperty(mobile, "Sexo", Cint(mobile.Gender) );
  endif

endfunction


function setupNPCBackPack(mobile)
	if ( !mobile.backpack )
		var backpack := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, "BackPack", 1);
		var result := EquipItem(mobile, backpack);
		if ( !result )
			DestroyItem(backpack);
			return result;
		else
			return backpack;
		endif
	endif

	return mobile.backpack;
endfunction


function setupNPCColor(who, colors)
    colors := SplitWords(colors);

    var color := Cint(colors[RandomInt(Len(colors)) + 1]);
    who.color := color;
    who.truecolor := color;
endfunction


function setupNPCGender(mobile, bossFemale := 0)
  var female := GetObjProperty(mobile, "female");

  if (female || bossFemale)
    case (mobile.graphic)
        400: mobile.graphic := 401;
        1830: mobile.graphic := 1831;
        1832: mobile.graphic := 1833;
        1828: mobile.graphic := 1829;
    endcase
    mobile.gender := 1;
  endif
endfunction


function setupAnchorNPC(mobile)
    var spawnpointserial := GetObjProperty(mobile, "spawnpoint");
    var range := 6;
    var x := mobile.x;
    var y := mobile.y;
    var z := mobile.z;

    if (spawnpointserial)
        var spawnpoint := SystemFindObjectBySerial(spawnpointserial);
        if (spawnpoint)
            var spawn_range := CInt(GetObjProperty(spawnpoint, "Range"));
            if (spawn_range)
                range := spawn_range;
            endif
            x := spawnpoint.x;
            y := spawnpoint.y;
            z := spawnpoint.z;
        endif
    endif

    SetObjProperty(mobile, "anchor", array{ x, y, z + 2, range });
    SetObjProperty(mobile, "range", range);
    // SetAnchor(x, y, range, 90); // This function seems to be deprecated or not working as expected
endfunction


function setupChampspawnNPC(mobile)
    if (GetObjPRoperty(mobile, "championspawn") == 2)
        SetObjProperty(mobile, "#championspawn", 1);
        ApplyDamage( mobile, 20000);
    elseif (GetObjPRoperty(mobile, "championspawn") == 1)
        SetObjPRoperty(mobile, "championspawn", 2);
    endif
endfunction


function EquipNPCItemString(who, str, lastcolor, colors := 0, vestiments := 0)
    var params := SplitWords(str);
    var objname;
    var color;
    var chance;
    var lastc := 0;
    if (!colors) colors := ReadConfigFile(":ghaia:config/colors"); endif
    if (!vestiments) vestiments := ReadConfigFile(":ghaia:config/cloths"); endif
    var epicBoss := GetObjProperty(who, "EpicBossStruct");
    var bossColor := epicBoss.color_ranges;
    objname := params[1];
    if(CInt(objname))
        objname := CInt(objname);
    endif
    if(CInt(params[2]))
        color := CInt(params[2]);
    else
        color := 0;
    if(params[2] == "lastcolor" || params[2] == "lastcolour")
        color := lastcolor;
    else
        var colorg := FindConfigElem(color, params[2]);
        if(colorg)
            var str := GetConfigStringArray(colorg, "Color");
            var ar := array{};
            var rndint := RandomInt(len(str)) + 1;
            ar := SplitWords(str[rndint]);
        if(len(ar) > 1)
            if(CInt(ar[2]) > 0)
                color := RandomInt(CInt(ar[2]) - CInt(ar[1]))+1;
             else
                color := CInt(str[RandomInt(len(str))+1]);
            endif
            else
                color := CInt(str[RandomInt(len(str))+1]);
            endif
        if(color == 1)
            color := 0;
        endif
      endif
    endif
  endif
  lastc := color;
  if(CInt(params[3]))
    chance := CInt( params[3] );
  else
    chance := 100;
  endif
  if(CInt(RandomInt(99)+1) <= CInt(chance))
    var item := CreateItemAtLocation(5852, 1163, 0, objname, 1);
    if(!item)
      var itemg := FindConfigElem(vestiments, objname);
      if(itemg)
        var str := GetConfigStringArray(itemg, "Item");
        var str3;
        var newarray := array{};
        var i := 1;
        var sex := "";
        foreach str3 in str
          var str2 := SplitWords(str3);
          sex := CStr(str2[2]);
          if(sex != error)
            if(sex == "m" || sex == "f")
              if(sex == "m" && who.graphic == 400)
                newarray[i] := str2[1];
                i := i + 1;
              elseif(sex == "f" && who.graphic == 401)
                newarray[i] := str2[1];
                i := i + 1;
              endif
            else
              newarray[i] := str2[1];
              i := i + 1;
            endif
          else
            newarray[i] := str2[1];
            i := i + 1;
          endif
        endforeach
        objname := CInt(newarray[RandomInt(len(newarray))+1]);
        item := CreateItemAtLocation(5852, 1163, 0, objname, 1);
      endif
    endif
    item.color := color;
    if(CInt(params[4]) == 1)
      MoveItemToContainer(item, who.backpack);
    else
      if(!EquipItem(who, item))
        MoveItemToContainer(item, who.backpack);
      endif
    endif
  endif
  return lastc;
endfunction


function HandleInitializationLoot(mobile, boss_struct)   
    if (!boss_struct)
        return 0;
    endif
    
    if (!boss_struct.loot || boss_struct.loot.size() < 1)
        return 0;
    endif
    
    foreach loot_item in (boss_struct.loot)       
        if (!loot_item.on_death) // Only process initialization items
            var roll := RandomInt(100);
            
            if (roll < loot_item.drop_chance)
                // Create item directly with the template
                var item := CreateIteminBackpackFromTemplate(mobile, loot_item.template);
                if (item)
                    item.movable := 1;
                endif
            endif
        endif

        sleepms(2);
    endforeach

    return 1;
endfunction


function SetupNPCAppearence(mobile)
    // Get boss data to check for appearance settings
    var boss := GetObjProperty(mobile, "EpicBossStruct");
    if (mobile.gender == 1)
        var existing_beard := GetEquipmentByLayer(mobile, LAYER_BEARD);
        if (existing_beard)
            DestroyItem(existing_beard);
        endif
    endif
    
    if (!boss)
        return 0;
    endif
    
    if (!boss.appearance)
        return 0;
    endif
    
    var appearance := boss.appearance;
    // Setup hair if specified
    if (appearance.hair_style && appearance.hair_style != 0)
        // Check if hair color is specified
        // Remove existing hair first
        var existing_hair := GetEquipmentByLayer(mobile, LAYER_HAIR);
        if (existing_hair)
            DestroyItem(existing_hair);
        endif
        
        // Create and equip new hair using proper objtype
        var hair_item := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, CInt(appearance.hair_style), 1, mobile.realm);
        if (hair_item)
            if (appearance.hair_color)
                hair_item.color := CInt(appearance.hair_color);
            endif
            if (!EquipItem(mobile, hair_item))
                DestroyItem(hair_item);
            endif
        endif
    endif
    
    // Setup beard if specified
    if (appearance.beard_style && appearance.beard_style != 0)
        // Remove existing beard first
        var existing_beard := GetEquipmentByLayer(mobile, LAYER_BEARD);
        if (existing_beard)
            DestroyItem(existing_beard);
        endif
        
        // Create and equip new beard using proper objtype
        var beard_item := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, CInt(appearance.beard_style), 1, mobile.realm);
        if (beard_item)
            if (appearance.beard_color)
                beard_item.color := CInt(appearance.beard_color);
            endif
            if (!EquipItem(mobile, beard_item))
                DestroyItem(beard_item);
            endif
        endif
    endif
    
    // Setup skin color if specified
    if (appearance.skin_color)
        mobile.color := CInt(appearance.skin_color);
        mobile.truecolor := CInt(appearance.skin_color);
    endif
    
    return 1;
endfunction


// New function to apply boss colors to clothing only
function ApplyBossColors(mobile, color_ranges)    
    if (!color_ranges || color_ranges.size() == 0)
        return 0;
    endif
    
    // Define clothing layers (not armor or weapons)
    var clothing_layers := array{
        LAYER_SHIRT,      // Shirt
        LAYER_PANTS,      // Pants
        LAYER_SHOES,      // Shoes
        LAYER_ROBE,       // Robe
        LAYER_CAPE,       // Cloak/Cape
        LAYER_SKIRT,      // Skirt
        LAYER_TUNIC       // Tunic/Sash
    };
    
    // Define armor layers that we DON'T want to color
    var armor_layers := array{
        LAYER_HELM,       // Helmet
        LAYER_GLOVES,     // Gloves
        LAYER_CHEST,      // Chest armor
        LAYER_ARMS,       // Arm armor
        LAYER_LEGS        // Leg armor
    };
    
    var items_colored := 0;
    
    // Apply color to clothing items that have color 0 (uncolored)
    foreach layer in clothing_layers
        var item := GetEquipmentByLayer(mobile, layer);
        if (item)            
            if (item.color == 0)
                // Skip actual armor layers (not clothing that happens to be POLCLASS_ARMOR)
                if (layer in armor_layers)
                    continue;
                endif
                
                // Get a random color from the selected ranges for each item
                var selected_color := GetRandomColorFromRanges(color_ranges);
                
                item.color := selected_color;
                item.truecolor := selected_color;
                items_colored += 1;
            endif
        endif
        sleepms(2);
    endforeach
        
    return 1;
endfunction


// Copy the GetColorValue function from cor.src for consistency
function GetColorValue(color)
    case (color)
        "Rosa":      return RandomIntMinMax(1201, 1254);
        "Vermelho":  return RandomIntMinMax(1601, 1654);
        "Azul":      return RandomIntMinMax(1301, 1354);
        "Verde":     return RandomIntMinMax(1401, 1454);
        "Amarelo":   return RandomIntMinMax(1701, 1754);
        "Laranja":   return RandomIntMinMax(1501, 1554);
        "Neutro":    return RandomIntMinMax(1801, 1908);
        "Cinza":     return RandomInt({0, 946, 947});
        AIGLANA:     return RandomColorFromRanges({
                        RandomIntMinMax(1601, 1654),  // Vermelho
                        RandomIntMinMax(1701, 1754),  // Amarelo
                        RandomIntMinMax(1801, 1908)   // Neutro
                     });
        HAROLOTH:    return RandomColorFromRanges({
                        RandomIntMinMax(1201, 1254),  // Rosa
                        RandomInt({0, 946, 947}),     // Cinza
                        RandomIntMinMax(1301, 1354),  // Azul
                        RandomIntMinMax(1801, 1908)   // Neutro
                     });
        LESTE:       return RandomColorFromRanges({
                        RandomIntMinMax(1701, 1754),  // Amarelo
                        RandomIntMinMax(1501, 1554),  // Laranja
                        RandomIntMinMax(1201, 1254),  // Rosa
                        RandomInt({0, 946, 947})      // Cinza
                     });
        BJORSKA:     return RandomColorFromRanges({
                        RandomIntMinMax(1401, 1454),  // Verde
                        RandomIntMinMax(1701, 1754),  // Amarelo
                        RandomIntMinMax(1801, 1908)   // Neutro
                     });
        ALURIEL:     return RandomColorFromRanges({
                        RandomIntMinMax(1201, 1254),  // Rosa
                        RandomIntMinMax(1301, 1354),  // Azul
                        RandomIntMinMax(1401, 1454),  // Verde
                        RandomIntMinMax(1801, 1908)   // Neutro
                     });
        BADURAN:     return RandomColorFromRanges({
                        RandomIntMinMax(1501, 1554),  // Laranja
                        RandomIntMinMax(1701, 1754),  // Amarelo
                        RandomIntMinMax(1801, 1908),  // Neutro
                        RandomInt({0, 946, 947})      // Cinza
                     });
        GULTHRAK:    return RandomColorFromRanges({
                        RandomIntMinMax(1201, 1254),  // Rosa
                        RandomIntMinMax(1601, 1654),  // Vermelho
                        RandomIntMinMax(1801, 1908),  // Neutro
                        RandomInt({0, 946, 947})      // Cinza
                     });
        COLHESIANA:    return RandomColorFromRanges({
                        RandomIntMinMax(1501, 1554),  // Laranja
                        RandomIntMinMax(1701, 1754),  // Amarelo
                        RandomIntMinMax(1401, 1454),  // Verde
                        RandomIntMinMax(1301, 1354),  // Azul
                        RandomIntMinMax(1801, 1908)   // Neutro
                     });
        default:     return color; // In case color is already a numeric value
    endcase
endfunction

