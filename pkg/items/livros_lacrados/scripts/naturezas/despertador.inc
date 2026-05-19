// naturezas/despertador.inc
// Natureza Despertador (8%): spawna NPC fantasma temporario vinculado ao leitor.
// 12 tipos ancorados no lore de Valtheris.

include ":livros_lacrados:scripts/ler";

function _CriarNPCPerto(who, template)
    var offsets := array{
        array{0, 0}, array{1, 0}, array{-1, 0}, array{0, 1}, array{0, -1},
        array{1, 1}, array{-1, 1}, array{1, -1}, array{-1, -1},
        array{2, 0}, array{-2, 0}, array{0, 2}, array{0, -2}
    };
    var i := 1;
    while (i <= offsets.size())
        var off := offsets[i];
        var npc := CreateNpcFromTemplate(template, who.x + off[1], who.y + off[2], who.z, 0, who.realm);
        if (npc && !npc.errortext)
            return npc;
        endif
        i += 1;
    endwhile
    // ultimo recurso: forca a posicao exata do jogador
    return CreateNpcFromTemplate(template, who.x, who.y, who.z, 0, who.realm, 1);
endfunction

function ExecutarDespertador(who, book, nivel, afinidade, lugar)
    var tipo := SortearTipoDespertador(nivel);

    var template := GetTemplateDespertador(tipo);
    var npc := _CriarNPCPerto(who, template);

    if (!npc || npc.errortext)
 SendSysMessageEX(who, "O espirito tenta se manifestar mas dissolve-se no ar.", SSM_INFO);
        return;
    endif

    var nome := GerarNomeDespertador(tipo);
    SetName(npc, nome);
 SetObjProperty(npc, "despertador_tipo", tipo);
 SetObjProperty(npc, "despertador_leitor", who.serial);
 SetObjProperty(npc, "despertador_expira", ReadGameClock() + (4 * 3600));
 SetObjProperty(npc, "livro_serial", book.serial);
 SetObjProperty(npc, "nivel_bau", nivel);

    // Guarda no livro: serial do ghost (para checar se ainda vive) + lore para releitura
 SetObjProperty(book, "despertador_npc_serial", npc.serial);
    var texto_lore := GetTextoGumpDespertador(tipo);
 SetObjProperty(book, "titulo_gerado", texto_lore[1]);
 SetObjProperty(book, "conteudo_gerado", texto_lore[2]);
 SetObjProperty(book, "tipo_inerte", "sujeito");

    ApresentarDespertador(who, npc, tipo);

    var texto := GetTextoGumpDespertador(tipo);
 _PaginarEMostrar(who, texto[1], texto[2], "sujeito", nivel, afinidade, "");

 start_script(":livros_lacrados:scripts/invocacoes/fantasma_temporario", array{npc, who, tipo});
endfunction

function SortearTipoDespertador(nivel)
    // Todos os niveis podem ter qualquer tipo, mas alguns sao mais raros em niveis baixos
    if (nivel == 3)
        var pool3 := array{
 "legionario_aiglano",
 "minerador_khar",
 "refugiado_leste",
 "servo_senhor_guerra",
 "mercador_enganado",
 "legionario_aiglano", // peso extra
 "minerador_khar" // peso extra
        };
        return pool3[RandomInt(7)+1];
    elseif (nivel == 4)
        var pool4 := array{
 "legionario_aiglano",
 "minerador_khar",
 "servo_senhor_guerra",
 "cultista_convertido",
 "elfo_exilado",
 "refugiado_leste",
 "cacadora_noites",
 "drow_exploradora",
 "pesquisador_limiar",
 "xama_bjorske",
 "mercador_enganado",
 "agente_corrompido"
        };
        return pool4[RandomInt(12)+1];
    endif
    // nivel 5 - qualquer tipo, incluindo os mais densos
    var pool5 := array{
 "cultista_convertido",
 "elfo_exilado",
 "drow_exploradora",
 "cacadora_noites",
 "servo_senhor_guerra",
 "legionario_aiglano",
 "minerador_khar",
 "refugiado_leste",
 "pesquisador_limiar",
 "xama_bjorske",
 "mercador_enganado",
 "agente_corrompido"
    };
    return pool5[RandomInt(12)+1];
endfunction

function GetTemplateDespertador(tipo)
    case (tipo)
 "legionario_aiglano": return ":epicAI:ghost5";
 "minerador_khar": return ":epicAI:ghost3";
 "servo_senhor_guerra": return ":epicAI:ghost5";
 "cultista_convertido": return ":epicAI:ghost9";
 "elfo_exilado": return ":epicAI:ghost";
 "drow_exploradora": return ":epicAI:ghost6";
 "refugiado_leste": return ":epicAI:ghost4";
 "cacadora_noites": return ":epicAI:ghost6";
 "pesquisador_limiar": return ":epicAI:ghost";
 "xama_bjorske": return ":epicAI:ghost9";
 "mercador_enganado": return ":epicAI:ghost3";
 "agente_corrompido": return ":epicAI:ghost9";
    endcase
 return ":epicAI:ghost";
endfunction

function GerarNomeDespertador(tipo)
    case (tipo)
 "legionario_aiglano":
            var n_leg := array{
 "Eco de Quintus da Vigesima",
 "Sombra do Centuria Gaius",
 "Memoria de Brennus Porthus",
 "Espirito do Soldado Sem Nome",
 "Restante de Marcus da Coorte"
            };
            return n_leg[RandomInt(5)+1];

 "minerador_khar":
            var n_min := array{
 "Eco de Aldrun Ko Durham",
 "Sombra do Mestre de Galeria",
 "Memoria do Mineiro Borin",
 "Espirito de Durgin Ko Veldrak",
 "Restante de Um Khar Sem Registro"
            };
            return n_min[RandomInt(5)+1];

 "servo_senhor_guerra":
            var n_serv := array{
 "Eco de Um Servo Fiel",
 "Sombra da Ultima Ordem",
 "Memoria do Guarda Pessoal",
 "Espirito da Vigilia Eterna",
 "Restante da Lealdade"
            };
            return n_serv[RandomInt(5)+1];

 "cultista_convertido":
            var n_cult := array{
 "Eco de Quem Viu o Simbolo",
 "Sombra do Convertido",
 "Memoria da Triade Invertida",
 "Espirito do Arrependimento",
 "Restante da Devocao Quebrada"
            };
            return n_cult[RandomInt(5)+1];

 "elfo_exilado":
            var n_elfo := array{
 "Eco de Tsu'nara do Exilio",
 "Sombra do Que Sabia Demais",
 "Memoria do Estudioso Expulso",
 "Espirito do Conselheiro Calado",
 "Restante de Um Nome Apagado"
            };
            return n_elfo[RandomInt(5)+1];

 "drow_exploradora":
            var n_drow := array{
 "Eco de Vel'shara do Limiar",
 "Sombra da Expedicao Idrith'har",
 "Memoria da Decima Segunda",
 "Espirito que Escolheu Ficar",
 "Restante da Exploradora"
            };
            return n_drow[RandomInt(5)+1];

 "refugiado_leste":
            var n_ref := array{
 "Eco de Ishikawa Perdido",
 "Sombra do Exilado Imperial",
 "Memoria do Que Cruzou as Montanhas",
 "Espirito do Refugiado Confuso",
 "Restante de Alguem que Partiu"
            };
            return n_ref[RandomInt(5)+1];

 "cacadora_noites":
            var n_cac := array{
 "Eco da Cacadora Sem Trilha",
 "Sombra da Vigilia da Floresta",
 "Memoria da Que Viu a Arvore",
 "Espirito da Guardia Esquecida",
 "Restante da Cacadora Brumajord"
            };
            return n_cac[RandomInt(5)+1];

 "pesquisador_limiar":
            var n_pesq := array{
 "Eco do Estudioso Partido ao Meio",
 "Sombra do Que Ficou Entre Estados",
 "Memoria do Pesquisador Incompleto",
 "Espirito do Que Ainda Trabalha",
 "Restante de Um Nome em Dois Lugares"
            };
            return n_pesq[RandomInt(5)+1];

 "xama_bjorske":
            var n_xama := array{
 "Eco do Xama que Viu a Arvore",
 "Sombra de Gunnar o Silencioso",
 "Memoria do Que Falou com Vorar",
 "Espirito do Guardiao de Brumajord",
 "Restante do Que Ficou Embaixo"
            };
            return n_xama[RandomInt(5)+1];

 "mercador_enganado":
            var n_merc := array{
 "Eco do Mercador Sem Rota",
 "Sombra do Que Foi Usado",
 "Memoria do Negocio Errado",
 "Espirito do Que Nao Sabia o Que Carregava",
 "Restante de Uma Divida Que Nao Era Sua"
            };
            return n_merc[RandomInt(5)+1];

 "agente_corrompido":
            var n_agente := array{
 "Eco do Agente Que Viu o Simbolo",
 "Sombra do Que Serviu Sem Saber",
 "Memoria do Arrependido da Liga",
 "Espirito do Que Nao Consegue Descansar",
 "Restante de Uma Lealdade Roubada"
            };
            return n_agente[RandomInt(5)+1];
    endcase
 return "Espirito sem Nome";
endfunction

function ApresentarDespertador(who, npc, tipo)
    case (tipo)
 "legionario_aiglano":
 PrintTextAbove(npc, "*assume posicao de sentinela*");
 SendSysMessageEX(who, "Uma presenca marcial se materializa. Cheira a ferro velho e campanha longa.", SSM_INFO);
            break;

 "minerador_khar":
 PrintTextAbove(npc, "*olha ao redor como se procurasse as paredes de uma galeria*");
 SendSysMessageEX(who, "Uma figura robusta aparece. Seus olhos procuram o teto que nao esta la.", SSM_INFO);
            break;

 "servo_senhor_guerra":
 PrintTextAbove(npc, "*fica em posicao de espera, como alguem aguardando uma ordem*");
 SendSysMessageEX(who, "Um espirito disciplinado se manifesta. Parece estar esperando algo ha muito tempo.", SSM_INFO);
            break;

 "cultista_convertido":
 PrintTextAbove(npc, "*cobre o pulso esquerdo com a mao direita, ocultando algo*");
 SendSysMessageEX(who, "Uma figura agitada aparece. Parece carregar um arrependimento que nao cabe mais nela.", SSM_INFO);
            break;

 "elfo_exilado":
 PrintTextAbove(npc, "*observa voce com olhos que viram muitas eras*");
 SendSysMessageEX(who, "Uma presenca antiga e serena se forma. Ha uma tristeza nela que parece ter sido escolhida.", SSM_INFO);
            break;

 "drow_exploradora":
 PrintTextAbove(npc, "*move-se com precisao que nao pertence ao lugar onde esta*");
 SendSysMessageEX(who, "Uma figura adaptada ao limiar aparece. Esta aqui mas tambem nao esta completamente.", SSM_INFO);
            break;

 "refugiado_leste":
 PrintTextAbove(npc, "*olha ao redor com olhos que reconhecem mas nao entendem*");
 SendSysMessageEX(who, "Uma presenca confusa se manifesta. Esta desorientada - procura algo que este mundo nao tem.", SSM_INFO);
            break;

 "cacadora_noites":
 PrintTextAbove(npc, "*fica completamente imovel, como quem segura a respiracao antes de atirar*");
 SendSysMessageEX(who, "Uma cacadora fantasma aparece. Esta em alerta. Sempre em alerta.", SSM_INFO);
            break;

 "pesquisador_limiar":
 PrintTextAbove(npc, "*olha para as proprias maos como se nao tivesse certeza de onde elas estao*");
 SendSysMessageEX(who, "Uma presenca estranha se forma - parece estar aqui e em outro lugar ao mesmo tempo.", SSM_INFO);
            break;

 "xama_bjorske":
 PrintTextAbove(npc, "*levanta os bracos como invocando algo que nao vem mais*");
 SendSysMessageEX(who, "Uma figura possante de Brumajord se materializa. Carrega o cheiro de neve e resina de pinho.", SSM_INFO);
            break;

 "mercador_enganado":
 PrintTextAbove(npc, "*verifica o bolso como buscando documentos que nao estao la*");
 SendSysMessageEX(who, "Um mercador fantasma aparece. Parece surpreso de estar onde esta.", SSM_INFO);
            break;

 "agente_corrompido":
 PrintTextAbove(npc, "*olha sobre o proprio ombro como se esperasse ser seguido*");
 SendSysMessageEX(who, "Uma figura agitada se manifesta. Os olhos dela nao param de varrer o espaco ao redor.", SSM_INFO);
            break;
    endcase
endfunction

// aaa TEXTO DO GUMP DE LEITURA POR TIPO aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
// Retorna array{titulo, conteudo} exibido no gump antes do fantasma interagir.

function GetTextoGumpDespertador(tipo)
    case (tipo)
 "legionario_aiglano":
            return array{
 "Memoria de um Legionario",
 "As paginas exalam ferro e poeira de campanha. Um soldado que nao sabe que a guerra terminou - ou que nao aceita."
            };
 "minerador_khar":
            return array{
 "Eco de um Minerador Khar",
 "A tinta tem cor de basalto e o papel ainda parece conter o peso das profundezas. Alguem que foi longe demais nas galerias."
            };
 "servo_senhor_guerra":
            return array{
 "Sombra de uma Lealdade",
 "Escrito com mao firme - a mao de quem cumpriu ordens por tanto tempo que nao sabe mais existir sem elas."
            };
 "cultista_convertido":
            return array{
 "Arrependimento em Tinta",
 "As paginas queimam levemente ao toque. Um convertido que saiu da ordem mas nao conseguiu deixar a ordem sair dele."
            };
 "elfo_exilado":
            return array{
 "Palavras de um Exilio Escolhido",
 "Escrito em Tsu'ngathar e depois traduzido - ou o contrario. A ordem nao e clara. O peso, sim."
            };
 "drow_exploradora":
            return array{
 "Diario do Limiar",
 "Anotacoes de quem cruzou o Limiar e voltou. Ou voltou parcialmente. A fronteira entre os dois estados ainda esta impressa nas paginas."
            };
 "refugiado_leste":
            return array{
 "Memoria de Quem Cruzou as Montanhas",
 "Escrito em dois idiomas que se alternam sem padrao. A lingua vai saindo enquanto escreve - voce ve isso nas paginas."
            };
 "cacadora_noites":
            return array{
 "Rastro sem Trilha",
 "As palavras estao dispostas como pegadas. Alguem que rastreou algo que nao deveria ter seguido ate o fim."
            };
 "pesquisador_limiar":
            return array{
 "Notas Incompletas",
 "O texto termina no meio de uma frase e recomeca em outro lugar. Quem escreveu estava em dois lugares ao mesmo tempo."
            };
 "xama_bjorske":
            return array{
 "Visao do Xama",
 "Escrito em runico Bjorske com tinta que parece ter sangrado da pedra. Palavras sobre a Arvore e o que bebe embaixo dela."
            };
 "mercador_enganado":
            return array{
 "Manifesto de uma Carga Errada",
 "Contas, rotas, numeros - e no meio deles, algo que nao deveria estar ali. O mercador notou tarde demais."
            };
 "agente_corrompido":
            return array{
 "Confissao de Trinta e Dois Anos",
 "Relatorios da Liga com anotacoes a margem. O agente foi fiel por decadas ao servico errado - e so percebeu quando era tarde."
            };
    endcase
 return array{"Paginas Desbotadas", "Palavras que quase formam sentido. Uma presenca que quase forma forma."};
endfunction
