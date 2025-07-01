use uo;
use polsys;

/*
 * HandleChampionMobDeath(mobile)
 * Verifica se um NPC é de um champion spawn e notifica o altar.
 * Deve ser chamado de dentro do script de morte da IA principal (ex: brain.src).
 */
function HandleChampionMobDeath(mobile)
    const EVID_CHAMPION_KILL := 0xE001;

    var altar_serial := GetObjProperty(mobile, "#AltarSerial");
    if (!altar_serial)
        return; // Não é um monstro de champion, não faz nada.
    endif

    var altar := SystemFindObjectBySerial(CInt(altar_serial));
    if (altar)
        var ev := struct;
        ev.+type := EVID_CHAMPION_KILL;
        SendEvent(altar, ev);
        SysLog("[CHAMP_DEATH_HANDLER] Evento de morte enviado para o altar " + altar.serial);
    endif
endfunction