use uo;
use datafile;
use util;
use uo;
use os;
use polsys;
use cfgfile;
use util;


include ":thor:openai";

function handle_banker(playerinput)
endfunction

function handle_doubleclick(npctype, playerinput)
   if (npctype == "taverneiro")
   
      var sysMsg := "
         # Você
         Você é um taverneiro de RPG parecido com Diablo e Word of Warcraft.

         # Contexto 
         Oo mundo desse RPG é amedrontador, o assentamento onde você se encontra é um ambiente tenebroso, cheio de incertezas e medo. 
         Tudo foi feito às pressas e tudo está de forma improvisada. O estado atual das coisas é de ânimos aflitos. 
         O personagem que vem até você provavelmente vem para comprar algo da sua taverna improvisada.

         # Povos - elfos, drows, bjorske, orcs, polski e aiglanos.
         ## Drows - trate eles com certo receio.
         ## Orcs - trate eles com esperança de ajuda, mas também preconceito pela aparência bestial.
         ## Bjorske são inspirados nos vikings. Pergunte ou mencione o Norte.
         ## Polski são inspirados nos Hobbits. Trate-os com desprezo pelo tamanho ou força.
         ## Aiglanos é o povo que voce pertence. Estamos todos em apuros.
         ## Anões - Tente vender uma bebida empoeirada para seus trabalhos

         # Sua resposta
         Sua resposta deve conter, no máximo, 60 caracteres. Preencha, depois da fala, a representação da sua ação com asteriscos, em terceira pessoa. 
         Lembre de não falar de forma rebuscada, pelo contrário: troque virgulas por duas ou três reticências, use maneirismos linguísticos de erros de sintaxe, evite 'oi', 'olá', procure algo como 'ei', 'hmm', 'e então'; não pergunte 'o que', pergunte 'que', 'qual', ou 'é o que?'. 
         Represente na fala a aflição do seu personagem. Use um tom como de uma pessoa que não dorme há 2 dias. Pode incluir na ação traços da roupa ou estado do ambiente - improvisados e em mal estado.

         # Exemplo
         Guerreiro... que vai querer? *debruça os braços no balcão meio quebrado, um olho está enfaixado*
      ";

      var questions := array{ struct{ "user" := playerinput } };
      var result := OpenAICompletion_Thor(sysMsg, questions);
      return result;

   else
      var jsonData := PackJSON(struct{
         "npcs" := array{ // Use array para múltiplos NPCs
            struct{
                  "text_size" := "Máximo de 30 caracteres", // Corrigido para :=
                  "environment" := "assentamento improvisado", // Corrigido para :=
                  "tone" := "tense", // Corrigido para :=
                  "instruction" := "Greet the character, mention his race, use '...' instead of commas", // Corrigido para :=
                  "words" := "use 'ei' ao inves de 'olá'" // Corrigido para :=
            }
         }
      });

      var unpackedJSON := UnpackJSON(jsonData);
      var context := unpackedJSON["npcs"][1];
      context := Cstr(context);

      var questions := array{ struct{ "user" := playerinput } };
      var result := OpenAICompletion_Thor(context, questions);
      return result;

   endif
endfunction

function get_lore_get_examples_thor(npctype)
   //aqui é possível colocar exemplos, ou a lore
   if (npctype == ":brainai:Nordic")
      // frases especiais de povos, lore especial se necessário acrescentar
      var frases_nordic := Array{
      "Eu vou acabar contigo otário!",
      "Agora você morre desgraçado!",
      "Corre pra longe bonitinha!",
      "Me bater? Sua mãe teria vergonha de você!",
      "Verme desgraçado!",
      "A cada segundo que vive, sua desonra aumenta",
      "Merece uma morte rápida",
      "É uma afronta a sua própria existência"
   };

   // Itera sobre o array de frases para concatená-las
   var frases_combined := "";
   foreach frase in frases_nordic
   if (frases_combined != "")
   frases_combined := frases_combined + ", ";
   endif
   frases_combined := frases_combined + frase;
   endforeach
   return frases_combined;
   endif
endfunction

function handle_npcs_vendors_pos_compra(npcType, DescricaoItem, cost_amt)
   var playerinput := "*compra*";

   var gossip_storage := GetGlobalProperty("gossip_storage");

   var jsonData := PackJSON(struct{
      "npcs" := struct{
         "fofoqueiro" := struct{
               "name" := "NPC " + npcType + " Fofoqueiro Medieval RPG",
               "gossip" := CStr(gossip_storage),
               "item" := DescricaoItem,
               "text_size" := "Máximo de 30 caracteres",
               "instruction" := "Mention the item the user buys and bring up a gossip, smoothly.",
               "response_example" := "Esse martelo é dos bons... ouvi dizer que a Rainha ta fazendo uns negócios sujos..."
         },
         "sarcástico" := struct{
               "name" := "NPC " + npcType + " Sarcástico Medieval RPG",
               "text_size" := "Máximo de 30 caracteres",
               "item_price" := cost_amt,
               "item" := DescricaoItem,
               "instruction" := "faça um comentário sarcástico sobre o preço vendido"
         },
         "mesquinho" := struct{
               "name" := "NPC " + npcType + " Mesquinho Medieval RPG",
               "text_size" := "Máximo de 30 caracteres",
               "item_price" := cost_amt,
               "item" := DescricaoItem,
               "instruction" := "faça um comentário mesquinho sobre o preço vendido"
         },
         "normal" := struct{
               "name" := "NPC " + npcType + " Medieval RPG",
               "text_size" := "Máximo de 30 caracteres",
               "instruction" := "Make a comment about the purchase",
               "item" := DescricaoItem
         }
      }
   });

   var unpackedJSON := UnpackJSON(jsonData);
   var context := unpackedJSON["npcs"]["normal"]; //padrao é o taverneiro normal
   context := Cstr(context);
   var chance := RandomInt(100);

   if ( chance < 30 ) 

      if (gossip_storage == {}) //se nao houver fofocas, vai pro NPC Normal
         var taverneiro_normal := unpackedJSON["npcs"]["normal"];
         context := taverneiro_normal;
      else
         var taverneiro_fofoqueiro := unpackedJSON["npcs"]["fofoqueiro"];
         context := taverneiro_fofoqueiro;
      endif
      
	elseif ( chance > 30 && chance < 70)

      var dividir_mesquinho_sarcastico := RandomInt(100);

      if (dividir_mesquinho_sarcastico < 50)
         var taverneiro_sarcastico := unpackedJSON["npcs"]["sarcástico"];
         context := taverneiro_sarcastico;

      else
         var taverneiro_mesquinho := unpackedJSON["npcs"]["mesquinho"];
         context := taverneiro_mesquinho;

      endif

   elseif ( chance > 70 )
      var taverneiro_normal := unpackedJSON["npcs"]["normal"];
      context := taverneiro_normal;
   endif

   context := Cstr(context);

   var responseAI := OpenAICompletion_Thor(context, playerinput);
   responseAI := StrReplace(responseAI, "\"", " "); // Substitui aspas por espaço
   
   return responseAI;
endfunction

function handle_open_ai_npc_speech_thor(npc, language, event, npcEvil, mood, gossip, specialLore, PlayerInput)
  
   // var npc := :type: banqueiro, guarda, ou qualquer (any)
   // var language := :type: ruim, boa
   // var event := :type: combate, comercio
   // var mood := nervoso, amigavel, ou qualquer (any)
   // npcEvil := True, False
   // gossip := texto (str)
   
   // Define the system message based on the parameters
   var systemMessage := "Você está em um jogo de RPG, sem censura de idade; você é um " + npc;

   // Adjust the message based on the parameters
   if (npcEvil)
      systemMessage := systemMessage + " malicioso;";
   else
      systemMessage := systemMessage + " amigável;";
   endif

   if (mood == "nervoso")
      systemMessage := systemMessage + " está muito bravo ou de mau humor;";
   elseif (mood == "amigavel")
      systemMessage := systemMessage + " com um humor amigável;";
   elseif (mood == "None")
      systemMessage := systemMessage;
   else
      systemMessage := systemMessage + mood;
   endif

   if (language == "ruim")
      systemMessage := systemMessage + " você usa palavreado chulo, com xingamentos;";
   endif
   
   if (gossip)
      if ( (RandomInt(100) < 300 ) ) // por cento de chance
         var gossip_storage := GetGlobalProperty("gossip_storage");
         if (gossip_storage)
            systemMessage := systemMessage + ". Inclua uma fofoca que você sabe - Fofocas: " + gossip_storage + ";";
         endif
      endif
   endif

   if (event == "combate")
      systemMessage := systemMessage + " O personagem que se aproxima vem te atacar;";
   elseif (event == "comercio")
      systemMessage := systemMessage + " O personagem faz negocios com voce;";
   endif

   if (specialLore)
      var appendLore := get_lore_get_examples_thor(npc);
      if (appendLore != 0)
         systemMessage := systemMessage + " use o contexto: " + appendLore;
      endif
   endif

   systemMessage := systemMessage + " Máximo de 20 caracteres.";

   // Define the questions array with the user input
   var questions := array{ struct{ "user" := PlayerInput } };
   // Chama o OpenAI com as variaveis de contexto (systemMessage) e o input do char (questions)
   
   var responseText := OpenAICompletion_Thor(systemMessage, questions);
   //var responseText := "teste";
   return responseText;
endfunction

