// naturezas/manchado.inc
// Natureza Manchado (10%): fragmentos diferentes para cada leitor.
// Seed baseada em book.serial XOR who.serial  texto unico por par.

include ":livros_lacrados:scripts/ler";

function ExecutarManchado(who, book, nivel, afinidade, lugar)
    var seed := CInt(book.serial) ^ CInt(who.serial);
    var fragmento := GerarFragmento(seed, nivel);

 SetObjProperty(book, "fragmento_" + who.serial, seed % 25);

 _PaginarEMostrar(who, "Paginas Manchadas", fragmento, "manchado", nivel, afinidade, "");

 SendSysMessageEX(who, "As paginas estao cobertas de manchas. Voce consegue ler apenas partes.", SSM_INFO);
 SendSysMessageEX(who, "Outros leitores podem ver fragmentos diferentes.", SSM_INFO);
endfunction

function GerarFragmento(seed, nivel)
    // 25 fragmentos por nivel  -  cotidiano/estranho  arcano  proibido

    var frags3 := array{
 "...o sino tocou tres vezes antes que a porta se fechasse para sempre. O que entrou depois nao era o que havia saido antes.",
 "...a segunda expedicao Idrith'har nunca foi registrada oficialmente. Os doze nomes foram apagados. O decimo terceiro nome nao existia no comeco.",
 "...ele disse que a mare seguia o peso errado. Ninguem perguntou o que isso queria dizer. Ninguem queria saber.",
 "...o simbolo estava na parede antes de qualquer um de nos chegar. Isso foi ontem. Esta manha o simbolo estava em outro lugar.",
 "...a crianca nao era deles. Nunca foi. O que me perturbou nao foi isso  -  foi que a crianca sabia.",
 "...nao abra o segundo bau. O primeiro ja e suficiente para mudar o que voce acredita.",
 "...ela tinha os olhos do pai, mas as maos de outro. Ninguem comentou. Esse e o tipo de coisa que ninguem comenta.",
 "...o mapa estava errado. Propositalmente. E alguem havia corrigido o erro de volta para o estado errado.",
 "...havia marcas no chao que ninguem havia feito. Ou alguem havia feito enquanto todos dormiam, o que e pior.",
 "...o soldado voltou. Mas ficou de costas para a janela durante toda a refeicao, e ninguem perguntou por que.",
 "...tres viajantes nunca chegaram. O quarto chegou e nao sabia dos outros tres. Ou sabia e nao queria falar.",
 "...a chave nao abre nada. A fechadura abre a chave. Eu precisei de dois anos para entender o que isso significava.",
 "...o nome dele foi apagado de todos os registros da guarnicao. O rosto nas pinturas foi coberto. Mas a sombra dele ficou.",
 "...havia uma segunda porta. Nao estava no projeto original. Nao estava em nenhum projeto. E ainda assim estava la.",
 "...o animal nao bebia da fonte ha tres semanas. Os animais geralmente sabem antes de nos.",
 "...ele sorriu quando o prendemos. Disse apenas: 'voces chegaram no dia certo.' Nunca entendemos o que quis dizer.",
 "...a carta foi escrita duas semanas depois da morte da remetente. O selo era legitimo. A letra era dela.",
 "...o menino descreveu a rua com detalhes que nenhuma crianca de sua idade deveria conhecer. Incluindo o que havia embaixo dela.",
 "...alguem havia lido este livro antes de mim. As anotacoes nas margens estavam em minha propria letra. Eu nunca havia estado nesta cidade.",
 "...a testemunha disse que o homem subiu as escadas. As escadas terminavam no teto. Nao havia segundo andar.",
 "...o mercador tinha a mesma cicatriz que o ladrao que haviamos executado no inverno anterior. Perguntei. Ele disse que nao era cicatriz.",
 "...os registros de nascimento paravam em determinado ano. Os de morte continuavam. Ninguem na prefeitura soube explicar.",
 "...ela pediu que nao abrissemos a janela do quarto norte. Nao deu motivo. Tres dias depois entendemos o motivo.",
 "...o passaro trouxe uma mensagem enderecada a alguem que havia morrido ha quarenta anos. A mensagem estava fresca. A tinta ainda estava umida.",
 "...eu perguntei quem havia acendido as velas. Todos disseram que nao tinham sido eles. As velas continuaram acesas."
    };

    var frags4 := array{
 "...o ritual exige uma ausencia, nao uma presenca. Os que vieram antes nao entenderam isso e pagaram com algo que nao era deles para pagar.",
 "...existem doze livros lacrados nesta sequencia. Este e o nono. Os outros oito nao contem o que voce espera que contenham.",
 "...a ordem dos selos importa mais do que o conteudo. Abrir na sequencia errada nao destroi o livro  -  destroi outra coisa.",
 "...o local foi marcado antes que qualquer humano chegasse. A pedra ja tinha a inscricao quando os primeiros assentamentos foram fundados.",
 "...nao e a tinta que guarda. E o pergaminho. E o pergaminho lembra o que foi escrito antes, mesmo depois de apagado.",
 "...o nome que foi dado a ele nao era o nome real. O nome real foi dividido em tres partes e distribuido para tres pessoas que nunca se encontrariam.",
 "...a segunda lua do decimo mes e quando ele retorna. Nao a ele como era. A ele como se tornou depois de ser tocado pela Rota.",
 "...havia alguem observando enquanto eu escrevia. Nao atras de mim. Dentro do texto. Entre as palavras.",
 "...o fogo nao queimava o livro. O livro queimava o fogo. Jogamos agua. A agua evaporou antes de tocar a capa.",
 "...o lacrador original nunca foi encontrado porque nunca existiu como pessoa. O selo foi feito por um processo, nao por uma mao.",
 "...a Liga dos Regentes tinha um arquivo sobre isto. O arquivo foi classificado. O classificador foi reclassificado. Nao sei o que isso significa.",
 "...os Khar tinham um nome para o fenomeno. Esta gravado em algum lugar nas galerias mais profundas, em pedra que nao lascou em tres mil anos.",
 "...o Aiglano registrou a anomalia em latao. Doze copias foram feitas. Onze foram destruidas por ordem do Regente Terceiro. A decima segunda foi perdida.",
 "...ela disse que o documento estava na Catedral dos Mortos, em um arquivo que oficialmente nao existe. Ela havia lido. Havia entendido. Nao havia contado a ninguem antes de mim.",
 "...os Retornados que voltaram atraves da Rota nao voltaram completos. Falta algo. Uma camada. A questao e se essa camada era necessaria ou apenas usual.",
 "...o simbolo das tres curvas aparece em documentos que antecedem qualquer registro de Vael'shorath por nome. O simbolo veio primeiro. O nome veio depois.",
 "...a Fea de um ser pode ser mapeada. Os estudiosos que tentaram faze-lo desapareceram. Nao morreram  -  desapareceram, que e diferente.",
 "...o mapa da Rota foi desenhado por quem nunca a havia percorrido. Era preciso demais para ser adivinhado. Impreciso demais para ter sido copiado.",
 "...a investigacao foi encerrada por ordem de alguem sem nome no documento. O espaco onde o nome deveria estar nao esta em branco  -  esta preenchido por algo que meus olhos nao conseguem reter.",
 "...havia uma decima terceira pessoa na sala quando o acordo foi assinado. Nenhum dos doze signatarios a mencionou. O artista que pintou a cena incluiu uma sombra extra.",
 "...o iniciado fez as perguntas corretas. A Catedral dos Mortos nao respondeu. Nao porque nao soubesse  -  porque responder seria reconhecer que a pergunta era valida.",
 "...tres linguas foram usadas no documento original. Duas eram conhecidas. A terceira ninguem reconheceu. Foi catalogada como 'erro de copista' por oito geracoes.",
 "...o guardiao que deveria impedir o acesso estava la. Ficou de lado. Nao por corrupcao  -  por algo que parecia resignacao. Como quem deixa alguem entrar porque sabe que entrar e inevitavel.",
 "...a memoria do lugar e mais longa do que a memoria de quem vive nele. A pedra lembra o que o povo esqueceu. E a pedra, desta vez, estava tentando falar.",
 "...o exilado estudou Fea por vinte anos antes de perceber que havia aprendido a coisa errada do jeito certo, o que e mais perigoso do que aprender a coisa certa do jeito errado."
    };

    var frags5 := array{
 "...antes do primeiro selador, havia outro. Este nunca foi mencionado porque menciona-lo implicaria admitir o que foi selado antes.",
 "...o limiar nao e um lugar. E um estado de abandono que o universo reconhece como valido para certas transacoes.",
 "...doze foram selados pela Rota das Mares. Onze foram encontrados. O decimo segundo ainda esta em transito  -  desde quando, ninguem sabe ao certo.",
 "...o nome proibido em Tsu'ngathar tem sete silabas. Este livro conhece seis. A setima foi removida antes do lacre. Por precaucao ou por misericordia, nao sei dizer.",
 "...a criacao do mundo exigiu que algo fosse esquecido de proposito. O que foi esquecido nao ficou satisfeito com o esquecimento.",
 "...o selador que falhou nao morreu. Foi transformado em algo que a Rota das Mares reconhece como parte da infraestrutura.",
 "...este livro foi escrito depois de ser aberto. A ordem do tempo aqui e diferente. Nao sei se sou eu quem esta lendo ou eu quem esta escrevendo.",
 "...existe um decimo terceiro livro que nunca foi lacrado porque o lacre nao ficou. Algo dentro rejeita o fechamento.",
 "...a Rota das Mares nao foi construida para capturar almas. Essa foi uma consequencia. Os construtores sabiam da consequencia e construiram mesmo assim.",
 "...o primeiro a ler este fragmento ja foi observado. Nao por mim. Por algo que usa este texto como janela.",
 "...Tsu'ngathar e pronunciavel por humanos. A questao nao e a pronuncia  -  e que pronunciar altera algo no falante. Os tres arquivos no mundo estao trancados por esta razao.",
 "...Alora nao se aproxima do litoral. Eu a vi uma vez, a distancia. Ela me viu tambem. Ela desviou o olhar primeiro, o que nao me confortou  -  me perturbou.",
 "...Vael'shorath se devorou para crescer. O que me assombra nao e o ato  -  e que funcionou. E que o crescimento ainda nao parou.",
 "...a Arvore lanca sombra no lado errado. Algo bebe embaixo dela antes de Vorar chegar. Vorar e maior do que qualquer nome que lhe atribuiram ate hoje.",
 "...os construtores da Rota das Mares deixaram documentacao na Catedral dos Mortos. Os arquivos existem. Ninguem com autoridade para le-los acredita no que esta escrito.",
 "...O Andarilho saiu de onde nao se sai. Nao envelheceu desde entao. A questao que ninguem faz e: o que ele deixou para tras quando saiu?",
 "...Fea nao e alma. Nao e espirito. E o espaco entre ser e potencial que a Rota das Mares aprendeu a usar como moeda.",
 "...o mecanismo de liberacao nunca foi implementado. Isso foi uma escolha, nao um esquecimento. Quem escolheu nao esta mais disponivel para explicar.",
 "...os Retornados que vieram pela Rota falam de uma sala que existe entre a partida e a chegada. Todos descrevem a mesma sala. Nenhum deveria conseguir descreve-la.",
 "...a resposta estava na primeira pagina. Que desapareceu. Nao foi arrancada  -  desapareceu. A encadernacao ainda esta intacta onde a pagina esteve.",
 "...o simbolo das tres curvas e o unico elemento que aparece em Tsu'ngathar, na escrita Khar e nos documentos da Catedral dos Mortos. Ninguem estudou o que e o unico elemento em comum.",
 "...foi escrito em Tsu'ngathar antes do primeiro rei por maos que nao sangravam. Eu sei o que isso implica. Prefiro que voce chegue a conclusao sozinho.",
 "...a Rota das Mares pulsa. Ha quem afirme que o pulso corresponde a algo. Ninguem conseguiu sincronizar o suficiente para descobrir a que.",
 "...O Primeiro Puxado ainda esta aqui. Nao parece como deveria parecer. Conversei com ele sem saber. So percebi depois, e so porque ele me deixou perceber.",
 "...a divida que a Rota cobra nao e sangue nem tempo. E possibilidade. Cada alma em transito e uma linha de futuro que nao acontecera. Isso foi calculado antes da construcao. O calculo foi aprovado."
    };

    var frags := frags3;
    if (nivel == 4) frags := frags4; endif
    if (nivel == 5) frags := frags5; endif

    var idx := (seed % 25) + 1;
    return frags[idx];
endfunction
