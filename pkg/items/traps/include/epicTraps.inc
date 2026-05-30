/*
 * epicTraps.inc
 *
 * Helper functions and data for Epic Trap system
 * Provides curated lists of sound effects, graphic effects,
 * and trap execution logic
 */

use uo;
use os;

include "include/client";
include "include/epicConstants";
include ":combat:Controller/DamageController";
include ":combat:Model/Damage";
include ":diceroll:model/Dice";
include ":combat:Model/DamageDice";
include ":charactercreation:resists";
include ":charactercreation:habilidades";
include "include/say";

/*
 * GetCommonSFXList()
 * Returns a curated array of useful sound effects for traps
 * Each entry: struct{ name, id, category }
 */
function GetCommonSFXList()
    var sfx_list := array{};

    // Combat/Magic Sounds
    sfx_list.append(struct{ name := "Explosao", id := SFX_SPELL_EXPLOSION, category := "Combate" });
    sfx_list.append(struct{ name := "Bola de Fogo", id := SFX_SPELL_FIREBALL, category := "Combate" });
    sfx_list.append(struct{ name := "Raio", id := SFX_SPELL_LIGHTNING, category := "Combate" });
    sfx_list.append(struct{ name := "Chamas", id := SFX_SPELL_FLAME_STRIKE, category := "Combate" });
    sfx_list.append(struct{ name := "Veneno", id := SFX_SPELL_POISON, category := "Combate" });
    sfx_list.append(struct{ name := "Cura", id := SFX_SPELL_HEAL, category := "Combate" });
    sfx_list.append(struct{ name := "Relampago em Cadeia", id := SFX_SPELL_CHAIN_LIGHTNING, category := "Combate" });
    sfx_list.append(struct{ name := "Meteorito", id := SFX_SPELL_METEOR_SWARM, category := "Combate" });
    sfx_list.append(struct{ name := "Vortex de Energia", id := SFX_SPELL_ENERGY_VORTEX, category := "Combate" });
    sfx_list.append(struct{ name := "Terremoto", id := SFX_SPELL_EARTHQUAKE, category := "Combate" });

    // Environmental Sounds
    sfx_list.append(struct{ name := "Trovao 1", id := SFX_29, category := "Ambiente" });
    sfx_list.append(struct{ name := "Trovao 2", id := SFX_2A, category := "Ambiente" });
    sfx_list.append(struct{ name := "Vento 1", id := SFX_15, category := "Ambiente" });
    sfx_list.append(struct{ name := "Vento 2", id := SFX_16, category := "Ambiente" });
    sfx_list.append(struct{ name := "Bolhas", id := SFX_21, category := "Ambiente" });
    sfx_list.append(struct{ name := "Gotejamento", id := SFX_23, category := "Ambiente" });
    sfx_list.append(struct{ name := "Respingo", id := SFX_26, category := "Ambiente" });

    // Trap Sounds
    sfx_list.append(struct{ name := "Armadilha de Agulha", id := SFX_TRAP_NEEDLE, category := "Armadilha" });
    sfx_list.append(struct{ name := "Armadilha de Explosao", id := SFX_TRAP_EXPLOSION, category := "Armadilha" });
    sfx_list.append(struct{ name := "Armadilha de Veneno", id := SFX_TRAP_POISON, category := "Armadilha" });
    sfx_list.append(struct{ name := "Lamina", id := SFX_219, category := "Armadilha" });
    sfx_list.append(struct{ name := "Espinho", id := SFX_22C, category := "Armadilha" });
    sfx_list.append(struct{ name := "Pedregulho", id := SFX_21E, category := "Armadilha" });
    sfx_list.append(struct{ name := "Desmoronamento", id := SFX_220, category := "Armadilha" });

    // Creature Sounds (for horror/themed traps)
    sfx_list.append(struct{ name := "Dragao", id := SFX_16B, category := "Criatura" });
    sfx_list.append(struct{ name := "Fantasma", id := SFX_17F, category := "Criatura" });
    sfx_list.append(struct{ name := "Zumbi", id := SFX_1D8, category := "Criatura" });
    sfx_list.append(struct{ name := "Lich", id := SFX_19D, category := "Criatura" });
    sfx_list.append(struct{ name := "Demonio", id := SFX_166, category := "Criatura" });
    sfx_list.append(struct{ name := "Lobo", id := SFX_E6, category := "Criatura" });
    sfx_list.append(struct{ name := "Serpente", id := SFX_DC, category := "Criatura" });
    sfx_list.append(struct{ name := "Corvo", id := SFX_7E, category := "Criatura" });

    // Miscellaneous
    sfx_list.append(struct{ name := "Quebrar Vidro", id := SFX_3F, category := "Misc" });
    sfx_list.append(struct{ name := "Tocha", id := SFX_48, category := "Misc" });
    sfx_list.append(struct{ name := "Bigorna", id := SFX_ANVL, category := "Misc" });
    sfx_list.append(struct{ name := "Martelo", id := SFX_HAMER, category := "Misc" });
    sfx_list.append(struct{ name := "Sino", id := SFX_B, category := "Misc" });

    return sfx_list;
endfunction

/*
 * GetEpicFXList()
 * Returns a curated array of EPIC_FX constants for trap effects
 * Each entry: struct{ name, id, category }
 */
function GetEpicFXList()
    var fx_list := array{};

    // Fire Effects
    fx_list.append(struct{ name := "Fogo - Envolver", id := FX_FIRE_ENGULF, category := "Fogo" });
    fx_list.append(struct{ name := "Fogo - Chamas", id := FX_FIRE_FLAME, category := "Fogo" });
    fx_list.append(struct{ name := "Fogo - Chamas Pequenas", id := FX_FIRE_FLAME_SMALL, category := "Fogo" });
    fx_list.append(struct{ name := "Fogo - Explosao", id := FX_FIRE_SKULL_EXPLOSION, category := "Fogo" });
    fx_list.append(struct{ name := "Fogo - Ciclone", id := FX_FIRE_CYCLONE, category := "Fogo" });
    fx_list.append(struct{ name := "Fogo - Geiser", id := FX_GEISER_DE_FOGO, category := "Fogo" });
    fx_list.append(struct{ name := "Fogo - Seta", id := FX_FIRE_ARROW, category := "Fogo" });
    fx_list.append(struct{ name := "Fogo - Raio", id := FX_FIRE_BOLT, category := "Fogo" });
    fx_list.append(struct{ name := "Fogo - Fumaca", id := FX_FUMACA, category := "Fogo" });

    // Ice/Cold Effects
    fx_list.append(struct{ name := "Gelo - Bola", id := FX_ICE_BALL, category := "Gelo" });
    fx_list.append(struct{ name := "Gelo - Espinho", id := FX_ICE_SPIKE_GIANT, category := "Gelo" });
    fx_list.append(struct{ name := "Gelo - Chuva", id := FX_ICE_SPIKE_RAIN, category := "Gelo" });
    fx_list.append(struct{ name := "Gelo - Dardos", id := FX_DARDOS_CONGELANTES, category := "Gelo" });
    fx_list.append(struct{ name := "Gelo - Tempestade", id := FX_ICE_TEMPEST_RIGHT, category := "Gelo" });
    fx_list.append(struct{ name := "Gelo - Ataque", id := FX_ATAQUE_DE_GELO, category := "Gelo" });

    // Lightning/Energy Effects
    fx_list.append(struct{ name := "Raio - Protector", id := FX_RAIO_PROTECTOR, category := "Raio" });
    fx_list.append(struct{ name := "Raio - Contundente", id := FX_RAIO_CONTUNDENTE, category := "Raio" });
    fx_list.append(struct{ name := "Raio - Pequeno", id := FX_RAIO_PEQUENO, category := "Raio" });
    fx_list.append(struct{ name := "Raio - Fulminante", id := FX_RAIO_FULMINANTE, category := "Raio" });
    fx_list.append(struct{ name := "Raio - Cabeca", id := FX_RAIO_NA_CABECA, category := "Raio" });
    fx_list.append(struct{ name := "Energia - Explosao", id := FX_EXPLOSAO_DE_ENERGIA, category := "Raio" });
    fx_list.append(struct{ name := "Energia - Eletricidade", id := FX_ELETRICIDADE, category := "Raio" });
    fx_list.append(struct{ name := "Energia - Raio", id := FX_ELETRIC_BOLT, category := "Raio" });

    // Poison/Nature Effects
    fx_list.append(struct{ name := "Veneno - Gotas", id := FX_POISON_DROPS, category := "Veneno" });
    fx_list.append(struct{ name := "Veneno - Gas", id := FX_POISON_GAS_MAG_2, category := "Veneno" });
    fx_list.append(struct{ name := "Veneno - Caveira", id := FX_POISON_SKULL, category := "Veneno" });
    fx_list.append(struct{ name := "Veneno - Gas Mortal", id := FX_DEATH_POISON_GAS, category := "Veneno" });
    fx_list.append(struct{ name := "Veneno - Esfera", id := FX_POISON_SPHERE, category := "Veneno" });
    fx_list.append(struct{ name := "Veneno - Morte", id := FX_POISON_DEATH, category := "Veneno" });
    fx_list.append(struct{ name := "Planta - Mortal", id := FX_PLANTA_MORTAL, category := "Veneno" });
    fx_list.append(struct{ name := "Raizes - Agarradoras", id := FX_PLANTA_MORTAL, category := "Veneno" });

    // Explosion Effects
    fx_list.append(struct{ name := "Explosao - Grande", id := FX_EXPLOSAO_ENORME, category := "Explosao" });
    fx_list.append(struct{ name := "Explosao - Normal", id := FX_EXPLOSAO, category := "Explosao" });
    fx_list.append(struct{ name := "Explosao - Pequena", id := FX_PEQUENA_EXPLOSAO, category := "Explosao" });
    fx_list.append(struct{ name := "Explosao - Purpura", id := FX_PURPLE_EXPLOSION, category := "Explosao" });
    fx_list.append(struct{ name := "Explosao - Vermelha", id := FX_RED_EXPLOSION, category := "Explosao" });
    fx_list.append(struct{ name := "Explosao - Azul", id := FX_BLUE_EXPLOSION_A, category := "Explosao" });
    fx_list.append(struct{ name := "Explosao - Agua", id := FX_EXPLOSAO_DE_AGUA, category := "Explosao" });
    // Dark/Death Effects
    fx_list.append(struct{ name := "Morte - Portal", id := FX_DEATH_PORTAL_S, category := "Morte" });
    fx_list.append(struct{ name := "Morte - Arvore", id := FX_DEATH_TREE, category := "Morte" });
    fx_list.append(struct{ name := "Morte - Terremoto", id := FX_DEATH_EARTHQUAKE, category := "Morte" });
    fx_list.append(struct{ name := "Caveira - Magica", id := FX_SKULL_MAGIC, category := "Morte" });
    fx_list.append(struct{ name := "Energia Negra", id := FX_BLACK_ENERGY, category := "Morte" });
    fx_list.append(struct{ name := "Dano Necrotico", id := FX_DANO_NECROTICO, category := "Morte" });

    // Miscellaneous
    fx_list.append(struct{ name := "Teleporte", id := FX_TELEPORT, category := "Misc" });
    fx_list.append(struct{ name := "Atordoamento", id := FX_STUN, category := "Misc" });
    fx_list.append(struct{ name := "Portal", id := FX_PORTAL, category := "Misc" });
    fx_list.append(struct{ name := "Tornado", id := FX_TORNADO, category := "Misc" });
    fx_list.append(struct{ name := "Furacao", id := FX_FURACAO, category := "Misc" });
    fx_list.append(struct{ name := "Tempestade Verde", id := FX_GREEN_THUNDERSTORM, category := "Misc" });
    fx_list.append(struct{ name := "Bolhas", id := FX_BOLHAS, category := "Misc" });
    fx_list.append(struct{ name := "Sangue - Gota", id := FX_BLOOD_DROP, category := "Misc" });
    fx_list.append(struct{ name := "Passaros", id := FX_PASSAROS, category := "Misc" });

    return fx_list;
endfunction

/*
 * GetDamageTypeConstants()
 * Returns damage type mapping for the combat system
 */
function GetDamageTypeConstants()
    return dictionary{
        "Physical" -> DAMAGE_TYPE::BLUNT,
        "Fire" -> DAMAGE_TYPE::FIRE,
        "Cold" -> DAMAGE_TYPE::COLD,
        "Poison" -> DAMAGE_TYPE::POISON,
        "Energy" -> DAMAGE_TYPE::ENERGY
    };
endfunction

/*
 * ExecuteEpicTrapEffect(mobile, trap)
 * Main trap execution function
 * Handles sound, visual effect, and damage application
 */
function ExecuteEpicTrapEffect(mobile, trap)
    if (!mobile || !trap)
        return 0;
    endif

    // Read trap properties
    var trap_name := GetObjProperty(trap, "epic_trap_name");
    var sfx_id := GetObjProperty(trap, "epic_trap_sfx");
    var fx_id := GetObjProperty(trap, "epic_trap_fx");
    var damage_dice := GetObjProperty(trap, "epic_trap_damage_dice");
    var damage_types := GetObjProperty(trap, "epic_trap_damage_types");
    var fx_type := GetObjProperty(trap, "epic_trap_fx_type"); // "Stationary" or "Moving"
    var fx_source := GetObjProperty(trap, "epic_trap_fx_source"); // struct{x, y, z, realm}

    if (!trap_name)
        trap_name := "Armadilha Epica";
    endif

    if (!damage_dice || damage_dice == "")
        damage_dice := "1d20"; // Default
    endif

    if (!damage_types || !damage_types.size())
        damage_types := array{"Physical"}; // Default
    endif

    if (!fx_type || fx_type == "")
        fx_type := "Stationary"; // Default
    endif

    // Show trap
    trap.invisible := 0;

    // Play sound effect
    if (sfx_id)
        PlaySoundEffect(trap, CInt(sfx_id));
    endif

    sleepms(200);

    // Play visual effect
    if (fx_id)
        if (fx_type == "Moving" && fx_source && fx_source.x)
            // Moving effect from source coordinates to trap
            PlayMovingEffectXYZ(
                CInt(fx_source.x), CInt(fx_source.y), CInt(fx_source.z),
                trap.x, trap.y, trap.z,
                CInt(fx_id), 7, 10, 0, fx_source.realm
            );
        else
            // Stationary effect at trap location
            PlayObjectCenteredEffect(trap, CInt(fx_id), 7, 10);
        endif
    endif

    // Message to player
    SendSysMessageEx(mobile, "Voce ativou " + trap_name + "!", SSM_FAIL);

    sleepms(200);

    // Calculate damage
    var dice := Dice(damage_dice);
    var base_damage := dice.Roll();

    // Check for resist/evasion
    var damage_multiplier := 1.0;
    if (CheckResist(mobile, REFLEXES, RESIST_HARD))
        if (MobileWrapper(mobile).HasAbility(ABILITY_ID::DESBRAVADOR_NATO))
            SendSysMessageEx(mobile, "Voce escapou completamente da armadilha.", SSM_INFO);
            return 1;
        else
            SendSysMessageEx(mobile, "Voce evitou parte do dano da armadilha.", SSM_INFO);
            damage_multiplier := 0.5;
        endif
    endif

    var final_damage := CInt(base_damage * damage_multiplier);

    // Apply damage by type using DamageDice
    var damage_type_map := GetDamageTypeConstants();

    foreach dmg_type in damage_types
        var dmg_constant := damage_type_map[dmg_type];
        if (dmg_constant)
            DealDamage(mobile, final_damage, dmg_constant);
        endif
        sleepms(50);
    endforeach

    // Hit animation and sound
    PerformAction(mobile, 0x14);
    if (mobile.graphic == 401)
        PlaySoundEffect(mobile, 0x32F);
    elseif (mobile.graphic == 400)
        PlaySoundEffect(mobile, 0x441);
    endif

    return 1;
endfunction