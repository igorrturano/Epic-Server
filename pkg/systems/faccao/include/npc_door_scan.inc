// npc_door_scan.inc — Varredura de portas trancadas (lei casafechada)
//
// Requer no contexto: me, guild, guild_laws
// Chamar do loop principal com cooldown ~12s:
//   if (!GetCooldown(me, "casafechada_cd"))
//       ScanLockedDoors();
//       SetCooldown(me, "casafechada_cd", 12);
//   endif

function ScanLockedDoors()
    foreach law in guild_laws
        sleepms(2);
        if (!law.on || CStr(law.type) != "casafechada") continue; endif
        if (!law.opt || law.opt.hora_inicio == error) break; endif

        var _hora := (polcore().systime / 3600) % 24;
        var _ativo := 0;
        if (law.opt.hora_inicio < law.opt.hora_fim)
            _ativo := (_hora >= law.opt.hora_inicio && _hora < law.opt.hora_fim);
        else
            _ativo := (_hora >= law.opt.hora_inicio || _hora < law.opt.hora_fim);
        endif
        if (!_ativo) break; endif

        foreach door in ListItemsNearLocation(me.x, me.y, me.z, 10, me.realm)
            sleepms(2);
            if (!door.locked) continue; endif
            if (!door.isA(POLCLASS_DOOR)) continue; endif

            var door_key := "#casafechada_{}".format(door.serial);
            var door_ts  := CInt(GetObjProperty(me, door_key));

            if (!door_ts)
                PrintTextAbove(me, "*bate na porta trancada*");
                WalkTowardLocation(door.x, door.y);
                SetObjProperty(me, door_key, polcore().systime + 30);
                AddReport(guild, "{} encontrou porta trancada em horario comercial ({},{})".format(
                    me.name, door.x, door.y));
            elseif (polcore().systime > door_ts)
                if (door.locked)
                    door.locked := 0;
                    PrintTextAbove(me, "*destrava a porta em horario comercial*");
                    AddReport(guild, "{} destravou porta em ({},{}) por lei casafechada".format(
                        me.name, door.x, door.y));
                endif
                EraseObjProperty(me, door_key);
            endif
        endforeach
        break;
    endforeach
endfunction
