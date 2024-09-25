include ":gumps:gumps";
include ":attributes:attributes";
include ":gumps:gumps_ex";
include ":gumps:requestGump";
include "include/say";
include ":tn:tngumps";
include "include/client";
include ":charactercreation:habilidades";
include ":timedscripts:timedScripts";

// lore_items.inc

const LORE_ITEMS_COUNT := 150;

function GetLoreItemInfo(item_id)
    case (item_id)
        0xE340: return GetMarcaGadoDuwInfo();
        0xE341: return GetDados6LadosInfo();
        0xE342: return GetCranioHumanInfo();
        0xE343: return GetPergaminhoOrientalInfo();
        0xE344: return GetTrompaDeChifreInfo();
        0xE345: return GetRedeBorboletasInfo();
        0xE346: return GetMacaEstrelaQuebradaInfo();
        0xE347: return GetMarteloGuerraQuebradoInfo();
        0xE348: return GetGarfoGuerraQuebradoInfo();
        0xE349: return GetEspadaQuebradoInfo();
        0xE34A: return GetEspadaLongaQuebradoInfo();
        0xE34B: return GetMacaQuebradaInfo();
        0xE34C: return GetCimitarraQuebradaInfo();
        0xE34D: return GetKatanaQuebradaInfo();
        0xE34E: return GetLancaQuebradaInfo();
        0xE34F: return GetMachadoDuploQuebradoInfo();
        0xE350: return GetMachadoExecutorQuebradoInfo();
        0xE351: return GetBonecaInfantilBrancaInfo();
        0xE352: return GetColarPerolasAntigoInfo();
        0xE353: return GetLoricaSegmentataInfo();
        0xE354: return GetGaleaInfo();
        0xE355: return GetEstatuaFilosofoEstoicoInfo();
        0xE356: return GetMascaraMortuariaOuroInfo();
        0xE357: return GetChapeuPlumaCorvoNegroInfo();
        0xE358: return GetMascaraAranhaNegra();
        0xE359: return GetMascaraCranioCervo();
        0xE35A: return GetFerraduraOuro();
        0xE35B: return GetTacaVitoriaOlimpiada();
        0xE35C: return GetEstatuaAranhaNegra();
        0xE35D: return GetBustoPatriciaAiglana();
        0xE35E: return GetBustoSenadorAiglano();
        0xE35F: return GetTochasAventureiro();
        0xE360: return GetDocumentosAiglanos();
        0xE361: return GetTrombetaAiglana();
        0xE362: return GetQuadroLadyLavinia();
        0xE363: return GetSignPostTaverna();
        0xE364: return GetCartaTarotLouco();
        0xE365: return GetCartaTarotMago();
        0xE366: return GetCartaTarotSacerdotisa();
        0xE367: return GetCartaTarotImperatriz();
        0xE368: return GetCartaTarotImperador();
        0xE369: return GetCartaTarotSacerdote();
        0xE36A: return GetCartaTarotEnamorados();
        0xE36B: return GetCartaTarotCarruagem();
        0xE36C: return GetCartaTarotForca();
        0xE36D: return GetCartaTarotSabio();
        0xE36E: return GetCartaTarotRodaFortuna();
        0xE36F: return GetCartaTarotJustica();
        0xE370: return GetCartaTarotEnforcado();
        0xE371: return GetCartaTarotRenascimento();
        0xE372: return GetCartaTarotTranquilidade();
        0xE373: return GetCartaTarotCobra();
        0xE374: return GetCartaTarotTorre();
        0xE375: return GetCartaTarotEstrela();
        0xE376: return GetCartaTarotLua();
        0xE377: return GetCartaTarotSol();
        0xE378: return GetCartaTarotJulgamento();
        0xE379: return GetCartaTarotMundo();
        0xE37A: return GetGloboDeGeloPolski();
        0xE37B: return GetGloboDeGeloAluriel();
        0xE37C: return GetGloboDeGeloGulthrak();
        0xE37D: return GetGloboDeGeloKaijing();
        0xE37E: return GetGloboDeGeloAigla();
        0xE37F: return GetGloboDeGeloKhronKazad();
        0xE380: return GetGloboDeGeloBorfjell();
        0xE381: return GetGloboDeGeloNygurath();
        0xE382: return GetCraneoOuroMonstro();
        0xE383: return GetCraneoDeemonioCornudo();
        0xE384: return GetPosteLampadaOleo();
        0xE385: return GetTrevoQuatroFolhas();
        0xE386: return GetQuadroGrandeLich();
        /*0xE387: return GetMapaNautico();
        0xE388: return GetBarcoGarrafa();
        0xE389: return GetColarOuroPrefeito();
        0xE38A: return GetVasoAiglanoLargo();
        0xE38B: return GetGrandeLivroMagia();
        0xE38C: return GetGrandeAdagaCerimonial();
        0xE38D: return GetPequenoFrascoVeneno();
        0xE38E: return GetPequenaEscalaPesagemAlquimica();
        0xE38F: return GetBoleadera();
        0xE390: return GetCapaceteGuerraSamurai();
        0xE391: return GetKatanaDisplayPe();
        0xE392: return GetBonsai();
        0xE393: return GetDuasKatanasDisplayHorizontal();
        0xE394: return GetChapeuCapitaoPirata();
        0xE395: return GetEscalaPesagemAna();
        0xE396: return GetEscudoRomanoTriangular();
        0xE397: return GetCapaceteCranioOrc();
        0xE398: return GetAbajurOrientalAntigo();
        0xE399: return GetEstandarteRomano();
        0xE39A: return GetEstandarteDrow();
        0xE39B: return GetArmaduraCouro();
        0xE39C: return GetArmaduraCotaMalha();
        0xE39D: return GetMiniaturaNavioRomano();
        0xE39E: return GetCapaceteVikingAsasAnjo();
        0xE39F: return GetMarteloGuerraAnao();
        0xE3A0: return GetEspadasDuplas();
        0xE3A1: return GetEstandarteOrcsCaveirasSangue();
        0xE3A2: return GetEstandarteElfosAzulBranco();
        0xE3A3: return GetCimitarraEnsanguentada();
        0xE3A4: return GetEspadaLongaEnsanguentada();
        0xE3A5: return GetEscudoRedondoBronzeEnsanguentado();
        0xE3A6: return GetEstandarteElfosFlorestaArvore();
        0xE3A7: return GetEscudoElficoAzulUnicornioBranco();
        0xE3A8: return GetLivroNecromanciaCapaCouroCaveira();
        0xE3A9: return GetLivroLeisSenadorCapaCinza();
        0xE3AA: return GetLivroCapaCinzaMarcaPaginaVerde();
        0xE3AB: return GetDisplayPeitoralFullplate();
        0xE3AC: return GetPitchforkEnsanguentada();
        0xE3AD: return GetEspelhoCorpo();
        0xE3AE: return GetCabecaCiclopeEmpalhada();
        0xE3AF: return GetLanternaFurtaFogo();
        0xE3B0: return GetRifleCacaAnao();
        0xE3B1: return GetHandcannonAnao();
        0xE3B2: return GetLancaCavalariaJusta();
        0xE3B3: return GetSaisDuplos();
        0xE3B4: return GetMaoRessecadaDemonio();
        0xE3B5: return GetMiniaturaBjorskeKnight();
        0xE3B6: return GetEstatuaPedraAnaoMachado();*/
        default: return 0;
    endcase
endfunction

function GetMarcaGadoDuwInfo()
    var info := struct;
    info.+name := "Marca de Gado de Duw";
    info.+image := 0xF8E4;
    info.+flavor_image := 18004;
    info.+culture_name := "Björske";
    info.+main_text := "Durante um rigoroso inverno no norte, a tribo Björske lutava para manter seu rebanho de gado. Bjorn, o líder da aldeia, pegou o ferro de marcar recém-forjado e, sob a lua cheia, marcou o touro mais forte com o símbolo sagrado de Duw, seu deus do sol. A cerimônia de marcação garantiu que o touro lideraria o rebanho com segurança pelas traiçoeiras florestas cobertas de neve. Semanas depois, durante uma tempestade, o touro, agora infundido com proteção divina, guiou os aldeões e seu rebanho por um desfiladeiro perigoso, salvando-os de uma matilha de lobos famintos.";
    info.+skill_text := "O ferro sagrado, gravado com o símbolo de Duw, acreditava-se conceder orientação divina ao gado, protegendo a tribo durante as migrações. Tornou-se uma ferramenta reverenciada entre os Björske";
    info.+archaeology_text := "O poder desse ferro ligava a sobrevivência do gado Björske à proteção de seus deuses.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction


function GetDados6LadosInfo()
    var info := struct;
    info.+name := "Dados de 6 lados";
    info.+image := 0xF904;
    info.+flavor_image := 18005;
    info.+culture_name := "Aiglano";
    info.+main_text := "Nas fronteitas de Aigla, um grupo de legionários estava sentado em volta de uma fogueira após uma batalha exaustiva. Crispinus, um centurião, tirou um par de dados de seis lados esculpidos em marfim. Os dados haviam sido abençoados por um augúrio do templo, assegurando o destino dos guerreiros. Uma noite, durante uma aposta, os dados previram uma emboscada. Em vez de ignorar o presságio, os legionários se prepararam. No dia seguinte, uma força inimiga tentou um ataque surpresa, mas foi rapidamente derrotada graças à vigilância deles.";
    info.+skill_text := " Esses dados abençoados se tornaram um símbolo de previsão e sorte entre os legionários Aiglanos, guiando-os em inúmeras batalhas perigosas.";
    info.+archaeology_text := "Destino e fortuna se entrelaçaram enquanto os dados guiavam os legionários Aiglanos a evitar desastres.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction


function GetCranioHumanInfo()
    var info := struct;
    info.+name := "Crânio Human";
    info.+image := 0xF924;
    info.+flavor_image := 18006;
    info.+culture_name := "Har'oloth";
    info.+main_text := "Em uma câmara escura sob a antiga cidade drow, um ritual solene ocorria. O ancião Har'oloth, envolto em vestes cerimoniais, colocou o crânio de um antepassado caído em um altar de obsidiana. O crânio pertencia a um grande feiticeiro Har'oloth que morreu durante a primeira invasão da superfície. Enquanto o incenso enchia a sala, o ancião invocou o espírito do crânio, buscando orientação sobre a guerra iminente. À luz tremeluzente das velas, os olhos do crânio pareciam brilhar enquanto o espírito falava, entregando uma mensagem de dor e morte.";
    info.+skill_text := "O crânio de um feiticeiro reverenciado era usado pelos Har'oloth para se comunicar com os ancestrais antes de grandes conflitos.";
    info.+archaeology_text := "O crânio brilhante revelou sabedoria, garantindo que os Har'oloth estivessem preparados para a guerra.";
    info.+required_skill := "Survival";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction


function GetPergaminhoOrientalInfo()
    var info := struct;
    info.+name := "Pergaminho Oriental";
    info.+image := 0xF936;
    info.+flavor_image := 18007;
    info.+culture_name := "Kaijing";
    info.+main_text := "No Templo de Kaijing, um monge aprendiz desenrolou um pergaminho antigo com segredos de vida e morte. Durante um cerco, o monge-chefe usou o pergaminho para invocar uma névoa mortal, escondendo o templo dos invasores. Os inimigos foram consumidos pela neblina sobrenatural, permitindo que os monges escapassem. O pergaminho tornou-se um símbolo de proteção e sabedoria ancestral, guardado zelosamente pelos mestres do templo.";
    info.+skill_text := "Este pergaminho antigo, contendo poderosos feitiços de vida e morte, foi fundamental na defesa do Templo de Kaijing durante um cerco inimigo.";
    info.+archaeology_text := "O poder do pergaminho invocou uma névoa, garantindo a sobrevivência dos monges Kaijing durante a invasão.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetTrompaDeChifreInfo()
    var info := struct;
    info.+name := "Trompa de Chifre";
    info.+image := 0xfa55;
    info.+flavor_image := 18008;
    info.+culture_name := "Björske";
    info.+main_text := "Numa manhã enevoada, o chefe Björske soprou uma trompa feita do chifre de um touro sagrado. Seu som ecoou pelos fiordes, convocando guerreiros. A trompa, esculpida com runas de força e união, foi usada para reunir defensores durante o Cerco de Aigla. O chamado encheu os guerreiros de coragem, lembrando-os das vitórias ancestrais. A trompa tornou-se um símbolo de unidade e força diante da adversidade.";
    info.+skill_text := "O poderoso chamado da trompa ecoou pelas terras Björske, convocando guerreiros para a batalha, seu som sagrado lembrando a força dos ancestrais.";
    info.+archaeology_text := "O som da trompa incendiou o espírito Björske, unindo a tribo para a batalha que se aproximava.";
    info.+required_skill := "Survival";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetRedeBorboletasInfo()
    var info := struct;
    info.+name := "Rede de Borboletas";
    info.+image := 0xf88a;
    info.+flavor_image := 18009;
    info.+culture_name := "Polski";
    info.+main_text := "Nos campos de Colhesol, uma criança Polski corria com uma rede de pegar borboletas finamente tecida. A rede, passada por gerações, simbolizava a conexão com a natureza. Ao capturar uma borboleta azul, sua avó contou sobre o festival da colheita, onde borboletas raras eram capturadas como bênçãos. Cada borboleta era solta com uma oração, garantindo colheitas abundantes e boa sorte para a aldeia.";
    info.+skill_text := "A rede de pegar borboletas, tecida com cuidado, é mais que uma ferramenta — é um instrumento sagrado dos Polski, usado durante festivais de colheita.";
    info.+archaeology_text := "Capturar borboletas com esta rede traz bênçãos, garantindo a prosperidade do povo Polski.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMacaEstrelaQuebradaInfo()
    var info := struct;
    info.+name := "Maça Estrela Quebrada";
    info.+image := 0xf845;
    info.+flavor_image := 18010;
    info.+culture_name := "Badûran";
    info.+main_text := "Durante uma batalha sob Khron Kaz'ad, o general Badûran Dorrak empunhava uma maça estrela forjada num vulcão. A maça quebrou ao atingir a armadura de um invasor gigante. Os fragmentos espalharam-se, brilhando com runas flamejantes. Apesar da arma quebrada, os guerreiros de Dorrak, inspirados por sua determinação, lutaram ferozmente e repeliram o inimigo. A maça tornou-se um símbolo de resiliência Badûran.";
    info.+skill_text := "Embora quebrada, as peças fragmentadas da maça tornaram-se símbolos dae resiliência para os Badûran, representando a determinação inabalável dos anões diante da derrota.";
    info.+archaeology_text := "A maça quebrada simbolizava o espírito inabalável dos Badûran, inspirando-os à vitória.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMarteloGuerraQuebradoInfo()
    var info := struct;
    info.+name := "Martelo de Guerra Quebrada";
    info.+image := 0xf843;
    info.+flavor_image := 18011;
    info.+culture_name := "Björske";
    info.+main_text := "Após derrotar uma legião Aiglana, o chefe Björske Sigmar ergueu seu martelo de guerra ensanguentado. Ao desferir o golpe final no escudo inimigo, o cabo partiu-se. Sigmar ergueu a arma quebrada como símbolo de vitória. O martelo tornou-se um emblema da perseverança Björske, exposto no grande salão. Cada geração adicionava uma marca, transformando-o numa crônica viva das batalhas da tribo.";
    info.+skill_text := "O martelo de guerra quebrado de Sigmar tornou-se um símbolo reverenciado, lembrando os Björske de que a vitória não vem de ferramentas perfeitas, mas de uma vontade inquebrável.";
    info.+archaeology_text := "O martelo quebrado simbolizava a força dos Björske na batalha, persistindo mesmo com ferramentas destruídas.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction
function GetGarfoGuerraQuebradoInfo()
    var info := struct;
    info.+name := "Garfo de Guerra Quebrada";
    info.+image := 0xf841;
    info.+flavor_image := 18012;
    info.+culture_name := "Aigla";
    info.+main_text := "Nas escaramuças contra orcs, um centurião Aiglano empunhava um pesado garfo de guerra de três pontas. Durante o confronto final, o garfo foi capturado pelo escudo de um orc, quebrando uma ponta. Com duas pontas restantes, o centurião improvisou, desarmando e derrotando seu oponente. O dano à arma tornou-se um testemunho da adaptabilidade Aiglana em combate. O garfo quebrado foi consagrado no quartel da legião como símbolo de engenhosidade.";
    info.+skill_text := "O garfo de guerra quebrado, apesar de perder uma ponta, tornou-se um testemunho da adaptabilidade Aiglana em combate, mostrando a engenhosidade do centurião sob pressão.";
    info.+archaeology_text := "O garfo de guerra quebrado simbolizou a habilidade dos Aiglanos de se adaptar, transformando falhas em força.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEspadaQuebradoInfo()
    var info := struct;
    info.+name := "Espada Quebrada";
    info.+image := 0xf83f;
    info.+flavor_image := 18013;
    info.+culture_name := "Björske";
    info.+main_text := "Durante a invasão de Aigla, o herói Bjorske Vargus enfrentou um comandante Aiglano. Suas espadas colidiram, e a lâmina de Vargus partiu-se ao meio. Recusando-se a se render, Vargus empunhou a espada quebrada como uma adaga, movendo-se mais rápido e com precisão. Ele desarmou seu oponente e reivindicou a vitória. A espada quebrada tornou-se lendária, simbolizando a resiliência Bjorske e sua capacidade de transformar ferramentas quebradas em instrumentos de vitória.";
    info.+skill_text := "A espada quebrada de Vargus tornou-se um símbolo da resiliência Bjorske, mostrando que mesmo armas destruídas ainda podem servir nas mãos de um guerreiro habilidoso.";
    info.+archaeology_text := "A espada quebrada simbolizava a adaptabilidade dos Bjorske em batalha, onde até ferramentas destruídas encontravam propósito.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEspadaLongaQuebradoInfo()
    var info := struct;
    info.+name := "Espada Longa Quebrada";
    info.+image := 0xF835;
    info.+flavor_image := 18014;
    info.+culture_name := "Aigla";
    info.+main_text := "No cerco a Aigla, o jovem cavaleiro Marcus defendia as muralhas com uma espada longa reluzente. No confronto final, a lâmina quebrou ao meio. Com apenas o punho e um fragmento da lâmina, ele continuou lutando, cravando a espada quebrada na garganta do comandante inimigo. A vitória foi conquistada, e a espada tornou-se um símbolo da tenacidade de Marcus. Foi encravada no Salão dos Heróis, lembrando a todos da determinação inabalável dos defensores de Aigla.";
    info.+skill_text := "A espada longa quebrada de Marcus, embora estilhaçada, simbolizava a resiliência e coragem aiglanas, provando que até mesmo uma lâmina quebrada pode garantir a vitória.";
    info.+archaeology_text := "A espada longa quebrada incorpora o espírito dos guerreiros aiglanos: lutando, mesmo com armas quebradas.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMacaQuebradaInfo()
    var info := struct;
    info.+name := "Maça Quebrada";
    info.+image := 0XF837;
    info.+flavor_image := 18015;
    info.+culture_name := "Badûran";
    info.+main_text := "Nos túneis de Khron Kaz'ad, o Badûran Thorek brandia uma maça de ferro contra golems invasores. Ao atingir o mais duro dos golems, a maça se despedaçou. Thorek usou os restos irregulares para esmagar o núcleo do golem, transformando a ferramenta quebrada em arma mortal. A maça quebrada tornou-se um artefato lendário, simbolizando a adaptabilidade e feroz determinação Badûran em batalha, mesmo contra probabilidades esmagadoras.";
    info.+skill_text := "A maça quebrada de Thorek, embora destruída em combate, tornou-se uma ferramenta de vitória, destacando a engenhosidade e o poder de combate dos anões.";
    info.+archaeology_text := "A maça quebrada simboliza a adaptabilidade dos Badûran, transformando derrota em triunfo no campo de batalha.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCimitarraQuebradaInfo()
    var info := struct;
    info.+name := "Cimitarra Quebrada";
    info.+image := 0XF839;
    info.+flavor_image := 18016;
    info.+culture_name := "Kaijing";
    info.+main_text := "Nos degraus do Templo de Kaijing, o monge guerreiro Jinhai enfrentou um invasor aiglano. Sua cimitarra bloqueou um golpe fatal, mas a lâmina se estilhaçou. Sem hesitar, Jinhai girou a lâmina quebrada em empunhadura reversa e desarmou seu oponente. A cimitarra quebrada foi preservada no templo, simbolizando o estilo de combate fluido do monge, onde até armas estilhaçadas podiam ser usadas com efeito devastador.";
    info.+skill_text := "A cimitarra quebrada de Jinhai demonstrou a maestria fluida dos monges Kaijing, transformando derrota em uma vitória rápida e inesperada.";
    info.+archaeology_text := "A cimitarra estilhaçada tornou-se um símbolo da maestria Kaijing, onde armas quebradas ainda trazem vitória.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetKatanaQuebradaInfo()
    var info := struct;
    info.+name := "Katana Quebrada";
    info.+image := 0XF83B;
    info.+flavor_image := 18017;
    info.+culture_name := "Kaijing";
    info.+main_text := "No templo Kaijing, o Mestre Yurei treinava com uma katana sagrada. Durante um duelo com um senhor da guerra invasor, a lâmina colidiu contra um escudo encantado, quebrando-se. Com meia lâmina, Yurei continuou lutando, aparando e desviando golpes até desarmar o inimigo. A katana quebrada tornou-se uma relíquia sagrada, simbolizando a filosofia Kaijing de encontrar força na quebra e a habilidade de superar adversidades com técnica e sabedoria.";
    info.+skill_text := "A katana quebrada de Yurei tornou-se um símbolo reverenciado no templo, incorporando a resiliência do monge e a força encontrada na imperfeição.";
    info.+archaeology_text := "A katana quebrada é uma relíquia Kaijing, simbolizando resiliência e maestria diante da adversidade.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetLancaQuebradaInfo()
    var info := struct;
    info.+name := "Lança Quebrada";
    info.+image := 0XF83D;
    info.+flavor_image := 18018;
    info.+culture_name := "Aigla";
    info.+main_text := "Na batalha por Aigla, um jovem legionário lançou sua lança contra um chefe inimigo. A ponta quebrou na armadura do adversário, deixando apenas um cabo irregular. O legionário usou a lança quebrada para derrubar seu oponente e desferir o golpe fatal. A lança foi exposta nos quartéis aiglanos, lembrando que até uma arma quebrada pode superar um inimigo poderoso quando empunhada com coragem e determinação.";
    info.+skill_text := "A lança quebrada, apesar de perder a ponta, provou que os legionários aiglanos podiam transformar até mesmo armas danificadas em instrumentos de vitória.";
    info.+archaeology_text := "A lança quebrada tornou-se um símbolo da engenhosidade aiglana em combate, transformando infortúnio em sucesso.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 75.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMachadoDuploQuebradoInfo()
    var info := struct;
    info.+name := "Machado Duplo Quebrado";
    info.+image := 0XF829;
    info.+flavor_image := 18019;
    info.+culture_name := "Gulthrak";
    info.+main_text := "Durante uma batalha contra um posto avançado Aiglano, o berserker Gulthrak, Varn, brandiu seu machado duplo com fúria. Ao colidir com um escudo inimigo, o cabo partiu-se e uma lâmina quebrou. Varn agarrou a parte quebrada e continuou lutando com a lâmina restante, derrubando mais três inimigos. O machado quebrado foi levado de volta à tribo e exposto como símbolo da brutalidade e força dos Gulthrak, inspirando gerações de guerreiros.";
    info.+skill_text := "O machado duplo quebrado de Varn tornou-se um símbolo da ferocidade Gulthrak, mostrando que até mesmo armas danificadas em suas mãos poderiam causar destruição implacável.";
    info.+archaeology_text := "O machado quebrado simboliza a fúria indomável dos guerreiros Gulthrak no calor da batalha.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMachadoExecutorQuebradoInfo()
    var info := struct;
    info.+name := "Machado do Executor Quebrado";
    info.+image := 0XF82B;
    info.+flavor_image := 18021;
    info.+culture_name := "Bjorske";
    info.+main_text := "Durante uma execução pública em Börfjell, o famoso executor da cidade abateu um traidor com seu machado lendário. Ao atingir o pescoço do criminoso, o machado rachou. Apesar da quebra, o executor cumpriu seu dever com precisão. O machado estilhaçado tornou-se um símbolo da justiça aiglana e da imperfeição do poder, exposto no tribunal da cidade para lembrar a todos do peso do julgamento e da responsabilidade que vem com autoridade.";
    info.+skill_text := "O machado quebrado do executor, embora estilhaçado, se tornou um símbolo da justiça Bjorske, representando tanto sua severidade quanto fragilidade.";
    info.+archaeology_text := "O machado quebrado serviu como um lembrete do peso da justiça e da imperfeição do poder.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetBonecaInfantilBrancaInfo()
    var info := struct;
    info.+name := "Boneca Infantil Branca";
    info.+image := 0XF0A4;
    info.+flavor_image := 18023;
    info.+culture_name := "Aigla";
    info.+main_text := "Em Aigla, uma criança nobre carregava uma delicada boneca branca, seu rosto de porcelana pintado por artesãos habilidosos. Vestida com finas sedas, a boneca representava riqueza e status. Durante uma celebração no palácio imperial, era carregada como símbolo de prosperidade familiar. Em tempos difíceis, era exibida na casa, acreditando-se trazer sorte. A boneca tornou-se um símbolo de continuidade e tradição na sociedade aiglana.";
    info.+skill_text := "A boneca branca, símbolo da riqueza e tradição aiglanas, era passada de família nobre para família, representando continuidade e prosperidade.";
    info.+archaeology_text := "A boneca branca simboliza o status da elite aiglana, incorporando riqueza e esperança de prosperidade.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetColarPerolasAntigoInfo()
    var info := struct;
    info.+name := "Colar de Pérolas Antigo";
    info.+image := 0XF1D2;
    info.+flavor_image := 18024;
    info.+culture_name := "Aluriel";
    info.+main_text := "Nos salões dos elfos Aluriel, um antigo colar de pérolas adornava o pescoço da alta sacerdotisa Ellendora durante cerimônias sagradas. Cada pérola carregava a sabedoria dos ancestrais, coletadas das profundezas do oceano. Durante um ritual de previsão, o colar brilhava suavemente, suas pérolas cintilando com luz etérea. Passado entre as sacerdotisas, o colar servia como conexão com os espíritos do mar e a antiga magia dos Aluriel, simbolizando sabedoria e o vínculo profundo dos elfos com a natureza.";
    info.+skill_text := "O antigo colar de pérolas, usado por gerações de sacerdotisas Aluriel, simboliza a sabedoria, tradição e conexão dos elfos com o mar, carregando a magia de seus ancestrais.";
    info.+archaeology_text := "O colar de pérolas brilhantes conecta as sacerdotisas Aluriel à sabedoria e à magia antiga de seus ancestrais.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetLoricaSegmentataInfo()
    var info := struct;
    info.+name := "Lorica Segmentata";
    info.+image := 0XF27F;
    info.+flavor_image := 18025;
    info.+culture_name := "Aigla";
    info.+main_text := "Nas legiões aiglanas, a lorica segmentata era usada pelos soldados de elite. O centurião Crassus vestiu sua lorica durante o Cerco de Borfjell, onde sua unidade suportou incontáveis ataques. A armadura, marcada por flechas e golpes de espada, o protegeu enquanto liderava seus soldados à vitória. Após a batalha, tornou-se um símbolo de força e resistência aiglana, passada por gerações de legionários, cada um a vestindo com orgulho e honra.";
    info.+skill_text := "A lorica segmentata, usada pelos legionários aiglanos, simboliza força e resistência, suas placas interligadas oferecendo proteção nas batalhas mais ferozes e passada como um símbolo de orgulho.";
    info.+archaeology_text := "A armadura segmentada simboliza a disciplina, força e proteção aiglanas no calor da batalha.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetGaleaInfo()
    var info := struct;
    info.+name := "Gálea";
    info.+image := 0XF27E;
    info.+flavor_image := 18026;
    info.+culture_name := "Aigla";
    info.+main_text := "A gálea, capacete icônico dos legionários aiglanos, foi usada pelo centurião Marcus na Batalha da Frente Ocidental. Seu design reforçado o protegeu de golpes fatais enquanto liderava seus homens. Após a batalha, o capacete, marcado pela luta, tornou-se uma relíquia passada para futuros líderes da legião. A gálea simbolizava coragem e liderança no campo de batalha, inspirando gerações de soldados aiglanos a enfrentar o perigo com bravura.";
    info.+skill_text := "A gálea de Marcus, marcada pela batalha, tornou-se um símbolo de coragem e liderança aiglana, passada por gerações de legionários como símbolo de força e honra.";
    info.+archaeology_text := "A gálea simboliza a liderança aiglana, sua superfície desgastada representando coragem e sobrevivência em batalha.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 75.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEstatuaFilosofoEstoicoInfo()
    var info := struct;
    info.+name := "Estátua de um Antigo Filósofo Estoico Aiglano Feita de Mármore";
    info.+image := 0xED2A;
    info.+flavor_image := 18027;
    info.+culture_name := "Aigla";
    info.+main_text := "No fórum de Aigla, erguia-se a estátua de mármore do filósofo Neronas, cujos ensinamentos moldaram os valores imperiais de disciplina e autocontrole. Esculpida em mármore branco, retratava Neronas em reflexão profunda, segurando um pergaminho. Erguida após sua morte, honrava seu papel em guiar o senado e acalmar turbulências políticas. A estátua tornou-se ponto de encontro para estudantes de filosofia, lembrando a todos a importância da razão sobre a paixão na governança de Aigla.";
    info.+skill_text := "A estátua de mármore de Neronas, um filósofo estoico influente, simboliza os ideais aiglanos de disciplina e razão, permanecendo orgulhosamente no fórum onde estudantes ainda se reúnem para aprender.";
    info.+archaeology_text := "A estátua simboliza a sabedoria filosófica aiglana, promovendo autocontrole e governo racional.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 90.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMascaraMortuariaOuroInfo()
    var info := struct;
    info.+name := "Máscara Mortuária de Ouro";
    info.+image := 0xF0DF;
    info.+flavor_image := 18028;
    info.+culture_name := "Aigla";
    info.+main_text := "Nos momentos finais do Imperador Crassus, artesãos criaram uma máscara mortuária de ouro para imortalizar suas feições. Gravada com padrões de vitória e liderança, foi colocada sobre seu rosto durante o funeral imperial. Exibida nas criptas reais, tornou-se símbolo de glória eterna, lembrando o povo aiglano do espírito indomável de seu imperador. Dizia-se conter um fragmento da sabedoria de Crassus, abençoando futuros governantes com sua orientação através das eras.";
    info.+skill_text := "A máscara mortuária de ouro do Imperador Crassus, criada para preservar suas feições, simboliza glória eterna e a sabedoria duradoura dos governantes aiglanos.";
    info.+archaeology_text := "A máscara representa a influência duradoura de Crassus, abençoando futuros governantes com sua sabedoria.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 95.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetChapeuPlumaCorvoNegroInfo()
    var info := struct;
    info.+name := "Chapéu de Pluma de um Corvo Negro";
    info.+image := 0XF0B6;
    info.+flavor_image := 18029;
    info.+culture_name := "Kaijing";
    info.+main_text := "Durante um festival Björske, o chefe Sigmar usou um chapéu cerimonial com penas de corvo negro, simbolizando sabedoria e visão. Usado em reuniões estratégicas e preparações para batalha, acreditava-se conceder visão divina. Em uma batalha decisiva contra os aiglanos, Sigmar, com o chapéu, fez um movimento inesperado que virou a maré. O chapéu tornou-se um tesouro sagrado, passado para líderes Kaijing como símbolo de vitória e conexão mística com o mundo espiritual.";
    info.+skill_text := "O chapéu cerimonial de Sigmar, adornado com penas de corvo, simboliza sabedoria e poder, tornando-se um tesouro sagrado para líderes Kaijing após uma vitória decisiva.";
    info.+archaeology_text := "O chapéu representa conexão mística e poder, guiando líderes Kaijing em batalhas e decisões.";
    info.+required_skill := "Domestic_Working";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMascaraAranhaNegra()
    var info := struct;
    info.+name := "Máscara de Aranha Negra";
    info.+image := 0XF0C4;
    info.+flavor_image := 18030;
    info.+culture_name := "Har'oloth";
    info.+main_text := "Na cidade subterrânea de Har'oloth, uma máscara de aranha negra era usada pelas sacerdotisas Drow em rituais sagrados. Feita de ônix e obsidiana, simbolizava a precisão mortal e astúcia da deusa Lolth. No Festival das Sombras, uma sacerdotisa a vestia para invocar o favor da deusa. A portadora era vista como encarnação de Lolth, e o festival determinava o destino da política e guerra em Har'oloth, reforçando o domínio dos Drow sobre as sombras.";
    info.+skill_text := "A máscara de aranha negra, usada em rituais sagrados em Har'oloth, simboliza a precisão mortal dos Drow, canalizando o poder de sua deusa Lolth em assuntos políticos e militares.";
    info.+archaeology_text := "A máscara de aranha simboliza a devoção dos Drow a Lolth, incorporando sua astúcia e poder mortal.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 90.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMascaraCranioCervo()
    var info := struct;
    info.+name := "Máscara Feita com Crânio de Cervo";
    info.+image := 0XF0CC;
    info.+flavor_image := 18031;
    info.+culture_name := "Björske";
    info.+main_text := "Usada pelos xamãs Björske, a máscara de crânio de cervo simbolizava a conexão com a natureza e os deuses. Feita do crânio de um veado sagrado, era usada em rituais para garantir boas caçadas e clima favorável. O xamã Grimnir a vestiu ao invocar os deuses antes de um inverno rigoroso. Após a cerimônia, a sorte da aldeia mudou, permitindo que reunissem recursos suficientes para sobreviver. A máscara tornou-se um poderoso símbolo de favor divino e do vínculo Björske com o mundo natural.";
    info.+skill_text := "A máscara de crânio de cervo, usada pelos xamãs Björske, representa sua profunda conexão com a natureza e os deuses, invocando favor divino em tempos de dificuldade.";
    info.+archaeology_text := "A máscara de crânio simboliza o vínculo dos Björske com a natureza, canalizando as bênçãos dos deuses para a sobrevivência.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetFerraduraOuro()
    var info := struct;
    info.+name := "Ferradura de Ouro";
    info.+image := 0XEBF9;  // Gráfico do item
    info.+flavor_image := 18032;  // ID da imagem de flavor
    info.+culture_name := "Polski";  // Nome da cultura
    info.+main_text := "Na aldeia rural de Verdeflor, a ferradura de ouro era um símbolo de boa sorte, pendurada acima das portas das casas para trazer proteção e prosperidade. Diz-se que a primeira ferradura foi forjada pelo mestre ferreiro da aldeia e abençoada pelos druidas locais. Quando a aldeia enfrentou um iminente ataque, o ferreiro pendurou a ferradura de ouro nos portões da vila e, por algum milagre, os invasores se retiraram antes de atacar. A ferradura tornou-se um símbolo venerado, acreditado por afastar o mal e trazer bênçãos ao povo Polski.";
    info.+skill_text := "A ferradura de ouro, abençoada pelos druidas, tornou-se um símbolo de proteção e prosperidade para os Polski, acreditando-se que afasta o mal e traz bênçãos à aldeia.";
    info.+archaeology_text := "A ferradura de ouro representa proteção divina, salvaguardando os Polski de danos e trazendo prosperidade.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetTacaVitoriaOlimpiada()
    var info := struct;
    info.+name := "Taça de Vitória da Olimpíada Feita de Ouro";
    info.+image := 0XEC10;  // Gráfico do item
    info.+flavor_image := 18033;  // ID da imagem de flavor
    info.+culture_name := "Aigla";  // Nome da cultura
    info.+main_text := "A taça de vitória dourada, concedida ao campeão dos jogos imperiais Aiglano, era um símbolo de conquista e honra suprema. Durante os jogos anuais, os melhores guerreiros e atletas de Aigla competiam pela glória, e a taça dourada era apresentada ao vencedor pelo próprio imperador. A taça, intricadamente gravada com cenas de batalhas lendárias e vitórias, simbolizava a força e a unidade do povo Aiglano. Para o vencedor, era mais que um prêmio—era um testemunho de seu talento e uma marca de glória eterna.";
    info.+skill_text := "A taça de vitória dourada, concedida nos jogos Aiglano, simboliza a honra suprema, representando a força, a unidade e a glória eterna dos melhores atletas e guerreiros do império.";
    info.+archaeology_text := "A taça dourada incorpora a força e a glória Aiglano, concedida apenas aos maiores campeões do império.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 95.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEstatuaAranhaNegra()
    var info := struct;
    info.+name := "Pequena Estátua de Aranha Negra";
    info.+image := 0XECC0;  // Gráfico do item
    info.+flavor_image := 18034;  // ID da imagem de flavor
    info.+culture_name := "Har'oloth";  // Nome da cultura
    info.+main_text := "Feita de obsidiana e jato, a pequena estátua de aranha negra era um objeto sagrado em Har'oloth, representando a deusa Lolth. Cada casa Drow possuía uma versão da estátua, colocada em altares para invocar a proteção e o favor de Lolth. Durante o Festival das Sombras, as estátuas eram carregadas pelas sacerdotisas pela cidade como parte de um grandioso ritual para renovar as bênçãos de Lolth. A estátua, pequena, mas intricadamente detalhada, incorporava a astúcia e o poder dos Drow. Acreditava-se que aqueles que honravam a aranha recebiam seu favor, enquanto os que não o faziam enfrentavam a desgraça.";
    info.+skill_text := "A pequena estátua de aranha negra, feita de obsidiana, representa a devoção dos Drow a Lolth. Ela é carregada em rituais para invocar seu favor e garantir proteção contra a desgraça.";
    info.+archaeology_text := "A estátua de aranha simboliza o poder de Lolth, oferecendo proteção e bênçãos aos devotos Drow.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetBustoPatriciaAiglana()
    var info := struct;
    info.+name := "Busto de uma Patrícia Aiglana Feito de Mármore";
    info.+image := 0X3D3A;  // Gráfico do item
    info.+flavor_image := 18035;  // ID da imagem de flavor
    info.+culture_name := "Aigla";  // Nome da cultura
    info.+main_text := "Nos grandiosos salões do distrito senatorial de Aigla, um busto de mármore da Lady Octavia, uma venerada patricia, erguia-se orgulhosamente. Conhecida por sua sabedoria e diplomacia, Octavia desempenhou um papel fundamental na formação de alianças com estados vizinhos. Após sua morte, o senado comissionou o busto para imortalizar seu legado. Esculpido com detalhes intrincados, o busto retratava sua expressão serena e porte régio. Tornou-se um símbolo da elegância Aiglana e da astúcia política, admirado por jovens mulheres que aspiravam a seguir seus passos como líderes do império.";
    info.+skill_text := "O busto de mármore da Lady Octavia, uma destacada patricia Aiglana, simboliza a elegância, sabedoria e diplomacia das mulheres Aiglano no cenário político do império.";
    info.+archaeology_text := "O busto simboliza o legado da Lady Octavia, representando a elegância e sabedoria Aiglana.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 90.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetBustoSenadorAiglano()
    var info := struct;
    info.+name := "Busto de um Senador Aiglano Feito de Mármore";
    info.+image := 0XED30;  // Gráfico do item
    info.+flavor_image := 18036;  // ID da imagem de flavor
    info.+culture_name := "Aigla";  // Nome da cultura
    info.+main_text := "Na Casa do Senado de Aigla, o busto de mármore do Senador Valerianus, um famoso orador e político, ergue-se como um tributo a seus anos de serviço. Valerianus foi instrumental na elaboração de legislações-chave que expandiram a influência do império e protegeram seus cidadãos. Após sua morte, o senado o homenageou com um busto finamente elaborado, capturando sua expressão severa, mas sábia. O busto serviu como um lembrete para futuros senadores da importância da integridade e dedicação ao império. Tornou-se um símbolo venerado da governança Aiglana, inspirando gerações de políticos.";
    info.+skill_text := "O busto de mármore do Senador Valerianus, um lendário orador Aiglano, simboliza integridade e dedicação à governança, inspirando futuras gerações de políticos.";
    info.+archaeology_text := "O busto simboliza o legado de Valerianus, representando a governança, sabedoria e serviço político Aiglano.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 90.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetTochasAventureiro()
    var info := struct;
    info.+name := "Conjunto de Tochas Usadas do Aventureiro";
    info.+image := 0Xc1b7;  // Gráfico do item
    info.+flavor_image := 18037;  // ID da imagem de flavor
    info.+culture_name := "Björske";  // Nome da cultura
    info.+main_text := "As tochas, carbonizadas e queimadas até se tornarem restos, uma vez iluminaram as cavernas traiçoeiras sob as terras Björske. Carregadas pelo lendário aventureiro Vikar, essas tochas eram sua linha de vida enquanto ele descia ao abismo, em busca da relíquia perdida de seus ancestrais. Cada tocha conta uma história—uma se apagou justo quando Vikar lutou contra um troll de caverna, enquanto outra durou pouco enquanto ele escapava de um deslizamento de rochas. Após seu retorno, as tochas tornaram-se relíquias, simbolizando os perigos enfrentados e a resiliência necessária para emergir vitorioso da escuridão.";
    info.+skill_text := "As tochas de Vikar, carbonizadas pelo uso em cavernas perigosas, tornaram-se símbolos de sobrevivência, iluminando seu caminho através do traiçoeiro subsolo Björske em busca de uma relíquia perdida.";
    info.+archaeology_text := "Essas tochas simbolizam a resiliência do aventureiro, guiando-o pelos perigos das cavernas Björske.";
    info.+required_skill := "Survival";
    info.+required_skill_value := 75.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetDocumentosAiglanos()
    var info := struct;
    info.+name := "Conjunto de Documentos";
    info.+image := 0xe9ce;  // Gráfico do item
    info.+flavor_image := 18038;  // ID da imagem de flavor
    info.+culture_name := "Aigla";  // Nome da cultura
    info.+main_text := "Descobertos nos arquivos da cidade imperial, esses documentos envelhecidos pertenciam ao senado Aiglano. Escritos com meticuloso cuidado, os pergaminhos contêm tratados secretos e estratégias militares que ajudaram o império a expandir suas fronteiras. Um documento, assinado pelo grande senador Valerianus, delineia uma aliança com um reino vizinho, garantindo paz por décadas. Os documentos, agora frágeis devido à idade, são mantidos sob vigilância, servindo como lembranças das conquistas diplomáticas e políticas do império.";
    info.+skill_text := "O conjunto de documentos contém tratados e estratégias cruciais que ajudaram a moldar a expansão do império Aiglano, com assinaturas de senadores proeminentes, como Valerianus.";
    info.+archaeology_text := "Os documentos simbolizam a destreza diplomática e militar Aiglana, moldando a dominação do império.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetTrombetaAiglana()
    var info := struct;
    info.+name := "Trombeta";
    info.+image := 0xea39;  // Gráfico do item
    info.+flavor_image := 18039;  // ID da imagem de flavor
    info.+culture_name := "Aigla";  // Nome da cultura
    info.+main_text := "A trombeta dourada era carregada pelos arautos das legiões Aiglano, seu som sinalizando o início da batalha ou a chegada de um nobre visitante. Durante o Cerco de Aigla, o chamado da trombeta reuniu os defensores, ecoando através das paredes de pedra da cidade enquanto os legionários marchavam em uníssono. O instrumento, finamente elaborado com designs intrincados, foi mais tarde preservado na Arma Imperial como um símbolo de unidade e força diante de adversidades esmagadoras. A trombeta tornou-se uma ferramenta cerimonial, tocada durante procissões imperiais para honrar a bravura das legiões Aiglano.";
    info.+skill_text := "A trombeta dourada, símbolo de unidade e força Aiglana, reuniu legionários durante a batalha e foi usada posteriormente em cerimônias imperiais para homenagear sua bravura.";
    info.+archaeology_text := "A trombeta simboliza o espírito Aiglano, seu som unindo legiões e comemorando suas vitórias.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetQuadroLadyLavinia()
    var info := struct;
    info.+name := "Quadro Pequeno com uma Imagem de uma Mulher";
    info.+image := 0xeb4d;  // Gráfico do item
    info.+flavor_image := 18040;  // ID da imagem de flavor
    info.+culture_name := "Aigla";  // Nome da cultura
    info.+main_text := "Em uma câmara modesta do palácio Aiglano, um pequeno quadro de uma nobre, Lady Lavinia, pendia na parede. O quadro, delicadamente emoldurado em ouro, capturava sua expressão serena e traje elegante. Lavinia era renomada por sua caridade e sabedoria, influenciando as reformas sociais do império. Após sua morte, o retrato tornou-se um símbolo de compaixão e graça, admirado por todos que entravam na sala. Serviu como um lembrete silencioso da importância da humildade e bondade em um mundo frequentemente dominado pela ambição política.";
    info.+skill_text := "O pequeno quadro da Lady Lavinia, uma nobre conhecida por sua caridade, simboliza compaixão e graça no império Aiglano, influenciando suas reformas sociais.";
    info.+archaeology_text := "O retrato da Lady Lavinia incorpora os valores Aiglano de caridade, sabedoria e influência silenciosa.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetSignPostTaverna()
    var info := struct;
    info.+name := "Sign Post de uma Taverna com a Imagem de uma Árvore Branca e uma Caneca no Centro";
    info.+image := 0xeb85;  // Gráfico do item
    info.+flavor_image := 18041;  // ID da imagem de flavor
    info.+culture_name := "Polski";  // Nome da cultura
    info.+main_text := "No coração de uma vila animada, o sinal de uma taverna, adornado com a imagem de uma majestosa árvore branca e uma caneca, balançava ao vento. A Árvore Branca, conhecida como 'Sombra do Descanso', era considerada sagrada pelos habitantes, oferecendo abrigo e consolo aos viajantes cansados. A taverna, famosa por sua hidromel e petiscos caseiros, atraía tanto moradores quanto visitantes. As histórias de heróis e aventuras eram contadas sob suas robustas paredes de madeira. O sinal, esculpido em madeira rústica, tornou-se um símbolo de camaradagem e alegria, onde laços eram forjados em cada brinde.";
    info.+skill_text := "O sinal da taverna 'Sombra do Descanso,' com uma árvore branca e uma caneca, simboliza camaradagem e alegria, atraindo viajantes e moradores para compartilhar histórias e aventuras.";
    info.+archaeology_text := "O sinal representa a união e alegria da taverna, onde laços são forjados a cada brinde.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotLouco()
    var info := struct;
    info.+name := "O Louco";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18042;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "A carta 'O Louco' era um símbolo de liberdade e novas aventuras entre os Aluriels. Retratava uma figura dançante, com um pé no precipício e o outro firmemente no chão. Este espírito audacioso encorajava os elfos a seguir seus instintos e explorar o desconhecido. Em tempos de estagnação, os Aluriels recorriam a 'O Louco' para encontrar coragem e inspiração. A carta era frequentemente utilizada por jovens aventureiros em busca de desafios, lembrando-os de que cada jornada começa com um primeiro passo.";
    info.+skill_text := "A carta 'O Louco' simboliza liberdade e coragem, inspirando os Aluriels a seguir seus instintos e abraçar novas aventuras em momentos de incerteza.";
    info.+archaeology_text := "O Louco representa o início de novas jornadas, encorajando a ousadia e a exploração entre os Aluriels.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotMago()
    var info := struct;
    info.+name := "O Mago";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18043;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "A carta 'O Mago' era reverenciada pelos Aluriels por sua capacidade de manifestar desejos e transformar realidades. Retratava um elfo elegante, com um cajado na mão e elementos mágicos flutuando ao seu redor. 'O Mago' simbolizava habilidade, criatividade e poder pessoal. Durante épocas de crise, os Aluriels buscavam esta carta para lembrar que tinham o controle de seu destino. O Mago inspirava confiança, ensinando que a verdadeira magia vem de dentro.";
    info.+skill_text := "A carta 'O Mago' simboliza poder e manifestação, lembrando os Aluriels de que a verdadeira magia reside na confiança em suas habilidades e intuições.";
    info.+archaeology_text := "O Mago representa a habilidade de transformar a realidade, guiando os Aluriels em suas jornadas.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotSacerdotisa()
    var info := struct;
    info.+name := "A Sacerdotisa";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18044;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Sacerdotisa' era a guardiã do conhecimento oculto entre os Aluriels. A carta mostrava uma figura serena, sentada entre cortinas de veludo, segurando um pergaminho. Ela simbolizava intuição, sabedoria e o poder do inconsciente. Usada em momentos de incerteza, esta carta encorajava os elfos a olhar para dentro, buscando respostas nas profundezas de suas almas. A Sacerdotisa lembrava que o verdadeiro poder reside na compreensão dos mistérios da vida.";
    info.+skill_text := "A carta 'A Sacerdotisa' simboliza intuição e sabedoria oculta, guiando os Aluriels a buscar respostas interiores em tempos de incerteza.";
    info.+archaeology_text := "A Sacerdotisa representa o poder do inconsciente, conectando os Aluriels a suas verdades interiores.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 90.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotImperatriz()
    var info := struct;
    info.+name := "A Imperatriz";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18045;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Imperatriz' era um símbolo de fertilidade e abundância entre os Aluriels. A carta apresentava uma figura majestosa, cercada por campos floridos e natureza vibrante. Ela representava a nutrição e o amor incondicional, guiando os elfos na criação e no cuidado. Durante os tempos de prosperidade, os Aluriels invocavam 'A Imperatriz' para fortalecer suas conexões com a terra e a comunidade. A carta servia como um lembrete da importância de cuidar uns dos outros e do ambiente ao seu redor.";
    info.+skill_text := "A carta 'A Imperatriz' simboliza fertilidade e abundância, inspirando os Aluriels a nutrir a terra e a comunidade com amor e cuidado.";
    info.+archaeology_text := "A Imperatriz representa a abundância, guiando os Aluriels em sua conexão com a natureza.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotImperador()
    var info := struct;
    info.+name := "O Imperador";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18046;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'O Imperador' simbolizava estabilidade e autoridade entre os Aluriels. A carta mostrava uma figura imponente em seu trono, adornada com símbolos de poder e justiça. Ele representava a liderança, a proteção e a ordem na sociedade. Em tempos de desordem, os Aluriels buscavam 'O Imperador' para restaurar a paz e a estabilidade. Esta carta lembrava que a verdadeira força vem da responsabilidade e da sabedoria na tomada de decisões.";
    info.+skill_text := "A carta 'O Imperador' simboliza estabilidade e liderança, guiando os Aluriels a restaurar a ordem e a justiça em tempos de desordem e incerteza.";
    info.+archaeology_text := "O Imperador representa autoridade, lembrando os Aluriels da importância de liderança sábia e responsável.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotSacerdote()
    var info := struct;
    info.+name := "O Sacerdote";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18047;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'O Sacerdote' representava a tradição e a espiritualidade entre os Aluriels. A carta mostrava um sábio ancião, cercado por seguidores e livros antigos, ensinando os caminhos da sabedoria ancestral. Ele simbolizava a busca por conhecimento e a importância da comunidade. Durante crises espirituais, os Aluriels invocavam 'O Sacerdote' para se reconectar com suas raízes e crenças. A carta lembrava que a verdadeira sabedoria é compartilhada e que a união fortalece a espiritualidade.";
    info.+skill_text := "A carta 'O Sacerdote' simboliza tradição e espiritualidade, guiando os Aluriels a buscar conhecimento e conexão com suas raízes e comunidade em momentos de crise.";
    info.+archaeology_text := "O Sacerdote representa a sabedoria compartilhada, conectando os Aluriels às suas tradições e crenças.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 90.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotEnamorados()
    var info := struct;
    info.+name := "Os Enamorados";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18048;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'Os Enamorados' simbolizavam amor, união e harmonia entre os Aluriels. A carta mostrava duas figuras entrelaçadas, cercadas por flores vibrantes e luz suave. Representava não apenas o amor romântico, mas também a importância das escolhas e da conexão espiritual. Em momentos de conflito, os Aluriels recorriam a esta carta para encontrar o equilíbrio e a compreensão nas relações. 'Os Enamorados' lembravam que o verdadeiro amor é baseado na confiança e na união.";
    info.+skill_text := "A carta 'Os Enamorados' simboliza amor e harmonia, guiando os Aluriels a encontrar equilíbrio e compreensão nas relações em tempos de conflito.";
    info.+archaeology_text := "Os Enamorados representam união e escolhas, lembrando os Aluriels da importância da conexão e confiança.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotCarruagem()
    var info := struct;
    info.+name := "A Carruagem";
    info.+image := 0xb40c; // Gráfico do item
    info.+flavor_image := 18049;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Carruagem' simbolizava determinação e conquista entre os Aluriels. A carta mostrava um elfo guerreiro em uma carruagem majestosa, avançando com confiança. Representava a capacidade de superar obstáculos e controlar as forças opostas. Em tempos de desafio, os Aluriels buscavam 'A Carruagem' para lembrar da força interior e da resiliência. Esta carta incentivava a ação decisiva e a coragem para seguir em frente, mesmo diante das adversidades.";
    info.+skill_text := "A carta 'A Carruagem' simboliza determinação e conquista, inspirando os Aluriels a superar desafios com coragem e confiança em tempos difíceis.";
    info.+archaeology_text := "A Carruagem representa ação e controle, guiando os Aluriels a avançar em suas jornadas.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotForca()
    var info := struct;
    info.+name := "A Força";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18050;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Força' simbolizava coragem e compaixão entre os Aluriels. A carta mostrava uma figura serena acariciando um leão, representando o domínio sobre as emoções e a força interior. Era um lembrete de que a verdadeira força vem da gentileza e da paciência. Durante períodos de conflito, os Aluriels recorriam a 'A Força' para encontrar coragem dentro de si e abordar os desafios com amor. Esta carta inspirava a harmonia entre a força e a vulnerabilidade.";
    info.+skill_text := "A carta 'A Força' simboliza coragem e compaixão, lembrando os Aluriels de que a verdadeira força reside na gentileza e na paciência em tempos de desafio.";
    info.+archaeology_text := "A Força representa domínio sobre as emoções, guiando os Aluriels a enfrentar desafios com amor.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotSabio()
    var info := struct;
    info.+name := "O Sábio";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18051;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'O Sábio' representava introspecção e busca por conhecimento entre os Aluriels. A carta mostrava uma figura solitária, com uma lanterna, caminhando por uma floresta sombria. Ele simbolizava a importância da reflexão e do autoconhecimento. Em momentos de incerteza, os Aluriels buscavam 'O Sábio' para encontrar clareza em suas decisões. Esta carta encorajava a busca pela verdade interior e a importância do silêncio em meio ao caos.";
    info.+skill_text := "A carta 'O Sábio' simboliza introspecção e autoconhecimento, guiando os Aluriels a encontrar clareza em momentos de incerteza através da reflexão e do silêncio.";
    info.+archaeology_text := "O Sábio representa a busca pela verdade interior, ajudando os Aluriels a se entenderem melhor.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 90.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotRodaFortuna()
    var info := struct;
    info.+name := "A Roda da Fortuna";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18052;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Roda da Fortuna' simbolizava ciclos e mudanças entre os Aluriels. A carta mostrava uma roda giratória com diferentes criaturas representando os altos e baixos da vida. Ela lembrava os elfos de que a vida é repleta de surpresas e que cada ciclo traz novas oportunidades. Em tempos de transição, os Aluriels buscavam 'A Roda da Fortuna' para abraçar as mudanças com otimismo e fé. A carta inspirava aceitação e sabedoria ao lidar com as inevitáveis reviravoltas da vida.";
    info.+skill_text := "A carta 'A Roda da Fortuna' simboliza ciclos e mudanças, encorajando os Aluriels a abraçar as transições com otimismo e sabedoria em tempos de incerteza.";
    info.+archaeology_text := "A Roda da Fortuna representa a aceitação dos altos e baixos da vida, guiando os Aluriels.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotJustica()
    var info := struct;
    info.+name := "A Justiça";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18053;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Justiça' simbolizava equidade e verdade entre os Aluriels. A carta mostrava uma figura imponente segurando uma balança e uma espada, representando a importância de tomar decisões justas. Ela lembrava os elfos de que a justiça é fundamental para a harmonia na sociedade. Em tempos de conflito, os Aluriels buscavam 'A Justiça' para garantir que suas ações fossem guiadas pela ética e pela verdade. Esta carta inspirava responsabilidade e integridade nas decisões.";
    info.+skill_text := "A carta 'A Justiça' simboliza equidade e verdade, guiando os Aluriels a tomar decisões justas e éticas em tempos de conflito.";
    info.+archaeology_text := "A Justiça representa a busca pela verdade, ajudando os Aluriels a garantir a harmonia social.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotEnforcado()
    var info := struct;
    info.+name := "O Enforcado";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18054;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'O Enforcado' simbolizava sacrifício e nova perspectiva entre os Aluriels. A carta mostrava uma figura pendurada de cabeça para baixo, cercada por folhas verdes. Este estado de suspensão representava a necessidade de desacelerar e refletir. Em tempos de crise, os Aluriels recorriam a 'O Enforcado' para encontrar clareza e entender que, às vezes, é preciso abrir mão de algo para ganhar uma nova visão. A carta encorajava a aceitação e a adaptação.";
    info.+skill_text := "A carta 'O Enforcado' simboliza sacrifício e reflexão, guiando os Aluriels a encontrar clareza e novas perspectivas em tempos de crise.";
    info.+archaeology_text := "O Enforcado representa a necessidade de desacelerar e refletir, ajudando os Aluriels a se adaptarem.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotRenascimento()
    var info := struct;
    info.+name := "O Renascimento";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18055;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'O Renascimento' simbolizava renovação e autoavaliação entre os Aluriels. A carta mostrava figuras levantando-se em resposta a uma chamada celestial, representando a importância de confrontar o passado e buscar a redenção. Em tempos de mudança, os Aluriels buscavam 'O Renascimento' para refletir sobre suas escolhas e encontrar um novo caminho. Esta carta encorajava a transformação e a busca por um propósito mais elevado na vida.";
    info.+skill_text := "A carta 'O Renascimento' simboliza renovação e autoavaliação, guiando os Aluriels a confrontar o passado e buscar um novo caminho em tempos de mudança.";
    info.+archaeology_text := "O Renascimento representa a transformação e a busca por propósito, ajudando os Aluriels a se redimirem.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 90.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotTranquilidade()
    var info := struct;
    info.+name := "A Tranquilidade";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18056;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Tranquilidade' simbolizava esperança e serenidade entre os Aluriels. A carta mostrava uma figura nua, derramando água em um lago sob um céu estrelado. Representava a cura e a conexão com o divino. Durante períodos de incerteza, os Aluriels buscavam 'A Tranquilidade' para encontrar paz interior e renovação espiritual. Esta carta encorajava a fé e a busca pela beleza nas pequenas coisas da vida.";
    info.+skill_text := "A carta 'A Tranquilidade' simboliza esperança e serenidade, guiando os Aluriels a encontrar paz interior e renovação espiritual em tempos de incerteza.";
    info.+archaeology_text := "A Tranquilidade representa cura e conexão espiritual, ajudando os Aluriels a manterem a fé.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotCobra()
    var info := struct;
    info.+name := "A Cobra";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18057;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Cobra' simbolizava transformação e sabedoria entre os Aluriels. A carta mostrava uma cobra majestosa envolvendo uma árvore antiga, representando a sabedoria que vem com a experiência. Os Aluriels acreditavam que, assim como a cobra troca de pele, eles também poderiam se transformar e se renovar. Em tempos de crise, buscavam 'A Cobra' para lembrar que a mudança é essencial para o crescimento. Esta carta encorajava a aceitação das transformações como parte da vida.";
    info.+skill_text := "A carta 'A Cobra' simboliza transformação e sabedoria, guiando os Aluriels a aceitarem as mudanças como parte essencial do crescimento em tempos de crise.";
    info.+archaeology_text := "A Cobra representa renovação e aprendizado, ajudando os Aluriels a se adaptarem às transformações.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotTorre()
    var info := struct;
    info.+name := "A Torre";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18058;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Torre' simbolizava catástrofe e revelação entre os Aluriels. A carta mostrava uma torre em chamas, cercada por relâmpagos, representando a destruição necessária para a reconstrução. Era um lembrete de que as crises podem levar a novas revelações e mudanças significativas. Em momentos de conflito, os Aluriels buscavam 'A Torre' para entender que a dor pode trazer crescimento e clareza. Esta carta encorajava a resiliência e a aceitação das mudanças inesperadas.";
    info.+skill_text := "A carta 'A Torre' simboliza catástrofe e revelação, guiando os Aluriels a encontrar crescimento e clareza nas crises e mudanças significativas.";
    info.+archaeology_text := "A Torre representa destruição e reconstrução, ajudando os Aluriels a aceitarem as mudanças inesperadas.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 90.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotEstrela()
    var info := struct;
    info.+name := "A Estrela";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18059;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Estrela' simbolizava esperança e inspiração entre os Aluriels. A carta mostrava uma figura brilhante sob um céu estrelado, com estrelas dançando ao seu redor. Representava a luz que guia em tempos de escuridão. Os Aluriels buscavam 'A Estrela' para lembrar que, mesmo nas noites mais sombrias, sempre há uma luz a ser encontrada. Esta carta encorajava a fé no futuro e a busca pela beleza nas experiências.";
    info.+skill_text := "A carta 'A Estrela' simboliza esperança e inspiração, guiando os Aluriels a encontrarem luz e fé em tempos de escuridão e incerteza.";
    info.+archaeology_text := "A Estrela representa luz e esperança, ajudando os Aluriels a manterem a fé em tempos difíceis.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotLua()
    var info := struct;
    info.+name := "A Lua";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18060;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'A Lua' simbolizava intuição e ilusões entre os Aluriels. A carta mostrava uma lua cheia iluminando um caminho misterioso, cercado por criaturas noturnas. Representava a dualidade entre a luz e a escuridão, lembrando os elfos de que nem tudo é o que parece. Em tempos de incerteza, os Aluriels buscavam 'A Lua' para acessar sua intuição e entender as verdades ocultas. Esta carta encorajava a exploração do inconsciente e a aceitação das sombras pessoais.";
    info.+skill_text := "A carta 'A Lua' simboliza intuição e ilusões, guiando os Aluriels a explorarem seu inconsciente e a aceitarem as sombras em tempos de incerteza.";
    info.+archaeology_text := "A Lua representa intuição e dualidade, ajudando os Aluriels a descobrirem verdades ocultas em si mesmos.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 88.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotSol()
    var info := struct;
    info.+name := "O Sol";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18061;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'O Sol' simbolizava alegria e sucesso entre os Aluriels. A carta mostrava uma figura radiante sob um sol brilhante, cercada por flores e crianças brincando. Representava o poder da luz e da vida, trazendo felicidade e realização. Em tempos de celebração, os Aluriels buscavam 'O Sol' para recordar que a alegria é uma força vital. Esta carta encorajava a gratidão e a celebração das pequenas alegrias da vida.";
    info.+skill_text := "A carta 'O Sol' simboliza alegria e sucesso, guiando os Aluriels a celebrarem a vida e a gratidão em tempos de felicidade.";
    info.+archaeology_text := "O Sol representa alegria e realização, ajudando os Aluriels a apreciarem as pequenas alegrias da vida.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 82.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotJulgamento()
    var info := struct;
    info.+name := "O Julgamento";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18062;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'O Julgamento' simbolizava avaliação e renascimento entre os Aluriels. A carta mostrava figuras levantando-se em resposta a uma chamada celestial, simbolizando a importância de confrontar o passado e buscar a redenção. Durante períodos de mudança, os Aluriels buscavam 'O Julgamento' para refletir sobre suas escolhas e encontrar um novo caminho. Esta carta encorajava a transformação e a busca por um propósito mais elevado na vida.";
    info.+skill_text := "A carta 'O Julgamento' simboliza avaliação e renascimento, guiando os Aluriels a refletirem sobre suas escolhas e buscarem um novo caminho em tempos de mudança.";
    info.+archaeology_text := "O Julgamento representa transformação e redenção, ajudando os Aluriels a se reavivarem em novos caminhos.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 92.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCartaTarotMundo()
    var info := struct;
    info.+name := "O Mundo";
    info.+image := 0xb40c;  // Gráfico do item
    info.+flavor_image := 18063;  // ID da imagem de flavor
    info.+culture_name := "Aluriel";  // Nome da cultura
    info.+main_text := "'O Mundo' simbolizava realização e integração entre os Aluriels. A carta mostrava uma figura dançando sob um céu radiante, cercada por símbolos de harmonia e plenitude. Representava a união de todas as experiências e a culminação de jornadas. Os Aluriels buscavam 'O Mundo' em momentos de celebração, reconhecendo que cada passo os levou a essa realização. Esta carta encorajava a apreciação da jornada e a busca pela harmonia em todos os aspectos da vida.";
    info.+skill_text := "A carta 'O Mundo' simboliza realização e integração, guiando os Aluriels a apreciarem suas jornadas e a buscarem harmonia em todos os aspectos da vida.";
    info.+archaeology_text := "O Mundo representa a culminação de jornadas, ajudando os Aluriels a celebrarem suas realizações.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 95.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetGloboDeGeloPolski()
    var info := struct;
    info.+name := "Globo de Gelo de Colhesol";
    info.+image := 0xb5c6;
    info.+flavor_image := 18068;
    info.+culture_name := "Polski";
    info.+main_text := "No coração de Colhesol, um globo de gelo eterno captura a essência da capital Polski. Dentro, minúsculos Polski cultivam jardins em miniatura, seus movimentos congelados no tempo. As tocas-casas brilham com luz acolhedora, enquanto diminutas tavernas transbordam de alegria silenciosa. Um mercado em miniatura exibe vegetais do tamanho de grãos de areia, cada um perfeitamente formado. Observadores atentos juram ver os pequenos habitantes piscando ocasionalmente, como se prestes a ganhar vida. Rumores persistem sobre vozes sussurrantes emanando do globo em noites de lua cheia, contando segredos Polski há muito esquecidos. Este item extraordinário foi produzido pelo famoso artesão Polski e viajante Adelard Riberin.";
    info.+skill_text := "O globo revela segredos Polski ocultos. Observadores pacientes podem testemunhar movimentos sutis e ouvir sussurros ancestrais emanando da miniatura encantada.";
    info.+archaeology_text := "Artefato Polski único: globo vivo capturando a essência mágica de Colhesol.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetGloboDeGeloAluriel()
    var info := struct;
    info.+name := "Globo de Gelo de Aldorathrim Ennorathel";
    info.+image := 0xb5c6;
    info.+flavor_image := 18065;
    info.+culture_name := "Polski";
    info.+main_text := "Aldorathrim Ennorathel cintila eternamente neste globo de gelo, suas torres élficas alcançando o topo da esfera. Minúsculas figuras élficas parecem mover-se entre as árvores cristalinas, suas canções silenciosas quase audíveis. A cidade muda sutilmente a cada olhar, revelando novos segredos e caminhos ocultos. No centro, uma versão em miniatura do conselho Valarindië Quendi se reúne em sessão eterna, suas expressões serenas congeladas no tempo. Dizem que elfos podem se comunicar com seus ancestrais através deste globo, embora nenhum não-elfo tenha testemunhado tal fenômeno. À noite, o globo emite um brilho suave, projetando sombras élficas dançantes nas paredes. Este item extraordinário foi produzido pelo famoso artesão e viajante Polski Adelard Riberin.";
    info.+skill_text := "O globo de Aldorathrim Ennorathel muda sutilmente, revelando segredos élficos. Alguns afirmam que serve como portal de comunicação ancestral.";
    info.+archaeology_text := "Artefato Polski: captura a essência viva e mutável de Aldorathrim Ennorathel.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetGloboDeGeloGulthrak()
    var info := struct;
    info.+name := "Globo de Gelo de Gulshur";
    info.+image := 0xb5c6;
    info.+flavor_image := 18069;
    info.+culture_name := "Polski";
    info.+main_text := "Gulshur emerge das areias dentro deste globo, um oásis congelado no tempo. Minúsculos orcs Gulthrak movem-se eternamente, seus rituais de sangue e areia reproduzidos em loop silencioso. O globo emite um calor sutil, como se o deserto dentro dele ainda ardesse. À noite, grãos de areia parecem se mover, remodelando continuamente a paisagem em miniatura. Observadores relatam ouvir batidas de tambores distantes emanando do globo durante a lua nova. No centro, uma réplica minúscula do Grande Oásis pulsa com energia, suas águas cristalinas aparentemente vivas. Xamãs orcs afirmam que o globo contém um fragmento real do espírito de Adar Kor Doom. Este item extraordinário foi produzido pelo famoso artesão e viajante Polski Adelard Riberin.";
    info.+skill_text := "O globo de Gulshur emana calor e vida. Observadores relatam mudanças na paisagem e sons de rituais orcs durante a lua nova.";
    info.+archaeology_text := "Artefato Polski: captura a essência viva de Gulshur e o espírito de Adar Kor Doom.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetGloboDeGeloKaijing()
    var info := struct;
    info.+name := "Globo de Gelo de Kaijing";
    info.+image := 0xb5c6;
    info.+flavor_image := 18071;
    info.+culture_name := "Polski";
    info.+main_text := "Kaijing repousa serenamente nas montanhas dentro deste globo, envolto em névoa eterna. Minúsculos monges e guerreiros praticam artes marciais em câmera lenta perpétua, seus movimentos fluidos como água congelada. O templo no centro parece mudar de forma dependendo do ângulo de visão, revelando salas ocultas e passagens secretas. À noite, o globo emite um brilho suave que se move como chi fluindo. Observadores relatam sentir uma calma profunda ao contemplar o globo, alguns até experimentando visões momentâneas do futuro. Rumores sugerem que mestres Kaijin podem projetar suas mentes para dentro do globo, explorando seus segredos. Este item extraordinário foi produzido pelo famoso artesão e viajante Polski Adelard Riberin.";
    info.+skill_text := "O globo de Kaijing emana serenidade e mistério. Observadores experientes podem sentir o fluxo de chi e vislumbrar segredos ocultos do templo.";
    info.+archaeology_text := "Artefato Polski: captura a essência mística e imutável do templo nas montanhas.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetGloboDeGeloAigla()
    var info := struct;
    info.+name := "Globo de Gelo de Aigla";
    info.+image := 0xb5c6;
    info.+flavor_image := 18064;
    info.+culture_name := "Polski";
    info.+main_text := "Aigla resplandece em glória congelada dentro deste globo, suas muralhas e torres desafiando o próprio tempo. Minúsculas legiões marcham eternamente pelas ruas, enquanto no palácio central, uma versão em miniatura do Imperador Crassus preside sobre seu reino imóvel. O globo parece pulsar com poder, como se o próprio império vivesse dentro dele. À noite, luzes diminutas acendem-se nas janelas, criando uma cidade estelar em miniatura. Dizem que generais Aiglanos podem ver o futuro de batalhas observando os movimentos das tropas minúsculas. Augures afirmam que o globo contém uma fração do poder de cada deus do panteão Veneratio. Este item extraordinário foi produzido pelo famoso artesão e viajante Polski Adelard Riberin.";
    info.+skill_text := "O globo de Aigla pulsa com o poder do império. Observadores habilidosos podem prever eventos futuros estudando os movimentos congelados dentro dele.";
    info.+archaeology_text := "Artefato Polski: encapsula a essência viva e poderosa do Império de Aigla.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetGloboDeGeloKhronKazad()
    var info := struct;
    info.+name := "Globo de Gelo de Khron Kaz'ad";
    info.+image := 0xb5c6;
    info.+flavor_image := 18066;
    info.+culture_name := "Polski";
    info.+main_text := "Khron Kaz'ad brilha eternamente neste globo, suas cavernas e salões se estendendo aparentemente ao infinito. Anões minúsculos trabalham incessantemente em forjas microscópicas, criando obras-primas invisíveis a olho nu. O globo emite um calor suave e um zumbido baixo, como se as forjas dentro dele ainda estivessem ativas. Observadores atentos juram ver novos túneis e câmaras surgindo a cada olhar, o reino subterrâneo crescendo constantemente. Diz-se que mestres ferreiros anões podem entrar mentalmente no globo para aprender segredos ancestrais de metalurgia. À noite, o globo brilha com o reflexo de tesouros ocultos nas profundezas. Este item extraordinário foi produzido pelo famoso artesão e viajante Polski Adelard Riberin.";
    info.+skill_text := "O globo de Khron Kaz'ad revela segredos anões ocultos. Observadores habilidosos podem ver o reino crescer e aprender antigas técnicas de metalurgia.";
    info.+archaeology_text := "Artefato Polski: captura a essência viva e em expansão de Khron Kaz'ad.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetGloboDeGeloBorfjell()
    var info := struct;
    info.+name := "Globo de Gelo de Börfjell";
    info.+image := 0xb5c6;
    info.+flavor_image := 18067;
    info.+culture_name := "Polski";
    info.+main_text := "Börfjell ergue-se orgulhosamente dentro do globo, suas estruturas de madeira e pedra desafiando a neve eterna. Guerreiros Björske em miniatura treinam incessantemente nos pátios congelados, suas armas tilintando silenciosamente. O globo emana um frio intenso, como se o inverno nórdico estivesse preso em seu interior. À noite, auroras boreais em miniatura dançam dentro da esfera, iluminando a cidade com cores etéreas. Observadores relatam ouvir ecos distantes de canções de guerra emanando do globo durante a lua cheia. Diz-se que xamãs Björske podem comungar com os espíritos do Sol e da Lua através deste artefato. Este item extraordinário foi produzido pelo famoso artesão e viajante Polski Adelard Riberin.";
    info.+skill_text := "O globo de Börfjell ecoa com a força Björske. Observadores podem testemunhar treinamentos eternos e auroras boreais em miniatura à noite.";
    info.+archaeology_text := "Artefato Polski: encapsula a essência guerreira e espiritual de Börfjell.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction


function GetGloboDeGeloNygurath()
    var info := struct;
    info.+name := "Globo de Gelo de Nygurath";
    info.+image := 0xb5c6;
    info.+flavor_image := 18070;
    info.+culture_name := "Polski";
    info.+main_text := "Nygurath serpenteia nas profundezas deste globo, suas torres e palácios subterrâneos brilhando com luz sombria. Drows em miniatura movem-se furtivamente pelas ruas, envolvidos em intrigas eternas. O globo parece absorver a luz ao seu redor, criando uma aura de escuridão palpável. Observadores relatam ver reflexos de si mesmos distorcidos na superfície, como se o globo se alimentasse de suas sombras. À noite, minúsculas aranhas de cristal tecem teias que redefinem constantemente a arquitetura da cidade. Dizem que sacerdotisas de Idrith podem usar o globo para espionar o mundo da superfície. Este item extraordinário foi produzido pelo famoso artesão e viajante Polski Adelard Riberin.";
    info.+skill_text := "O globo de Nygurath pulsa com intriga Drow. Observadores cuidadosos podem vislumbrar tramas em desenvolvimento e sentir a presença de Idrith.";
    info.+archaeology_text := "Artefato Polski: captura a essência sombria e mutável de Nygurath.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction


function GetCraneoOuroMonstro()
    var info := struct;
    info.+name := "Crâneo de Ouro de um Monstro";
    info.+image := 0xb7f3;  // Gráfico do item
    info.+flavor_image := 18072;  // ID da imagem de flavor
    info.+culture_name := "Badûran";  // Nome da cultura
    info.+main_text := "Nas profundezas das minas de Khron Kaz'ad, os anões desenterraram o crânio massivo de uma besta lendária que outrora aterrorizava seu reino. Após abater a criatura, os anões banharam seu crânio em ouro puro para comemorar a difícil vitória. O crânio dourado, adornado com runas intrincadas de proteção, foi colocado no Salão Principal como um troféu de seu domínio sobre o subterrâneo e um aviso para futuras ameaças. Tornou-se um símbolo de sua coragem e habilidade de conquistar até os monstros mais temíveis que espreitavam nas profundezas. Este item extraordinário foi produzido pelo famoso artesão Badûran Thorin Barba de Ferro.";
    info.+skill_text := "O crânio dourado da besta, banhado em ouro pelos Badûran, simboliza seu triunfo sobre os terrores subterrâneos, demonstrando sua coragem e domínio sob as montanhas.";
    info.+archaeology_text := "O crânio dourado representa a vitória Badûran sobre as profundezas, simbolizando sua força incomparável no submundo.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCraneoDeemonioCornudo()
    var info := struct;
    info.+name := "Crâneo de um Demônio Chifrudo";
    info.+image := 0xb852;
    info.+flavor_image := 18073;
    info.+culture_name := "Har'oloth";
    info.+main_text := "Na cidade sombria de Har'oloth, o crânio chifrudo de um senhor demoníaco abatido foi consagrado no Templo de Lolth. O demônio havia sido invocado por facções rivais, apenas para ser destruído pelas sacerdotisas Drow. Seu crânio, distorcido com magia negra e adornado com chifres curvos, foi exibido como símbolo do poder dos Drows sobre os reinos mortais e infernais. Durante o Festival das Sombras, o crânio era carregado em uma procissão ritual, lembrando os Drow de seu domínio sobre demônios e sua devoção inabalável à deusa aranha, Lolth. Este item extraordinário foi preservado pela famosa sacerdotisa Drow Zara'thel.";
    info.+skill_text := "O crânio de demônio chifrudo, preservado pelos Drow, simboliza seu domínio sobre o infernal, manejando poder sombrio sob os olhos vigilantes de Lolth.";
    info.+archaeology_text := "O crânio demoníaco representa o domínio Drow sobre forças sombrias, um lembrete de sua dominância e devoção.";
    info.+required_skill := "Magery";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetPosteLampadaOleo()
    var info := struct;
    info.+name := "Poste de Lâmpada de Óleo";
    info.+image := 0xbcf2;
    info.+flavor_image := 18074;
    info.+culture_name := "Aigla";
    info.+main_text := "Nas ruas movimentadas de Aigla, altos postes de lâmpadas a óleo alinhavam as avenidas, sua luz quente iluminando as grandes praças e mercados do império. Cada poste de lâmpada, feito de bronze, era adornado com gravuras do brasão do imperador e criaturas míticas. Essas lâmpadas foram instaladas pela primeira vez durante o reinado do Imperador Crassus para proporcionar segurança e ordem à noite. As lâmpadas tornaram-se um símbolo do progresso do império, iluminando o caminho para comerciantes, viajantes e políticos. O brilho das lâmpadas a óleo trazia conforto e um senso de unidade ao povo Aiglano. Este item extraordinário foi projetado pelo renomado arquiteto Aiglano Marcus Lucilius.";
    info.+skill_text := "Os postes de lâmpadas a óleo de bronze, adornados com gravuras imperiais, simbolizam o progresso e a unidade Aiglana, iluminando as ruas e praças durante as noites agitadas do império.";
    info.+archaeology_text := "As lâmpadas a óleo simbolizam o progresso Aiglano, iluminando as ruas do império e promovendo segurança e unidade.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetTrevoQuatroFolhas()
    var info := struct;
    info.+name := "Trevo de Quatro Folhas";
    info.+image := 0x74a1;
    info.+flavor_image := 18075;
    info.+culture_name := "Polski";
    info.+main_text := "Nas colinas ondulantes das terras Polski, um trevo de quatro folhas foi descoberto por uma jovem fazendeira chamada Anya. Conhecido por trazer boa sorte, acreditava-se que este trevo havia sido abençoado pelos espíritos da floresta. Anya guardou o trevo em um medalhão por anos, e durante um inverno particularmente rigoroso, dizia-se que o trevo havia salvado sua aldeia da fome. Passado de geração em geração, o trevo tornou-se um símbolo de esperança e prosperidade, sempre carregado durante eventos importantes e colheitas, garantindo as bênçãos da natureza para o povo Polski. Este item extraordinário foi preservado pela família de Anya por gerações.";
    info.+skill_text := "O trevo de quatro folhas de Anya, acreditando-se ser abençoado pelos espíritos da floresta, tornou-se um símbolo de sorte e prosperidade, protegendo sua aldeia das dificuldades.";
    info.+archaeology_text := "O trevo de quatro folhas simboliza a conexão dos Polski com a natureza e as bênçãos da fortuna.";
    info.+required_skill := "Gathering";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetQuadroGrandeLich()
    var info := struct;
    info.+name := "Quadro Grande de um Lich";
    info.+image := 0xa1e3;
    info.+flavor_image := 18076;
    info.+culture_name := "Aluriel";
    info.+main_text := "Nos salões sombrios da Torre Aluriel, uma grande e sinistra pintura de um lich pendurada na parede, seus olhos brilhantes assombrando todos que passavam. O lich, outrora um poderoso feiticeiro Aluriel, trocara sua vida mortal por conhecimento eterno. A pintura capturava o lich em meio a um feitiço, cercado por energias sombrias. Esta obra de arte era usada em rituais, acreditando-se canalizar a antiga magia do lich durante tempos de guerra. A pintura tornou-se um símbolo de poder proibido e dos perigos de mergulhar muito profundamente na magia negra. Este item extraordinário foi criado pelo renomado artista Aluriel Elyndria, o Pintor das Sombras.";
    info.+skill_text := "A pintura do lich, um feiticeiro Aluriel que trocou a vida pelo poder, simboliza os perigos e o fascínio da magia negra, usada em rituais por sua energia ancestral.";
    info.+archaeology_text := "A pintura do lich incorpora o poder sombrio, lembrando os Aluriels do custo do conhecimento eterno.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

/*function GetMapaNautico()
    var info := struct;
    info.+name := "Mapa Náutico";
    info.+image := 0xa2c8;
    info.+flavor_image := 18077;
    info.+culture_name := "Aigla";
    info.+main_text := "A posse mais preciosa da frota Aiglana era um mapa náutico centenário, desenhado à mão pelo lendário explorador Marcius, o Navegador. Este mapa, meticulosamente detalhado com recifes perigosos, ilhas inexploradas e portos seguros, guiava os navios do império através de mares traiçoeiros. Foi usado durante as viagens mais importantes do Império, incluindo a colonização de terras distantes. O mapa, preservado no Arquivo Imperial, simbolizava o domínio Aiglano sobre os mares, um testemunho de seu poder marítimo e exploração. Tornou-se um artefato precioso, estudado por cada nova geração de marinheiros. Este item extraordinário foi criado pelo próprio Marcius, o Navegador.";
    info.+skill_text := "O mapa náutico centenário, criado por Marcius o Navegador, simboliza o domínio marítimo Aiglano, guiando suas frotas através de mares traiçoeiros para colonizar terras distantes.";
    info.+archaeology_text := "O mapa representa a exploração e supremacia naval Aiglana, guiando navios através de águas perigosas.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetBarcoGarrafa()
    var info := struct;
    info.+name := "Barco Dentro de uma Garrafa";
    info.+image := 0xa322;
    info.+flavor_image := 18078;
    info.+culture_name := "Aigla";
    info.+main_text := "O barco-na-garrafa intrincadamente elaborado foi um presente do famoso almirante Aiglano Septimus ao imperador após uma campanha naval bem-sucedida. Dentro da garrafa, uma réplica perfeitamente detalhada de seu navio-almirante navegava em mares vítreos. Dizia-se que a garrafa carregava uma bênção de viagens seguras, garantindo que todos que navegassem sob a bandeira Aiglana retornassem em segurança. Exibida no palácio imperial, a garrafa tornou-se um símbolo da força naval Aiglana, lembrando todos que a viam do controle do império sobre os mares e da glória de sua frota. Este item extraordinário foi criado pelo mestre artesão Aiglano Lucius Claudius.";
    info.+skill_text := "O barco-na-garrafa, um presente do Almirante Septimus, simboliza a força naval Aiglana e a bênção de viagens seguras, comemorando suas vitórias no mar.";
    info.+archaeology_text := "A garrafa simboliza o poder naval Aiglano, representando jornadas seguras e vitoriosas pelos mares.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetColarOuroPrefeito()
    var info := struct;
    info.+name := "Grande Colar de Ouro do Prefeito";
    info.+image := 0xb40e;  // Gráfico do item
    info.+flavor_image := 18079;  // ID da imagem de flavor
    info.+culture_name := "Aigla";  // Nome da cultura
    info.+main_text := "O grandioso colar de ouro, pesado com ornamentos elaborados, era usado pelo prefeito da capital durante importantes cerimônias cívicas. Adornado com o brasão imperial e gemas, o colar simbolizava a autoridade e a riqueza do cargo. O Prefeito Crassus usou o colar durante o discurso público que marcou o fim de uma grande fome, garantindo a lealdade do povo. Passado de um prefeito para o próximo, o colar tornou-se um símbolo da governança, responsabilidade e prosperidade Aiglana, lembrando os cidadãos do compromisso de seu líder com o bem-estar do império. Este item extraordinário foi forjado pelo renomado ourives Aiglano Lucius Aurelius.";
    info.+skill_text := "O colar de ouro do prefeito, adornado com o brasão imperial, simboliza a autoridade e prosperidade Aiglana, usado durante importantes cerimônias cívicas como um lembrete da responsabilidade do líder para com o povo.";
    info.+archaeology_text := "O colar simboliza autoridade cívica e prosperidade, usado pelos prefeitos Aiglanos para inspirar lealdade.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetVasoAiglanoLargo()
    var info := struct;
    info.+name := "Vaso Aiglano Largo";
    info.+image := 0x42b3;
    info.+flavor_image := 18080;
    info.+culture_name := "Aigla";
    info.+main_text := "O vaso Aiglano largo, pintado com cenas de batalha e vitória, era usado para conter oferendas durante festivais imperiais. Feito de argila fina e adornado com designs intrincados, retratava momentos lendários da história do império. O vaso foi passado por famílias nobres e colocado em templos para honrar os deuses. Durante o reinado do Imperador Crassus, o vaso foi preenchido com moedas de ouro e colocado aos pés de uma estátua para garantir a prosperidade da cidade. O vaso tornou-se um símbolo de riqueza, tradição e reverência aos deuses. Este item extraordinário foi criado pelo mestre oleiro Aiglano Marcus Claudius.";
    info.+skill_text := "O vaso Aiglano largo, adornado com cenas de batalha, simboliza riqueza, tradição e reverência, usado em festivais imperiais e oferendas em templos para honrar os deuses.";
    info.+archaeology_text := "O vaso simboliza a riqueza e tradição Aiglana, usado para honrar deuses durante festivais imperiais.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetGrandeLivroMagia()
    var info := struct;
    info.+name := "Grande Livro de Magia";
    info.+image := 0x42bf;
    info.+flavor_image := 18081;
    info.+culture_name := "Aluriel";
    info.+main_text := "Na biblioteca arcana dos elfos Aluriel, um grande e antigo livro de magia repousava sobre um pedestal de obsidiana, suas páginas brilhando com o poder de feitiços centenários. O livro, encadernado em couro encantado, dizia-se conter o conhecimento do primeiro Alto Mago, que dominou a magia elemental e celestial. Foi usado por gerações de magos Aluriel para proteger seu reino, e suas páginas eram consultadas em tempos de crise. O livro tornou-se um símbolo do domínio dos Aluriels sobre o arcano, representando seu controle sobre as forças da natureza e da magia. Este item extraordinário foi compilado pelo lendário Arquimago Aluriel Elyndor, o Tecelão de Feitiços.";
    info.+skill_text := "O grande livro de magia, repleto de feitiços centenários, simboliza o domínio dos Aluriels sobre a magia e a natureza, consultado por gerações de magos para proteger seu reino.";
    info.+archaeology_text := "O livro de magia simboliza o domínio arcano dos Aluriels, repleto de antigos feitiços de proteção.";
    info.+required_skill := "Magery";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetGrandeAdagaCerimonial()
    var info := struct;
    info.+name := "Grande Adaga Cerimonial";
    info.+image := 0x48ae;
    info.+flavor_image := 18082;
    info.+culture_name := "Björske";
    info.+main_text := "Forjada nos fogos sagrados das montanhas Björske, a grande adaga cerimonial foi criada para honrar os deuses da guerra. Sua lâmina, adornada com runas antigas, era usada durante rituais importantes pelos chefes para abençoar armas e guerreiros antes da batalha. Em uma cerimônia memorável, o Chefe Ragnar usou a adaga para esculpir símbolos de proteção na terra antes de uma grande batalha, garantindo a vitória de sua tribo. A adaga, passada por gerações de líderes Björske, tornou-se um símbolo reverenciado do favor divino, tradição e da conexão inquebrável da tribo com seus deuses guerreiros. Este item extraordinário foi forjado pelo lendário ferreiro Björske Bjorn Martelo de Ferro.";
    info.+skill_text := "A adaga cerimonial, gravada com runas e usada em rituais sagrados, simboliza o favor divino e a profunda conexão dos Björske com seus deuses da guerra e da vitória.";
    info.+archaeology_text := "A adaga cerimonial representa a tradição Björske, usada para abençoar guerreiros e garantir favor divino.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetPequenoFrascoVeneno()
    var info := struct;
    info.+name := "Pequeno Frasco de Veneno";
    info.+image := 0x5748;
    info.+flavor_image := 18083;
    info.+culture_name := "Har'oloth";
    info.+main_text := "Na cidade subterrânea mal iluminada de Har'oloth, o pequeno frasco de veneno era uma ferramenta mortal usada por assassinos Drow. Preenchido com um líquido esverdeado extraído de aranhas venenosas, o frasco poderia matar com uma única gota. O veneno era usado durante operações secretas para eliminar rivais políticos ou trair inimigos poderosos. Um desses frascos foi empregado no assassinato da matriarca de uma casa rival, envenenando-a durante um banquete. O frasco tornou-se um símbolo da precisão letal dos Drow e de seu domínio sobre o veneno, representando a política sombria de sua sociedade. Este item extraordinário foi criado pela infame alquimista Drow Zar'leth, a Mestra dos Venenos.";
    info.+skill_text := "O frasco de veneno, preenchido com veneno das aranhas mortais de Har'oloth, simboliza o domínio dos Drow sobre o assassinato e suas implacáveis lutas de poder político nas sombras.";
    info.+archaeology_text := "O frasco de veneno representa a precisão letal dos Drow, usado para eliminar inimigos com eficiência mortal.";
    info.+required_skill := "Alchemy";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetPequenaEscalaPesagemAlquimica()
    var info := struct;
    info.+name := "Pequena Escala de Pesagem Alquímica";
    info.+image := 0x573a;
    info.+flavor_image := 18084;
    info.+culture_name := "Aluriel";
    info.+main_text := "A pequena escala de pesagem alquímica, feita de prata e encantada para garantir medições precisas, era uma ferramenta crucial para os alquimistas Aluriel. Usada em seus experimentos mais delicados, a escala equilibrava ingredientes raros para poções e elixires que podiam curar, ferir ou aprimorar habilidades mágicas. Em uma ocasião, o alto alquimista Elandir usou a escala para equilibrar perfeitamente os ingredientes necessários para um elixir que salvou a rainha Aluriel de um veneno mortal. A escala tornou-se um símbolo da precisão e domínio da alquimia dos Aluriel, garantindo o sucesso de suas concocções mágicas. Este item extraordinário foi criado pelo renomado alquimista Aluriel Elyndria, o Balanceador de Essências.";
    info.+skill_text := "A escala alquímica, encantada para precisão, simboliza o domínio Aluriel na fabricação de poções, ajudando seus alquimistas a equilibrar ingredientes raros para elixires poderosos que curam, ferem ou aprimoram habilidades mágicas.";
    info.+archaeology_text := "A escala representa a precisão Aluriel na alquimia, garantindo sucesso na criação de elixires poderosos.";
    info.+required_skill := "Alchemy";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetBoleadera()
    var info := struct;
    info.+name := "Boleadera (Bolas)";
    info.+image := 0x26ac;
    info.+flavor_image := 18085;
    info.+culture_name := "Polski";
    info.+main_text := "Nas planícies gramadas de Verdeflor, os caçadores Polski usavam a boleadera, ou bolas, para capturar presas em movimento rápido. Feitas de cordas de couro trançadas e pedras pesadas, as bolas eram giradas em amplos arcos antes de serem lançadas para enredar as pernas de animais em fuga. Um caçador chamado Yarek usou suas bolas durante uma caçada perigosa a um javali selvagem, habilmente derrubando a fera para o banquete da aldeia. A boleadera tornou-se um símbolo de habilidade, precisão e da conexão do povo Polski com a terra e sua generosidade. Este item extraordinário foi criado pelo renomado caçador Polski Yarek, o Lançador Preciso.";
    info.+skill_text := "A boleadera, usada pelos caçadores Polski, simboliza precisão e habilidade na caça, capturando presas ao enredar suas pernas com lançamentos rápidos.";
    info.+archaeology_text := "A boleadera representa as tradições de caça Polski, garantindo caçadas bem-sucedidas com lançamentos habilidosos e precisos.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCapaceteGuerraSamurai()
    var info := struct;
    info.+name := "Capacete de Guerra Samurai";
    info.+image := 0x2778;
    info.+flavor_image := 18086;
    info.+culture_name := "Kaijing";
    info.+main_text := "O capacete de guerra samurai, ou kabuto, usado pelo lendário guerreiro Kaijing Hitoshi, foi intrincadamente projetado com chifres curvos e adornado com símbolos do clã. Hitoshi usou o capacete durante a Batalha do Vale Vermelho, onde liderou suas forças à vitória contra um clã rival. O kabuto, passado por gerações de guerreiros, tornou-se um símbolo de honra, força e disciplina inflexível. Após a morte de Hitoshi, o capacete foi colocado no templo como uma relíquia, inspirando futuros guerreiros a seguir o caminho do samurai. Este item extraordinário foi forjado pelo mestre armeiro Kaijing Takeshi, o Moldador de Aço.";
    info.+skill_text := "O capacete de guerra samurai, usado por Hitoshi, simboliza honra, força e a disciplina dos guerreiros Kaijing, passado como uma relíquia de vitória.";
    info.+archaeology_text := "O capacete representa a honra e disciplina dos guerreiros Kaijing, um legado de força inflexível.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetKatanaDisplayPe()
    var info := struct;
    info.+name := "Katana no Display em Pé";
    info.+image := 0x2855;
    info.+flavor_image := 18087;
    info.+culture_name := "Kaijing";
    info.+main_text := "No silencioso salão de meditação do templo Kaijing, uma katana exibida em pé era testemunha da maestria em esgrima do Mestre Kaito. A katana, com sua lâmina curva e fio perfeito, dizia-se ter sido usada em um duelo que salvou o templo de invasores. Após sua vitória, Kaito colocou a espada em exibição, em pé, para simbolizar tanto a paz quanto a prontidão. A katana tornou-se uma relíquia sagrada do templo, representando o equilíbrio entre agressão e serenidade, lembrando todos que a viam da devoção dos Kaijing à excelência marcial. Este item extraordinário foi forjado pelo lendário espadachim Kaijing Kaito, o Guardião do Templo.";
    info.+skill_text := "A katana exibida em pé, uma vez empunhada pelo Mestre Kaito, simboliza o equilíbrio entre paz e prontidão na tradição Kaijing, incorporando excelência marcial e contenção.";
    info.+archaeology_text := "A katana simboliza o equilíbrio Kaijing entre agressão e serenidade, permanecendo como uma relíquia sagrada.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetBonsai()
    var info := struct;
    info.+name := "Bonsai";
    info.+image := 0x28de;
    info.+flavor_image := 18088;
    info.+culture_name := "Kaijing";
    info.+main_text := "Nos jardins sagrados do templo Kaijing, crescia uma árvore bonsai meticulosamente podada, simbolizando paciência, disciplina e a conexão dos monges com a natureza. A árvore era cuidada pelos monges, com cada galho cuidadosamente moldado para refletir a harmonia da vida. Durante as sessões de meditação, os monges se reuniam ao redor do bonsai para contemplar o equilíbrio entre crescimento e controle. A árvore, passada por gerações, tornou-se um símbolo da filosofia Kaijing de domínio sobre si mesmo e o mundo ao seu redor, ensinando paciência e atenção plena. Este item extraordinário foi cultivado pelo venerável Mestre Kaijing Hiroshi, o Guardião do Jardim.";
    info.+skill_text := "O bonsai, podado e moldado pelos monges Kaijing, simboliza paciência, disciplina e a harmonia da natureza, ensinando atenção plena e autodomínio através do cultivo cuidadoso.";
    info.+archaeology_text := "A árvore bonsai simboliza a disciplina e o equilíbrio Kaijing, refletindo a harmonia entre crescimento e controle.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetDuasKatanasDisplayHorizontal()
    var info := struct;
    info.+name := "Duas Katanas no Display Horizontal";
    info.+image := 0x2a2a;
    info.+flavor_image := 18089;
    info.+culture_name := "Kaijing";
    info.+main_text := "No salão do templo Kaijing, duas katanas eram exibidas horizontalmente em um suporte de madeira, representando a dualidade de defesa e ataque. As espadas emparelhadas foram forjadas pelo mestre ferreiro Hayato e usadas na defesa do templo durante um grande cerco. Uma espada simbolizava proteção, enquanto a outra incorporava agressão, e juntas se tornaram símbolos de equilíbrio e proeza marcial. As katanas duplas eram honradas como relíquias, ensinando às futuras gerações a importância de entender tanto a arte da defesa quanto o domínio de atacar com precisão. Este item extraordinário foi forjado pelo renomado mestre ferreiro Kaijing Hayato, o Forjador das Duas Lâminas.";
    info.+skill_text := "As duas katanas exibidas horizontalmente simbolizam a dualidade de defesa e ataque, incorporando a tradição Kaijing de equilíbrio e proeza marcial, forjadas pelo mestre ferreiro Hayato.";
    info.+archaeology_text := "As katanas emparelhadas simbolizam o equilíbrio entre defesa e ataque, representando o domínio marcial Kaijing.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetChapeuCapitaoPirata()
    var info := struct;
    info.+name := "Chapéu do Capitão Pirata";
    info.+image := 0x171B;
    info.+flavor_image := 18090;
    info.+culture_name := "Aigla";
    info.+main_text := "O chapéu tricórnio preto do capitão pirata, adornado com uma única pena vermelha, era usado pelo infame pirata Aiglano, Capitão Adrastos. Conhecido por seus ousados ataques a navios imperiais, o chapéu tornou-se um símbolo de rebelião e liberdade nos mares. Durante a Batalha das Ilhas Estilhaçadas, Adrastos foi capturado, mas sua tripulação escapou, levando o chapéu do capitão como um símbolo de sua lealdade e desafio. O chapéu foi posteriormente exibido em um covil pirata secreto, inspirando futuras gerações de marinheiros e foras-da-lei. Este item extraordinário pertenceu ao lendário Capitão Adrastos, o Terror dos Mares Aiglanos.";
    info.+skill_text := "O chapéu do capitão pirata, usado pelo Capitão Adrastos, simboliza rebelião, liberdade e lealdade, passado por sua tripulação como um símbolo de desafio contra o império Aiglano.";
    info.+archaeology_text := "O chapéu pirata simboliza o desafio de Adrastos e o espírito rebelde dos foras-da-lei Aiglanos.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEscalaPesagemAna()
    var info := struct;
    info.+name := "Escala de Pesagem Anã";
    info.+image := 0x1851;
    info.+flavor_image := 18091;
    info.+culture_name := "Badûran";
    info.+main_text := "A escala de pesagem anã, forjada em ferro resistente e intrincadamente gravada com runas, era uma ferramenta crítica nas forjas Badûran. Usada para medir com precisão metais preciosos e gemas, a escala garantia que cada item fabricado fosse perfeitamente equilibrado. Uma escala, usada pelo mestre ferreiro Thrain, dizia-se ter sido empregada na criação dos lendários machados de batalha Badûran, garantindo que seu peso fosse perfeito tanto para o combate quanto para o equilíbrio. A escala tornou-se um símbolo da precisão e do artesanato dos anões, passado por gerações de ferreiros. Este item extraordinário foi criado pelo renomado mestre ferreiro Badûran Thrain, o Balanceador de Lâminas.";
    info.+skill_text := "A escala de pesagem anã, usada por mestres ferreiros, simboliza a precisão e o artesanato Badûran, garantindo que cada item, de metais preciosos a machados de batalha, seja perfeitamente equilibrado.";
    info.+archaeology_text := "A escala de pesagem simboliza a precisão e o artesanato Badûran, fundamentais para sua lendária fabricação de armas.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEscudoRomanoTriangular()
    var info := struct;
    info.+name := "Escudo Romano Triangular";
    info.+image := 0x1b76;
    info.+flavor_image := 18092;
    info.+culture_name := "Aigla";
    info.+main_text := "O escudo triangular de estilo romano, decorado com a águia imperial, era usado pelas legiões Aiglanas em suas campanhas para defender as fronteiras do império. A forma única do escudo permitia uma maior defesa durante formações em falange, e seu peso o tornava uma ferramenta formidável em combate. Um desses escudos, carregado pelo Centurião Cassius durante o Cerco de Borfjell, ficou amassado e ensanguentado de incontáveis batalhas, mas nunca quebrou. O escudo tornou-se um símbolo da disciplina, força e lealdade Aiglana, passado pelas fileiras da legião. Este item extraordinário pertenceu ao valente Centurião Cassius, o Escudo de Borfjell.";
    info.+skill_text := "O escudo romano triangular, usado pelas legiões Aiglanas, simboliza disciplina, força e lealdade, projetado para máxima defesa em batalha e passado pelas fileiras de soldados.";
    info.+archaeology_text := "O escudo simboliza a disciplina e força Aiglana, protegendo os legionários em incontáveis batalhas.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCapaceteCranioOrc()
    var info := struct;
    info.+name := "Capacete de Crânio de Orc";
    info.+image := 0x1f0B;
    info.+flavor_image := 18093;
    info.+culture_name := "Björske";
    info.+main_text := "O capacete de crânio de orc, usado pelo senhor da guerra Björske Ragnar, foi feito do crânio de um chefe orc abatido após uma batalha brutal. O capacete, reforçado com ferro, simbolizava a vitória de Ragnar sobre os orcs selvagens e se tornou um símbolo de seu domínio. Usado durante conselhos tribais e batalhas, o capacete de crânio infundia medo nos corações dos inimigos. Passado pela linhagem de Ragnar, tornou-se um artefato reverenciado, simbolizando a força dos Björske e sua disposição para conquistar qualquer inimigo, por mais temível que fosse.";
    info.+skill_text := "O capacete de crânio de orc, usado por Ragnar, simboliza o domínio e a força Björske, um troféu de vitória sobre os orcs selvagens, infundindo medo nos corações dos inimigos.";
    info.+archaeology_text := "O capacete de crânio de orc representa a força Björske, simbolizando sua vitória sobre inimigos temíveis.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetAbajurOrientalAntigo()
    var info := struct;
    info.+name := "Abajur de Chão Oriental Antigo Rústico de Pedra de Três Andares";
    info.+image := 0x24bf;
    info.+flavor_image := 18094;
    info.+culture_name := "Kaijing ";
    info.+main_text := "No pátio tranquilo do templo Kaijing, um abajur de chão de pedra de três andares erguia-se como um símbolo da conexão dos monges com a terra e o equilíbrio entre luz e sombra. O abajur, esculpido em pedra bruta, era usado durante as meditações noturnas, seu brilho suave guiando os monges em suas reflexões sobre os mistérios da vida. Cada andar do abajur simbolizava um estágio diferente de iluminação, lembrando os monges de sua jornada espiritual. O antigo abajur, passado por gerações, tornou-se um artefato reverenciado, incorporando a filosofia Kaijing de equilíbrio, paciência e paz interior.";
    info.+skill_text := "O abajur de pedra de três andares, usado durante meditações, simboliza a jornada espiritual, o equilíbrio e a iluminação dos monges Kaijing, guiando-os através de reflexões noturnas com seu brilho suave e calmante.";
    info.+archaeology_text := "O abajur de pedra representa o equilíbrio Kaijing, iluminando o caminho espiritual dos monges em direção à iluminação e paz interior.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEstandarteRomano()
    var info := struct;
    info.+name := "Estandarte Romano";
    info.+image := 0xf93a;
    info.+flavor_image := 18095;
    info.+culture_name := "Aigla";
    info.+main_text := "O estandarte Aiglano de estilo romano, carregado pelas legiões, ostentava a águia imperial e era coberto de carmesim. Erguido durante as batalhas, o estandarte simbolizava o poder e a unidade do império. Ele liderava os soldados em todas as campanhas, tremulando sobre campos vitoriosos. O centurião Marcus carregou famosamente o estandarte durante o Cerco de Borfjell, onde permaneceu de pé apesar da feroz resistência. O estandarte tornou-se um símbolo de força inabalável e orgulho, carregado por todas as gerações de legionários para inspirar coragem e lealdade ao império.";
    info.+skill_text := "O estandarte romano, ostentando a águia imperial, simboliza a força e a unidade do império Aiglano, carregado em batalha pelos legionários para inspirar coragem e lealdade.";
    info.+archaeology_text := "O estandarte representa a força e unidade Aiglana, tremulando alto sobre as legiões vitoriosas em batalha.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEstandarteDrow()
    var info := struct;
    info.+name := "Estandarte Drow";
    info.+image := 0xf93d;
    info.+flavor_image := 18096;
    info.+culture_name := "Har'oloth";
    info.+main_text := "O estandarte escuro e sedoso dos Drow, tecido com fios de sombra e bordado com o símbolo de uma aranha negra, era hasteado durante incursões e cerimônias políticas em Har'oloth. O estandarte representava a devoção dos Drow a Lolth e seu domínio sobre os reinos sombrios e facções rivais. Durante o Festival das Sombras, o estandarte era carregado pela Alta Sacerdotisa como um símbolo da superioridade dos Drow. Sua presença no campo de batalha inspirava medo, lembrando seus inimigos da astúcia e poder implacáveis dos Drow.";
    info.+skill_text := "O estandarte Drow, adornado com o símbolo da aranha negra de Lolth, representa o domínio e a devoção dos Drow, inspirando medo e reverência durante cerimônias políticas e batalhas.";
    info.+archaeology_text := "O estandarte Drow simboliza o poder de Lolth e o domínio astuto de seus seguidores em batalha.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetArmaduraCouro()
    var info := struct;
    info.+name := "Display de Armadura de Couro Batido";
    info.+image := 0x13d9;
    info.+flavor_image := 18097;
    info.+culture_name := "Björske";
    info.+main_text := "A armadura de couro batido, exibida no grande salão de uma aldeia Björske, foi usada pelo guerreiro Eirik durante a Batalha do Passo Congelado. A armadura, marcada e endurecida por incontáveis golpes, protegeu-o em combate corpo a corpo enquanto lutava ao lado de seu clã. Após a batalha, a armadura de Eirik foi exposta como um símbolo da resiliência e sobrevivência dos Björske em condições adversas. O couro batido, embora desgastado, representava a força e determinação da tribo, passado como uma relíquia de resistência e orgulho guerreiro.";
    info.+skill_text := "A armadura de couro batido, marcada pela batalha, simboliza a resiliência e sobrevivência Björske, representando a força da tribo e o orgulho guerreiro em suportar condições adversas.";
    info.+archaeology_text := "A armadura de couro batido representa a resiliência Björske, suas cicatrizes simbolizando a resistência da tribo em batalha.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetArmaduraCotaMalha()
    var info := struct;
    info.+name := "Display de Armadura de Cota de Malha";
    info.+image := 0x13e7;
    info.+flavor_image := 18098;
    info.+culture_name := "Aigla";
    info.+main_text := "A armadura de cota de malha, forjada pelos ferreiros Aiglanos, foi usada pelo Centurião Crassus durante a conquista dos Territórios do Norte. Os finos elos da cota de malha brilhavam enquanto o protegiam de incontáveis golpes inimigos. Após as vitórias de Crassus, a armadura foi exibida no Salão dos Heróis, seus elos perfeitamente conservados como testemunho da força e habilidade artesanal do império. A cota de malha tornou-se um símbolo de proteção, disciplina e superioridade tática das legiões Aiglanas, lembrando as gerações futuras da importância de manter tanto as defesas físicas quanto mentais em batalha.";
    info.+skill_text := "A armadura de cota de malha, usada pelo Centurião Crassus, simboliza a disciplina e proteção Aiglana, seus elos finamente forjados representando a superioridade tática e resiliência das legiões do império.";
    info.+archaeology_text := "A cota de malha representa a força Aiglana, protegendo seus soldados e simbolizando disciplina e habilidade tática.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMiniaturaNavioRomano()
    var info := struct;
    info.+name := "Miniatura de Navio Romano";
    info.+image := 0x14f3;
    info.+flavor_image := 18099;
    info.+culture_name := "Aigla";
    info.+main_text := "No estudo de um almirante Aiglano, uma miniatura detalhada de um navio de guerra romano estava em exibição. O navio, esculpido em madeira fina e apresentando velas delicadas, representava o poder naval do império e suas vitórias através dos mares. O Almirante Septimus encomendou a miniatura para comemorar a derrota das frotas piratas na Batalha das Ilhas Estilhaçadas. O navio tornou-se um símbolo do controle do império sobre os mares, lembrando todos que o viam da força da marinha Aiglana e seu domínio da guerra marítima.";
    info.+skill_text := "A miniatura do navio de guerra simboliza o domínio naval Aiglano, comemorando vitórias como a Batalha das Ilhas Estilhaçadas e representando o domínio do império sobre os mares.";
    info.+archaeology_text := "O navio simboliza o poder naval Aiglano, lembrando a todos do seu controle sobre os mares.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction


function GetCapaceteVikingAsasAnjo()
    var info := struct;
    info.+name := "Capacete com Asas de Anjo";
    info.+image := 0xf9bb;
    info.+flavor_image := 18100;
    info.+culture_name := "Björske";
    info.+main_text := "O capacete Björske, adornado com asas de anjo estilizadas, foi usado pelo chefe Björske Alrik durante o lendário Ataque às Costas do Sul. As asas, feitas de prata, simbolizavam tanto o favor divino quanto a velocidade de seus guerreiros. Os inimigos de Alrik acreditavam que as asas lhe davam o poder de atacar rapidamente e sem aviso, deixando-os despreparados para a ferocidade de seus ataques. O capacete foi exibido no grande salão após a morte de Alrik, tornando-se um símbolo da proteção divina da tribo e sua reputação como temíveis invasores.";
    info.+skill_text := "O capacete Björske, adornado com asas de anjo de prata, simboliza a velocidade e o favor divino dos Björske, usado pelo Chefe Alrik durante lendários ataques que atacaram rapidamente e sem aviso.";
    info.+archaeology_text := "O capacete simboliza a velocidade e proteção divina dos Björske, incorporando a lendária proeza de ataque do chefe.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMarteloGuerraAnao()
    var info := struct;
    info.+name := "Martelo de Guerra Badûran";
    info.+image := 0xf9aa;
    info.+flavor_image := 18101;
    info.+culture_name := "Badûran";
    info.+main_text := "Forjado nas profundezas das chamas de Khron Kaz'ad, o martelo de guerra Badûran foi criado pelo mestre ferreiro Thrain para a Batalha do Passo de Ferro. Sua cabeça pesada e intrincadamente gravada esmagava as armaduras inimigas com força devastadora. Thrain empunhou o martelo para liderar os anões Badûran à vitória e, após a batalha, tornou-se um símbolo da força e do artesanato Badûran. O martelo de guerra foi passado por gerações de guerreiros, cada um adicionando novas gravuras para honrar suas vitórias. Foi exibido no Salão dos Heróis como uma relíquia do poder Badûran.";
    info.+skill_text := "O martelo de guerra Badûran, forjado pelo mestre ferreiro Thrain, simboliza a força e o artesanato Badûran, sua cabeça pesada e gravada esmagando inimigos em batalha e tornando-se uma relíquia de vitória.";
    info.+archaeology_text := "O martelo de guerra simboliza a força Badûran, suas gravuras representando gerações de guerreiros anões vitoriosos.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEspadasDuplas()
    var info := struct;
    info.+name := "Espadas Duplas";
    info.+image := 0xf9a1;
    info.+flavor_image := 18102;
    info.+culture_name := "Kaijing";
    info.+main_text := "As espadas gêmeas, conhecidas como Lâminas da Harmonia, foram empunhadas pelo Mestre Jian dos monges Kaijing durante a defesa do templo sagrado. As espadas, perfeitamente equilibradas e forjadas do aço mais puro, representavam a natureza dual do ataque e da defesa. O Mestre Jian usava as lâminas para golpear com a velocidade de um raio, desarmando seus inimigos sem derramar sangue desnecessário. As espadas gêmeas foram posteriormente exibidas no templo como um símbolo da filosofia Kaijing de equilíbrio, lembrando os futuros guerreiros da importância de dominar tanto o ataque quanto a defesa em harmonia.";
    info.+skill_text := "As espadas gêmeas, empunhadas pelo Mestre Jian, simbolizam a filosofia Kaijing de equilíbrio, representando o domínio do ataque e da defesa em harmonia durante a defesa do templo sagrado.";
    info.+archaeology_text := "As espadas gêmeas representam o equilíbrio Kaijing, sua perfeita harmonia de ataque e defesa simbolizando o domínio marcial.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEstandarteOrcsCaveirasSangue()
    var info := struct;
    info.+name := "Estandarte de Orcs com Caveira de Sangue";
    info.+image := 0xf942;
    info.+flavor_image := 18103;
    info.+culture_name := "Gulthrak";
    info.+main_text := "O estandarte de guerra orc, adornado com o crânio ensanguentado de um inimigo caído, era erguido alto durante ataques e batalhas. Pintado com símbolos cruéis e violentos, o estandarte representava a natureza selvagem dos orcs e sua sede de conquista. Carregado pelo chefe Gorrak durante o Cerco de Blackstone, o estandarte foi encharcado com o sangue daqueles que caíram em batalha, tornando-se um lembrete temível da brutalidade dos orcs. O estandarte da caveira de sangue tornou-se um símbolo de força e dominação, passado por gerações de chefes orcs para inspirar terror em seus inimigos.";
    info.+skill_text := "O estandarte orc da caveira de sangue, erguido durante a batalha, simboliza a força selvagem e a conquista dos orcs, encharcado com o sangue de seus inimigos e passado para inspirar terror em seus adversários.";
    info.+archaeology_text := "O estandarte representa a brutalidade orc, simbolizando força e dominação com seu crânio ensanguentado.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEstandarteElfosAzulBranco()
    var info := struct;
    info.+name := "Estandarte de Elfos Azul e Branco";
    info.+image := 0xf93f;
    info.+flavor_image := 18104;
    info.+culture_name := "Aluriel";
    info.+main_text := "O elegante estandarte azul e branco dos Aluriels, bordado com fios de prata, era hasteado durante cerimônias de paz e eventos diplomáticos. Seu design suave e fluido simbolizava a conexão dos elfos com os elementos celestiais e sua busca pela harmonia. Durante a assinatura do Tratado das Águas Prateadas, o estandarte foi erguido para marcar o fim de um conflito de longa data. As cores azul e branca representavam pureza e paz, e o estandarte tornou-se um símbolo do compromisso dos Aluriels com a diplomacia, justiça e equilíbrio em seu reino.";
    info.+skill_text := "O estandarte élfico azul e branco, simbolizando pureza e paz, representa o compromisso dos Aluriels com a harmonia, erguido durante eventos diplomáticos e marcando o fim de conflitos como o Tratado das Águas Prateadas.";
    info.+archaeology_text := "O estandarte simboliza a harmonia e diplomacia Aluriel, representando paz e equilíbrio em seu reino.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCimitarraEnsanguentada()
    var info := struct;
    info.+name := "Cimitarra Ensanguentada";
    info.+image := 0xef61;
    info.+flavor_image := 18105;
    info.+culture_name := "Kaijing";
    info.+main_text := "A cimitarra ensanguentada, empunhada pelo monge-guerreiro Kaijing Jinhai durante a Batalha do Rio Nebuloso, era uma arma de precisão e graça mortal. A lâmina curva, ainda manchada com o sangue de inimigos caídos, havia sido usada em uma última defesa desesperada do templo sagrado. A habilidade de Jinhai com a cimitarra permitiu que ele cortasse as forças inimigas com golpes rápidos e mortais. Após a batalha, a cimitarra ensanguentada foi colocada no templo como um lembrete dos sacrifícios feitos para proteger seu lar. A arma tornou-se um símbolo da resiliência e da proeza marcial Kaijing.";
    info.+skill_text := "A cimitarra ensanguentada de Jinhai, usada na defesa do templo Kaijing, simboliza resiliência e precisão mortal, representando a habilidade e os sacrifícios dos monges-guerreiros em batalha.";
    info.+archaeology_text := "A cimitarra ensanguentada simboliza a resiliência Kaijing, um lembrete da precisão mortal na defesa.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEspadaLongaEnsanguentada()
    var info := struct;
    info.+name := "Espada Longa Ensanguentada";
    info.+image := 0xef62;
    info.+flavor_image := 18106;
    info.+culture_name := "Aigla";
    info.+main_text := "A espada longa ensanguentada foi carregada pelo Centurião Cassius durante a defesa das muralhas do norte de Aigla. A espada, forjada em aço imperial, foi manchada com o sangue de incontáveis inimigos enquanto Cassius liderava seus legionários contra os bárbaros invasores. A cada golpe, a lâmina cortava armaduras e carne, virando a maré da batalha. Após a vitória, a espada longa ensanguentada foi preservada no Salão dos Heróis, um símbolo da força do império e da lealdade inabalável de seus soldados. A espada tornou-se uma relíquia reverenciada, incorporando o poder e o sacrifício das legiões Aiglanas.";
    info.+skill_text := "A espada longa ensanguentada, empunhada pelo Centurião Cassius, simboliza a força e a lealdade das legiões Aiglanas, um lembrete do poder do império e dos sacrifícios feitos em sua defesa.";
    info.+archaeology_text := "A espada longa ensanguentada simboliza a força e o sacrifício Aiglano, um lembrete de batalhas vitoriosas.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEscudoRedondoBronzeEnsanguentado()
    var info := struct;
    info.+name := "Escudo Redondo de Bronze Ensanguentado";
    info.+image := 0xef5d;
    info.+flavor_image := 18107;
    info.+culture_name := "Björske";
    info.+main_text := "O escudo redondo de bronze ensanguentado foi usado pelo guerreiro Björske Skaldar durante o Cerco da Fortaleza de Gelo. O escudo, marcado por profundas amassaduras e coberto com o sangue de amigos e inimigos, havia protegido Skaldar enquanto ele lutava em um combate corpo a corpo brutal. Sua robusta superfície de bronze desviou incontáveis golpes, permitindo-lhe virar a maré da batalha. Após o cerco, o escudo ensanguentado foi exibido no grande salão, tornando-se um símbolo da força inabalável dos Björske e sua feroz determinação em sobreviver em terras duras e implacáveis.";
    info.+skill_text := "O escudo de bronze ensanguentado de Skaldar simboliza a força e determinação dos Björske, protegendo o guerreiro em combate brutal e tornando-se uma relíquia de sobrevivência e resiliência.";
    info.+archaeology_text := "O escudo ensanguentado simboliza a resiliência Björske, protegendo guerreiros em batalhas duras e implacáveis.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEstandarteElfosFlorestaArvore()
    var info := struct;
    info.+name := "Estandarte dos Elfos da Floresta com uma Árvore no Meio";
    info.+image := 0xf920;
    info.+flavor_image := 18108;
    info.+culture_name := "Aluriel";
    info.+main_text := "O estandarte dos elfos da floresta, feito da madeira de árvores sagradas e bordado com o emblema de um grande carvalho, era hasteado durante o festival anual de renovação. O estandarte simbolizava a profunda conexão dos elfos com a floresta e seu papel como seus protetores. Em tempos de guerra, o estandarte era carregado pelo alto guardião Elandor em batalha, sua presença invocando as bênçãos dos espíritos da floresta. Após uma grande vitória contra os invasores, o estandarte foi colocado no coração da floresta como um símbolo do compromisso dos elfos com a defesa da natureza.";
    info.+skill_text := "O estandarte dos elfos da floresta, feito de árvores sagradas, simboliza a conexão dos elfos com a natureza, carregado em batalha para invocar as bênçãos dos espíritos da floresta e proteger sua terra.";
    info.+archaeology_text := "O estandarte simboliza a profunda conexão dos elfos com a floresta, invocando sua proteção em batalha.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEscudoElficoAzulUnicornioBranco()
    var info := struct;
    info.+name := "Escudo Élfico Azul com Brasão de Unicórnio Branco";
    info.+image := 0xfa5f;
    info.+flavor_image := 18109;
    info.+culture_name := "Aluriel";
    info.+main_text := "O escudo élfico azul, feito de madeira encantada e adornado com o brasão de um unicórnio branco, era carregado pela guarda élfica de elite durante cerimônias reais. As propriedades místicas do escudo protegiam seu portador de ataques físicos e mágicos. Durante a defesa da Clareira Prateada, o escudo foi usado pelo capitão da guarda, desviando feitiços e flechas mortais. Após a vitória, o escudo tornou-se um símbolo de pureza, proteção e do vínculo inquebrantável dos elfos com as criaturas mágicas de seu reino, exibido orgulhosamente no salão real.";
    info.+skill_text := "O escudo élfico azul com o brasão de um unicórnio branco simboliza pureza, proteção e o vínculo mágico dos elfos, usado pela guarda de elite para defender seu reino de ameaças mágicas e físicas.";
    info.+archaeology_text := "O escudo representa a proteção mágica dos elfos e seu vínculo com criaturas místicas, simbolizando pureza.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetLivroNecromanciaCapaCouroCaveira()
    var info := struct;
    info.+name := "Livro de Necromancia de Capa de Couro e Caveira na Capa";
    info.+image := 0xe9c0;
    info.+flavor_image := 18110;
    info.+culture_name := "Har'oloth";
    info.+main_text := "Na cidade subterrânea de Har'oloth, um livro proibido de necromancia, encadernado em couro negro e gravado com uma caveira, repousava sobre um altar de obsidiana. O livro continha poderosos feitiços para ressuscitar os mortos e controlar espíritos, usado pelas sacerdotisas Drow para expandir sua influência sombria. A alta sacerdotisa Lilithra usou o livro para invocar um exército de mortos-vivos durante a Guerra das Profundezas Sombrias. Após sua vitória, o livro de necromancia tornou-se um símbolo do domínio Drow sobre a morte, preservado no templo como um lembrete de seu controle sobre a vida e a morte.";
    info.+skill_text := "O livro de necromancia, encadernado em couro com um emblema de caveira, simboliza o domínio dos Drows sobre a morte, contendo poderosos feitiços usados para invocar e controlar os mortos durante rituais sombrios.";
    info.+archaeology_text := "O livro simboliza o controle dos Drows sobre a morte, contendo sombrios feitiços necromânticos para invocar os mortos.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetLivroLeisSenadorCapaCinza()
    var info := struct;
    info.+name := "Livro de Leis do Senado";
    info.+image := 0xea04;
    info.+flavor_image := 18111;
    info.+culture_name := "Aigla";
    info.+main_text := "O Livro de Leis do Senado, encadernado em couro cinza e gravado com o brasão imperial, continha os principais estatutos legais que governavam o império Aiglano. Cada senador carregava uma cópia do livro durante as sessões, consultando suas páginas durante debates acalorados e reformas legislativas. Escrito pelo grande legislador Senador Valerianus, o livro tornou-se a base do sistema legal do império. Após sua morte, foi revisado por senadores subsequentes, mas o livro original de capa cinza permaneceu um símbolo de justiça, lei e ordem na governança de Aigla.";
    info.+skill_text := "O Livro de Leis do Senado, gravado com o brasão imperial, simboliza a justiça e a lei no império Aiglano, usado pelos senadores para governar e manter a ordem através de reformas legais.";
    info.+archaeology_text := "O livro de leis representa a governança Aiglana, garantindo justiça e ordem através de seus abrangentes estatutos legais.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetLivroCapaCinzaMarcaPaginaVerde()
    var info := struct;
    info.+name := "Livro de Estudos";
    info.+image := 0xea05;
    info.+flavor_image := 18112;
    info.+culture_name := "Aluriel";
    info.+main_text := "O livro de capa cinza, marcado com uma fita de seda verde, pertencia ao estudioso Aluriel Elandir, que o usava para registrar os segredos da magia elemental. Cada página estava preenchida com delicada escrita élfica, detalhando o equilíbrio da terra, água, vento e fogo. O marcador verde indicava um feitiço particularmente poderoso, usado para invocar uma tempestade que salvou a cidade élfica de uma seca. O livro tornou-se um símbolo do domínio dos elfos sobre os elementos naturais e sua conexão com o equilíbrio da vida. Foi passado para futuras gerações de magos Aluriel.";
    info.+skill_text := "O livro de capa cinza, preenchido com segredos de magia elemental, simboliza o domínio dos elfos sobre a natureza, com um marcador verde indicando um feitiço que invocou uma tempestade salvadora.";
    info.+archaeology_text := "O livro simboliza o domínio Aluriel da magia elemental, suas páginas repletas dos segredos da natureza.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetDisplayPeitoralFullplate()
    var info := struct;
    info.+name := "Display de Peitoral de Fullplate";
    info.+image := 0xec95;
    info.+flavor_image := 18113;
    info.+culture_name := "Aigla";
    info.+main_text := "O peitoral de fullplate, exibido na armaria imperial de Aigla, foi usado pelo campeão gladiador Tiberius na Grande Arena. Feito de aço reluzente e gravado com símbolos imperiais, o peitoral o protegeu durante inúmeras batalhas, permitindo-lhe ganhar o favor do imperador. Após sua aposentadoria, a armadura foi exibida como um símbolo de seu recorde invicto e do compromisso do império com a excelência marcial. O peitoral tornou-se uma relíquia de glória, representando a força, resistência e habilidade necessárias para sobreviver nos mortais combates da arena.";
    info.+skill_text := "O peitoral de fullplate, usado pelo gladiador invicto Tiberius, simboliza a excelência marcial e a força Aiglana, preservado como uma relíquia de glória na armaria imperial.";
    info.+archaeology_text := "O peitoral simboliza a excelência marcial, representando força e resistência nos mortais combates de Aigla.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetPitchforkEnsanguentada()
    var info := struct;
    info.+name := "Pitchfork Ensanguentada";
    info.+image := 0xef6b;
    info.+flavor_image := 18114;
    info.+culture_name := "Polski";
    info.+main_text := "O forcado ensanguentado foi empunhado por um bravo fazendeiro Polski chamado Borik durante a Revolta de Colhesol. Quando a vila foi atacada por bandidos, Borik e os outros aldeões usaram quaisquer ferramentas que pudessem encontrar para defender suas casas. O forcado, manchado com o sangue dos inimigos, tornou-se um símbolo de desafio e da força do povo comum resistindo à opressão. Após a revolta, o forcado foi exibido na praça da vila, servindo como um lembrete do poder da unidade e da coragem do povo Polski.";
    info.+skill_text := "O forcado ensanguentado, empunhado por Borik durante a Revolta de Colhesol, simboliza o desafio e a força dos aldeões Polski em sua luta contra a opressão.";
    info.+archaeology_text := "O forcado simboliza a unidade e a coragem dos aldeões Polski na defesa de suas casas.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEspelhoCorpo()
    var info := struct;
    info.+name := "Espelho de Corpo";
    info.+image := 0xafc8;
    info.+flavor_image := 18115;
    info.+culture_name := "Aluriel";
    info.+main_text := "O espelho de corpo inteiro, emoldurado em prata e intrincadamente esculpido com runas élficas, foi encantado pelos magos Aluriel para revelar a verdadeira natureza de uma pessoa. Usado pela alta sacerdotisa durante rituais, o espelho mostrava não apenas o reflexo, mas também a essência interior do observador. Em uma ocasião, o espelho foi consultado para descobrir um traidor dentro da corte real, revelando a alma corrupta do indivíduo. Tornou-se uma ferramenta reverenciada no templo, simbolizando a autorreflexão, a verdade e a busca pela pureza espiritual no reino élfico.";
    info.+skill_text := "O espelho encantado, emoldurado em prata e esculpido com runas, revela a essência interior do observador, simbolizando verdade, autorreflexão e pureza espiritual na cultura Aluriel.";
    info.+archaeology_text := "O espelho simboliza a busca pela verdade e autorreflexão, revelando a verdadeira natureza do observador.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetCabecaCiclopeEmpalhada()
    var info := struct;
    info.+name := "Cabeça Empalhada de Ciclope";
    info.+image := 0xc0d9;
    info.+flavor_image := 18116;
    info.+culture_name := "Björske";
    info.+main_text := "A cabeça empalhada de um ciclope, montada na parede do grande salão Björske, foi tomada como troféu pelo chefe Sigmund após derrotar a besta na Batalha do Grande Desfiladeiro. O ciclope havia aterrorizado as terras circundantes, e a vitória de Sigmund sobre a criatura cimentou sua reputação como um guerreiro lendário. A cabeça, com seu único olho brilhante, tornou-se um símbolo de força, coragem e da capacidade dos Björske de conquistar até os inimigos mais poderosos. Foi exibida como um lembrete do domínio e destemor da tribo diante do perigo.";
    info.+skill_text := "A cabeça de ciclope, tomada como troféu pelo Chefe Sigmund, simboliza a força e coragem Björske, um lembrete de seu domínio sobre os inimigos mais temíveis.";
    info.+archaeology_text := "A cabeça de ciclope simboliza a destemor e força Björske, conquistada por seus lendários guerreiros.";
    info.+required_skill := "Domestic_Work";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetLanternaFurtaFogo()
    var info := struct;
    info.+name := "Lanterna Furta Fogo";
    info.+image := 0xc626;
    info.+flavor_image := 18117;
    info.+culture_name := "Kaijing";
    info.+main_text := "A lanterna furta-fogo, forjada de ferro escuro e gravada com símbolos antigos, era um artefato raro usado pelos monges Kaijing durante suas missões noturnas. A lanterna podia capturar o fogo dos acampamentos inimigos sem deixar rastros, deixando-os indefesos na escuridão. Durante uma operação secreta, o Mestre Ren usou a lanterna para extinguir as tochas de um posto avançado inimigo inteiro, permitindo que seus guerreiros se infiltrassem sem serem detectados. A lanterna tornou-se um símbolo da astúcia e furtividade Kaijing, e do poder de usar os elementos contra seus inimigos.";
    info.+skill_text := "A lanterna furta-fogo, usada pelo Mestre Ren, simboliza a furtividade e astúcia Kaijing, capturando o fogo dos acampamentos inimigos e deixando-os indefesos no escuro.";
    info.+archaeology_text := "A lanterna representa a furtividade Kaijing, capturando fogo e usando os elementos contra os inimigos.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 75.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetRifleCacaAnao()
    var info := struct;
    info.+name := "Rifle de Caça Badûran";
    info.+image := 0xc601;
    info.+flavor_image := 18118;
    info.+culture_name := "Badûran";
    info.+main_text := "O rifle de caça Badûran, forjado de aço negro e gravado com runas intrincadas, era o orgulho dos caçadores Badûran. Fabricado com engenharia de precisão, o rifle era usado para caçar bestas perigosas nas profundezas das florestas montanhosas. Um desses rifles foi empunhado pelo mestre caçador Thorgrim, que o usou para derrubar um monstruoso urso das cavernas que ameaçava a vila anã de Khron Kaz'ad. O rifle tornou-se um símbolo da inovação anã e do domínio sobre o mundo natural, passado por gerações de caçadores como uma relíquia preciosa.";
    info.+skill_text := "O rifle de caça Badûran, gravado com runas, simboliza a precisão e o domínio Badûran sobre o mundo natural, usado por Thorgrim para derrubar um urso monstruoso que ameaçava sua vila.";
    info.+archaeology_text := "O rifle representa a inovação anã, fabricado para caçadas de precisão e domínio sobre as bestas selvagens.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetHandcannonAnao()
    var info := struct;
    info.+name := "Handcannon Badûran";
    info.+image := 0xc5ff;
    info.+flavor_image := 18119;
    info.+culture_name := "Badûran";
    info.+main_text := "O handcannon Badûran, uma arma de fogo poderosa projetada para combate de curta distância, foi criado pelo mestre armeiro Thrandir durante a defesa de Khron Kaz'ad. O handcannon, com seu cano largo e estrutura robusta, disparava projéteis explosivos capazes de estilhaçar escudos e armaduras inimigas. Thrandir o usou para defender o desfiladeiro da montanha contra uma horda de orcs, destruindo suas fileiras com força devastadora. Após a batalha, o handcannon tornou-se um símbolo da engenharia e força militar anã, exibido no Salão dos Heróis como um lembrete de seu poder de fogo incomparável.";
    info.+skill_text := "O handcannon Badûran, forjado por Thrandir, simboliza a engenharia e força militar Badûran, usado para defender Khron Kaz'ad de invasores orcs com poder de fogo explosivo.";
    info.+archaeology_text := "O handcannon representa o poder de fogo e a proeza de engenharia anã, capaz de estilhaçar as defesas inimigas.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 85.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetLancaCavalariaJusta()
    var info := struct;
    info.+name := "Lança de Cavalaria de Justa";
    info.+image := 0x26c0;
    info.+flavor_image := 18120;
    info.+culture_name := "Aigla";
    info.+main_text := "A lança de justa, usada nos grandiosos torneios do império Aiglano, era um símbolo de honra e cavalaria. Feita de madeira polida e com ponta de aço, a lança era empunhada por cavaleiros competindo por glória na arena imperial. Uma dessas lanças, usada pelo Sir Valerius em uma justa famosa, derrubou o campeão do reino rival, garantindo a vitória para Aigla. Após o torneio, a lança foi exibida na armaria real, representando a habilidade e o valor dos cavaleiros do império tanto na guerra quanto no esporte.";
    info.+skill_text := "A lança de justa, usada por Sir Valerius em um torneio vitorioso, simboliza a cavalaria e o valor Aiglano, representando a honra e habilidade dos cavaleiros na arena imperial.";
    info.+archaeology_text := "A lança representa a cavalaria Aiglana, simbolizando a habilidade e honra dos cavaleiros tanto na guerra quanto no esporte.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 75.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetSaisDuplos()
    var info := struct;
    info.+name := "Sais Duplos";
    info.+image := 0x27fa;
    info.+flavor_image := 18121;
    info.+culture_name := "Kaijing";
    info.+main_text := "Os sais duplos, usados pela monge-guerreira Kaijing Mei Ling, eram armas de precisão e defesa. Com seu design bifurcado, os sais permitiam que ela desarmasse oponentes e bloqueasse ataques com facilidade. Durante a Defesa do Santuário do Lótus, Mei Ling usou os sais para derrotar um grupo de invasores, movendo-se rápida e silenciosamente pelas sombras. Os sais tornaram-se um símbolo de equilíbrio, habilidade e a capacidade de transformar defesa em ataque, e foram exibidos no templo para inspirar futuras gerações de guerreiros.";
    info.+skill_text := "Os sais duplos, empunhados por Mei Ling durante a Defesa do Santuário do Lótus, simbolizam equilíbrio, habilidade e a arte de transformar defesa em ataque nas tradições marciais Kaijing.";
    info.+archaeology_text := "Os sais representam equilíbrio e precisão, usados para desarmar oponentes e defender com habilidade.";
    info.+required_skill := "Metal_Working";
    info.+required_skill_value := 70.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMaoRessecadaDemonio()
    var info := struct;
    info.+name := "Mão Ressecada de um Demônio";
    info.+image := 0x2db8;
    info.+flavor_image := 18122;
    info.+culture_name := "Har'oloth";
    info.+main_text := "A mão ressecada de um demônio, cortada durante um ritual sombrio, foi preservada pelas sacerdotisas Drow como um poderoso talismã. A mão, enegrecida e retorcida com energia maligna, era usada para canalizar magia negra e invocar criaturas infernais. A Alta Sacerdotisa Vileara empunhou a mão durante a Cerimônia da Lua de Sangue, vinculando a essência do demônio à sua vontade. A mão tornou-se um símbolo de poder sombrio, representando o domínio dos Drows sobre as sombras e sua habilidade de manipular as forças do mal para seus próprios fins.";
    info.+skill_text := "A mão demoníaca ressecada, empunhada pela Alta Sacerdotisa Vileara, simboliza o domínio Drow sobre a magia negra e sua habilidade de invocar poderes infernais durante rituais sinistros.";
    info.+archaeology_text := "A mão representa o poder sombrio, um talismã para invocar e canalizar energias malignas.";
    info.+required_skill := "Magery";
    info.+required_skill_value := 80.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetMiniaturaBjorskeKnight()
    var info := struct;
    info.+name := "Miniatura de um Cavaleiro de Björske";
    info.+image := 0x7182;
    info.+flavor_image := 18123;
    info.+culture_name := "Björske";
    info.+main_text := "A miniatura de um cavaleiro Björske, esculpida em madeira fina e pintada intrincadamente, era uma lembrança preciosa do Rei Brago. Retratava um valente senhor dos cavalos, com o elmo adornado de penas, em pé orgulhosamente com a espada desembainhada. Esta miniatura foi presenteada a Brago por seu leal amigo Bór após uma batalha vitoriosa contra as forças de uma tribo rival. Tornou-se um símbolo de esperança e bravura para o povo Björske, lembrando-os de sua nobre linhagem e da força de sua cavalaria diante da escuridão.";
    info.+skill_text := "A miniatura do cavaleiro Björske simboliza esperança e bravura, uma lembrança preciosa da linhagem nobre e força na batalha contra as forças do mal.";
    info.+archaeology_text := "A miniatura representa o valor Björske, lembrando o povo de seu legado heroico em batalha.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 65.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

function GetEstatuaPedraAnaoMachado()
    var info := struct;
    info.+name := "Estátua de Pedra de um Badûran com Machado";
    info.+image := 0x7264;
    info.+flavor_image := 18124;
    info.+culture_name := "Badûran";
    info.+main_text := "A estátua de pedra de um Badûran empunhando um poderoso machado guardava a entrada da cidade anã de Khron Kaz'ad. Esculpida em granito sólido, a estátua retratava Thrain, um guerreiro lendário conhecido por seu valor em batalhas contra trolls e dragões. Servia como um lembrete da força e resistência duradouras dos anões. Os viajantes frequentemente deixavam oferendas na base da estátua, buscando a bênção de Thrain para proteção em suas jornadas. A estátua tornou-se um símbolo reverenciado de coragem, camaradagem e o espírito indomável do povo Badûran.";
    info.+skill_text := "A estátua de pedra de Thrain simboliza a força e resistência duradouras dos Badûran, uma figura guardiã na entrada de Khron Kaz'ad, inspirando coragem entre os viajantes.";
    info.+archaeology_text := "A estátua representa o valor Badûran, um guardião de Khron Kaz'ad e símbolo de resistência.";
    info.+required_skill := "Wood_Working";
    info.+required_skill_value := 75.0;
    info.+required_habilidade := "Arqueologo";
    return info;
endfunction

*/
// Adicione funções semelhantes para os outros itens de lore