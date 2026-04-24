use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":gumps:yesNo";
include ":gumps:requestGump";
include "include/sounds";

/**
 * Abrir Portais (Open Portals) - ABILITY_ID::ABRIR_PORTAIS (2204)
 *
 * Active (ON_USE):
 * - Mark up to arcana/33 locations (max 3)
 * - Open portals (telepads) to marked locations lasting arcana/10 seconds
 * - NPC behavior: open portal to random location
 */
class OpenPortals(CombatAbility)

    function OpenPortals(this)
        Super(ABILITY_ID::ABRIR_PORTAIS);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        var arcana := CInt(mobile.GetSkill(ARCANA));

        // NPC behavior
        if (mobile.IsNPC())
            _OpenNpcPortal(who);
            return 1;
        endif

        var input := _CreatePortalGump(who, arcana);
        if (!input)
            return 0;
        endif

        var buttonId := input[0];

        // Mark portal buttons: 201, 202, 203
        if (buttonId >= 201 && buttonId <= 203)
            _MarkPortal(who, buttonId - 200);
        // Open portal buttons: 211, 212, 213
        elseif (buttonId >= 211 && buttonId <= 213)
            _OpenPortal(who, buttonId - 210, arcana);
        endif

        return 1;
    endfunction

endclass

function _CreatePortalGump(mobile, arcana)
    var gump := GFCreateGump();
    var portals := GetObjProperty(mobile, "portals");
    var maxPortals := CInt(arcana / 33);

    if (maxPortals < 1)
        maxPortals := 1;
    endif
    if (maxPortals > 3)
        maxPortals := 3;
    endif

    if (!portals)
        portals := struct{};
        SetObjProperty(mobile, "portals", portals);
    endif

    GFResizePic(gump, 17, 31, 9390, 359, 125 + maxPortals * 26);
    GFHTMLArea(gump, 114, 69, 192, 20, "<BASEFONT Size=5 Color=#999999>Viajante Planar", 0, 0);
    GFHTMLArea(gump, 116, 71, 192, 20, "<BASEFONT Size=5 Color=#999999>Viajante Planar", 0, 0);
    GFHTMLArea(gump, 115, 70, 192, 20, "<BASEFONT Size=5 Color=#ffffff>Viajante Planar", 0, 0);
    GFGumpPic(gump, 63, 91, 1469);
    GFGumpPic(gump, 80, 91, 1469);
    GFTextLine(gump, 63, 100, 0, "Portais");
    GFTextLine(gump, 259, 100, 0, "Marcar");
    GFTextLine(gump, 320, 100, 0, "Abrir");
    GFGumpPic(gump, 38, 103, 9773, 0);
    GFGumpPic(gump, 296, 0, 9000, 0);

    var y := 121;
    var i;
    for (i := 1; i <= maxPortals; i++)
        var portalName := "portal" + CStr(i);
        var portal := portals[portalName];

        if (!portal)
            portal := struct{
                name := portalName,
                desc := "",
                created := 0,
                x := mobile.x,
                y := mobile.y,
                z := mobile.z,
                r := mobile.realm
            };
            portals[portalName] := portal;
        endif

        var color := "#000000";
        GFHTMLArea(gump, 63, y, 192, 24, $"<BASEFONT color={color}>{portal.desc}", 1, 0);
        GFAddButton(gump, 274, y + 5, 10006, 10006, GF_CLOSE_BTN, 200 + i);
        GFAddButton(gump, 324, y + 4, 10006, 10006, GF_CLOSE_BTN, 210 + i);
        sleepms(2);
        y += 26;
    endfor

    SetObjProperty(mobile, "portals", portals);
    return GFSendGump(mobile, gump);
endfunction

function _MarkPortal(mobile, portalNumber)
    var portals := GetObjProperty(mobile, "portals");

    Message("Escolha o local para marcar seu portal.", Color(SYSTEM_COLOR::INFO)).SendSys(mobile);
    var targ := TargetCoordinates(mobile);

    if (!targ)
        Message("Marcacao cancelada.", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile);
        return 0;
    endif

    if (!CheckLosAt(mobile, targ.x, targ.y, targ.z))
        Message("Voce nao consegue ver o alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile);
        return 0;
    endif

    var portalName := "portal" + CStr(portalNumber);
    portals[portalName].x := targ.x;
    portals[portalName].y := targ.y;
    portals[portalName].z := targ.z;
    portals[portalName].r := mobile.realm;
    portals[portalName].created := 1;

    var name := RequestGump(mobile, "Deseja adicionar um nome ao portal?");
    portals[portalName].desc := name;

    SetObjProperty(mobile, "portals", portals);
    PlayStationaryEffect(targ.x, targ.y, targ.z, 0x6D70, 0, 40, 0, mobile.realm);
    PlaySoundEffect(mobile, 0x1f0);

    Message($"Portal '{name}' marcado com sucesso!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(mobile);

    return 1;
endfunction

function _SetDestination(tile, x, y, z, realm)
    var coords := struct{
        x := CInt(x),
        y := CInt(y),
        z := CInt(z),
        realm := CStr(realm),
        race := "all"
    };
    return SetObjProperty(tile, "Destination", coords);
endfunction

function _OpenPortal(mobile, portalNumber, arcana)
    var portals := GetObjProperty(mobile, "portals");
    var portalName := "portal" + CStr(portalNumber);
    var portal := portals[portalName];

    if (!portal || !portal.created)
        Message("Este portal ainda nao foi marcado.", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile);
        return 0;
    endif

    // Create telepad at player location
    var tile := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, "telepad", 1, mobile.realm);
    if (!tile)
        Message("Nao foi possivel criar o portal.", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile);
        return 0;
    endif

    // Set destination
    _SetDestination(tile, portal.x, portal.y, portal.z, portal.r);

    // Calculate portal duration
    var duration := CInt(arcana / 10);
    if (duration < 5)
        duration := 5;
    endif
    if (duration > 60)
        duration := 60;
    endif

    // Effects
    PlayStationaryEffect(tile.x, tile.y, tile.z, 0x0DDA, 0, 255, 0, mobile.realm);
    PlayStationaryEffect(portal.x, portal.y, portal.z, 0x0DDA, 0, 255, 0, portal.r);
    PlaySoundEffect(mobile, 0x1f2);

    Message($"Portal para '{portal.desc}' aberto por {duration}s!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(mobile);

    // Wait and refresh effects
    sleep(duration / 3);
    PlayStationaryEffect(tile.x, tile.y, tile.z, 0x0DDA, 0, 255, 0, mobile.realm);
    PlayStationaryEffect(portal.x, portal.y, portal.z, 0x0DDA, 0, 255, 0, portal.r);

    sleep(duration / 3);
    PlayStationaryEffect(tile.x, tile.y, tile.z, 0x0DDA, 0, 255, 0, mobile.realm);
    PlayStationaryEffect(portal.x, portal.y, portal.z, 0x0DDA, 0, 255, 0, portal.r);

    sleep(duration / 3);

    DestroyItem(tile);
    Message("O portal se fechou.", Color(SYSTEM_COLOR::INFO)).SendSys(mobile);

    return 1;
endfunction

function _OpenNpcPortal(npc)
    var tile := CreateItemAtLocation(npc.x, npc.y, npc.z, "telepad", 1, npc.realm);
    if (!tile)
        return 0;
    endif

    var destX := RandomIntMinMax(500, 2000);
    var destY := RandomIntMinMax(500, 2000);
    var destZ := GetWorldHeight(destX, destY, npc.realm);

    _SetDestination(tile, destX, destY, destZ, npc.realm);

    PlayStationaryEffect(tile.x, tile.y, tile.z, 0x0DDA, 0, 255, 0, npc.realm);
    PlayStationaryEffect(destX, destY, destZ, 0x0DDA, 0, 255, 0, npc.realm);
    PlaySoundEffect(npc, 0x1f2);

    sleep(10);

    PlayStationaryEffect(tile.x, tile.y, tile.z, 0x0DDA, 0, 255, 0, npc.realm);
    PlayStationaryEffect(destX, destY, destZ, 0x0DDA, 0, 255, 0, npc.realm);

    sleep(10);
    sleep(10);

    DestroyItem(tile);

    return 1;
endfunction
