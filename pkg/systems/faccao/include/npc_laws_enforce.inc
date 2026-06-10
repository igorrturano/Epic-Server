include ":gumps:gumps";
include ":gumps:gumps_ex";
include "include/arrays";

// npc_laws_enforce.inc — Enforcement de leis de faccao para NPCs de combate.
//
// REQUERIMENTOS do script que inclui este arquivo:
//   Variaveis globais ja definidas: me, guild, fighting, following, fix_location, wait_time, state
//   Constantes ja definidas: HELP_CMD
//   Includes ja carregados: :faccao:membership, :faccao:faccao, :tn:cooldown
//
// FORNECE:
//   Variaveis: warning, warn_type, warn_tries, warn_tier, warn_opt, guild_laws, guild_leader
//   Funcoes:   CheckPlayerLaws, WarnViolator, HandleLawEvent, disableWarn, CheckKnockedOut
//              HasEnemyNearLaw, CheckIfHavePlayerGuild_Law, CheckItemLaw, EngageViolator
//              DefinirPostoFixo, ProcessDoubleClick, ShowFaccaoFacingGump, ShowRevealGump
//              CheckAwarenessDetection, CheckRevealTimeout, WanderNPC, HandleRevealSpeech
//   Constantes: REVELADO_EVENT, PICKPOCKET_EVENT, LOCKPICK_EVENT, CERCA_EVENT, RESET_SETUP_EVENT

// --- variaveis de estado de aviso ---
var warning    := 0;
var warn_type  := 0;
var warn_tries := 0;
var warn_tier  := 3;   // tier da infracao atual (1=leve, 2=moderado, 3=grave)
var warn_opt   := 0;
var warn_rect  := 0;

// --- modo de combate persistente (para assassinos/cazadores) ---
// Quando 1: NPC nao volta ao stealth ate eliminar o alvo e area estar limpa.
// Setado por SneakAttack (inicio de emboscada) ou EngageViolator (revelado).
var stand_fight := 0;

// --- carregados pelo Setup do script pai ---
var guild_laws   := array{};
var guild_leader := 0;

// Objtypes de ferramentas (nao sao "armas" para efeito de noweapon)
var law_tools := array{ 0xec4, 0x13e3, 0xFB4, 0xFB5, 0x13f6, 0xe85, 0xf43, 0xec2, 0x1F020 };

// --- eventos customizados de leis ---
const REVELADO_EVENT    := 0x78460;
const PICKPOCKET_EVENT  := 0x78459;
const LOCKPICK_EVENT    := 0x78455;
const CERCA_EVENT       := 0x78456;
const RESET_SETUP_EVENT := 0x78457;

// --- multas ---
const FINE_AMOUNT := 10;   // multa imediata
const FINE_PRISON := 20;   // multa na prisao (dobrada)

// --- expulsao ---
const EXPULSION_X     := 1320;
const EXPULSION_Y     := 1014;
const EXPULSION_Z     := 1;
const EXPULSION_REALM := "britannia_alt";

// --- Rufioes: dump no centro ---
const RUFIAO_DUMP_X := 1175;
const RUFIAO_DUMP_Y := 1062;
const RUFIAO_DUMP_Z := 0;

// ============================================================
function disableWarn()
    if (warning && warning.isA(POLCLASS_MOBILE) && guild)
        EraseObjProperty(warning, "#guardando_{}".format(guild.guildid));
    endif
    warn_tries := 0;
    warn_type  := 0;
    warn_opt   := 0;
    warning    := 0;
    warn_tier  := 3;
endfunction

// Verifica se ha inimigo (NPC de alinhamento diferente) proximo do player
function HasEnemyNearLaw(what)
    foreach mob in ListMobilesNearLocationEx(what.x, what.y, what.z, 8,
            LISTEX_FLAG_NORMAL | LISTEX_FLAG_NPC_ONLY, what.realm)
        sleepms(2);
        if (mob.alignment && mob.alignment != me.alignment && mob != what && mob != me)
            return 1;
        endif
    endforeach
    return 0;
endfunction

// Verifica se algum mobile da lista e membro com privilegio acima dos guardas
function CheckIfHavePlayerGuild_Law(mobile_list)
    foreach player in mobile_list
        sleepms(2);
        if (IsMemberOfFaccao(guild, player))
            var privs := GetMemberPrivs(guild, player);
            if (player.serial == guild_leader || (privs && privs.above_guards))
                return 1;
            endif
        endif
    endforeach
    return 0;
endfunction

// Verifica se o player esta equipando item proibido
function CheckItemLaw(what, itemobj)
    foreach item in ListEquippedItems(what)
        sleepms(2);
        if (item.objtype == itemobj.objtype) return itemobj; endif
    endforeach
    return 0;
endfunction

// Engaja o violador em combate e avisa outros NPCs da faccao
function EngageViolator(targ)
    if (!targ || !targ.isA(POLCLASS_MOBILE) || targ.dead) return; endif
    me.frozen   := 0;
    me.hidden   := 0;
    fighting    := targ;
    state       := 3;
    stand_fight := 1;  // revelado: nao volta ao stealth ate eliminar o alvo
    SetOpponent(targ);
    SetWarMode(1);
    if (guild)
        AddReport(guild, "{} entrou em combate com {} por violar as leis".format(
            me.name, targ.name));
    endif
    // Chamar outros NPCs da faccao proximos
    foreach monster in ListMobilesNearLocationEx(me.x, me.y, me.z, 8,
            LISTEX_FLAG_NORMAL | LISTEX_FLAG_HIDDEN | LISTEX_FLAG_NPC_ONLY, me.realm)
        sleepms(2);
        if (IsMemberOfFaccao(guild, monster) && monster != me)
            monster.frozen := 0;
            SendEvent(monster, struct{ "type" := HELP_CMD, source := me, targ := targ });
        endif
    endforeach
endfunction

// ============================================================
// Prisoes hardcoded por staff_tipo (fallback quando guild.prisions nao foi configurado)
// ============================================================
function GetPrisonPoints(g)
    var pts := g.getProp("prisions");
    if (pts && pts.size() > 0) return pts; endif

    var _raw_tipo := g.getProp("staff_tipo");
    // Faccao player nao tem prisao hardcoded — sem teleporte
    if (_raw_tipo == error || !_raw_tipo) return array{}; endif
    var tipo := CStr(_raw_tipo);

    case (tipo)
        "conselho_kaijin":
        "conselho_shaman":
        "conselho_aradal":
        "conselho_velkyn":
        "conselho_evocatium":
            return array{
                struct{ "x" := 3481, "y" := 63, "z" := 0, "realm" := "britannia_alt" }
            };

        "regentes":
            return array{
                struct{ "x" := 3431, "y" := 65, "z" := 0, "realm" := "britannia_alt" },
                struct{ "x" := 3435, "y" := 64, "z" := 0, "realm" := "britannia_alt" },
                struct{ "x" := 3439, "y" := 63, "z" := 0, "realm" := "britannia_alt" },
                struct{ "x" := 3443, "y" := 63, "z" := 0, "realm" := "britannia_alt" }
            };

        default:
            return array{
                struct{ "x" := 3530, "y" := 64, "z" := 0, "realm" := "britannia_alt" },
                struct{ "x" := 3535, "y" := 63, "z" := 0, "realm" := "britannia_alt" }
            };
    endcase
endfunction

// Ponto de saida apos pagar multa na prisao (sede da faccao)
function _GetReleasePoint(g)
    var tipo := CStr(g.getProp("staff_tipo"));
    case (tipo)
        "regentes":
            return struct{ "x" := 1170, "y" := 1044, "z" := 0, "realm" := "britannia_alt" };
        "conselho_kaijin": "conselho_shaman": "conselho_aradal":
        "conselho_velkyn": "conselho_evocatium":
            return struct{ "x" := 1149, "y" := 1091, "z" := 0, "realm" := "britannia_alt" };
        default:
            return struct{ "x" := 1195, "y" := 1083, "z" := 0, "realm" := "britannia_alt" };
    endcase
endfunction

// Tier da infracao por warn_type (1=leve→soltura 1h, 2=moderado, 3=grave)
function _GetLawTier(wt)
    if (wt in array{ 20, 21, 18, 30, 19, 10, 26, 25, 24 }) return 1; endif
    if (wt in array{ 5, 33, 14, 15, 6, 3 })                return 2; endif
    return 3;
endfunction

// Marca o player para acordar na prisao ao voltar do desmaio
function TeleportToJail(who, g, tier := 3)
    var pts := GetPrisonPoints(g);
    if (!pts || !pts.size()) return; endif
    var p := pts[RandomInt(pts.size()) + 1];

    // Salva o destino da prisao no player — KataWakeUp vai usar ao ressuscitar
    SetObjProperty(who, "#prisao_destino", struct{
        "x"       := p.x,
        "y"       := p.y,
        "z"       := p.z,
        "realm"   := p.realm,
        "tier"    := tier,
        "guildid" := g.guildid
    });

    // Salva #prisao_pendente aqui ja — garante persistencia mesmo se servidor crashar
    // antes do prisaotimer rodar (que tambem seta essa prop, mas mais tarde)
    SetObjProperty(who, "#prisao_pendente", struct{
        "guildid" := g.guildid,
        "tier"    := tier
    });

    var msg := "Voce foi derrotado pelos guardas.";
    if (tier == 1) msg += " Infracao leve.";
    elseif (tier == 2) msg += " Infracao moderada.";
    else msg += " Infracao grave.";
    endif
    SendSysMessageEx(who, msg + " Voce acordara na prisao da faccao.", SSM_HARMFUL);

    AddReport(g, "{} foi derrotado e acordara na prisao (tier {})".format(who.name, tier));
endfunction

// ============================================================
// Helpers de moedas
// ============================================================
function _CountCoins(who)
    var bp := who.backpack;
    if (!bp) return 0; endif
    var total := 0;
    foreach item in EnumerateItemsInContainer(bp)
        sleepms(2);
        if (item.objtype == 0xba63)      total += item.amount;
        elseif (item.objtype == 0xba64)  total += item.amount * 10;
        elseif (item.objtype == 0xba65)  total += item.amount * 100;
        endif
    endforeach
    return total;
endfunction

function _TakeCoins(who, amount)
    var bp := who.backpack;
    if (!bp) return 0; endif
    var left := amount;
    set_critical(1);
    foreach item in EnumerateItemsInContainer(bp)
        sleepms(2);
        if (left <= 0) break; endif
        var v := 1;
        if (item.objtype == 0xba64) v := 10;
        elseif (item.objtype == 0xba65) v := 100;
        elseif (item.objtype != 0xba63) continue;
        endif
        var tc := item.amount * v;
        if (tc <= left)
            left -= tc; DestroyItem(item);
        else
            SubtractAmount(item, CInt(left / v)); left := 0;
        endif
    endforeach
    set_critical(0);
    return (left <= 0);
endfunction

// ============================================================
// Helpers internos de deteccao
// ============================================================

function _WitnessCheck(who, loc_prop, range := 8)
    var loc := GetObjProperty(who, loc_prop);
    if (!loc) return 1; endif
    if (loc.realm != me.realm) return 0; endif
    return CoordinateDistance(me.x, me.y, loc.x, loc.y) <= range;
endfunction

function _CheckLawTarget(law, who, is_member, is_full, is_honorary, is_leader)
    // Trata target nao definido (leis antigas sem target) como "foramembros"
    if (!law.target || typeof(law.target) == "Uninit")
        return !is_member;
    endif
    var _tgt := CStr(law.target);
    if (!_tgt || _tgt == "" || _tgt == "foramembros" || _tgt == "0")
        return !is_member;
    endif
    case (_tgt)
        "todos":
            return 1;
        "nao_lider":
            return !is_leader;
        "apenas_full":
            return is_full;
        "apenas_honorarios":
            return is_honorary;
        "nao_membro_x":
            var exceto := law.exceto_serials;
            if (!exceto || !exceto.size()) return 1; endif
            return !(who.serial in exceto);
        "exceto_faccoes":
            // Aplica a lei a todos, EXCETO membros das guilds listadas em law.exceto_guilds
            var _egids := law.exceto_guilds;
            if (!_egids || !_egids.size()) return !is_member; endif
            foreach _egid in _egids
                sleepms(2);
                var _eg := FindGuild(CInt(_egid));
                if (_eg && IsMemberOfFaccao(_eg, who)) return 0; endif
            endforeach
            return 1;
        default:
            return !is_member;
    endcase
endfunction

function _CheckPets(who)
    var pets := GetObjProperty(who, "TamedPets");
    if (!pets || !pets.size()) return 0; endif
    foreach serial in pets
        sleepms(2);
        var pet := SystemFindObjectBySerial(CInt(serial));
        if (pet && pet.isA(POLCLASS_MOBILE) && !pet.dead &&
                pet.master == who && Distance(pet, who) < 12)
            return 1;
        endif
    endforeach
    return 0;
endfunction

function _CheckHeavyArmor(who)
    var itemdesc := ReadConfigFile("::itemdesc");
    foreach item in ListEquippedItems(who)
        sleepms(2);
        var tipo := CStr(itemdesc[item.objtype].TipoProtecao);
        if (tipo && tipo["Pesada"] != error) return 1; endif
    endforeach
    return 0;
endfunction

function _CheckMagicItems(who)
    foreach item in ListEquippedItems(who)
        sleepms(2);
        if (GetObjProperty(item, "magic") != error) return 1; endif
    endforeach
    return 0;
endfunction

function _CheckColors(who, hue_list)
    if (!hue_list || !hue_list.size()) return 0; endif
    foreach item in ListEquippedItems(who)
        sleepms(2);
        if (item.color in hue_list) return 1; endif
    endforeach
    return 0;
endfunction

// ============================================================
// Deteccao de fraude de identidade (documento emitido sob disfarce)
// ============================================================
function _FindCarta(who, guildid)
    var bp := who.backpack;
    if (!bp) return 0; endif
    foreach item in EnumerateItemsInContainer(bp)
        sleepms(2);
        if (item.objtype == 0x748B &&
                CStr(GetObjProperty(item, "doc_tipo")) == "carta" &&
                CInt(GetObjProperty(item, "doc_guild")) == guildid)
            return item;
        endif
    endforeach
    return 0;
endfunction

function _CheckDocFraud(who, g)
    var carta := _FindCarta(who, g.guildid);
    if (!carta) return 0; endif
    if (!CInt(GetObjProperty(carta, "doc_disfarce"))) return 0; endif
    // Documento emitido sob disfarce — verificar se ainda esta disfarcado
    var d := GetObjProperty(who, "disfarce");
    if (d != error && d) return 0; endif
    return 1;  // nao esta mais disfarcado mas doc foi emitido sob disfarce → fraude
endfunction

// ============================================================
// CheckPlayerLaws — detecta violacoes e seta warning/warn_type/warn_tier
// ============================================================
function CheckPlayerLaws(who)
    if (who.npctemplate && !who.master) return; endif
    if (!who || !who.isA(POLCLASS_MOBILE) || who.dead || who.concealed) return; endif

    if (GetCooldown(me, "recentcheck_{}".format(who.serial)))
        return;
    endif

    // Lock: apenas 1 guarda por vez pode advertir este player nesta guild
    var _lock_key    := "#guardando_{}".format(guild.guildid);
    var _lock_serial := CInt(GetObjProperty(who, _lock_key));
    if (_lock_serial && _lock_serial != me.serial)
        var _lock_holder := SystemFindObjectBySerial(_lock_serial);
        if (_lock_holder && _lock_holder.isA(POLCLASS_MOBILE) && !_lock_holder.dead)
                return;
        endif
        EraseObjProperty(who, _lock_key);  // guarda anterior sumiu, liberar
    endif

    var is_member   := IsMemberOfFaccao(guild, who);
    var is_full     := IsFullMember(guild, who);
    var is_honorary := IsHonoraryMember(guild, who);
    var user_subguilds := who.guild ? who.guild.getProp("subguilds") : array{};
    if (!user_subguilds) user_subguilds := array{}; endif
    var is_leader := (who.serial == guild_leader || guild.guildid in user_subguilds);

    if (is_member)
        var privs := GetMemberPrivs(guild, who);
        if (privs && (privs.abovelaws || privs.above_guards))
            return;
        endif
    endif

    // Imunidade pos-multa (1 min)
    var _multa_exp := CInt(GetObjProperty(who, "#multa_paga_{}".format(guild.guildid)));
    if (_multa_exp > polcore().systime)
        return;
    endif

    var has_enemy := HasEnemyNearLaw(who);

    foreach law in guild_laws
        sleepms(2);
        var _law_target_ok := _CheckLawTarget(law, who, is_member, is_full, is_honorary, is_leader);
        if (!law.on)     continue; endif
        if (!_law_target_ok) continue; endif

        case (law.type)

            "nosneak":
                var _hiding := GetProcess(CInt(GetObjProperty(who, "#HidePid")));
                if (_hiding)
                    warning := who; warn_type := 6; warn_tier := 2;
                endif
                break;

            "userban":
                if (who.name == law.opt)
                    warning := who; warn_type := 2; warn_tier := 3; warn_opt := law.opt;
                endif
                break;

            "nofight":
                if (who.warmode == 1 && !has_enemy)
                    warning := who; warn_type := 1; warn_tier := 3;
                endif
                break;

            "nohood":
                if (who.name == "Desconhecido")
                    warning := who; warn_type := 5; warn_tier := 2;
                endif
                break;

            "noweapon":
                var _is_tool := who.weapon ? (who.weapon.objtype in law_tools) : 0;
                if (who.weapon && !_is_tool && !has_enemy)
                    warning := who; warn_type := 3; warn_tier := 2;
                endif
                break;

            "noitem":
                if (CheckItemLaw(who, law.opt))
                    warning := who; warn_type := 4; warn_tier := 2; warn_opt := law.opt;
                endif
                break;

            "norace":
                var _raca := cstr(GetObjProperty(who, "raca"));
                if (_raca && _raca == cstr(law.opt))
                    warning := who; warn_type := 2; warn_tier := 3; warn_opt := law.opt;
                endif
                break;

            "nooutsider":
                warning := who; warn_type := 2; warn_tier := 3;
                break;

            "nooutsideron":
                var rect := law.opt.rect;
                if (who.x >= CInt(rect[1]) && who.x <= CInt(rect[3]) &&
                    who.y >= CInt(rect[2]) && who.y <= CInt(rect[4]) &&
                    who.z >= CInt(rect[5]) && who.z < CInt(rect[5]) + 15)
                    warning := who; warn_rect := law.opt.rect; warn_type := 2; warn_tier := 3;
                endif
                break;

            "noguild":
                if (who.guildid == law.opt)
                    warning := who; warn_type := 2; warn_tier := 3; warn_opt := law.opt;
                endif
                break;

            "noduel":
                if (who.warmode && who.opponent && !who.opponent.npctemplate)
                    warning := who; warn_type := 11; warn_tier := 3;
                endif
                break;

            "notransformation":
                if (GetObjProperty(who, "metamorfose") != error)
                    warning := who; warn_type := 9; warn_tier := 3;
                endif
                break;

            "noanimals":
                if (_CheckPets(who))
                    warning := who; warn_type := 10; warn_tier := 1;
                endif
                break;

            "noheavyarmor":
                if (_CheckHeavyArmor(who))
                    warning := who; warn_type := 14; warn_tier := 2;
                endif
                break;

            "nomagicitems":
                if (_CheckMagicItems(who))
                    warning := who; warn_type := 15; warn_tier := 2;
                endif
                break;

            "nocolor":
                if (_CheckColors(who, law.opt.hues))
                    warning := who; warn_type := 18; warn_tier := 1; warn_opt := law.opt.hues;
                endif
                break;

            "nomount_run":
                if (who.running && who.mounted)
                    warning := who; warn_type := 21; warn_tier := 1;
                endif
                break;

            "nowrestling":
                if (who.warmode && !who.weapon)
                    warning := who; warn_type := 24; warn_tier := 1;
                endif
                break;

            "noplant":
                var _last_plant := CInt(GetObjProperty(who, "#lastplant"));
                if (_last_plant && polcore().systime - _last_plant < 10)
                    warning := who; warn_type := 29; warn_tier := 1;
                endif
                break;

            "noharvest":
                var _last_harv := CInt(GetObjProperty(who, "#lastharvest"));
                if (_last_harv && polcore().systime - _last_harv < 10)
                    warning := who; warn_type := 29; warn_tier := 1;
                endif
                break;

            "notirefruits":
                var _last_trf := CInt(GetObjProperty(who, "#lasttirefruit"));
                if (_last_trf && polcore().systime - _last_trf < 10)
                    warning := who; warn_type := 29; warn_tier := 1;
                endif
                break;

            "noloot_unconscious":
                var _loot_ts := CInt(GetObjProperty(who, "#loot_unconscious"));
                if (_loot_ts && polcore().systime - _loot_ts < 10 &&
                        _WitnessCheck(who, "#loot_unconscious_loc"))
                    warning := who; warn_type := 28; warn_tier := 3;
                endif
                break;

            "toquerecolher":
                if (!law.opt || law.opt.hora_inicio == error) break; endif
                var _hora_tr := (polcore().systime / 3600) % 24;
                var _ativo_tr := 0;
                if (law.opt.hora_inicio < law.opt.hora_fim)
                    _ativo_tr := (_hora_tr >= law.opt.hora_inicio && _hora_tr < law.opt.hora_fim);
                else
                    _ativo_tr := (_hora_tr >= law.opt.hora_inicio || _hora_tr < law.opt.hora_fim);
                endif
                if (_ativo_tr)
                    warning := who; warn_type := 7; warn_tier := 2;
                endif
                break;

            "leimarcia":
                if (GetObjProperty(who, "carteira_faccao_{}".format(guild.guildid)) == error)
                    warning := who; warn_type := 13; warn_tier := 2;
                elseif (_CheckDocFraud(who, guild))
                    warning := who; warn_type := 38; warn_tier := 3;
                endif
                break;

            "norun":
                if (who.running)
                    var _run_key := "#running_since_{}".format(me.serial);
                    var _rs := CInt(GetObjProperty(who, _run_key));
                    if (!_rs)
                        SetObjProperty(who, _run_key, polcore().systime);
                    elseif (polcore().systime - _rs >= 3)
                        warning := who; warn_type := 20; warn_tier := 1;
                    endif
                else
                    EraseObjProperty(who, "#running_since_{}".format(me.serial));
                endif
                break;

            "nightsight_ban":
                var _hora_ns := (polcore().systime / 3600) % 24;
                if ((_hora_ns >= 20 || _hora_ns < 6) && GetObjProperty(who, "#nightsight") != error)
                    var _has_torch := 0;
                    foreach _eq in ListEquippedItems(who)
                        sleepms(2);
                        if (_eq.objtype in array{ 0xA12, 0xA13, 0xF6B }) _has_torch := 1; break; endif
                    endforeach
                    if (!_has_torch) warning := who; warn_type := 19; warn_tier := 1; endif
                endif
                break;

            "nopotions":
                var _lp := CInt(GetObjProperty(who, "#lastpotion"));
                if (_lp && polcore().systime - _lp < 30)
                    warning := who; warn_type := 26; warn_tier := 1;
                endif
                break;

            "nodrugs":
                var _ld := CInt(GetObjProperty(who, "#lastdrug"));
                if (_ld && polcore().systime - _ld < 30)
                    warning := who; warn_type := 27; warn_tier := 3;
                endif
                break;

            "antialcool":
                if (GetObjProperty(who, "baderneiro") != error)
                    if (CInt(GetObjProperty(who, "baderneiro")) > polcore().systime)
                        warning := who; warn_type := 22; warn_tier := 3;
                    else
                        EraseObjProperty(who, "baderneiro");
                    endif
                endif
                break;

            "notaming":
                var _tam := GetObjProperty(who, "TamedPets");
                if (!_tam || !_tam.size()) break; endif
                foreach _ts_serial in _tam
                    sleepms(2);
                    var _tp := SystemFindObjectBySerial(CInt(_ts_serial));
                    if (_tp && _tp.isA(POLCLASS_MOBILE) && !_tp.dead &&
                            _tp.master == who && Distance(_tp, me) < 12)
                        warning := who; warn_type := 25; warn_tier := 1; warn_opt := _tp;
                        break;
                    endif
                endforeach
                break;

            "nomusic":
                var _lm := CInt(GetObjProperty(who, "#lastbard"));
                if (_lm && polcore().systime - _lm < 30)
                    var _music_active := 1;
                    if (law.opt && law.opt.hora_inicio != error)
                        var _hm := (polcore().systime / 3600) % 24;
                        if (law.opt.hora_inicio < law.opt.hora_fim)
                            _music_active := (_hm >= law.opt.hora_inicio && _hm < law.opt.hora_fim);
                        else
                            _music_active := (_hm >= law.opt.hora_inicio || _hm < law.opt.hora_fim);
                        endif
                    endif
                    if (_music_active) warning := who; warn_type := 30; warn_tier := 1; endif
                endif
                break;

            "noclotheschange":
                var _ec  := CInt(GetObjProperty(who, "#equip_changes"));
                var _ect := CInt(GetObjProperty(who, "#equip_change_time"));
                if (_ec >= 3 && _ect && polcore().systime - _ect <= 30)
                    warning := who; warn_type := 33; warn_tier := 2;
                endif
                break;

            "quarentena":
                var _sick := who.poisoned ||
                             GetObjProperty(who, "Diseases_System") != error;
                if (_sick)
                    warning := who; warn_type := 32; warn_tier := 2;
                endif
                break;

            "nocrowd":
                if (!who.warmode && !who.running)
                    var _crowd_count := 0;
                    foreach _near in ListMobilesNearLocationEx(who.x, who.y, who.z, 3,
                            LISTEX_FLAG_NORMAL, who.realm)
                        sleepms(2);
                        if (_near != who && !_near.npctemplate &&
                                !_near.warmode && !_near.running && !_near.dead)
                            _crowd_count += 1;
                        endif
                    endforeach
                    var _permit_key := "alvara_reuniao_{}".format(guild.guildid);
                    if (_crowd_count >= 4 && GetObjProperty(who, _permit_key) == error)
                        var _crowd_ts_key := "#crowd_since_{}".format(guild.guildid);
                        var _crowd_ts := CInt(GetObjProperty(who, _crowd_ts_key));
                        if (!_crowd_ts)
                            SetObjProperty(who, _crowd_ts_key, polcore().systime);
                        elseif (polcore().systime - _crowd_ts >= 60)
                            warning := who; warn_type := 31; warn_tier := 2;
                        endif
                    else
                        EraseObjProperty(who, "#crowd_since_{}".format(guild.guildid));
                    endif
                endif
                break;

            "notax_exempt":
                var _paid_key := "toll_paid_{}".format(guild.guildid);
                var _paid_ts  := CInt(GetObjProperty(who, _paid_key));
                if (!_paid_ts || polcore().systime - _paid_ts >= 600)
                    warning := who; warn_type := 12; warn_tier := 3; warn_opt := law.opt;
                endif
                break;

            "nospell":
                var _last_combat := CInt(GetObjProperty(who, "#lastcombatspell"));
                if (_last_combat && polcore().systime - _last_combat < 10 &&
                        _WitnessCheck(who, "#lastspell_loc"))
                    warning := who; warn_type := 8; warn_tier := 3;
                endif
                break;

            "nospellatall":
                var _last_any := CInt(GetObjProperty(who, "#lastanysell"));
                if (_last_any && polcore().systime - _last_any < 10 &&
                        _WitnessCheck(who, "#lastspell_loc"))
                    warning := who; warn_type := 8; warn_tier := 3;
                endif
                break;

            "nospellschool":
                var _lst := CInt(GetObjProperty(who, "#lastanysell"));
                if (_lst && polcore().systime - _lst < 10 &&
                        _WitnessCheck(who, "#lastspell_loc"))
                    if (CStr(GetObjProperty(who, "#lastspellschool")) == law.opt)
                        warning := who; warn_type := 16; warn_tier := 3;
                    endif
                endif
                break;

            "nospelllist":
                var _lid := CInt(GetObjProperty(who, "#lastanysell"));
                if (_lid && polcore().systime - _lid < 10 &&
                        _WitnessCheck(who, "#lastspell_loc"))
                    if (CInt(GetObjProperty(who, "#lastspellid")) in law.opt)
                        warning := who; warn_type := 17; warn_tier := 3;
                    endif
                endif
                break;

            "lenco":
                var _last_tosse := CInt(GetObjProperty(who, "#lasttosse"));
                if (_last_tosse && polcore().systime - _last_tosse < 10)
                    warning := who; warn_type := 35; warn_tier := 1;
                endif
                break;

            "nomadeira":
                var _last_log := CInt(GetObjProperty(who, "#lastlogging"));
                if (_last_log && polcore().systime - _last_log < 10)
                    warning := who; warn_type := 39; warn_tier := 1;
                endif
                break;

            "nomining":
                var _last_mine := CInt(GetObjProperty(who, "#lastmining"));
                if (_last_mine && polcore().systime - _last_mine < 10)
                    warning := who; warn_type := 40; warn_tier := 1;
                endif
                break;

            "noeat":
                var _last_eat := CInt(GetObjProperty(who, "#lasteat"));
                if (_last_eat && polcore().systime - _last_eat < 10)
                    warning := who; warn_type := 43; warn_tier := 1;
                endif
                break;

            "nofishing":
                var _last_fish := CInt(GetObjProperty(who, "#lastfishing"));
                if (_last_fish && polcore().systime - _last_fish < 10)
                    warning := who; warn_type := 41; warn_tier := 1;
                endif
                break;

            "noalchemy":
                var _last_alch := CInt(GetObjProperty(who, "#lastalchemy"));
                if (_last_alch && polcore().systime - _last_alch < 10)
                    warning := who; warn_type := 42; warn_tier := 1;
                endif
                break;

            "nolitter":
                foreach _litter in ListItemsNearLocation(me.x, me.y, me.z, 8, me.realm)
                    sleepms(2);
                    if (CInt(GetObjProperty(_litter, "dropped_by")) != who.serial) continue; endif
                    if (_litter.decayat == error) continue; endif
                    var _drop_ts := CInt(GetObjProperty(_litter, "#drop_time"));
                    if (!_drop_ts || polcore().systime - _drop_ts > 120) continue; endif
                    warning := who; warn_type := 23; warn_tier := 1; warn_opt := _litter;
                    break;
                endforeach
                break;

        endcase
    endforeach

    // Adquirir lock se detectou violacao
    if (warning)
        SetObjProperty(who, "#guardando_{}".format(guild.guildid), me.serial);
    endif

    SetCooldown(me, "recentcheck_{}".format(who.serial), 2);
    return warning ? 1 : 0;
endfunction

// ============================================================
// Verifica se a violacao ainda esta ativa (player pode ter se corrigido)
// ============================================================
function _CheckViolationActive(wt)
    case (wt)
        1:  if (!warning.warmode) disableWarn(); return 0; endif return 1;
        3:  if (!warning.weapon || (warning.weapon.objtype in law_tools)) disableWarn(); return 0; endif return 1;
        4:  if (!CheckItemLaw(warning, warn_opt)) disableWarn(); return 0; endif return 1;
        5:  if (warning.name != "Desconhecido") disableWarn(); return 0; endif return 1;
        6:  if (!GetProcess(CInt(GetObjProperty(warning, "#HidePid")))) disableWarn(); return 0; endif return 1;
        9:  if (GetObjProperty(warning, "metamorfose") == error) disableWarn(); return 0; endif return 1;
        10: if (!_CheckPets(warning)) disableWarn(); return 0; endif return 1;
        11: if (!warning.warmode || !warning.opponent || warning.opponent.npctemplate) disableWarn(); return 0; endif return 1;
        14: if (!_CheckHeavyArmor(warning)) disableWarn(); return 0; endif return 1;
        15: if (!_CheckMagicItems(warning)) disableWarn(); return 0; endif return 1;
        18: if (!_CheckColors(warning, warn_opt)) disableWarn(); return 0; endif return 1;
        20: if (!warning.running) disableWarn(); return 0; endif return 1;
        21: if (!warning.running || !warning.mounted) disableWarn(); return 0; endif return 1;
        24: if (!warning.warmode || warning.weapon) disableWarn(); return 0; endif return 1;
        25:
            if (!warn_opt || !warn_opt.isA(POLCLASS_MOBILE) || warn_opt.dead ||
                    Distance(warn_opt, warning) >= 14)
                disableWarn(); return 0;
            endif
            return 1;
        26:
            if (!CInt(GetObjProperty(warning, "#lastpotion")) ||
                    polcore().systime - CInt(GetObjProperty(warning, "#lastpotion")) >= 30)
                disableWarn(); return 0;
            endif
            return 1;
        30:
            if (!CInt(GetObjProperty(warning, "#lastbard")) ||
                    polcore().systime - CInt(GetObjProperty(warning, "#lastbard")) >= 30)
                disableWarn(); return 0;
            endif
            return 1;
        31: // nocrowd: verificar se crowd ainda >= 5
            var _cc := 0;
            foreach _nn in ListMobilesNearLocationEx(warning.x, warning.y, warning.z, 3,
                    LISTEX_FLAG_NORMAL, warning.realm)
                sleepms(2);
                if (_nn != warning && !_nn.npctemplate && !_nn.warmode && !_nn.running && !_nn.dead)
                    _cc += 1;
                endif
            endforeach
            if (_cc < 4) disableWarn(); return 0; endif
            return 1;
        32: // quarentena: verificar se ainda doente
            if (!warning.poisoned && GetObjProperty(warning, "Diseases_System") == error)
                EraseObjProperty(warning, "#quarentena_{}".format(guild.guildid));
                disableWarn(); return 0;
            endif
            return 1;
        default: return 1;
    endcase
endfunction

// ============================================================
// Mensagens de aviso por tipo (warn_tries: 0=1o, 1=2o, 2=3o aviso)
// ============================================================
function _PrintWarning(wt)
    var msgs := array{};
    case (wt)
        1:  msgs := array{ "Nao e permitido brigar aqui!",
                           "Segundo aviso — pare com o combate!",
                           "Ultimo aviso antes da multa!" }; break;
        3:  msgs := array{ "Nao e permitido usar armas aqui! Guarde-a!",
                           "Segundo aviso — guarde essa arma!",
                           "Ultimo aviso antes da multa!" }; break;
        4:  msgs := array{ "Nao e permitido usar {} aqui! Guarde!".format(warn_opt ? warn_opt.name : "esse item"),
                           "Segundo aviso — guarde o item proibido!",
                           "Ultimo aviso antes da multa!" }; break;
        5:  msgs := array{ "Nao e permitido cobrir o rosto aqui! Revele-se!",
                           "Segundo aviso — revele seu rosto!",
                           "Ultimo aviso antes da multa!" }; break;
        6:  msgs := array{ "Nao e permitido esgueirar aqui! Saia das sombras!",
                           "Segundo aviso — saia das sombras!",
                           "Ultimo aviso antes da multa!" }; break;
        7:  msgs := array{ "Toque de recolher! Retire-se das ruas agora!",
                           "Segundo aviso — saia das ruas!",
                           "Ultimo aviso antes da multa!" }; break;
        9:  msgs := array{ "Transformacoes sao proibidas! Retorne a sua forma!",
                           "Segundo aviso — retorne a sua forma natural!",
                           "Ultimo aviso antes da multa!" }; break;
        10: msgs := array{ "Animais nao sao permitidos aqui! Retire-o!",
                           "Segundo aviso — retire seu animal!",
                           "Ultimo aviso antes da multa!" }; break;
        11: msgs := array{ "Duelos sao proibidos neste territorio!",
                           "Segundo aviso — saia do modo de combate!",
                           "Ultimo aviso antes da multa!" }; break;
        14: msgs := array{ "Armadura pesada proibida aqui! Remova-a!",
                           "Segundo aviso — remova essa armadura!",
                           "Ultimo aviso antes da multa!" }; break;
        15: msgs := array{ "Itens magicos proibidos aqui! Remova-os!",
                           "Segundo aviso — remova os itens magicos!",
                           "Ultimo aviso antes da multa!" }; break;
        18: msgs := array{ "Suas vestes violam as normas daqui! Troque de roupa!",
                           "Segundo aviso — troque de roupa!",
                           "Ultimo aviso antes da multa!" }; break;
        19: msgs := array{ "Visao arcana proibida a noite sem lanterna!",
                           "Segundo aviso — apague a visao ou pegue uma lanterna!",
                           "Ultimo aviso antes da multa!" }; break;
        20: msgs := array{ "Proibido correr neste territorio! Reduza o passo!",
                           "Segundo aviso — reduza a velocidade!",
                           "Ultimo aviso antes da multa!" }; break;
        21: msgs := array{ "Cavalos andam no passo dentro das muralhas!",
                           "Segundo aviso — reduza a velocidade!",
                           "Ultimo aviso antes da multa!" }; break;
        24: msgs := array{ "Luta desarmada proibida aqui!",
                           "Segundo aviso — pare com a luta!",
                           "Ultimo aviso antes da multa!" }; break;
        29: msgs := array{ "Cultivo e colheita proibidos neste territorio!",
                           "Segundo aviso — pare imediatamente!",
                           "Ultimo aviso antes da multa!" }; break;
        26: msgs := array{ "Pocoes proibidas neste territorio!",
                           "Segundo aviso — pare de usar pocoes!",
                           "Ultimo aviso antes da multa!" }; break;
        30: msgs := array{ "Musica proibida aqui agora! Pare imediatamente!",
                           "Segundo aviso — pare de tocar!",
                           "Ultimo aviso antes da multa!" }; break;
        31: msgs := array{ "Dispersem imediatamente! Reuniao nao autorizada!",
                           "Segundo aviso — dispersem ou serao multados!",
                           "Ultimo aviso — proximo e multa!" }; break;
        32: msgs := array{ "INFECTADO! Voce tem 60 segundos para se curar antes de ser expulso!",
                           "Trate-se agora! O tempo esta esgotando!",
                           "Ultima chance — cure-se ou sera removido da cidade!" }; break;
        35: msgs := array{ "Cubra o rosto ao tossir! Voce precisa de um lenco!",
                           "Segundo aviso — adquira um lenco na Liga!",
                           "Ultimo aviso — proxima infração sera multada!" }; break;
        39: msgs := array{ "Proibido cortar madeira neste territorio!",
                           "Segundo aviso — pare com o lenhador!",
                           "Ultimo aviso antes da multa!" }; break;
        40: msgs := array{ "Proibido minerar neste territorio!",
                           "Segundo aviso — pare com a mineracao!",
                           "Ultimo aviso antes da multa!" }; break;
        41: msgs := array{ "Proibido pescar neste territorio!",
                           "Segundo aviso — recolha a vara de pescar!",
                           "Ultimo aviso antes da multa!" }; break;
        42: msgs := array{ "Proibido fazer pocoes alquimicas aqui!",
                           "Segundo aviso — guarde seus frascos!",
                           "Ultimo aviso antes da multa!" }; break;
        43: msgs := array{ "Proibido comer em via publica neste territorio!",
                           "Segundo aviso — guarde a comida!",
                           "Ultimo aviso antes da multa!" }; break;
        23: msgs := array{ "Proibido jogar lixo no territorio! Recolha agora!",
                           "Segundo aviso — recolha o lixo ou sera multado!",
                           "Ultimo aviso antes da multa!" }; break;
        default:
            msgs := array{ "Voce esta violando as leis deste territorio!",
                           "Segundo aviso — corrija sua conduta!",
                           "Ultimo aviso antes da multa!" }; break;
    endcase

    var idx := warn_tries + 1;
    if (idx > msgs.size()) idx := msgs.size(); endif
    PrintTextAbove(me, msgs[idx]);
endfunction

// ============================================================
// Gump universal de multa
// ============================================================
function _FineGump(who, amount, tier, wt)
    var coins := _CountCoins(who);
    if (coins < amount)
        PrintTextAbove(me, "Sem moedas para pagar a multa! Prepara-se para o combate!");
        SetObjProperty(who, "#pending_tier_{}".format(guild.guildid), tier);
        EngageViolator(who);
        disableWarn();
        return 0;
    endif

    var gump := GFCreateGump(180, 180);
    GFResizePic(gump, 0, 0, 9270, 400, 235);
    GFTextMid(gump, 0, 12, 400, 1153, "Multa — {} Moedas de Cobre".format(amount));
    GFHtmlArea(gump, 20, 45, 360, 110,
        "<basefont color=#FFFFFF>Voce infringiu as leis deste territorio.<br>" +
        "Pague agora <b>{} moedas de cobre</b> ou sera atacado e levado a prisao.<br>".format(amount) +
        "O valor vai direto ao bau da faccao.<br>" +
        "<basefont color=#FFD700>Recusar ou ignorar = combate imediato.", 0);
    GFAddButton(gump, 55, 192, 4005, 4007, GF_CLOSE_BTN, 1);
    GFTextLine(gump, 90, 194, 2100, "Aceitar e pagar");
    GFAddButton(gump, 265, 192, 4017, 4019, GF_CLOSE_BTN, 2);
    GFTextLine(gump, 300, 194, 33, "Recusar");

    PrintTextAbove(me, "Pague a multa ou prepare-se para o combate!");
    me.frozen  := 1;
    who.frozen := 1;
    var result := GFSendGump(who, gump);
    me.frozen  := 0;
    who.frozen := 0;

    if (!result || result[0] != 1)
        PrintTextAbove(me, "Recusou a multa! Guardas, a postos!");
        SetObjProperty(who, "#pending_tier_{}".format(guild.guildid), tier);
        EngageViolator(who);
        disableWarn();
        return 0;
    endif

    _TakeCoins(who, amount);
    var atual := CInt(guild.GetProp("money"));
    guild.SetProp("money", atual + amount);
    AddReport(guild, "{} pagou multa de {} moedas".format(who.name, amount));
    SetObjProperty(who, "#multa_paga_{}".format(guild.guildid), polcore().systime + 60);
    PrintTextAbove(me, "Multa paga. Pode seguir por 1 minuto sem ser abordado.");
    SendSysMessageEx(who, "Voce pagou {} moedas de multa ao bau da faccao.".format(amount), SSM_INFO);
    disableWarn();
    return 1;
endfunction

// ============================================================
// Fluxo: EXPULSAO (warn_type 2 — nooutsider, norace, noguild, userban)
// ============================================================
function _HandleExpulsion()
    if (!inGuildRect(guild, warning, 0))
        PrintTextAbove(me, "Muito bem. Foi embora.");
        AddReport(guild, "{} alertou {} — saiu voluntariamente".format(me.name, warning.name));
        disableWarn();
        return;
    endif

    if (warn_tries < 3)
        case (warn_tries)
            0: PrintTextAbove(me, "Voce nao e bem-vindo neste territorio! Retire-se imediatamente!"); break;
            1: PrintTextAbove(me, "Segundo aviso — saia agora ou sera expulso!"); break;
            2: PrintTextAbove(me, "Ultimo aviso — proxima vez sera expulso a forca!"); break;
        endcase
        SetCooldown(me, "last_warn_{}".format(warning.serial), 5);
        warn_tries += 1;
        return;
    endif

    var reason := CStr(warn_opt) ? CStr(warn_opt) : "PRESENCA NAO AUTORIZADA";
    MoveObjectToLocation(warning, EXPULSION_X, EXPULSION_Y, EXPULSION_Z,
        EXPULSION_REALM, MOVEOBJECT_FORCELOCATION);
    SendSysMessageEx(warning,
        "VOCE FOI EXPULSO DE VALTHERYS! MELHOR NAO VOLTAR. {} NAO EH BEM-VINDA AQUI.".format(reason),
        SSM_HARMFUL);
    AddReport(guild, "{} expulsou {} de Valtherys".format(me.name, warning.name));
    disableWarn();
endfunction

// ============================================================
// Fluxo: PEDAGIO (warn_type 12 — notax_exempt)
// ============================================================
function _HandleToll()
    var toll := CInt(warn_opt);
    if (toll <= 0) toll := 50; endif

    var paid_key := "toll_paid_{}".format(guild.guildid);
    if (CInt(GetObjProperty(warning, paid_key)) > polcore().systime - 600)
        disableWarn(); return;
    endif

    var tipo := CStr(guild.getProp("staff_tipo"));

    // Rufioes + player correndo = perseguicao e roubo
    if (tipo == "rufioes" && warning.running)
        SetObjProperty(warning, "#rufiao_robbery_{}".format(guild.guildid), 1);
        SetObjProperty(warning, "#pending_tier_{}".format(guild.guildid), 3);
        PrintTextAbove(me, "*persegue o fugitivo*");
        EngageViolator(warning);
        disableWarn();
        return;
    endif

    PrintTextAbove(me, "Pedagio! {} moedas de cobre para passar!".format(toll));

    var coins := _CountCoins(warning);
    if (coins >= toll)
        var gump := GFCreateGump(180, 180);
        GFResizePic(gump, 0, 0, 9270, 360, 195);
        GFTextMid(gump, 0, 12, 360, 1153, "Pedagio — {} Moedas".format(toll));
        GFHtmlArea(gump, 20, 40, 320, 100,
            "<basefont color=#FFFFFF>Este territorio e privado.<br>" +
            "Pague o pedagio de <b>{} moedas</b> para ter acesso.<br>".format(toll) +
            "Recusar resulta em bloqueio e possivel ataque.", 0);
        GFAddButton(gump, 50, 153, 4005, 4007, GF_CLOSE_BTN, 1);
        GFTextLine(gump, 85, 155, 2100, "Pagar pedagio");
        GFAddButton(gump, 230, 153, 4017, 4019, GF_CLOSE_BTN, 2);
        GFTextLine(gump, 265, 155, 33, "Recusar");

        var result := GFSendGump(warning, gump);

        if (result && result[0] == 1)
            _TakeCoins(warning, toll);
            var atual := CInt(guild.GetProp("money"));
            guild.SetProp("money", atual + toll);
            SetObjProperty(warning, paid_key, polcore().systime);
            AddReport(guild, "{} pagou pedagio de {} moedas".format(warning.name, toll));
            PrintTextAbove(me, "Pode passar.");
            disableWarn();
            return;
        endif
    endif

    PrintTextAbove(me, "Nao passa sem pagar!");
    SetObjProperty(warning, "#pending_tier_{}".format(guild.guildid), 3);
    EngageViolator(warning);
    disableWarn();
endfunction

// ============================================================
// Fluxo: MAGIA (warn_type 8, 16, 17 — nospell*)
// ============================================================
function _HandleSpellViolation()
    var reoffense_key := "#spell_warned_{}".format(guild.guildid);
    var prev_warn     := CInt(GetObjProperty(warning, reoffense_key));

    if (prev_warn && polcore().systime - prev_warn < 120)
        PrintTextAbove(me, "Reincidente! Magia proibida usada novamente. Guardem!");
        SetObjProperty(warning, "#pending_tier_{}".format(guild.guildid), 3);
        EngageViolator(warning);
        disableWarn();
        return;
    endif

    PrintTextAbove(me, "E PROIBIDO ESSE TIPO DE MAGIA EM NOSSA CIDADE!");
    Sleep(1);
    PrintTextAbove(me, "VOCE SERA MULTADO. NA PROXIMA VEZ SERA PRESO!");
    SetObjProperty(warning, reoffense_key, polcore().systime);
    _FineGump(warning, FINE_AMOUNT, 3, warn_type);
endfunction

// ============================================================
// Fluxo: SAQUE DE DESMAIADO (warn_type 28 — noloot_unconscious)
// ============================================================
function _HandleLootViolation()
    var loot_ts := CInt(GetObjProperty(warning, "#loot_unconscious"));
    if (!loot_ts || polcore().systime - loot_ts >= 30)
        disableWarn(); return;
    endif

    PrintTextAbove(me, "Alto ai! Flagrado saqueando um desmaiado! Isso e crime neste territorio!");
    Sleep(1);

    var gump := GFCreateGump(180, 180);
    GFResizePic(gump, 0, 0, 9270, 450, 240);
    GFTextMid(gump, 0, 12, 450, 1153, "Flagrante — Saque de Desmaiado");
    GFHtmlArea(gump, 20, 40, 410, 120,
        "<basefont color=#FFFFFF>Voce foi flagrado saqueando um combatente desmaiado.<br>" +
        "Ao aceitar a multa de <b>{} moedas</b>, se compromete a devolver o que levou.<br>".format(FINE_AMOUNT) +
        "Ao recusar, sera atacado e levado a prisao.<br>" +
        "<basefont color=#FFD700>Multa: {} moedas. Na prisao: {} moedas.".format(FINE_AMOUNT, FINE_PRISON), 0);
    GFAddButton(gump, 50, 195, 4005, 4007, GF_CLOSE_BTN, 1);
    GFTextLine(gump, 85, 197, 2100, "Aceitar — pagar e devolver item");
    GFAddButton(gump, 300, 195, 4017, 4019, GF_CLOSE_BTN, 2);
    GFTextLine(gump, 335, 197, 33, "Recusar");

    var result := GFSendGump(warning, gump);

    if (result && result[0] == 1)
        var coins := _CountCoins(warning);
        if (coins >= FINE_AMOUNT)
            _TakeCoins(warning, FINE_AMOUNT);
            var atual := CInt(guild.GetProp("money"));
            guild.SetProp("money", atual + FINE_AMOUNT);
            AddReport(guild, "{} pagou multa por saque de desmaiado".format(warning.name));
            EraseObjProperty(warning, "#loot_unconscious");
            SetObjProperty(warning, "#multa_paga_{}".format(guild.guildid), polcore().systime + 60);
            PrintTextAbove(me, "Multa paga. Voce se comprometeu a devolver o que levou.");
            SendSysMessageEx(warning, "Devolva os itens do desmaiado. Voce esta sendo observado.", SSM_HARMFUL);
        else
            PrintTextAbove(me, "Sem moedas! Vai para a prisao!");
            SetObjProperty(warning, "#pending_tier_{}".format(guild.guildid), 3);
            EngageViolator(warning);
        endif
    else
        PrintTextAbove(me, "Recusou! Guardem!");
        SetObjProperty(warning, "#pending_tier_{}".format(guild.guildid), 3);
        EngageViolator(warning);
    endif

    disableWarn();
endfunction

// ============================================================
// Fluxo: TROCA SUSPEITA DE ROUPA — revista voluntaria (warn_type 33)
// ============================================================
function _HandleSearchViolation()
    var ect := CInt(GetObjProperty(warning, "#equip_change_time"));
    if (!ect || polcore().systime - ect > 30)
        EraseObjProperty(warning, "#equip_changes");
        disableWarn(); return;
    endif

    TurnToward(me, warning);
    PrintTextAbove(me, "{}, fique onde esta! Revista de rotina.".format(warning.name));
    Sleep(1);

    var gump := GFCreateGump(180, 180);
    GFResizePic(gump, 0, 0, 9270, 420, 210);
    GFTextMid(gump, 0, 12, 420, 1153, "Revista de Rotina");
    GFHtmlArea(gump, 20, 40, 380, 110,
        "<basefont color=#FFFFFF>O guarda solicita uma revista. Notou troca rapida de trajes.<br>" +
        "<b>Aceitar</b>: o guarda inspeciona e segue em frente.<br>" +
        "<b>Recusar ou fugir</b>: suspeito elevado. Sera atacado e levado a prisao.", 0);
    GFAddButton(gump, 70, 168, 4005, 4007, GF_CLOSE_BTN, 1);
    GFTextLine(gump, 105, 170, 2100, "Aceitar revista");
    GFAddButton(gump, 270, 168, 4017, 4019, GF_CLOSE_BTN, 2);
    GFTextLine(gump, 305, 170, 33, "Recusar");

    var result := GFSendGump(warning, gump);

    EraseObjProperty(warning, "#equip_changes");
    EraseObjProperty(warning, "#equip_change_time");

    if (result && result[0] == 1)
        PrintTextAbove(me, "Nunca troque de roupa no meio da cidade. Entendido?");
        Sleep(1);
        PrintTextAbove(me, "Pode seguir. Fique atento as normas.");
        AddReport(guild, "{} revistou {} (troca suspeita)".format(me.name, warning.name));
    else
        PrintTextAbove(me, "Recusou a revista! Suspeito confirmado! Guardem!");
        SetObjProperty(warning, "#pending_tier_{}".format(guild.guildid), 2);
        EngageViolator(warning);
    endif

    disableWarn();
endfunction

// ============================================================
// Fluxo: ESCOLTA — leimarcia (13), antialcool (22), nodrugs (27)
// ============================================================
function _HandleEscortFlow()
    var active := 0;
    var reason := "";
    case (warn_type)
        13:
            active := (GetObjProperty(warning, "carteira_faccao_{}".format(guild.guildid)) == error);
            reason := "sem documentos de identificacao";
            break;
        22:
            active := (GetObjProperty(warning, "baderneiro") != error &&
                       CInt(GetObjProperty(warning, "baderneiro")) > polcore().systime);
            reason := "perturbando a ordem publica (embriaguez)";
            break;
        27:
            var _ld27 := CInt(GetObjProperty(warning, "#lastdrug"));
            active := (_ld27 && polcore().systime - _ld27 < 60);
            reason := "uso de substancias proibidas";
            break;
    endcase

    if (!active) disableWarn(); return; endif

    PrintTextAbove(me, "Senhor, voce esta {}. Multa de {} moedas.".format(reason, FINE_AMOUNT));
    Sleep(1);

    var coins := _CountCoins(warning);
    if (coins >= FINE_AMOUNT)
        var gump := GFCreateGump(180, 180);
        GFResizePic(gump, 0, 0, 9270, 450, 250);
        GFTextMid(gump, 0, 12, 450, 1153, "Multa — {} Moedas".format(FINE_AMOUNT));
        GFHtmlArea(gump, 20, 42, 410, 120,
            "<basefont color=#FFFFFF>Infracao: <b>{}</b>.<br>".format(reason) +
            "Multa: <b>{} moedas de cobre</b>.<br>".format(FINE_AMOUNT) +
            "Ao pagar, ficara 1 minuto sem ser abordado novamente.<br>" +
            "Ao recusar, podera escolher ser escoltado ou resistir.", 0);
        GFAddButton(gump, 60, 205, 4005, 4007, GF_CLOSE_BTN, 1);
        GFTextLine(gump, 95, 207, 2100, "Aceitar e pagar");
        GFAddButton(gump, 280, 205, 4017, 4019, GF_CLOSE_BTN, 2);
        GFTextLine(gump, 315, 207, 33, "Recusar");

        var result := GFSendGump(warning, gump);

        if (result && result[0] == 1)
            _TakeCoins(warning, FINE_AMOUNT);
            var atual := CInt(guild.GetProp("money"));
            guild.SetProp("money", atual + FINE_AMOUNT);
            AddReport(guild, "{} pagou multa por {} ({} cobre)".format(warning.name, reason, FINE_AMOUNT));
            SetObjProperty(warning, "#multa_paga_{}".format(guild.guildid), polcore().systime + 60);
            PrintTextAbove(me, "Multa paga. Pode seguir.");
            disableWarn();
            return;
        endif
    else
        SendSysMessageEx(warning, "Voce nao tem moedas suficientes para a multa.", SSM_HARMFUL);
    endif

    // Sem moedas ou recusou — opcao de escolta pacifica
    var escort_gump := GFCreateGump(180, 180);
    GFResizePic(escort_gump, 0, 0, 9270, 430, 210);
    GFTextMid(escort_gump, 0, 12, 430, 1153, "Escolta para a Prisao");
    GFHtmlArea(escort_gump, 20, 40, 390, 110,
        "<basefont color=#FFFFFF>" +
        ((coins < FINE_AMOUNT) ? "Sem moedas suficientes.<br>" : "Multa recusada.<br>") +
        "Aceite ser escoltado pacificamente ou resista e sera atacado.<br>" +
        "<basefont color=#FFD700>Sem dinheiro? Sera solto em 1 hora (infracao leve).", 0);
    GFAddButton(escort_gump, 65, 168, 4005, 4007, GF_CLOSE_BTN, 1);
    GFTextLine(escort_gump, 100, 170, 2100, "Aceitar escolta");
    GFAddButton(escort_gump, 270, 168, 4017, 4019, GF_CLOSE_BTN, 2);
    GFTextLine(escort_gump, 305, 170, 33, "Resistir");

    var escort_result := GFSendGump(warning, escort_gump);

    if (escort_result && escort_result[0] == 1)
        PrintTextAbove(me, "Escoltando para a prisao pacificamente.");
        AddReport(guild, "{} escoltou {} ({})".format(me.name, warning.name, reason));
        TeleportToJail(warning, guild, warn_tier);
    else
        PrintTextAbove(me, "Resistindo! Guardem o territorio!");
        SetObjProperty(warning, "#pending_tier_{}".format(guild.guildid), warn_tier);
        EngageViolator(warning);
    endif

    disableWarn();
endfunction

// ============================================================
// Fluxo: QUARENTENA (warn_type 32) — expulsao de infectados
// ============================================================
function _HandleQuarentena()
    var sick := warning.poisoned || GetObjProperty(warning, "Diseases_System") != error;
    if (!sick)
        EraseObjProperty(warning, "#quarentena_{}".format(guild.guildid));
        disableWarn(); return;
    endif

    var timer_key := "#quarentena_{}".format(guild.guildid);
    var timer_val := CInt(GetObjProperty(warning, timer_key));

    if (!timer_val)
        // Primeiro contato: dar 60s
        SetObjProperty(warning, timer_key, polcore().systime + 60);
        PrintTextAbove(me, "INFECTADO! " + warning.name + " tem 60 segundos para se curar!");
        SendSysMessageEx(warning,
            "Voce esta infectado! Cure-se em 60 segundos ou sera expulso da cidade!",
            SSM_HARMFUL);
        disableWarn(); return;
    endif

    if (polcore().systime < timer_val)
        var restante := timer_val - polcore().systime;
        SendSysMessageEx(warning, "{} segundos para se curar!".format(restante), SSM_HARMFUL);
        disableWarn(); return;
    endif

    // Timer expirado e ainda doente → expulsar
    EraseObjProperty(warning, timer_key);
    PrintTextAbove(me, "Infectado removido para quarentena!");
    MoveObjectToLocation(warning, EXPULSION_X, EXPULSION_Y, EXPULSION_Z,
        EXPULSION_REALM, MOVEOBJECT_FORCELOCATION);
    SendSysMessageEx(warning,
        "Voce foi expulso por estar infectado. Trate-se antes de retornar.",
        SSM_HARMFUL);
    AddReport(guild, "{} expulsou {} por quarentena".format(me.name, warning.name));
    disableWarn();
endfunction

include ":faccao:include/npc_door_scan";

// ============================================================
// Fluxo: IDENTIDADE IDEOLOGICA (warn_type 38)
// ============================================================
function _HandleIdentityFraud()
    if (!_CheckDocFraud(warning, guild)) disableWarn(); return; endif

    PrintTextAbove(me, "FRAUDE DE IDENTIDADE! " + warning.name + " usou documento falso!");
    Sleep(1);

    // Confiscar carta falsa
    var carta := _FindCarta(warning, guild.guildid);
    if (carta) DestroyItem(carta); endif
    EraseObjProperty(warning, "carteira_faccao_{}".format(guild.guildid));

    SendSysMessageEx(warning,
        "Sua identidade falsa foi descoberta e confiscada. Voce sera preso!",
        SSM_HARMFUL);
    AddReport(guild, "Fraude de identidade: {} usou documento emitido sob disfarce".format(warning.name));

    // Preso diretamente — nao ha negociacao para identidade ideologica
    SetObjProperty(warning, "#pending_tier_{}".format(guild.guildid), 3);
    EngageViolator(warning);
    disableWarn();
endfunction

// ============================================================
// Roubo dos Rufioes — executado quando o alvo e desmaiado com flag
// ============================================================
function _DoRufiaoRobbery(targ)
    EraseObjProperty(targ, "#rufiao_robbery_{}".format(guild.guildid));

    var stolen_gold := 0;
    var roubados    := 0;
    var bp := targ.backpack;
    if (bp)
        foreach item in EnumerateItemsInContainer(bp)
            sleepms(2);
            if (item.objtype == 0xba63)
                stolen_gold += item.amount; DestroyItem(item);
            elseif (item.objtype == 0xba64)
                stolen_gold += item.amount * 10; DestroyItem(item);
            elseif (item.objtype == 0xba65)
                stolen_gold += item.amount * 100; DestroyItem(item);
            endif
        endforeach
        if (stolen_gold > 0)
            var atual := CInt(guild.GetProp("money"));
            guild.SetProp("money", atual + stolen_gold);
        endif

        var itens_roubaveis := array{};
        foreach item in EnumerateItemsInContainer(bp)
            sleepms(2);
            if (!item.newbie && !item.isA(POLCLASS_CONTAINER))
                itens_roubaveis.append(item);
            endif
        endforeach
        while (roubados < 2 && itens_roubaveis.size() > 0)
            sleepms(2);
            var idx := RandomInt(itens_roubaveis.size()) + 1;
            var it  := itens_roubaveis[idx];
            if (it && it.isA(POLCLASS_ITEM) && me.backpack)
                MoveItemToContainer(it, me.backpack);
                roubados += 1;
            endif
            itens_roubaveis.Erase(idx);
        endwhile
    endif

    MoveObjectToLocation(targ, RUFIAO_DUMP_X, RUFIAO_DUMP_Y, RUFIAO_DUMP_Z,
        "britannia_alt", MOVEOBJECT_FORCELOCATION);
    AddReport(guild, "Rufioes roubaram {} ({} cobre + {} itens)".format(
        targ.name, stolen_gold, roubados));
    SendSysMessageEx(targ,
        "Voce foi roubado pelos Rufioes! {} moedas e {} itens foram levados.".format(
            stolen_gold, roubados), SSM_HARMFUL);
endfunction

// ============================================================
// CheckKnockedOut — chamar do loop de combate do NPC quando fighting.hits <= 0
// ============================================================
function CheckKnockedOut()
    if (!fighting || !fighting.isA(POLCLASS_MOBILE)) return; endif
    if (fighting.npctemplate) return; endif  // NPCs nao tem KO nem sao mandados para prisao
    if (fighting.hits > 0) return; endif
    // Exige que o player esteja de fato morto/desmaiado — evita falso positivo quando hits=uninitialized
    if (!fighting.dead) return; endif

    var targ := fighting;
    var tipo := CStr(guild.getProp("staff_tipo"));

    if (tipo == "rufioes" &&
            GetObjProperty(targ, "#rufiao_robbery_{}".format(guild.guildid)) != error)
        fighting := 0;
        state    := 1;
        SetWarMode(0);
        _DoRufiaoRobbery(targ);
        return;
    endif

    var tier := CInt(GetObjProperty(targ, "#pending_tier_{}".format(guild.guildid)));

    if (!tier || tier < 1 || tier > 3)
        fighting := 0;
        state    := 0;
        SetOpponent(0);
        SetWarMode(0);
        return;
    endif

    fighting := 0;
    state    := 1;
    SetWarMode(0);
    EraseObjProperty(targ, "#pending_tier_{}".format(guild.guildid));
    TeleportToJail(targ, guild, tier);
endfunction

// ============================================================
// _HandleLitter — multa por lixo no chao (warn_type 23)
// ============================================================
function _HandleLitter()
    if (!warn_opt || !warn_opt.isA(POLCLASS_ITEM)) disableWarn(); return; endif
    var item := warn_opt;
    if (item.container != error)   // item foi recolhido
        EraseObjProperty(item, "dropped_by");
        disableWarn(); return;
    endif
    if (CInt(GetObjProperty(item, "dropped_by")) != warning.serial)
        disableWarn(); return;
    endif

    TurnToward(me, warning);
    PrintTextAbove(me, "Recolha esse lixo do chao imediatamente!");
    Sleep(1);

    var gump := GFCreateGump(180, 180);
    GFResizePic(gump, 0, 0, 9270, 400, 215);
    GFTextMid(gump, 0, 12, 400, 1153, "Lixo no Chao — Multa");
    GFHtmlArea(gump, 20, 40, 360, 110,
        "<basefont color=#FFFFFF>Voce descartou lixo no territorio.<br>" +
        "Pague <b>{} moedas</b> de multa ou recolha o item agora.<br>".format(FINE_AMOUNT) +
        "Recusar = combate.", 0);
    GFAddButton(gump, 50, 172, 4005, 4007, GF_CLOSE_BTN, 1);
    GFTextLine(gump, 85, 174, 2100, "Pagar multa");
    GFAddButton(gump, 220, 172, 4017, 4019, GF_CLOSE_BTN, 2);
    GFTextLine(gump, 255, 174, 33, "Recusar");

    var result := GFSendGump(warning, gump);

    if (result && result[0] == 1)
        if (_TakeCoins(warning, FINE_AMOUNT))
            var atual := CInt(guild.GetProp("money"));
            guild.SetProp("money", atual + FINE_AMOUNT);
            AddReport(guild, "{} pagou multa de lixo ({} cobre)".format(warning.name, FINE_AMOUNT));
            EraseObjProperty(item, "dropped_by");
            SetObjProperty(warning, "#multa_paga_{}".format(guild.guildid), polcore().systime + 60);
            PrintTextAbove(me, "Multa paga. Mantenha a cidade limpa.");
        else
            PrintTextAbove(me, "Sem moedas! Para a prisao!");
            SetObjProperty(warning, "#pending_tier_{}".format(guild.guildid), warn_tier);
            EngageViolator(warning);
        endif
    else
        PrintTextAbove(me, "Recusou! Guardem!");
        SetObjProperty(warning, "#pending_tier_{}".format(guild.guildid), warn_tier);
        EngageViolator(warning);
    endif

    disableWarn();
endfunction

// ============================================================
// WarnViolator — fluxo principal de aviso/punicao
// ============================================================
function WarnViolator()
    if (fighting || !warning) return; endif
    if (!warning.isA(POLCLASS_MOBILE) || warning.concealed || warning.hidden || warning.dead)
        disableWarn(); return;
    endif
    if (GetCooldown(me, "last_warn_{}".format(warning.serial))) return; endif

    // Imunidade pos-multa
    if (CInt(GetObjProperty(warning, "#multa_paga_{}".format(guild.guildid))) > polcore().systime)
        disableWarn(); return;
    endif

    // --- LEIS COM FLUXO PROPRIO ---
    case (warn_type)
        2:          _HandleExpulsion(); return;
        12:         _HandleToll(); return;
        8: 16: 17:  _HandleSpellViolation(); return;
        28:         _HandleLootViolation(); return;
        33:         _HandleSearchViolation(); return;
        13: 22: 27: _HandleEscortFlow(); return;
        32:         _HandleQuarentena(); return;
        38:         _HandleIdentityFraud(); return;
        23:         _HandleLitter(); return;
    endcase

    // --- FLUXO UNIVERSAL: 3 AVISOS → GUMP DE MULTA ---

    if (!_CheckViolationActive(warn_type)) return; endif

    if (warn_tries < 3)
        _PrintWarning(warn_type);
        SetCooldown(me, "last_warn_{}".format(warning.serial), 5);
        warn_tries += 1;
        return;
    endif

    _FineGump(warning, FINE_AMOUNT, warn_tier, warn_type);
endfunction

// ============================================================
// HandleLawEvent — eventos customizados de lei
// ============================================================
function HandleLawEvent(ev)
    case (ev.type)

        PICKPOCKET_EVENT:
            if (!CheckIfHavePlayerGuild_Law(ListMobilesInLineOfSight(me, 8)))
                AddReport(guild, "{} percebeu {} tentando cometer furto".format(me.name, ev.source.name));
                PrintTextAbove(me, "Ei! O que pensa que esta fazendo?");
                SetObjProperty(ev.source, "#pending_tier_{}".format(guild.guildid), 3);
                EngageViolator(ev.source);
                disableWarn();
            endif
            break;

        REVELADO_EVENT:
            if (!CheckIfHavePlayerGuild_Law(ListMobilesInLineOfSight(me, 8)))
                AddReport(guild, "{} encontrou {} tentando se esconder".format(me.name, ev.source.name));
                PrintTextAbove(me, "Ei! O que pensa que esta fazendo?");
                SetObjProperty(ev.source, "#pending_tier_{}".format(guild.guildid), 3);
                EngageViolator(ev.source);
                disableWarn();
            endif
            break;

        LOCKPICK_EVENT:
            if (!CheckIfHavePlayerGuild_Law(ListMobilesInLineOfSight(me, 8)))
                if (FindInArray(guild_laws, struct{ "type" := "nolockpick" }))
                    AddReport(guild, "{} alertou {} sobre arrombar fechaduras".format(me.name, ev.source.name));
                    PrintTextAbove(me, "Ei! O que pensa que esta fazendo?");
                    SetObjProperty(ev.source, "#pending_tier_{}".format(guild.guildid), 3);
                    EngageViolator(ev.source);
                    disableWarn();
                endif
            endif
            break;

        CERCA_EVENT:
            if (!CheckIfHavePlayerGuild_Law(ListMobilesInLineOfSight(me, 8)))
                if (FindInArray(guild_laws, struct{ "type" := "nocerca" }))
                    AddReport(guild, "{} alertou {} sobre pular cercas".format(me.name, ev.source.name));
                    PrintTextAbove(me, "Ei! O que pensa que esta fazendo?");
                    SetObjProperty(ev.source, "#pending_tier_{}".format(guild.guildid), 3);
                    EngageViolator(ev.source);
                    disableWarn();
                endif
            endif
            break;

        RESET_SETUP_EVENT:
            guild_leader := guild.getProp("leader");
            guild_laws   := guild.getProp("laws");
            if (!guild_laws) guild_laws := array{}; endif
            break;

    endcase
endfunction

// ============================================================
// Gump compartilhado de controle — todos os NPCs de combate de faccao
// ============================================================

// Movimenta o NPC ate o destino via FindPath.
function _FaccaoNPCGoTo(dest)
    var flags := FP_IGNORE_MOBILES;
    var path  := FindPath(me.x, me.y, me.z, dest.x, dest.y, dest.z, me.realm, flags, 15);
    var delay := CInt(275 - me.run_speed);
    if (delay < 30) delay := 30; endif
    foreach coord in path
        RunTowardLocation(coord.x, coord.y);
        sleepms(delay);
    endforeach
endfunction

// Define rota de patrulha via props (compativel com todos os NPCs).
function _FaccaoNPCSetPatrol(who)
    SendSysMessageEx(who, "Escolha o primeiro ponto de patrulha.", SSM_INFO);
    var pt1 := TargetCoordinates(who);
    if (!pt1) SendSysMessageEx(who, "Cancelado.", SSM_FAIL); return; endif
    if (GetObjProperty(me, "guild") && !IsNPCInWalkRect(pt1))
        SendSysMessageEx(who, "Ponto fora do territorio. Cancelado.", SSM_FAIL); return;
    endif

    SendSysMessageEx(who, "Escolha o segundo ponto de patrulha.", SSM_INFO);
    var pt2 := TargetCoordinates(who);
    if (!pt2) SendSysMessageEx(who, "Cancelado.", SSM_FAIL); return; endif
    if (GetObjProperty(me, "guild") && !IsNPCInWalkRect(pt2))
        SendSysMessageEx(who, "Ponto fora do territorio. Cancelado.", SSM_FAIL); return;
    endif
    if (CoordinateDistance(pt1.x, pt1.y, pt2.x, pt2.y) > 20)
        SendSysMessageEx(who, "Pontos muito distantes (max 20 tiles). Cancelado.", SSM_FAIL); return;
    endif

    var patrol := array{ pt1, pt2 };
    fix_location := 0;
    EraseObjProperty(me, "fix_loc");
    SetObjProperty(me, "patrolling", patrol);
    var dist := CoordinateDistance(pt1.x, pt1.y, pt2.x, pt2.y);
    PrintTextAbove(me, "Irei patrulhar. Distancia entre pontos: {} tiles.".format(dist));
endfunction

// Verifica se 'who' é um alvo válido para NPCs de combate do Sinodo/Rufioes/assassinos.
// Aceita: players nao-membros, animais tamados e montarias de players nao-membros.
// Rejeita: membros da faccao, NPCs sem dono, NPCs aliados.
function IsValidCombatTarget(who)
    if (!who || !who.isA(POLCLASS_MOBILE)) return 0; endif
    if (who == me || who.dead || who.concealed)  return 0; endif

    // Bloqueio cross-realm: NPC de faccao nunca persegue alvo em realm diferente
    // do realm canonico da guild. Evita travessia de moongate/teleporter.
    if (guild)
        var _g_realm := GetGuildRealm(guild);
        if (_g_realm && who.realm && who.realm != _g_realm) return 0; endif
    endif

    if (who.npctemplate)
        // NPC da propria faccao (ex: pantera do cazador) — nunca atacar
        if (guild && GetObjProperty(who, "guild") == guild.guildid) return 0; endif
        // Aceitar animais tamados/montarias cujo dono seja alvo valido
        var owner := who.master;
        if (!owner || !owner.isA(POLCLASS_MOBILE)) return 0; endif
        if (owner == me || owner.dead)             return 0; endif
        if (guild && IsMemberOfFaccao(guild, owner)) return 0; endif
        // Dono é NPC da propria faccao (ex: cazador)
        if (guild && GetObjProperty(owner, "guild") == guild.guildid) return 0; endif
        return 1;
    endif

    // Player
    if (guild && IsMemberOfFaccao(guild, who)) return 0; endif
    return 1;
endfunction

// Abre target para o jogador escolher o posto fixo, move o NPC ate la e salva.
function DefinirPostoFixo(who)
    SendSysMessageEx(who, "Clique no local que sera o posto fixo.", SSM_INFO);
    var dest := TargetCoordinates(who);
    if (!dest) SendSysMessageEx(who, "Cancelado.", SSM_FAIL); return; endif
    if (GetObjProperty(me, "guild") && !IsNPCInWalkRect(dest))
        SendSysMessageEx(who, "Ponto fora do territorio da faccao. Cancelado.", SSM_FAIL); return;
    endif

    fighting := 0; following := 0; warning := 0;
    SetOpponent(0); SetWarMode(0);
    EraseObjProperty(me, "patrolling");

    _FaccaoNPCGoTo(dest);

    fix_location := struct{ "x" := me.x, "y" := me.y, "z" := me.z, "realm" := me.realm, "facing" := me.facing };
    SetObjProperty(me, "fix_loc", fix_location);
    PrintTextAbove(me, "Posto fixado aqui. Voltarei aqui apos combate.");
    SendSysMessageEx(who, "Posto fixo definido em ({}, {}).".format(me.x, me.y), SSM_INFO);
endfunction

// Sub-gump de facing — reutilizavel por qualquer NPC.
function ShowFaccaoFacingGump(who)
    var gump := GFCreateGump(220, 70);
    GFResizePic(gump, 0, 0, 0x254E, 125, 100);

    GFTextLine(gump, 30, 15, 2100, "Norte");
    GFTooltipText(gump, "Olhar para o Norte");
    GFAddButton(gump, 15, 17, 2117, 2118, 1, 100);

    GFTextLine(gump, 30, 35, 2100, "Sul");
    GFTooltipText(gump, "Olhar para o Sul");
    GFAddButton(gump, 15, 37, 2117, 2118, 1, 110);

    GFTextLine(gump, 30, 55, 2100, "Leste");
    GFTooltipText(gump, "Olhar para o Leste");
    GFAddButton(gump, 15, 57, 2117, 2118, 1, 120);

    GFTextLine(gump, 30, 75, 2100, "Oeste");
    GFTooltipText(gump, "Olhar para o Oeste");
    GFAddButton(gump, 15, 77, 2117, 2118, 1, 130);

    var input := GFSendGump(who, gump)[0];
    var new_facing := -1;
    case (input)
        100: new_facing := 0; break;
        110: new_facing := 4; break;
        120: new_facing := 2; break;
        130: new_facing := 6; break;
    endcase

    if (new_facing >= 0)
        me.facing := new_facing;
        if (fix_location)
            fix_location.facing := new_facing;
            SetObjProperty(me, "fix_loc", fix_location);
        endif
    endif
endfunction

// Sub-gump de revelacao — exibido apos ativar hunt_mode em NPCs de sombra.
function ShowRevealGump(who)
    var gump := GFCreateGump(220, 70);
    GFResizePic(gump, 0, 0, 0x254E, 125, 40);
    GFTextLine(gump, 30, 10, 2100, "Revele-se");
    GFTooltipText(gump, "Ordena o NPC a sair das sombras imediatamente.");
    GFAddButton(gump, 15, 12, 2117, 2118, 1, 1);
    var input := GFSendGump(who, gump)[0];
    if (input == 1)
        me.hidden := 0;
        EraseObjProperty(me, "shadow_mode");
        EraseObjProperty(me, "hunt_mode");
        SetObjProperty(me, "#reveal_ts", polcore().systime);
        PrintTextAbove(me, "*emerge das sombras*");
    endif
endfunction

function CheckAwarenessDetection(player)
    if (!me.hidden) return; endif
    if (!player.connected || player.dead) return; endif
    var _cd_key := "aware_" + CStr(player.serial);
    if (GetCooldown(me, _cd_key)) return; endif
    if (!CheckLineOfSight(player, me)) return; endif
    SetCooldown(me, _cd_key, 8);

    var sneak := CInt(AP_GetSkill(me, SNEAK));
    var dist  := Distance(player, me);
    var difficulty;
    if (dist >= 8)      difficulty := sneak;
    elseif (dist >= 7)  difficulty := CInt(sneak * 0.95);
    elseif (dist >= 6)  difficulty := CInt(sneak * 0.9);
    elseif (dist >= 5)  difficulty := CInt(sneak * 0.85);
    elseif (dist >= 4)  difficulty := CInt(sneak * 0.8);
    elseif (dist >= 3)  difficulty := CInt(sneak * 0.7);
    elseif (dist >= 2)  difficulty := CInt(sneak * 0.6);
    elseif (dist >= 1)  difficulty := CInt(sneak * 0.5);
    else                difficulty := CInt(sneak * 0.25);
    endif

    if (SkillCheck(player, AWARENESS, difficulty) > 0)
        me.hidden := 0;
        EraseObjProperty(me, "shadow_mode");
        EraseObjProperty(me, "hunt_mode");
        stand_fight := 1;
        PrintTextAbove(me, "*revelado*");
        SendSysMessageEx(player, "Você encontrou " + me.name + ".", SSM_INFO);
    endif
endfunction

// Em IDLE: se o NPC foi revelado e nenhuma ordem chegou em 60s, retorna as sombras.
function CheckRevealTimeout()
    var _rts := GetObjProperty(me, "#reveal_ts");
    if (_rts == error) return; endif
    if ((polcore().systime - CInt(_rts)) < 60) return; endif
    EraseObjProperty(me, "#reveal_ts");
    if (!fighting && !following && !warning)
        SetObjProperty(me, "shadow_mode", 1);
        SetObjProperty(me, "hunt_mode", 1);
        me.stealthsteps := 9999;
        me.hidden := 1;
        PrintTextAbove(me, "*retorna as sombras*");
    endif
endfunction

// Vagueia oculto pelo territorio usando GetRandomWanderPoint.
function WanderNPC()
    if (!guild || GetCooldown(me, "wander_cd")) return; endif
    var _wp := GetRandomWanderPoint();
    if (_wp)
        me.stealthsteps := 9999;
        WalkTowardLocation(_wp.x, _wp.y);
        SetCooldown(me, "wander_cd", RandomInt(8) + 4);
        wait_time := 0;
    endif
endfunction

// Processa fala "revelar" do lider da faccao: tira o NPC das sombras.
// Retorna 1 se o comando foi reconhecido e executado.
function HandleRevealSpeech(ev)
    if (!ev.source || !guild) return 0; endif
    if (CInt(guild.GetProp("leader")) != ev.source.serial) return 0; endif
    if (lower(ev.speech) != "revelar") return 0; endif
    EraseObjProperty(me, "shadow_mode");
    me.hidden       := 0;
    me.stealthsteps := 0;
    return 1;
endfunction

// Gump principal de controle — igual ao do guardfaccao, shared para todos os NPCs de combate.
function ShowFaccaoCombatGump(ev)
    var who := ev.source;
    var privs := guild ? guild.GetProp(cstr(who.serial)) : 0;
    var user_subguilds := who.guild ? who.guild.getProp("subguilds") : array{};
    if (!user_subguilds) user_subguilds := array{}; endif
    var is_leader := (who.serial == CInt(guild_leader) || (guild && guild.guildid in user_subguilds));
    if (!privs.above_guards && !is_leader && !who.cmdlevel) return; endif

    var _is_shadow_npc := GetObjProperty(me, "is_shadow_npc");
    var _gump_h := _is_shadow_npc ? 220 : 200;

    var gump := GFCreateGump(220, 70);
    GFResizePic(gump, 0, 0, 0x254E, 125, _gump_h);

    GFTextLine(gump, 30, 15, 2100, "Ir");
    GFTooltipText(gump, "Ordena o NPC ir ate um local.");
    GFAddButton(gump, 15, 17, 2117, 2118, 1, 70);

    GFTextLine(gump, 30, 35, 2100, "Parar");
    GFTooltipText(gump, "Ordena o NPC a parar suas atividades.");
    GFAddButton(gump, 15, 37, 2117, 2118, 1, 10);

    GFTextLine(gump, 30, 55, 2100, "Seguir");
    GFTooltipText(gump, "Ordena o NPC a seguir algo.");
    GFAddButton(gump, 15, 57, 2117, 2118, 1, 20);

    GFTextLine(gump, 30, 75, 2100, "Atacar");
    GFTooltipText(gump, "Ordena o NPC a atacar algo.");
    GFAddButton(gump, 15, 77, 2117, 2118, 1, 30);

    GFTextLine(gump, 30, 95, 2100, "Patrulhar");
    GFTooltipText(gump, "Define dois pontos de patrulha para o NPC.");
    GFAddButton(gump, 15, 97, 2117, 2118, 1, 40);

    GFTextLine(gump, 30, 115, 2100, "Definir Posto");
    GFTooltipText(gump, "Marca um local como posto fixo. O NPC se move ate la e volta apos combate.");
    GFAddButton(gump, 15, 117, 2117, 2118, 1, 90);

    GFTextLine(gump, 30, 135, 2100, "Manter Posicao");
    GFTooltipText(gump, "O NPC assume a posicao atual do seu personagem como posto.");
    GFAddButton(gump, 15, 137, 2117, 2118, 1, 50);

    GFTextLine(gump, 30, 155, 2100, "Facing");
    GFTooltipText(gump, "Define a direcao para onde o NPC ficara olhando.");
    GFAddButton(gump, 15, 157, 2117, 2118, 1, 80);

    if (_is_shadow_npc)
        var _in_hunt := GetObjProperty(me, "hunt_mode");
        GFTextLine(gump, 30, 175, _in_hunt ? 38 : 2100, "Buscar Alvo");
        GFTooltipText(gump, "NPC vagueia invisivel pela cidade buscando alvos para atacar.");
        GFAddButton(gump, 15, 177, 2117, 2118, 1, 61);
    endif

    var input := GFSendGump(who, gump)[0];
    case (input)
        70: // Ir ate
            var _dest := TargetCoordinates(who);
            if (!_dest) break; endif
            if (GetObjProperty(me, "guild") && !IsNPCInWalkRect(_dest))
                SendSysMessageEx(who, "Destino fora do territorio da faccao.", SSM_FAIL); break;
            endif
            fighting := 0; following := 0; warning := 0;
            SetOpponent(0); SetWarMode(0);
            _FaccaoNPCGoTo(_dest);
            break;

        10: // Parar
            fighting := 0; following := 0; warning := 0;
            SetOpponent(0); SetWarMode(0);
            EraseObjProperty(me, "patrolling");
            PrintTextAbove(me, "Sim senhor.");
            break;

        20: // Seguir
            var _ftarg := Target(who);
            if (_ftarg && _ftarg.isA(POLCLASS_MOBILE))
                fighting := 0; warning := 0;
                SetOpponent(0); SetWarMode(0);
                EraseObjProperty(me, "patrolling");
                following := _ftarg;
                PrintTextAbove(me, "As suas ordens.");
            endif
            break;

        30: // Atacar
            var _atarg := Target(who, TGTOPT_HARMFUL + TGTOPT_CHECK_LOS);
            if (_atarg && _atarg.isA(POLCLASS_MOBILE) && !_atarg.dead)
                me.frozen    := 0;
                following    := 0;
                fighting     := _atarg;
                disableWarn();
                SetOpponent(_atarg);
                SetWarMode(1);
                wait_time    := 0;
                PrintTextAbove(me, "Falou a lei...");
            endif
            break;

        40: // Patrulhar
            _FaccaoNPCSetPatrol(who);
            break;

        90: // Definir Posto
            DefinirPostoFixo(who);
            break;

        50: // Manter Posicao (usa posicao do jogador, como o guardfaccao original)
            EraseObjProperty(me, "patrolling");
            fix_location := struct{ "x" := who.x, "y" := who.y, "z" := who.z,
                                    "realm" := who.realm, "facing" := who.facing };
            SetObjProperty(me, "fix_loc", fix_location);
            PrintTextAbove(me, "Sim senhor.");
            break;

        80: // Facing
            ShowFaccaoFacingGump(who);
            break;

        61: // Alternar busca de alvo (hunt_mode)
            var _cur_hunt := GetObjProperty(me, "hunt_mode");
            if (_cur_hunt)
                EraseObjProperty(me, "hunt_mode");
                PrintTextAbove(me, "*retorna ao posto*");
            else
                // Buscar Alvo requer shadow_mode ativo
                if (!GetObjProperty(me, "shadow_mode"))
                    SetObjProperty(me, "shadow_mode", 1);
                    me.stealthsteps := 9999;
                    me.hidden       := 1;
                endif
                SetObjProperty(me, "hunt_mode", 1);
                PrintTextAbove(me, "*sai a caca*");
                ShowRevealGump(who);
            endif
            break;
    endcase
endfunction

// ── Helpers compartilhados de loop — todos os NPCs de combate de faccao ─────
// Requer pre-declaracao no script pai: me, guild, guild_leader, guild_laws,
// fighting, following, fix_location, wait_time, state, track_target

// Recarrega guild, guild_leader e guild_laws se guild for 0.
function LazyReloadGuild()
    if (guild) return; endif
    var _gid := GetObjProperty(me, "guild");
    if (!_gid) return; endif
    guild        := FindGuild(_gid);
    guild_leader := guild ? guild.getProp("leader") : 0;
    guild_laws   := guild ? guild.getProp("laws")   : array{};
    if (!guild_laws) guild_laws := array{}; endif
endfunction

// ============================================================
// Configura skills por tier (fraco/medio/forte) para cada tipo de NPC de faccao.
// tier: 1=fraco, 2=medio, 3=forte — detectado automaticamente pelo sufixo do template.
// ============================================================
function _SetupNPCSkills(agent)
    var tname := lower(cstr(agent.npctemplate));
    var tier  := 3;
    if (tname["fraco"] != error) tier := 1;
    elseif (tname["fr"] != error && tname["forte"] == error && tname["medio"] == error) tier := 1;
    elseif (tname["medio"] != error || tname["med"] != error) tier := 2;
    endif

    // ----- CONSELHO SOFOCRATICO -----
    if (tname["guarda"] != error && (tname["kaijin"] != error || tname["shaman"] != error ||
            tname["aradal"] != error || tname["velkyn"] != error || tname["evocatium"] != error))
        AP_SetTrueSkill(agent, MAGERY,    array{ 40, 65, 90 }[tier]);
        AP_SetTrueSkill(agent, ARCANA,    array{ 40, 65, 90 }[tier]);
        AP_SetTrueSkill(agent, TACTICS,   array{ 30, 50, 70 }[tier]);
        AP_SetTrueSkill(agent, WRESTLING, array{ 25, 40, 55 }[tier]);
        AP_SetTrueSkill(agent, ONEHANDED, array{ 20, 30, 40 }[tier]);
        AP_SetTrueSkill(agent, THEOLOGY,  array{ 10, 20, 30 }[tier]);
        return;
    endif

    // ----- REGENTES (guardafaccao) -----
    if (tname["guardafaccao"] != error)
        AP_SetTrueSkill(agent, TACTICS,   array{ 20, 50, 80 }[tier]);
        AP_SetTrueSkill(agent, ONEHANDED, array{ 30, 55, 80 }[tier]);
        AP_SetTrueSkill(agent, TWOHANDED, array{ 20, 45, 70 }[tier]);
        AP_SetTrueSkill(agent, DUALWIELD, array{ 20, 37, 55 }[tier]);
        AP_SetTrueSkill(agent, WRESTLING, array{ 25, 45, 65 }[tier]);
        AP_SetTrueSkill(agent, RANGED,    array{  0, 15, 30 }[tier]);
        AP_SetTrueSkill(agent, PARRY,     array{  0, 40, 85 }[tier]);
        return;
    endif

    // ----- RUFIOES (ladraorufiao) -----
    if (tname["ladraoruf"] != error)
        AP_SetTrueSkill(agent, PICKPOCKETING, array{ 50, 70, 90 }[tier]);
        AP_SetTrueSkill(agent, TACTICS,       array{ 40, 55, 70 }[tier]);
        AP_SetTrueSkill(agent, ONEHANDED,     array{ 50, 65, 80 }[tier]);
        AP_SetTrueSkill(agent, DUALWIELD,     array{ 20, 40, 60 }[tier]);
        AP_SetTrueSkill(agent, WRESTLING,     array{ 20, 30, 40 }[tier]);
        AP_SetTrueSkill(agent, SNEAK,         array{ 50, 70, 90 }[tier]);
        return;
    endif

    // ----- SINODO MELEE (assassinosinodo) -----
    if (tname["assassinosinodo"] != error)
        AP_SetTrueSkill(agent, TACTICS,   array{ 50, 65, 80 }[tier]);
        AP_SetTrueSkill(agent, ONEHANDED, array{ 55, 70, 85 }[tier]);
        AP_SetTrueSkill(agent, DUALWIELD, array{ 30, 45, 60 }[tier]);
        AP_SetTrueSkill(agent, WRESTLING, array{ 20, 35, 50 }[tier]);
        AP_SetTrueSkill(agent, TWOHANDED, array{ 20, 30, 40 }[tier]);
        AP_SetTrueSkill(agent, SNEAK,     array{ 55, 70, 85 }[tier]);
        return;
    endif

    // ----- SINODO ARQUEIRO (assassinoarqueiro) -----
    if (tname["assassinoarqueiro"] != error)
        AP_SetTrueSkill(agent, RANGED,    array{ 55, 70, 85 }[tier]);
        AP_SetTrueSkill(agent, TACTICS,   array{ 45, 60, 75 }[tier]);
        AP_SetTrueSkill(agent, DUALWIELD, array{ 20, 35, 50 }[tier]);
        AP_SetTrueSkill(agent, ONEHANDED, array{ 20, 30, 40 }[tier]);
        AP_SetTrueSkill(agent, WRESTLING, array{ 15, 25, 35 }[tier]);
        AP_SetTrueSkill(agent, TWOHANDED, array{ 15, 25, 35 }[tier]);
        AP_SetTrueSkill(agent, SNEAK,     array{ 55, 70, 85 }[tier]);
        return;
    endif

    // ----- SINODO CAZADOR (assassinocazador) -----
    if (tname["assassinocazador"] != error)
        AP_SetTrueSkill(agent, TACTICS,   array{ 50, 65, 80 }[tier]);
        AP_SetTrueSkill(agent, ONEHANDED, array{ 55, 70, 85 }[tier]);
        AP_SetTrueSkill(agent, DUALWIELD, array{ 25, 40, 55 }[tier]);
        AP_SetTrueSkill(agent, WRESTLING, array{ 20, 30, 45 }[tier]);
        AP_SetTrueSkill(agent, TWOHANDED, array{ 15, 25, 35 }[tier]);
        AP_SetTrueSkill(agent, SNEAK,     array{ 50, 65, 80 }[tier]);
        return;
    endif
endfunction

// Base compartilhada do Setup() de NPCs de faccao.
// is_stealth=1: hiddenattack, runwhilestealth e shadow_mode.
// is_stealth=0: apenas oculta o NPC (ladroes).
// Retorna 0 se o NPC foi eliminado (guild ausente/invalida), 1 em sucesso.
function SetupFaccaoNPCBase(is_stealth := 0)
    var guildid := GetObjProperty(me, "guild");
    if (!guildid)
        sleep(5);
        SetupFaccaoNPCBase(is_stealth);
        return 0;
    endif
    guild := FindGuild(guildid);
    if (!guild)
        ApplyRawDamage(me, 600);
        return 0;
    endif

    guild_leader := guild.getProp("leader");
    guild_laws   := guild.getProp("laws");
    if (!guild_laws) guild_laws := array{}; endif

    fix_location := GetObjProperty(me, "fix_loc");
    if (fix_location == error) fix_location := 0; endif

    if (!me.backpack)
        var new_pack := CreateItemAtLocation(me.x, me.y, me.z, "backpack", 1);
        EquipItem(me, new_pack);
    endif

    me.cmdlevel := 2;

    _SetupNPCSkills(me);

    if (is_stealth)
        GrantPrivilege(me, "hiddenattack");
        me.enable("hiddenattack");
        GrantPrivilege(me, "runwhilestealth");
        me.enable("runwhilestealth");
        if (GetObjProperty(me, "shadow_mode"))
            me.stealthsteps := 9999;
            me.hidden := 1;
        endif
    else
        me.stealthsteps := 9999;
        me.hidden := 1;
    endif

    return 1;
endfunction

// Escaneia players proximos para violacoes de lei (cooldown de 3s).
function ScanNearbyLaws()
    if (warning || GetCooldown(me, "lawscan_cd")) return; endif
    foreach _mob in ListMobilesNearLocationEx(me.x, me.y, me.z, 6,
            LISTEX_FLAG_NORMAL, me.realm)
        sleepms(2);
        if (!_mob.npctemplate) CheckPlayerLaws(_mob); endif
        if (warning) break; endif
    endforeach
    SetCooldown(me, "lawscan_cd", 3);
endfunction

// Controle de territorio: retorna ao centro do rect da guild se fora dele.
// hunt_mode desativa o controle. Reseta estado de combate ao sair do territorio.
// Retorna 1 se o NPC foi redirecionado ao centro.
function EnforceTerritoryBounds()
    if (!guild || GetObjProperty(me, "hunt_mode")) return 0; endif

    // Realm tem precedencia: se NPC esta em realm errado, teleporte forcado
    // e reset completo de estado (sobrescreve qualquer combate em andamento).
    if (EnforceGuildRealm())
        state    := 0;
        fighting := 0;
        SetOpponent(0);
        SetWarMode(0);
        wait_time := 0;
        return 1;
    endif

    if (GetObjProperty(me, "tipo_faccao") == "staff")
        HandleWalkRectBreach(me, wait_time);
        return 0;
    endif
    // Faccao player sem territorio configurado (nenhum totem consumido): sem restricao
    var _area := guild.GetProp("area");
    var _terr  := guild.GetProp("territories");
    if ((!_area || !_area.size()) && (!_terr || !_terr.size())) return 0; endif
    if (inGuildRect(guild, me, 4)) return 0; endif
    if (state == 3) return 0; endif
    var _center := GetGuildCenter(guild, me.z);
    if (_center)
        me.stealthsteps := 9999;
        MoveOrTeleportToGuildPoint(_center);
        wait_time := 0;
    endif
    state    := 0;
    fighting := 0;
    SetOpponent(0);
    SetWarMode(0);
    return 1;
endfunction

// Retorna o primeiro mobile valido para combate dentro de 'range' tiles.
// IsValidTarget() deve estar definida no script que inclui este arquivo.
function GetNearTarget(range := 8)
    foreach mob in ListMobilesNearLocationEx(me.x, me.y, me.z, range,
            LISTEX_FLAG_NORMAL, me.realm)
        sleepms(2);
        if (IsValidTarget(mob)) return mob; endif
    endforeach
    return 0;
endfunction

// Retorna o percentual de HP do NPC (0-100).
function GetHPPercent()
    if (me.maxhits <= 0) return 100; endif
    return CInt((me.hits * 100) / me.maxhits);
endfunction

// Executa um passo de patrulha entre dois pontos ("patrolling" prop).
// Retorna 1 se a patrulha esta ativa e foi executada, 0 se nao ha rota.
function UpdatePatrolState()
    var _patrol := GetObjProperty(me, "patrolling");
    if (!_patrol || _patrol.size() < 2) return 0; endif
    var _pidx := GetObjProperty(me, "#patrol_idx");
    if (_pidx == error || !_pidx) _pidx := 1; endif
    var _ppt := _patrol[_pidx];
    if (CoordinateDistance(me.x, me.y, _ppt.x, _ppt.y) <= 1)
        _pidx := (_pidx % 2) + 1;
        SetObjProperty(me, "#patrol_idx", _pidx);
    else
        WalkTowardLocation(_ppt.x, _ppt.y);
        wait_time := 0;
    endif
    return 1;
endfunction
