// tax.inc — Helpers de taxas de facção para merchants da Liga do Pacto Comum
//
// Uso: include ":ligafaccao:include/tax";
//      var preco := CInt(base * GetMerchantTaxMultiplier(guild));
//
// Leis afetadas: merchanttax, zerotax
// Para guildtax (rivais pagam mais), use GetGuildTaxMultiplier(guild, player)

// Retorna multiplicador de preço base da facção (1.0 = normal, 1.5 = +50%, 0.0 = grátis)
function GetMerchantTaxMultiplier(guild)
    if (!guild) return 1.0; endif
    var laws := guild.getProp("laws");
    if (!laws) return 1.0; endif
    var multiplier := 1.0;
    foreach law in laws
        sleepms(2);
        if (!law.on) continue; endif
        case (law.type)
            "merchanttax":
                multiplier := multiplier * (100 + CInt(law.opt)) / 100.0;
                break;
            "zerotax":
                return 0.0;
        endcase
    endforeach
    return multiplier;
endfunction

// Retorna multiplicador extra para membros de facção rival (default 1.0 = sem taxa extra)
// opt = struct{ rivais: [guildid, ...], pct: int }
function GetGuildTaxMultiplier(guild, player)
    if (!guild || !player) return 1.0; endif
    var laws := guild.getProp("laws");
    if (!laws) return 1.0; endif
    var player_gid := player.guildid;
    foreach law in laws
        sleepms(2);
        if (!law.on || law.type != "guildtax") continue; endif
        if (!law.opt || !law.opt.rivais) continue; endif
        if (player_gid in law.opt.rivais)
            return (100 + CInt(law.opt.pct)) / 100.0;
        endif
    endforeach
    return 1.0;
endfunction

// Retorna multiplicador de reparo baseado na durabilidade do item e lei durability_tax
// ratio >= 1.0 → desconto; ratio < 0.5 → sobretaxa; entre 0.5-1.0 → normal
function GetDurabilityTaxMultiplier(guild, item)
    if (!guild) return 1.0; endif
    var laws := guild.getProp("laws");
    if (!laws) return 1.0; endif
    foreach law in laws
        sleepms(2);
        if (!law.on || law.type != "durability_tax") continue; endif
        if (!law.opt) continue; endif
        var maxhp := item.maxhp;
        if (!maxhp || maxhp <= 0) return 1.0; endif
        var ratio := item.hp * 1.0 / maxhp;
        if (ratio >= 1.0)
            return (100 - CInt(law.opt.desconto_perfeita)) / 100.0;
        elseif (ratio < 0.5)
            return (100 + CInt(law.opt.taxa_gasta)) / 100.0;
        endif
        return 1.0;
    endforeach
    return 1.0;
endfunction

// Retorna true se a lei notax_exempt existe e o player nao tem isencao
function CheckTollRequired(guild, player)
    if (!guild || !player) return 0; endif
    var laws := guild.getProp("laws");
    if (!laws) return 0; endif
    foreach law in laws
        sleepms(2);
        if (!law.on || law.type != "notax_exempt") continue; endif
        // Verificar se player ja pagou o pedagio nesta sessao
        var paid_key := "toll_paid_{}".format(guild.guildid);
        var paid_ts  := CInt(GetObjProperty(player, paid_key));
        if (paid_ts && polcore().systime - paid_ts < 600) continue; endif
        return law;
    endforeach
    return 0;
endfunction
