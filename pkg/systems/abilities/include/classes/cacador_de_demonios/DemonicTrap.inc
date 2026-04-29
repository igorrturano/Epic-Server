use uo;
use os;
use math;
use cfgfile;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedscripts:timedScripts";
include "include/shapes";

/**
 * Armadilha Demoniaca (Demonic Trap) - ABILITY_ID::ARMADILHA_DEMONIACA (1903)
 *
 * Active (ON_USE):
 * - Creates a hexagram (Star of David) pattern around caster
 * - Range based on THEOLOGY/10
 * - Paralyzes Demon/Undead NPCs for 30s
 * - Marks targets with chain-of-pain properties
 * - Vertex items auto-decay via Timer property
 */
class DemonicTrap(CombatAbility)

    function DemonicTrap(this)
        Super(ABILITY_ID::ARMADILHA_DEMONIACA);
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        var range := CInt(mobile.GetSkill(THEOLOGY) / 10);
        if (range < 1)
            range := 1;
        endif

        PerformAction(who, HUMAN_ANIM::CAST_AREA);

        var duration := 30;
        var cfg := ReadConfigFile(":*:npcdesc");
        var inimigo := array{ "Demon", "Undead" };

        // Create hexagram vertices
        var vertices := array{};
        var num_points := range;
        for i := 0 to (num_points - 1)
            var angle_rad := (30 + (i * 360 / num_points)) * ConstPi() / 180;
            var x := who.x + CInt(range * Cos(angle_rad));
            var y := who.y + CInt(range * Sin(angle_rad));

            var vertex_loc := CreateItemAtLocation(x, y, who.z, 0x1f16, 1, who.realm);
            if (!vertex_loc)
                return false;
            endif
            SetObjProperty(vertex_loc, "Timer", ReadGameClock() + duration + 5);
            vertex_loc.invisible := 1;
            vertex_loc.movable := 0;
            vertices.append(vertex_loc);
        endfor

        // Visual effects at vertices
        var vertex_effect := 0x484A;
        var line_effect := 0x484D;
        var effect_speed := 10;
        var effect_loop := 0;
        var effect_color := 2444;

        foreach vertex_item in vertices
            PlayObjectCenteredEffectEx(vertex_item, vertex_effect, effect_speed, effect_loop, effect_color);
            sleepms(10);
        endforeach

        // Draw lines between vertices (triangles)
        if (vertices.size() >= 5)
            PlayMovingEffectEx(vertices[1], vertices[3], line_effect, effect_speed, effect_loop, effect_color);
            sleepms(10);
            PlayMovingEffectEx(vertices[3], vertices[5], line_effect, effect_speed, effect_loop, effect_color);
            sleepms(10);
            PlayMovingEffectEx(vertices[5], vertices[1], line_effect, effect_speed, effect_loop, effect_color);
            sleepms(10);
            PlayMovingEffectEx(vertices[2], vertices[4], line_effect, effect_speed, effect_loop, effect_color);
            sleepms(10);
            PlayMovingEffectEx(vertices[4], vertices[6], line_effect, effect_speed, effect_loop, effect_color);
            sleepms(10);
            PlayMovingEffectEx(vertices[6], vertices[2], line_effect, effect_speed, effect_loop, effect_color);
            sleepms(10);
        endif

        // Find and affect targets
        var targets := array{};
        var nearby := mobile.GetNearbyMobiles(range);
        foreach mob in nearby
            if (mob.serial != who.serial)
                if (!(mob in who.party.members))
                    targets.append(mob.serial);

                    SetObjProperty(mob, "#chainofpain", ReadGameClock() + duration);
                    SetObjProperty(mob, "#mageserial", who.serial);

                    // Paralyze demons/undead
                    if (mob.npctemplate)
                        var npctemplate := NPC_ParseTemplateName(mob.npctemplate);
                        if (cfg[npctemplate.template].Environment in inimigo)
                            TS_StartTimer(mob, "paralysis", duration);
                            ApplyRawDamage(mob, 1);
                        endif
                    endif
                endif
            endif
        endforeach

        // Store chain serials and apply initial damage
        if (targets.size() > 0)
            foreach targs in targets
                var mob := SystemFindObjectBySerial(targs);
                if (mob)
                    SetObjProperty(mob, "#chainserials", targets);
                    ApplyRawDamage(mob, 2);
                endif
            endforeach
        endif

        Message("*armadilha demoniaca*").SendAbove(who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[ARMADILHA_DEMONIACA] Range: {range}, Duration: {duration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
