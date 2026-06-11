use uo;
use os;
use polsys;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include ":alchemy:alchemy";
include ":gumps:include/epicGumps";

/**
 * Zona de Pressao (Pressure Zone) - ABILITY_ID::ZONA_DE_PRESSAO (2501)
 *
 * Active (ON_USE): pre-commit a zone type, then target a ground tile. Stores a
 * single PressureZone CProp on the caster {x,y,z,realm,type,expires}. Any of the
 * caster's explosives detonating within 3 tiles of the zone, while it is active,
 * gets the zone effect (applied in exploder.src ExplosionEffect):
 *   Pressao  -> +50% explosion radius
 *   Lentidao -> victims slowed for Science/10s
 *   Precisao -> +50% damage
 * Zone lasts Science/5 seconds. One zone at a time (single CProp).
 */
class PressureZone(CombatAbility)

    function PressureZone(this)
        Super(ABILITY_ID::ZONA_DE_PRESSAO);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        if (mobile.IsNPC())
            return 0;
        endif

        var zoneType := _PromptZoneType(who);
        if (!zoneType)
            Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif

        Message("Escolha o local da zona.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        var targ := TargetCoordinates(who);
        if (!targ)
            Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif

        if (CoordinateDistance(who.x, who.y, targ.x, targ.y) > 15)
            Message("O local esta muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif
        if (!CheckLosAt(who, targ.x, targ.y, targ.z))
            Message("Voce nao consegue ver o local.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return 0;
        endif

        var science := CInt(mobile.GetSkill(SCIENCE));
        var duration := CInt(science / 5);
        if (duration < 4)
            duration := 4;
        endif

        var marker := CreateItemAtLocation(targ.x, targ.y, targ.z, 0xD586, 1, who.realm);
        if (marker)
            marker.movable := 0;
            marker.decayat := readgameclock() + duration;
        endif

        SetObjProperty(who, "PressureZone", struct{
            x       := targ.x,
            y       := targ.y,
            z       := targ.z,
            realm   := who.realm,
            type    := zoneType,
            expires := polcore().systime + duration,
            marker  := marker ? marker.serial : 0
        });

        Message($"*marca uma Zona de {_ZoneName(zoneType)} por {duration}s*").SendAbove(who);

        return 1;
    endfunction

endclass

function _ZoneName(zoneType)
    case (zoneType)
        ZONA_TIPO_PRESSAO:  return "Pressao";
        ZONA_TIPO_LENTIDAO: return "Lentidao";
        ZONA_TIPO_PRECISAO: return "Precisao";
    endcase
    return "?";
endfunction

// Small 3-button chooser; returns ZONA_TIPO_* or 0 on cancel.
function _PromptZoneType(who)
    var gump := GFECreateGump("Zona de Pressao", 360, 170);
    GFTextMid(gump, 0, 18, 360, 1153, "Escolha o tipo de zona:");

    GFResizePic(gump, 30, 50,  9350, 300, 28);
    GFAddButton(gump, 36, 55, 2151, 2153, 1, ZONA_TIPO_PRESSAO);
    GFTextLine(gump, 70, 56, 53, "Pressao  (+50% raio de explosao)");

    GFResizePic(gump, 30, 85,  9350, 300, 28);
    GFAddButton(gump, 36, 90, 2151, 2153, 1, ZONA_TIPO_LENTIDAO);
    GFTextLine(gump, 70, 91, 53, "Lentidao  (vitimas ficam lentas)");

    GFResizePic(gump, 30, 120, 9350, 300, 28);
    GFAddButton(gump, 36, 125, 2151, 2153, 1, ZONA_TIPO_PRECISAO);
    GFTextLine(gump, 70, 126, 53, "Precisao  (+50% dano)");

    var input := GFSendGump(who, gump);
    case (input[0])
        ZONA_TIPO_PRESSAO:  return ZONA_TIPO_PRESSAO;
        ZONA_TIPO_LENTIDAO: return ZONA_TIPO_LENTIDAO;
        ZONA_TIPO_PRECISAO: return ZONA_TIPO_PRECISAO;
    endcase
    return 0;
endfunction
