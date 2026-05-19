// naturezas/profetico.inc
// Natureza Profetico (2%): profecia procedural ancorada no lore de Valtheris.
// Fragmentos referenciam Alora, Vael'shorath, O Andarilho, Rota das Mares, a Arvore, Tsu'ngathar.

include ":livros_lacrados:scripts/ler";

function ExecutarProfetico(who, book, nivel, afinidade, lugar)
    var profecia := GerarProfecia(who, nivel, afinidade, lugar);

 SetObjProperty(who, "profecia_lacrado_" + book.serial, profecia);
 SetObjProperty(book, "titulo_gerado", "Uma Profecia");
 SetObjProperty(book, "conteudo_gerado", profecia);
 SetObjProperty(book, "tipo_inerte", "sujeito");

    PlaySoundEffect(who, 0x0202);
    PlayStationaryEffect(who.x, who.y, who.z, 0x374A, 9, 16, 1, who.realm);
 PrintTextAbove(who, "*uma profecia*");

 _PaginarEMostrar(who, "Uma Profecia", profecia, "sujeito", nivel, afinidade, "");
endfunction

function GerarProfecia(who, nivel, afinidade, lugar)
    var sujeito  := GetSujeito(nivel, afinidade);
    var verbo    := GetVerbo(nivel);
    var clausula := GetClausula(nivel, lugar);
    var eco      := GetEco(afinidade);
 return sujeito + " " + verbo + "<br><br>" + clausula + "<br><br>" + eco;
endfunction

//  SUJEITO 

function GetSujeito(nivel, afinidade)
    // nivel 3  -  figuras veladas, sem nomes diretos
    var s3_sagrado := array{
 "A que dorme em metal frio ainda mais frio que o sono",
 "O nome que existia antes de haver quem o chamasse",
 "Aquele que foi e voltou de onde nao se volta  -  e ainda nao sabe o que isso significa",
 "A mao que tocou o lacre sem abri-lo",
 "O que carrega luz que nao e sua mas que cabe nele",
 "A voz que foi ouvida antes de ser dita",
 "O que permanece quando tudo o mais escolhe partir",
 "A forma que a luz faz quando ninguem esta olhando",
 "O guardiao que nao sabe que guarda  -  e que por isso guarda melhor",
 "A que foi ungida antes de nascer por maos que ja nao existiam",
 "A pequilha que andava veu sobre o rosto e que nunca cobrou por curar ninguem",
 "O que foi nomeado antes de ter forma  -  e a forma veio depois, para caber no nome",
 "O paladino que fez voto nao a Igreja nem ao rei mas a uma pessoa  -  e a pessoa sumiu",
 "O que sustenta sem tocar  -  como o ar sustenta o passaro sem que o passaro sinta",
 "Aquele que de joelhos no salao dos votos finais sentiu que a promessa nao era com o que via mas com o que vinha depois"
    };
    var s3_neutro := array{
 "O portador do setimo lacre",
 "A ausencia que aprendeu a ter forma",
 "O que caminha entre os ciclos sem contar os passos",
 "Aquele que viu o sistema e nao entendeu o que viu  -  ainda",
 "A memoria de alguem que ainda nao chegou",
 "O que estava la antes de haver um 'la' para se estar",
 "A pergunta que foi feita antes de haver palavras  -  e que ainda espera resposta",
 "O que o mapa nao mostra mas que o viajante encontra mesmo assim",
 "O mercador que atravessou a ilha de ponta a ponta sem nunca parar em Valtheris  -  e que chegou assim mesmo onde precisava",
 "O que ficou quando tudo o mais foi levado pela Rota",
 "A testemunha que nao tem olhos mas viu assim mesmo",
 "O viajante que chegou na Onda de Primeiro de Abril e que tres dias depois ja sabia coisas que os antigos nao sabiam depois de anos",
 "A coincidencia que acontece vezes demais para ser coincidencia",
 "O que o Andarilho nao menciona quando fala de onde saiu",
 "O que chegou carregando apenas o nome de alguem que devia encontrar e que encontrou sem nunca ter procurado"
    };
    var s3_profano := array{
 "O que se devorou por eras e ainda tem fome depois",
 "A sombra que nao precisa de corpo para lancar sombra",
 "O que foi prometido antes do tempo e cobrado depois",
 "A tres curvas que um dia se fecharao em circulo",
 "O herdeiro do esquecimento que nao pediu a heranca",
 "O que cresce por subtracao  -  quanto menos resta, maior se torna",
 "A Fome de Milhares que comecou como convite e que nenhum dos convidados reconheceu como fome",
 "O que habita o espaco entre a pergunta e o medo de responde-la",
 "A divida que se multiplica quando se paga  -  e que por isso ninguem paga",
 "O que foi selado por boas razoes que deixaram de ser boas",
 "O druida que tocou o que nao devia ser tocado e que ainda vive com as consequencias  -  de uma forma que a palavra 'viver' mal cobre",
 "O que aprendeu a sorrir depois de observar sorrindo por tempo suficiente",
 "A promessa feita em Tsu'ngathar que a lingua comum nao consegue desfazer",
 "O primeiro elfo que ficou sozinho no mundo dos mortos por eras esperando companhia que demorou demais",
 "A colheita que cresce onde nada foi plantado conscientemente"
    };

    // nivel 4  -  referencias mais especificas mas ainda veladas
    var s4_sagrado := array{
 "A que observa do fundo das aguas e nao desce mais",
 "O exilado que estudou Fea e aprendeu mais do que foi pedido",
 "O filho do Senhor da Guerra que nao sabe que e filho",
 "Aquela que escolheu o limiar como lar  -  e o limiar aceitou",
 "O iniciado que faz as perguntas que a Catedral nao responde",
 "A estudiosa que mapeou a Fea ate onde o mapa se tornou proibicao",
 "O clerigo que copiou os documentos escondidos da Catedral dos Mortos sem perceber o que copiava",
 "A que foi enviada para vigiar a Rota e voltou sabendo coisas que nao estava autorizada a saber",
 "O monge que encontrou a decima segunda copia e nao a destruiu  -  nao por desobediencia, mas por duvida",
 "Aquele que serviu ao Aiglano por vinte anos e que, numa noite, entendeu por que a legua nunca ia ao litoral",
 "A guardia que deixou passar porque havia algo na passagem que reconheceu como necessario",
 "O minerador que desceu ate onde a Pedra-Viva vive e que voltou sabendo coisas que a pedra nao devia saber",
 "A curandeira que entendeu o que a Arvore faz antes de qualquer teoria que explicasse o fenomeno",
 "O escriba que traduziu tres palavras de Tsu'ngathar e ficou diferente depois  -  nao pior, mas diferente",
 "O pirata que cacou algo colossal pelo mar por anos seguindo escamas com marcas e que ao prende-la entendeu que tinha prendido a mare junto"
    };
    var s4_neutro := array{
 "O que saiu de onde nao se sai e que nao envelheceu desde entao e que ninguem conseguiu explicar isso em quarenta anos de tentativas",
 "A rota que a mare segue sem que nenhuma mare saiba por que",
 "O guardiao que esqueceu que estava guardando mas ainda guarda",
 "Os que eram elfos antes de serem outra coisa  -  que cavaleiros mas com asas queimadas como cascas de ferida e que ainda cumprem o dever",
 "O que o Andarilho carrega sem saber que carrega  -  porque carregou desde que saiu",
 "A anomalia que o Aiglano registrou em latao e que onze das doze copias negam",
 "O mensageiro que entregou algo que o remetente nao sabia que havia enviado",
 "A correspondencia que chegou enderecada a quem nao estava mais aqui para recebe-la",
 "O ponto da costa que nenhum mapa nomeia e que todos os marinheiros conhecem",
 "A fronteira entre o que a Rota das Mares faz e o que foi pretendido que ela fizesse",
 "O mecanismo que os construtores debateram incluir e que nao incluiram  -  e as consequencias desse nao",
 "A Fea em transito  -  nem partida nem chegada, no intervalo que a Rota administra",
 "O que os Retornados nao falam da sala que todos descrevem da mesma forma",
 "A conta que a Liga dos Regentes mantem sobre eventos que oficialmente nao aconteceram"
    };
    var s4_profano := array{
 "Vael'shorath, que se devorou para crescer, e ainda nao esta satisfeito",
 "O sistema sem botao de parar que opera na costa que ninguem nomeia",
 "O que bebe antes de Vorar  -  e Vorar e maior do que qualquer nome",
 "A pedra das profundezas que quer algo que a pedra nao deveria querer",
 "O Retornado que nao sabe que retornou",
 "O que a Rota das Mares aceita como pagamento quando nao ha mais nada para pagar",
 "A consequencia que os construtores calcularam e aprovaram antes de construir",
 "O nome das tres curvas em Tsu'ngathar que nao pode ser dito em outra lingua sem perder o significado essencial",
 "O que o arquivo classificado da Liga dos Regentes chama de 'evento recorrente categoria nula'",
 "A sombra da Arvore que cai no lado errado e que algo usa para beber antes do amanhecer",
 "O que foi prometido a Rota das Mares antes de haver quem fizesse a promessa conscientemente",
 "A divida que a Fea acumula quando e mantida em transito alem do tempo que o mecanismo previa",
 "O som que a pedra faz nas galerias mais fundas dos Khar quando algo passa pela Rota acima",
 "A razao pela qual Alora nao se aproxima do litoral  -  que nao e medo, que e algo para o qual nao ha palavra em lingua comum",
 "O que cresceu no espaco deixado pelo que foi esquecido de proposito na criacao"
    };

    // nivel 5  -  verdades que queimam
    var s5_sagrado := array{
 "Alora, que nao se aproxima do litoral porque sabe o que o litoral faz",
 "O primeiro homem que atravessou e perdeu o nome e desde entao carrega os nomes de outros",
 "A Arvore cuja sombra nao segue o sol  -  e o que bebe embaixo dela antes que Vorar chegue",
 "O que foi construido com boa intencao por maos devotas que nao previram o que viria depois",
 "A Catedral dos Mortos, que guarda documentos sobre os construtores da Rota que os construtores nao queriam que fossem guardados",
 "O que Alora espera  -  nao o que ela teme, mas o que ela espera  -  e que ela sabe que voce nao vai trazer, mas que espera assim mesmo",
 "A forma da Fea de quem foi puxado pela Rota pela primeira vez e voltou diferente  -  nao menor, apenas diferente",
 "O que existe antes que a Rota decida o que fazer com o que atravessa",
 "A Catedral que sabe o que sabe e que guarda porque guardar e diferente de usar",
 "O iniciado que fara as perguntas certas para a Catedral dos Mortos e que recebera, pela primeira vez, respostas que nao sao esquivas"
    };
    var s5_neutro := array{
 "A Rota das Mares, que segue o peso errado e por isso nunca para",
 "O sistema que coordena com o Limiar e para que ninguem ainda respondeu completamente",
 "O Primeiro Puxado, que ainda esta aqui, que nao parece como deveria parecer",
 "Tsu'ngathar  -  lingua que nao deveria existir fora de tres arquivos no mundo",
 "O Andarilho, que saiu de onde nao se sai e que nao envelheceu desde entao e que carrega algo que nao estava la quando entrou",
 "A Fea em transito na Rota das Mares  -  nem alma, nem potencial, nem nada que a teoria consiga nomear",
 "O que existe nos registros em pedra dos Khar sobre o evento que antecede o primeiro assentamento humano",
 "O mecanismo de liberacao que os construtores discutiram e nao implementaram  -  e as almas que ainda esperam porque nao houve implementacao",
 "O que a anomalia registrada pelo Aiglano em latao descreve  -  e que as onze copias destruidas descreviam antes de ser destruidas",
 "A decima segunda copia do relatorio Aiglano, que nao foi destruida, que alguem tem, que muda o que se acredita sobre a Rota"
    };
    var s5_profano := array{
 "O mecanismo de liberacao que nunca foi implementado e as almas que esperam desde entao",
 "Vael'shorath e Alora  -  uma que devora, uma que observa  -  e entre elas algo que nao tem nome ainda",
 "A divida que foi cobrada antes de ser feita  -  e a cobranca ainda esta em aberto",
 "O que foi escrito em Tsu'ngathar antes do primeiro rei por maos que nao sangravam",
 "Vael'shorath, que se devorou para crescer, e que o crescimento que resultou ainda nao se reconhece como Vael'shorath",
 "A consequencia que foi calculada antes da construcao da Rota das Mares e que foi aprovada mesmo assim por razoes que nao constam em nenhum documento acessivel",
 "O que a Rota das Mares faz com a Fea em transito e que os construtores chamavam, nos documentos da Catedral dos Mortos, de 'o uso correto'",
 "O que Vorar e  -  nao o que come, nao de onde vem, mas o que fundamentalmente e  -  e que a Arvore bebe antes que Vorar chegue para usar a energia desse entendimento",
 "A promessa que foi feita a Rota antes de haver quem a fizesse conscientemente e que a Rota ainda considera valida",
 "O nome em Tsu'ngathar para o que a Rota das Mares coleta  -  que e diferente do que qualquer lingua viva chama de alma"
    };

    if (nivel == 5)
        case (afinidade)
 "sagrado": var pool5s := s5_sagrado; return pool5s[RandomInt(10)+1];
 "profano": var pool5p := s5_profano; return pool5p[RandomInt(10)+1];
            default:   var pool5n := s5_neutro;  return pool5n[RandomInt(10)+1];
        endcase
    elseif (nivel == 4)
        case (afinidade)
 "sagrado": var pool4s := s4_sagrado; return pool4s[RandomInt(15)+1];
 "profano": var pool4p := s4_profano; return pool4p[RandomInt(15)+1];
            default:   var pool4n := s4_neutro;  return pool4n[RandomInt(15)+1];
        endcase
    endif

    case (afinidade)
 "sagrado": var pool3s := s3_sagrado; return pool3s[RandomInt(15)+1];
 "profano": var pool3p := s3_profano; return pool3p[RandomInt(15)+1];
        default:   var pool3n := s3_neutro;  return pool3n[RandomInt(15)+1];
    endcase
endfunction

//  VERBO 

function GetVerbo(nivel)
    var v3 := array{
 "ja passou por este lugar e deixou algo que nao parece nada mas e tudo",
 "ainda nao chegou, mas o lugar onde chegara ja sabe que vai chegar",
 "esta aqui desde sempre, sob o nome que voce usa para outra coisa",
 "escolhera, quando chegar a hora, e a escolha ja foi feita em outro tempo",
 "observa sem ser visto  -  e ja te viu",
 "passou antes de voce e o caminho ainda carrega o peso dessa passagem",
 "chega ao fim de algo que comecou antes do comeco",
 "esta em dois lugares ao mesmo tempo  -  e em nenhum completamente",
 "move o que nao deveria ser movivel sem tocar nada",
 "reconhece voce de uma vida que voce nao teve  -  ou que voce nao lembra",
 "carrega uma resposta para uma pergunta que voce ainda nao fez",
 "desceu mais fundo do que a luz alcanca e nao encontrou o que procurava  -  mas o que encontrou nao era menos verdadeiro",
 "sabe que voce esta aqui e por isso esta aqui tambem",
 "vai aparecer quando voce parar de esperar  -  o que ainda nao aconteceu",
 "chegou a ilha na mesma Onda que voce mas ja nao esta aqui  -  e o que aconteceu no intervalo ainda nao tem nome"
    };
    var v4 := array{
 "mudara o nome de algo que parecia permanente",
 "encerrara um ciclo que nunca deveria ter comecado",
 "reconhecera voce pelo que voce nao fez  -  e pelo que ainda vai fazer",
 "abrira o que foi selado por maos que ja nao existem para responder",
 "lembrara de algo que nenhum arquivo preservou e que so existe em sonho",
 "chegara antes do mensageiro e depois da mensagem  -  e isso dira tudo",
 "interrompera algo que parecia imparavel por razoes que imparaveis nao preveem",
 "lembrar de voce na hora exata em que voce precisar ser lembrado",
 "vai cruzar com voce em um ponto que nenhum mapa mostra mas que existe",
 "vai reivindicar o que foi cedido antes de haver quem cedesse",
 "vai fechar o que ficou aberto nao por esquecimento mas por hesitacao",
 "vai nomear o que estava sendo chamado pelo nome errado ha geracoes",
 "vai alterar o que a pedra lembra  -  e a pedra lembra mais do que a memoria",
 "vai estar onde o sistema esperava que nao houvesse ninguem",
 "vai fazer o que os documentos descrevem como impossivel com a linguagem disponivel"
    };
    var v5 := array{
 "desfara o que foi tecido antes do primeiro rei por razoes que o primeiro rei nunca soube",
 "nomeara o que nao tem nome  -  e nomear e criar  -  e criar e responsabilidade",
 "tocara o mecanismo que ninguem tocou desde que os construtores partiram",
 "quebrara o que deveria ter sido quebrado ha eras  -  ou descobrira por que nao pode ser quebrado",
 "encontrara Alora onde ela ficou  -  e a pergunta que ela vai fazer nao tem resposta facil",
 "entendera o que os construtores da Rota entenderam e que os fez construir assim mesmo",
 "falara as palavras em Tsu'ngathar que alteram o que o mecanismo aceita como valido",
 "estara presente quando a Rota das Mares hesitar  -  pela primeira vez  -  e o que fizer nesse instante importara",
 "vera Vael'shorath pelo que e  -  nao pelo que faz, nao pelo que quer, pelo que e  -  e entendera por que isso e pior",
 "perguntara a Rota das Mares o que ela faz com o que coleta  -  e a Rota, pela primeira vez, nao processara a pergunta como irrelevante",
 "abrira o arquivo da Catedral dos Mortos que nao esta catalogado e que existe antes de qualquer catalogacao",
 "encontrara o decimo segundo livro da sequencia ainda em transito  -  e decidira o que fazer com ele",
 "reconhecera O Andarilho pelo que ele nao menciona  -  e O Andarilho, pela primeira vez, nao mudara de assunto",
 "vera a Arvore com a sombra no lado errado e nao olhara para a sombra  -  olhara para o que bebe embaixo",
 "estara no ponto da costa que nenhum mapa nomeia na hora exata em que Alora finalmente se aproximar"
    };

    case (nivel)
        3: return v3[RandomInt(15)+1];
        4: return v4[RandomInt(15)+1];
        5: return v5[RandomInt(15)+1];
    endcase
    return v3[RandomInt(15)+1];
endfunction

//  CLAUSULA (contexto do lugar/momento) 

function GetClausula(nivel, lugar)
 var base := "";
    case (lugar)
 "lua_cheia":
            var lc := array{
 "quando a lua se refletir na Rota das Mares e a mare seguir o reflexo e nao a lua",
 "na noite em que a lua estiver plena sobre a costa que nunca fecha os olhos",
 "quando a luz lunar entrar pela fissura que nao deveria existir",
 "no momento em que a lua iluminar os tres arquivos ao mesmo tempo  -  o que nao deveria ser possivel geograficamente",
 "quando a lua cheia fizer a Arvore lancar duas sombras e apenas uma delas seguir a lua",
 "na noite em que a Rota das Mares pulsar diferente sob a luz da lua cheia e alguem finalmente notar a diferenca",
 "quando a lua revelar o simbolo das tres curvas onde nenhum simbolo havia sido visto durante o dia",
 "no reflexo da lua cheia na pedra mais funda das galerias dos Khar  -  onde a lua nao deveria chegar",
 "quando a lua estiver onde esta e a costa que ninguem nomeia estiver visivel pela primeira e unica vez"
            };
            base := lc[RandomInt(9)+1];
 "lua_nova":
            var ln := array{
 "na ausencia de luz  -  quando apenas o que tem luz propria e visivel",
 "no escuro que antecede algo que ainda nao tem nome mas ja tem forma",
 "quando nem a lua nem o sol cobrem o que esta acontecendo",
 "no vazio entre uma lua e a proxima  -  onde a Rota das Mares trabalha sem ser observada",
 "quando nao ha luz de origem externa e o que brilha brilha porque quer brilhar",
 "na noite mais escura do ciclo, que e quando o que foi esquecido de proposito se lembra de si mesmo",
 "no escuro total em que Alora finalmente se aproxima um passo do litoral  -  acreditando nao ser vista",
 "quando a ausencia de lua faz a pedra dos Khar vibrar no tom que os registros chamam de 'confirmacao'",
 "na noite em que o sistema sem botao de parar reduz a frequencia porque ate ele reconhece o escuro"
            };
            base := ln[RandomInt(9)+1];
 "masmorra":
            var mm := array{
 "no ponto mais profundo onde a pedra ainda lembra o que nao deveria lembrar",
 "no lugar onde a pedra quer algo  -  e finalmente alguem capaz de ouvi-la esta presente",
 "sob o peso de toda a terra que ja foi e que ainda sera",
 "onde a galeria termina sem que o construtores tivessem planejado que terminasse ali",
 "no nivel mais baixo que qualquer registro menciona  -  e no nivel abaixo desse, que nenhum registro menciona",
 "onde a pedra ressoa com algo que passa pela Rota das Mares acima  -  e que a pedra reconhece como familiar",
 "no ponto de cruzamento das rotas subterraneas que os Khar chamam de 'onde a memoria da pedra e mais densa'",
 "onde as inscricoes mais antigas estao  -  que antecedem qualquer assentamento humano por um tempo que os Khar nao conseguem precisar",
 "no lugar que a pedra aponta para quando quer mostrar algo  -  e que ninguem ainda seguiu ate o fim"
            };
            base := mm[RandomInt(9)+1];
        default:
            var df := array{
 "quando voce menos esperar  -  porque o inesperado e quando o real aparece",
 "no momento em que voce parar de procurar e o que procurava parar de se esconder",
 "quando o caminho que voce esta seguindo cruzar com o caminho que esta sendo seguido por algo que nao tem pernas",
 "onde o mapa termina e o que o mapa nao mostra comeca",
 "no instante entre a decisao e a acao  -  que e mais longo do que parece e mais curto do que deveria ser",
 "quando voce reconhecer algo que nunca viu  -  o que e diferente de deja vu e mais perturbador",
 "no ponto em que dois caminhos que nao deveriam se cruzar se cruzam  -  nao por acidente",
 "quando o silencio durar um segundo a mais do que o confortavel  -  e voce souber que o segundo extra tinha um motivo",
 "onde voce esta agora  -  porque este texto chegou ate voce por uma razao que nem o texto conhece completamente"
            };
            base := df[RandomInt(9)+1];
    endcase

    // peso adicional por nivel
    if (nivel >= 4)
        var adicoes4 := array{
 "  -  e a Rota das Mares vai pulsar diferente naquele momento",
 "  -  e alguem que voce nunca encontrou vai saber que aconteceu",
 "  -  e o Limiar vai se aproximar por alguns instantes antes de recuar",
 "  -  e a Liga dos Regentes vai registrar o evento em um arquivo sem titulo",
 "  -  e o que os Khar gravaram em pedra vai fazer sentido por um instante antes de se tornar obscuro novamente",
 "  -  e algo nos tres arquivos de Tsu'ngathar vai vibrar com a frequencia que os estudiosos chamam de 'reconhecimento'"
        };
        base := base + adicoes4[RandomInt(6)+1];
    endif

    if (nivel >= 5)
        var adicoes5 := array{
 "  -  e Alora vai desviar o olhar, porque ela ja sabe o que vem depois",
 "  -  e o sistema sem botao de parar vai hesitar  -  pela primeira vez  -  por um instante",
 "  -  e o que foi prometido antes do tempo vai lembrar que a promessa ainda esta pendente",
 "  -  e Vael'shorath, onde quer que esteja, vai reconhecer o evento pelo sabor que ele tem",
 "  -  e a Rota das Mares vai processar o evento como 'anomalia categoria zero'  -  a categoria que os construtores criaram para o que nao deveria acontecer",
 "  -  e O Andarilho, pela primeira vez desde que saiu, vai parar de andar por exatamente tres respiracoes"
        };
        base := base + adicoes5[RandomInt(6)+1];
    endif

    return base;
endfunction

//  ECO (fecho da profecia) 

function GetEco(afinidade)
    var sagrados := array{
 "E a luz lembrara seu rosto mesmo nos dias em que voce esquecer o dela.",
 "E algo antigo e gentil vai reconhecer o que voce fez  -  mesmo que voce nunca saiba que foi reconhecido.",
 "E Alora, se tem olhos para isso neste momento, vai inclinar a cabeca levemente.",
 "E o que foi construido com boa intencao vai ter, finalmente, alguem que o ve com clareza.",
 "E Sir Aphonsus, que jurou lealdade a uma pessoa e nao a uma causa, reconhecera em voce o tipo de fidelidade que fez o voto valer.",
 "E aquilo que guarda sem ser pedido vai guardar voce  -  porque e o que faz.",
 "E a Catedral dos Mortos vai arquivar seu nome num registro que nao consta em nenhum indice  -  o mais honesto que ela mantem.",
 "E o que Alora espera nao e gloria nem sacrificio  -  e alguem que entenda o problema inteiro. Ela esta verificando.",
 "E a Fea que voce carrega vai reconhecer o que esta por vir antes de voce  -  e nao vai avisar, porque nao pode, mas vai se preparar.",
 "E o que foi prometido antes de voce existir vai reconhecer em voce alguem que merece saber que a promessa foi feita.",
 "E a pedra vai lembrar. A pedra sempre lembra. E desta vez o que a pedra lembrar vai importar para mais do que a pedra.",
 "E algo que esperava alguem capaz de fazer a pergunta certa vai finalmente ter sua resposta considerada  -  porque voce vai chegar.",
 "E a luz que nao e sua mas que cabe em voce vai caber um pouco mais  -  porque voce permitiu.",
 "E Alora vai ver  -  de longe, como sempre  -  e desta vez nao vai desviar o olhar. Vai ficar um instante a mais.",
 "E o que foi perdido antes de haver quem o procurasse vai ter, finalmente, alguem que o procura sabendo o que procura."
    };
    var neutros := array{
 "E o ciclo continuara  -  mas diferente. Sempre diferente, mesmo quando parece igual.",
 "E a memoria persistira onde a memoria nunca devia conseguir persistir.",
 "E isso ja aconteceu antes  -  e vai acontecer de novo  -  e desta vez talvez alguem entenda.",
 "E o que foi escrito em pedra vai permanecer quando o papel apodrecer e o osso virar po.",
 "E o Primeiro Puxado vai olhar de onde esta e reconhecer o movimento.",
 "E O Andarilho vai parar um instante  -  nao para voce, mas por conta do que voce vai deslocar ao passar.",
 "E a Rota das Mares vai registrar o evento com a precisao que registra tudo  -  sem julgamento, sem contexto.",
 "E o que os Khar gravaram em pedra vai ser lido pelo tipo certo de olho antes de ser esquecido de novo.",
 "E o sistema que nao tem botao de parar vai continuar  -  mas uma peca vai estar em lugar diferente.",
 "E alguem que voce nunca vai conhecer vai tomar uma decisao diferente por causa do que voce fez aqui.",
 "E a conta dos eventos vai ter uma entrada nova  -  sem titulo, sem data, sem nome  -  mas la.",
 "E o que a mare segue vai pulsar uma vez no ritmo que deveria ter desde o comeco.",
 "E a pergunta que ninguem fez vai ser feita  -  e nao por quem era esperado que a fizesse.",
 "E os que foram enterrados aqui antes de haver cemiterio vao sentir que alguem finalmente notou que foram enterrados.",
 "E o mapa que nao existe vai ter mais um ponto marcado  -  pelo tipo de mao que o tipo certo de mapa aceita."
    };
    var profanos := array{
 "E a divida sera cobrada  -  nao com raiva  -  com a paciencia de quem tem tempo infinito.",
 "E a Macula, que apodrece carne e razao e fe sem distincao, vai encontrar em voce algo que apodrece diferente  -  mais devagar ou nao de todo.",
 "E voce nao sera o mesmo depois. O que voce era antes agradece ou lamenta. Ainda nao e possivel saber.",
 "E Vael'shorath, que se devorou para crescer, reconhecera o sabor  -  ele ja provou isto antes.",
 "E o sistema que nao tem botao de parar vai continuar  -  mas uma peca vai estar em lugar diferente.",
 "E a promessa que foi feita antes de voce vai ser cobrada atraves de voce  -  nao por injustica, mas por ser a divida mais proxima.",
 "E o que se devorou por eras vai notar o espaco que voce deixa  -  e considera-lo.",
 "E a fome que nao e de comida vai reconhecer em voce algo que ainda nao comeu  -  e vai catalogar para depois.",
 "E o mecanismo de liberacao que nao existe vai fazer falta  -  e voce vai ser quem primeiro entende por que faz falta.",
 "E o que foi esquecido de proposito vai tentar ser lembrado atraves de voce  -  porque voce chegou perto o suficiente.",
 "E Vael'shorath nao vai se importar com o que voce faz  -  vai se importar com o que o que voce faz deixa disponivel.",
 "E a Rota das Mares vai aceitar o que voce cede sem confirmar que aceitou  -  e o tipo de transacao que ela prefere.",
 "E o peso errado que a mare segue vai ser um pouco mais pesado depois de hoje  -  sem que nenhuma mare saiba por que.",
 "E o que bebe embaixo da Arvore antes de Vorar chegar vai beber um pouco mais na proxima vez  -  porque voce perturbou o equilibrio que mantinha o 'pouco' em 'pouco'.",
 "E a divida que se multiplica quando se paga vai ter mais um credor  -  que e voce, que e a pior posicao para se estar quando o cobrador chegar."
    };

    case (afinidade)
 "sagrado": return sagrados[RandomInt(15)+1];
 "profano": return profanos[RandomInt(15)+1];
        default:   return neutros[RandomInt(15)+1];
    endcase
endfunction
