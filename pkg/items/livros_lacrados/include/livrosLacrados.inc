// livrosLacrados.inc
// Include principal do sistema de Livros Lacrados
// Constantes, fun  es utilitrias e helpers compartilhados.
// REGRA: sem var em escopo de mdulo  - apenas const e functions.

use uo;
use os;
use util;

include ":nature:include/nature";
include ":message:include/model/Message";

// ============================================================
// CONSTANTES  - NATUREZAS
// ============================================================
const NAT_INERTE := "inerte";
const NAT_CIFRADO := "cifrado";
const NAT_MANCHADO := "manchado";
const NAT_DESPERTADOR := "despertador";
const NAT_CONVOCADOR := "convocador";
const NAT_PROFETICO := "profetico";

// ============================================================
// CONSTANTES  - AFINIDADE
// ============================================================
const AFI_SAGRADO := "sagrado";
const AFI_NEUTRO := "neutro";
const AFI_PROFANO := "profano";

// ============================================================
// CONSTANTES  - TIPO DE CONVOCAA'O
// ============================================================
const CONV_HOSTIL_IMEDIATO := "hostil_imediato";
const CONV_HOSTIL_RETARDADO := "hostil_retardado";
const CONV_LIBERADO := "liberado";
const CONV_FALSO := "falso";
const CONV_DESPERTADOR := "despertador";
const CONV_UNICO := "unico";
const CONV_INDEFINIDO := "indefinido";

// ============================================================
// FUN  AA'O: SortearNatureza()
// Pesos: Inerte=58 Cifrado=10 Manchado=10 Despertador=8
//        Convocador=7 Profetico=7
// ============================================================
function SortearNatureza()
    var roll := RandomInt(100);
    if (roll < 58) return NAT_INERTE;      endif  // 58%
    if (roll < 68) return NAT_CIFRADO;     endif  // 10%
    if (roll < 78) return NAT_MANCHADO;    endif  // 10%
    if (roll < 86) return NAT_DESPERTADOR; endif  //  8%
    if (roll < 93) return NAT_CONVOCADOR;  endif  //  7%
    return NAT_PROFETICO;                         //  7%
endfunction

// ============================================================
// FUN  AA'O: SortearAfinidade()
// ============================================================
function SortearAfinidade()
    var r := RandomInt(3);
    case (r)
        0: return AFI_SAGRADO;
        1: return AFI_NEUTRO;
        2: return AFI_PROFANO;
    endcase
    return AFI_NEUTRO;
endfunction

// ============================================================
// FUN  AA'O: SortearInvocacao(nivel)
// Usa arrays locais (sem var de mdulo)
// ============================================================
function SortearInvocacao(nivel)
    var chance_falso := 70;
    if (nivel == 4) chance_falso := 40; endif
    if (nivel == 5) chance_falso := 10; endif

    if (RandomInt(100) < chance_falso)
        return SortearEntidadeFalsa(nivel);
    endif

    if (nivel == 5)
        if (RandomInt(100) < 2)
            var unico := SortearUnico();
 if (unico != "") return unico; endif
        endif
    endif

    // Candidatos por nivel (pesos via repeti  o)
    var lista;
    if (nivel == 3)
        lista := array{
 "vexalim","vexalim","vexalim",
 "o_quieto","o_quieto","o_quieto",
 "coisa_quadrupede","coisa_quadrupede","coisa_quadrupede",
 "pequenos_limiar","pequenos_limiar","pequenos_limiar",
 "ovo","ovo","ovo",
 "soldado","soldado","soldado",
 "bibliotecario","bibliotecario","bibliotecario",
 "confissor","confissor","confissor",
 "recita_nomes","recita_nomes","recita_nomes",
 "pede_sangue","pede_sangue","pede_sangue",
 "mensageiro",
 "coletor_anos",
 "herdeiro_senhor_guerra",
 "fragmento_retornado",
 "conhecidas_da_liga"
        };
    elseif (nivel == 4)
        lista := array{
 "vexalim","vexalim","vexalim",
 "o_quieto","o_quieto","o_quieto",
 "coisa_quadrupede","coisa_quadrupede","coisa_quadrupede",
 "pequenos_limiar","pequenos_limiar","pequenos_limiar",
 "ovo","ovo","ovo",
 "soldado","soldado","soldado",
 "bibliotecario","bibliotecario","bibliotecario",
 "confissor","confissor","confissor",
 "recita_nomes","recita_nomes","recita_nomes",
 "pede_sangue","pede_sangue","pede_sangue",
 "pacto_roto",
 "mae_procura",
 "lirenne",
 "tres_irmaos",
 "cantante",
 "mensageiro",
 "voz_velho",
 "coletor_anos",
 "quer_nome",
 "cobrador",
 "o_primeiro_puxado",
 "voz_do_plano_oco",
 "herdeiro_senhor_guerra",
 "espirito_origem_lobo",
 "eco_do_andarilho",
 "fragmento_retornado",
 "conhecidas_da_liga",
 "essencia_fea"
        };
    else
        lista := array{
 "o_quieto","o_quieto","o_quieto",
 "soldado","soldado","soldado",
 "bibliotecario","bibliotecario","bibliotecario",
 "confissor","confissor","confissor",
 "recita_nomes","recita_nomes","recita_nomes",
 "pede_sangue","pede_sangue","pede_sangue",
 "iskaryon",
 "pacto_roto",
 "mae_procura",
 "lirenne",
 "tres_irmaos",
 "coisa_imita",
 "cantante",
 "mensageiro",
 "anuncio",
 "voz_velho",
 "coletor_anos",
 "quer_nome",
 "veio_memoria",
 "cobrador",
 "o_primeiro_puxado",
 "voz_do_plano_oco",
 "herdeiro_senhor_guerra",
 "espirito_origem_lobo",
 "mensageiro_ora",
 "eco_do_andarilho",
 "fragmento_retornado",
 "sombra_vaelshorath",
 "conhecidas_da_liga",
 "essencia_fea"
        };
    endif

    var tam := lista.size();
 if (tam == 0) return "vexalim"; endif
    return lista[RandomInt(tam) + 1];
endfunction

function SortearEntidadeFalsa(nivel)
    if (nivel == 3)
 var falsos3 := array{"sombra_iskaryon","demonio_papel","coro_vazio","eles"};
        return falsos3[RandomInt(4) + 1];
    elseif (nivel == 4)
 var falsos4 := array{"sombra_iskaryon","demonio_papel","coro_vazio","eles","propria_morte"};
        return falsos4[RandomInt(5) + 1];
    else
 var falsos5 := array{"sombra_iskaryon","demonio_papel","coro_vazio","eles","propria_morte"};
        return falsos5[RandomInt(5) + 1];
    endif
endfunction

function SortearUnico()
    var unicos := array{
 "primeiro_lacrador","antessenhora","sem_nome","filho_servidor","coisa_errada"
    };
    var candidatos := array{};
    var i := 1;
    while (i <= unicos.size())
        var id := unicos[i];
 if (!GetGlobalProperty("unico_usado_" + id))
            candidatos.append(id);
        endif
        i += 1;
    endwhile
 if (candidatos.size() == 0) return ""; endif
    return candidatos[RandomInt(candidatos.size()) + 1];
endfunction

// ============================================================
// FUN  AA'O: GetTipoEntidade(inv_id)
// Retorna o tipo de convoccao da entidade pelo ID
// ============================================================
function GetTipoEntidade(inv_id)
    // Hostil Retardado
 if (inv_id == "o_quieto") return CONV_HOSTIL_RETARDADO; endif

    // AAnicos
 if (inv_id == "primeiro_lacrador" || inv_id == "antessenhora" ||
 inv_id == "sem_nome" || inv_id == "filho_servidor" || inv_id == "coisa_errada")
        return CONV_UNICO;
    endif

    // Falsos
 if (inv_id == "sombra_iskaryon" || inv_id == "demonio_papel" ||
 inv_id == "coro_vazio" || inv_id == "eles" || inv_id == "propria_morte")
        return CONV_FALSO;
    endif

    // Despertadores  - cat4 e entidades informacionais
 if (inv_id == "mae_procura" || inv_id == "soldado" || inv_id == "bibliotecario" ||
 inv_id == "mensageiro" || inv_id == "voz_velho" || inv_id == "anuncio" ||
 inv_id == "confissor" || inv_id == "recita_nomes")
        return CONV_DESPERTADOR;
    endif

    // Despertadores  - cat8 (entidades de lore especfico)
 if (inv_id == "o_primeiro_puxado" || inv_id == "voz_do_plano_oco" ||
 inv_id == "herdeiro_senhor_guerra" || inv_id == "espirito_origem_lobo" ||
 inv_id == "mensageiro_ora" || inv_id == "eco_do_andarilho" ||
 inv_id == "fragmento_retornado" || inv_id == "sombra_vaelshorath" ||
 inv_id == "conhecidas_da_liga" || inv_id == "essencia_fea")
        return CONV_DESPERTADOR;
    endif

    // Liberados
 if (inv_id == "lirenne" || inv_id == "cantante" ||
 inv_id == "pequenos_limiar" || inv_id == "ovo" || inv_id == "tres_irmaos")
        return CONV_LIBERADO;
    endif

    // Hostil Imediato  - entidades conhecidas que demandam ou atacam
 if (inv_id == "vexalim" || inv_id == "coisa_quadrupede" ||
 inv_id == "pede_sangue" || inv_id == "quer_nome" ||
 inv_id == "veio_memoria" || inv_id == "coletor_anos" || inv_id == "cobrador")
        return CONV_HOSTIL_IMEDIATO;
    endif

    // Indefinido  - entidade desconhecida; NPC passivo por segurana
    return CONV_INDEFINIDO;
endfunction

// ============================================================
// FUN  AA'O: GetLugar(who)
// ============================================================
function GetLugar(who)
 if (InDungeon(who)) return "masmorra"; endif
    var moon := getMoonPhase();
 if (moon == "Idus") return "lua_cheia"; endif
 if (moon == "Kalendae") return "lua_nova"; endif
 return "normal";
endfunction

// ============================================================
// FUN  AA'O: _InvocarPadrao(who, template, nome, invocacao_id, cat_tag, offset_x)
// Boilerplate comum a todas as invocces: spawn, guard, SetName, invocacao_id.
// Retorna o NPC ou 0 em caso de erro.
// ============================================================
function _InvocarPadrao(who, template, nome, invocacao_id, cat_tag, offset_x := 2)
    var npc := CreateNpcFromTemplate(template, who.x + offset_x, who.y, who.z, 0, who.realm);
    if (!npc || npc.errortext)
        return 0;
    endif
    SetName(npc, nome);
    // TODO: spawns manuais (InvocarTresIrmaos, cantante via start_script, etc.) escrevem
    // invocacao_id diretamente  - grep por CreateNpcFromTemplate em scripts/invocacoes/ antes
    // de construir qualquer sistema de limite/cleanup que leia esta prop.
 SetObjProperty(npc, "invocacao_id", invocacao_id);
    return npc;
endfunction

// ============================================================
// FUN  AA'O: EmitirAssinaturaSensorial(who, tipo_conv)
// ============================================================
function EmitirAssinaturaSensorial(who, tipo_conv)
    case (tipo_conv)
        CONV_HOSTIL_IMEDIATO:
        CONV_FALSO:
            ScreenShake(who, 0.8, 15);
            PlaySoundEffect(who, 0x028F);
 SendSysMessageEX(who, "Algo cruzou para este lado.", SSM_INFO);
            break;

        CONV_HOSTIL_RETARDADO:
            ScreenShake(who, 0.3, 5);
            PlaySoundEffect(who, 0x015F);
 SendSysMessageEX(who, "Algo se lembrou de ti.", SSM_INFO);
            break;

        CONV_LIBERADO:
            ScreenShake(who, 0.4, 8);
            PlaySoundEffect(who, 0x014F);
 SendSysMessageEX(who, "Algo passou e seguiu.", SSM_INFO);
            break;

        CONV_DESPERTADOR:
            PlaySoundEffect(who, 0x0202);
 SendSysMessageEX(who, "Algo desperta.", SSM_INFO);
            break;

        CONV_UNICO:
            var mobs := ListMobilesNearLocation(who.x, who.y, who.z, 500, who.realm);
            var mi := 1;
            while (mi <= mobs.size())
                var mob := mobs[mi];
                if (!mob.npctemplate)
                    ScreenShake(mob, 5.0, 20);
                endif
                sleepms(2);
                mi += 1;
            endwhile
            PlaySoundEffect(who, 0x0029);
 Broadcast("Algo sem nome pisou no mundo.");
            break;

        CONV_INDEFINIDO:
            break;

        default:
            break;
    endcase
endfunction

// ============================================================
// FUN  AA'O: PlayNPCBowAnim(npc)
// Envia animcao de reverncia (bow) a todos os mobiles proximos.
// ============================================================
function PlayNPCBowAnim(npc)
    var sh := hex(npc.serial);
 if (sh[1,2] == "0x") sh := sh[3, len(sh)]; endif
 while (len(sh) < 8) sh := "0" + sh; endwhile
 var pkt := "6E" + sh + "0020" + "0003" + "0001" + "01" + "00" + "05";
    foreach chr in ListMobilesNearLocation(npc.x, npc.y, npc.z, 16, npc.realm)
        SendPacket(chr, pkt);
        sleepms(10);
    endforeach
endfunction

// ============================================================
// FUN  AA'O: ExecutarOnCreate(book, nivel)
// Logica compartilhada pelos 3 scripts onCreate (nv3/4/5).
// ============================================================
function ExecutarOnCreate(book, nivel)
    if (nivel < 3) nivel := 3; endif
    if (nivel > 5) nivel := 5; endif

    var natureza  := SortearNatureza();
    var afinidade := SortearAfinidade();
    var invocacao := "";

    if (natureza == NAT_CONVOCADOR || natureza == NAT_DESPERTADOR)
        invocacao := SortearInvocacao(nivel);
    endif

    SetObjProperty(book, "nivel_bau",  nivel);
    SetObjProperty(book, "natureza",   natureza);
    SetObjProperty(book, "afinidade",  afinidade);
    SetObjProperty(book, "invocacao",  invocacao);
    SetObjProperty(book, "aberto",     0);

    var graphics := array{
        0x0FEF, 0x0FF0, 0x0FF1, 0x0FF2, 0x36A2, 0x36A3, 0x42B7, 0x42B8,
        0x71B0, 0x71B1, 0x71B2, 0x71B3, 0x71B4, 0x9C16, 0x9C17, 0xAE31,
        0xAE32, 0xAE35, 0xAE36, 0xB257, 0xB258, 0xFF34, 0xFF35, 0xFF36,
        0xFF37, 0xFF38, 0xFF39, 0xF8D5, 0xF8D6, 0xF8D7, 0xF8D8, 0xEB2D,
        0xEB2E, 0xE9BE, 0xE9BF,
        0xE8A4, 0xE8A5, 0xE8A6, 0xE8A7, 0xE8A8, 0xE8A9, 0xE8AA, 0xE8AB,
        0xE8AC, 0xE8AD, 0xE8B2, 0xE8B3, 0xE8B4, 0xE8B5, 0xE8B6, 0xE8B7,
        0xE8C8, 0xE8C9, 0xE8CA, 0xE8CB, 0xE8CC, 0xE8CD, 0xE8CE, 0xE8CF,
        0xE8D0, 0xE8D1, 0xE8D2, 0xE8D3, 0xE8D4, 0xE8D5, 0xE8D6, 0xE8D7,
        0xE8D9, 0xE8DA, 0xE8DB, 0xE8E0, 0xE8E1, 0xE8E2, 0xE8E3, 0xE8E4,
        0xE8E5, 0xE8E6, 0xE8E7, 0xE8E8, 0xE8E9, 0xE8EA, 0xE8EF, 0xE8F0,
        0xE8F1, 0xE8F6, 0xE8F7, 0xE8F8, 0xE8F9, 0xE8FA, 0xE901, 0xE902
    };
    book.graphic := graphics[RandomInt(graphics.size()) + 1];
    SetName(book, "Livro Lacrado");
    return 1;
endfunction

// ============================================================
// FUN  AA'O: GerarMensagemRP(who)
// Gera mensagem de abertura para o Mensageiro baseada em props ativas.
// ============================================================
function GerarMensagemRP(who)
 var base := "O que voce carrega sem perceber ";
 var props_possiveis := array{"marca_lacrado", "profecia_lacrado", "transformacao_livro"};
    foreach prop in props_possiveis
 var val := GetObjProperty(who, prop + "_" + who.serial);
        if (val && !val.errortext)
 return base + "visvel para outros.";
        endif
    endforeach
    var frases := array{
 "pesado demais para ser esquecido.",
 "mais antigo do que voce pensa.",
 "algo que outro carregou antes de ti.",
 "a resposta para a pergunta errada."
    };
    return base + frases[RandomInt(4)+1];
endfunction

