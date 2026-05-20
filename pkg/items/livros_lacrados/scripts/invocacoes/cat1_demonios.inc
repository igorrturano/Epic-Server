// invocacoes/cat1_demonios.src
// Categoria 1  - Demnios Aprisionados



function AbrirPortalDemon(who, ox, oy)
    var portal := CreateItemAtLocation(who.x + ox, who.y + oy, who.z, 0x1FD4, 1, who.realm);
    if (portal && !portal.errortext)
        portal.movable := 0;
    endif
    PlaySoundEffect(who, 0x020E);
    sleep(2);
    if (portal && !portal.errortext) DestroyItem(portal); endif
endfunction

// Ritual com efeito de partcula  - sem item para evitar conflito com o filactrio (0x0240).
function FazerRitualDemon(who, npc)
    PlaySoundEffect(npc, 0x01E9);
    PlayStationaryEffectEx(npc.x, npc.y, npc.z, npc.realm, 0x374A, 10, 25);
    sleep(1);
    PlayStationaryEffectEx(npc.x, npc.y, npc.z, npc.realm, 0x374A, 10, 20);
    sleep(1);
    PlayStationaryEffectEx(npc.x, npc.y, npc.z, npc.realm, 0x374A, 10, 15);
    sleep(1);
endfunction

function InvocarVexalim(who, book, nivel, afinidade, lugar)
    AbrirPortalDemon(who, 2, 0);
 var npc := _InvocarPadrao(who, ":epicAI:quasit", "Vexalim, o Faminto", "vexalim", "cat1");
    if (!npc) return; endif
 SetObjProperty(npc, "livro_serial", book.serial);
    npc.frozen := 1;
    FazerRitualDemon(who, npc);
 PrintTextAbove(npc, "*olfata o ar*");
    sleep(3);
 PrintTextAbove(npc, "Tocou no que no era seu.");
    sleep(3);
    npc.frozen := 0;
    npc.opponent := who;
    MoveObjectToLocation(npc, who.x + 1, who.y, who.z, who.realm, MOVEOBJECT_FORCELOCATION);
 start_script(":epicAI:abilities/CurseHeal", array{npc, who});
 start_script(":epicAI:abilities/InfernalBurst", array{npc});
endfunction

function InvocarIskaryon(who, book, nivel, afinidade, lugar)
    AbrirPortalDemon(who, 2, 0);
 var npc := _InvocarPadrao(who, ":epicAI:demon8", "Iskaryon dos Sete Nomes", "iskaryon", "cat1");
    if (!npc) return; endif
 SetObjProperty(npc, "livro_serial", book.serial);
    npc.frozen := 1;
    FazerRitualDemon(who, npc);
 PrintTextAbove(npc, "Iskaryon. O primeiro dos sete. Os demais matam por si ss.");
    sleep(3);
 var nomes_sete := array{"Varleth", "Onsidrah", "Mazkar", "Thelvoryn", "Urskaim", "Dorvaxis", "Iskaryon"};
    PrintTextAbove(npc, nomes_sete[RandomInt(7) + 1]);
    sleep(3);
 PrintTextAbove(npc, "*fixa os olhos no invocador*");
    sleep(3);
    npc.frozen := 0;
    npc.opponent := who;
    MoveObjectToLocation(npc, who.x + 1, who.y, who.z, who.realm, MOVEOBJECT_FORCELOCATION);
 start_script(":epicAI:abilities/CurseHeal", array{npc, who});
 start_script(":epicAI:abilities/InfernalBurst", array{npc});
endfunction

function InvocarOQuieto(who, book, nivel, afinidade, lugar)
 var npc := _InvocarPadrao(who, ":epicAI:demon8", "O Quieto", "o_quieto", "cat1");
    if (!npc) return; endif
 SetObjProperty(npc, "falso_lacrado", 1);
 SetObjProperty(npc, "invulnerable", 1);
    npc.frozen := 1;
 SendSysMessageEX(who, "O ar fica levemente pesado.", SSM_INFO);
    sleep(2);
 PrintTextAbove(npc, "*observa voce em silencio absoluto*");
    sleep(10);
 PrintTextAbove(npc, "*fita profundamente*");
    sleep(8);
 PrintTextAbove(npc, "*no pisca. Nao respira. Apenas observa.*");
    sleep(15);
 PrintTextAbove(npc, "*some sem fazer barulho*");
    sleep(1);
    if (npc && !npc.errortext)
        MoveObjectToLocation(npc, 0, 0, 0, npc.realm, MOVEOBJECT_FORCELOCATION);
        npc.kill();
    endif
endfunction

function InvocarPactoRoto(who, book, nivel, afinidade, lugar)
    AbrirPortalDemon(who, 2, 0);
 var npc := _InvocarPadrao(who, ":epicAI:demon6", "O Pacto Roto", "pacto_roto", "cat1");
    if (!npc) return; endif
    npc.frozen := 1;
    FazerRitualDemon(who, npc);
 PrintTextAbove(npc, "Voce no deveria ter feito isso.");
    sleep(3);
 PrintTextAbove(npc, "*avana lentamente*");
    sleep(3);
    npc.frozen := 0;
    npc.opponent := who;
    MoveObjectToLocation(npc, who.x + 1, who.y, who.z, who.realm, MOVEOBJECT_FORCELOCATION);
 start_script(":epicAI:abilities/CurseHeal", array{npc, who});
 start_script(":epicAI:abilities/InfernalBurst", array{npc});
endfunction
