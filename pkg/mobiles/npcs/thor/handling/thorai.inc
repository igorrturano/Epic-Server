use uo;
use datafile;
use util;
use uo;
use os;
use polsys;
use cfgfile;
use util;


include ":thor:openai";

function AICompletion(context, questions, temperatura:=0.7, top_p:=1.0, skipSafetyCheck:=1)
    print("=== Starting AI Completion ===");
    
    // Validate questions parameter
    if (!questions || TypeOf(questions) != "Array" || !questions.size())
        print("Error: questions parameter is empty or not an array");
        return "Error: Invalid questions format. Please provide a valid array of questions.";
    endif
    
    // Check if each question has required fields
    var hasValidQuestion := 0;
    foreach question in questions
        if (TypeOf(question) == "Struct" && (question.user || question.assistant))
            hasValidQuestion := 1;
            break;
        endif
    endforeach
    
    if (!hasValidQuestion)
        print("Error: No valid questions found in the array");
        return "Error: Each question must contain 'assistant' or 'user' field.";
    endif
    
    // Try Claude first
    var responseAI;
    var success := 0;

    // Make sure the context is passed as the system message

    responseAI := ClaudeCompletion(context, questions, 0.7, 0.9, "claude-3-haiku-20240307", 1);
    if (responseAI && responseAI != "Failed to get a response after multiple attempts. Please try again later.")
      success := 1;
      print("Claude Response: "+responseAI);
   else
      print("Claude API error, falling back to OpenAI");
   endif
    
    // Fallback to OpenAI if Claude fails
    if (!success)
        print("Falling back to OpenAI");
        responseAI := OpenAICompletion_Thor(context, questions, temperatura, top_p, skipSafetyCheck);
    endif
    
    return responseAI;
endfunction

function handle_banker(playerinput)
endfunction

function handle_combat_ai(CategoriaMob, name_mob, playerinput, playerRaca)
   var temperatura := 1.0; // Reduced from 1.5 for better reliability
   // print("Categoria Mob: "+CategoriaMob);
   var jsonData := PackJSON(struct{
      "npcs" := struct{
         "EvilHuman" := struct{
               "name" := "" + CategoriaMob + " Medieval RPG",
               "text_size" := "Máximo de 20 caracteres",
               "context" := "você está atacando o personagem",
               "tone" := "aggressive",
               "enemy" := playerRaca,
               "exemple_response" := "Entrou no covil errado, " + playerRaca + " otário!"
         },
         "EvilHuman_backup" := struct{
               "name" := "" + CategoriaMob + " Medieval RPG",
               "text_size" := "Máximo de 20 caracteres",
               "context" := "você está atacando o personagem",
               "tone" := "aggressive",
               "exemple_response" := "Entrou no covil errado, otário!"
         },
         "EvilHuman_against_humano" := struct{
               "name" := "" + CategoriaMob + " Medieval RPG",
               "text_size" := "Máximo de 20 caracteres",
               "context" := "você está atacando o personagem",
               "tone" := "aggressive",
               "exemple_response" := "Entrou no covil errado, otário!"
         },
         "Goblin" := struct{
               "name" := "Você é um " + CategoriaMob + " de RPG",
               "context" := "um " + playerRaca + " vem te atacar, e você atacará de volta",
               "exemple_response" := "sai... odeia " + playerRaca + " ... sai daqui",
               "text" := "Máximo de 20 caracteres."
         },
         "Giants" := struct{
               "name" := "" + CategoriaMob + " RPG",
               "text_size" := "Máximo de 20 caracteres",
               "context" := "você está atacando o personagem",
               "tone" := "aggressive",
               "enemy" := playerRaca,
               "exemple_response" := "Esmaagaa " + playerRaca + "..."
         },
         "Kobold" := struct{
               "name" := "" + CategoriaMob + " RPG",
               "text_size" := "Máximo de 20 caracteres",
               "context" := "você está atacando o personagem",
               "exemple_response" := "kobold detesta drow... sai daqui!!"
         }
      }
   });
   var unpackedJSON := UnpackJSON(jsonData);
   var context := ""; 

   if (CategoriaMob == "EvilHuman")
      if (playerRaca == "humano")
         unpackedJSON := UnpackJSON(jsonData);
         context := unpackedJSON["npcs"]["EvilHuman_against_humano"];
      else
         unpackedJSON := UnpackJSON(jsonData);
         context := unpackedJSON["npcs"]["EvilHuman"];
      endif
   elseif (CategoriaMob == "Kobold")
      unpackedJSON := UnpackJSON(jsonData);
      context := unpackedJSON["npcs"]["Kobold"];
   elseif (CategoriaMob == "Giants")
      unpackedJSON := UnpackJSON(jsonData);
      context := unpackedJSON["npcs"]["Giants"];
   elseif (CategoriaMob == "Goblin")
      unpackedJSON := UnpackJSON(jsonData);
      context := unpackedJSON["npcs"]["Goblin"];
   endif

   context := Cstr(context);

   var questions := array{ struct{ "user" := playerinput } };
   var responseAI := AICompletion(context, questions, temperatura);
   responseAI := StrReplace(responseAI, "\"", " "); // Substitui aspas por espaço
   print("responseAI: "+responseAI);
   return responseAI;
endfunction

function handle_doubleclick(npctype, playerinput)
   if (npctype == "taverneiro")
   
      var sysMsg := "
         # Você
         Você é um taverneiro de RPG como Diablo.

         # Contexto 
         O mundo desse RPG é amedrontador, o assentamento onde você se encontra é um ambiente tenebroso, cheio de incertezas e medo. 
         Tudo foi feito às pressas e tudo está de forma improvisada.
         O personagem que vem até você provavelmente vem para comprar algo da sua taverna improvisada.

         # Povos
         ## Drows - trate eles com certo receio.
         ## Orcs - trate eles com esperança de ajuda, mas também preconceito pela aparência bestial.
         ## Bjorske - povos do norte - Pergunte ou mencione o Norte.
         ## Polski são inspirados nos Hobbits. Trate-os com desprezo pelo tamanho ou força.
         ## Aiglanos é o povo que voce pertence. Estamos todos em apuros.
         ## Anões - Tente vender uma bebida empoeirada para seus trabalhos
         ## Povos do Leste - Pergunte sarcasticamente se veio a procura de algum dragão

         # Sua resposta
         Sua resposta deve conter, no máximo, 60 caracteres. Preencha, depois da fala, a representação da sua ação com asteriscos, em terceira pessoa. 
         Lembre de não falar de forma rebuscada, pelo contrário: troque virgulas por duas ou três reticências, use maneirismos linguísticos de erros de sintaxe, evite 'oi', 'olá', procure algo como 'ei', 'hmm', 'e então'; não pergunte 'o que', pergunte 'que', 'qual', ou 'é o que?'. 
         Represente na fala a aflição do seu personagem. Use um tom como de uma pessoa que não dorme há 2 dias. Pode incluir na ação traços da roupa ou estado do ambiente - improvisados e em mal estado.

         # Exemplo
         Guerreiro... que vai querer? *debruça os braços no balcão meio quebrado, um olho está enfaixado*
      ";

      var questions := array{ struct{ "user" := playerinput } };
      var result := AICompletion(sysMsg, questions);
      return result;

   elseif (npctype == "vendedor")

      var sysMsg := "
         # Você
         Você é um vendedor de RPG como Diablo.

         # Contexto 
         O mundo desse RPG é amedrontador, o assentamento onde você se encontra é um ambiente tenebroso, cheio de incertezas e medo. 
         Tudo foi feito às pressas e tudo está de forma improvisada.
         O personagem que vem até você provavelmente vem para comprar algo do seu comércio improvisado.

         # Povos
         ## Drows - trate eles com certo receio.
         ## Orcs - trate eles com esperança de ajuda, mas também preconceito pela aparência bestial.
         ## Bjorske - povos do norte - Pergunte ou mencione o Norte.
         ## Polski são inspirados nos Hobbits. Trate-os com desprezo pelo tamanho ou força.
         ## Aiglanos é o povo que voce pertence. Estamos todos em apuros.
         ## Anões - Tente vender uma bebida empoeirada para seus trabalhos
         ## Povos do Leste - Pergunte sarcasticamente se veio a procura de algum dragã

         # Sua resposta
         Sua resposta deve conter, no máximo, 60 caracteres. Preencha, depois da fala, a representação da sua ação com asteriscos, em terceira pessoa. 
         Lembre de não falar de forma rebuscada, pelo contrário: troque virgulas por reticências, use maneirismos linguísticos de erros de sintaxe, evite 'oi', 'olá', procure algo como 'ei', 'hmm', 'e então'; não pergunte 'o que', pergunte 'que', 'qual', ou 'é o que?'. 
         Pode incluir na ação traços do seu estado ou estado do ambiente.

         # Exemplo
         Guerreiro... que vai querer? *arruma umas armaduras na mesa aos trapos*
      ";

      var questions := array{ struct{ "user" := playerinput } };
      var result := AICompletion(sysMsg, questions);
      return result;

   else
      var jsonData := PackJSON(struct{
         "npcs" := array{ // Use array para múltiplos NPCs
            struct{
                  "text_size" := "Máximo de 30 caracteres", // Corrigido para :=
                  "environment" := "assentamento improvisado", // Corrigido para :=
                  "tone" := "tense", // Corrigido para :=
                  "instruction" := "Greet the character, use '...' instead of commas", // Corrigido para :=
                  "words" := "use 'ei' ao inves de 'olá'" // Corrigido para :=
            }
         }
      });

      var unpackedJSON := UnpackJSON(jsonData);
      var context := unpackedJSON["npcs"][1];
      context := Cstr(context);

      var questions := array{ struct{ "user" := playerinput } };
      var result := AICompletion(context, questions);
      return result;

   endif
endfunction

function handle_npc_talker_openai(frase, temperatura:=1.0, top_p:=1)
   var contexto := "Você é um fofoqueiro de RPG, o personagem que fala contigo traz algo. Faça a frase de maneira totalmente diferente, mudando palavras e ordens das palavras, seja criativo";
   var frase_struct := array{ struct{ "user" := frase } };
   var result := AICompletion(contexto, frase_struct, temperatura, top_p);
   
   return result;
endfunction

function handle_npc_listen_openai(contexto, frase, temperatura:=0.7)
   var frase_struct := array{ struct{ "user" := frase } };
   var result := AICompletion(contexto, frase_struct, temperatura);
   
   return result;
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

   var questions := array{ struct{ "user" := playerinput } };
   var responseAI := AICompletion(context, questions);
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
   // Chama o AICompletion com as variaveis de contexto (systemMessage) e o input do char (questions)
   
   var responseText := AICompletion(systemMessage, questions);
   return responseText;
endfunction

function handle_combat_speech(npc_name, player_input, playerRaca, npcState := "healthy")
   if (player_input == error || !player_input || player_input == "")
      player_input := "* looks at you aggressively *"; // Default value if empty
   endif

   var base_prompt := "
         Você é um inimigo hostil no mundo de Eora, um mundo de fantasia medieval diverso de um RPG.

         Como NPC antagonista, seu objetivo é desafiar o jogador com uma frase intimidadora antes do combate.

         REGRAS DE COMPORTAMENTO:
         - Use entre 20-50 caracteres - seja breve e ameaçador
         - Expresse fúria extrema através de linguagem violenta e ameaças diretas
         - Trate o jogador como um intruso desprezível em seu território
         - Varie o tom entre ameaças físicas, promessas de tortura ou referências à sua superior habilidade em combate
         - Evite clichês genéricos; seja específico e memorável

         Exemplos de frases:
         - Seus ossos serão triturados e seus gritos ecoarão por estas paredes!
         - O cheiro do seu medo é delicioso. Vamos ver se seu sangue tem o mesmo gosto!
         - Sua cabeça será um excelente troféu para minha coleção!

      "+npcState;

    var cfg := ReadConfigFile(":epicAI:config/npc_speech");
    if (!cfg)
        print("Could not open npc_speech.cfg");
        return error{"errortext" := "Could not open npc_speech.cfg"};
    endif

    // Procura o elemento correto comparando o Name no config
    var elem;
    foreach entry in (GetConfigStringKeys(cfg))
        var temp_elem := cfg[entry];
        if (temp_elem.Name == npc_name)
            print("Found NPC type ["+npc_name+"] in npc_speech.cfg");
            elem := temp_elem;
            break;
        endif
    endforeach

    if (!elem)
        print("NPC type ["+npc_name+"] not found in npc_speech.cfg");
        return error{"errortext" := "NPC type ["+npc_name+"] not found in npc_speech.cfg"};
    endif

    // Build context
    var context := base_prompt + "\n";
    context += "# "+GetConfigString(elem, "Description")+"\n";
    context += "# Tom: "+GetConfigString(elem, "Tone")+"\n\n";
    
    // Add examples based on state
    context += "# Exemplos para seu estado atual:\n";
    var responses := GetConfigStringArray(elem, npcState);
    foreach response in responses
        context += response + "\n";
    endforeach

    // Use Claude instead of OpenAI
    var questions := array{ struct{ "user" := Cstr(player_input) } };
    
    // Try to use Claude first, with fallback to OpenAI if it fails
    var responseAI;
    var success := 0;
    // Debug output
   print("Questions array: " + questions);
   print("First question: " + questions[1]);
   print("User field: " + questions[1].user);
    
   // Skip safety check for combat speech
   // Make sure the context is passed as the system message
   var formattedQuestions := array{};
   foreach question in questions
       formattedQuestions.append(question);
   endforeach
   
   responseAI := ClaudeCompletion(context, questions, 0.7, 0.9, "claude-3-haiku-20240307", 1);
   if (responseAI && responseAI != "Failed to get a response after multiple attempts. Please try again later.")
      success := 1;
      print("Claude Response: "+responseAI);
   else
      print("Claude API error, falling back to OpenAI");
   endif

    // Fallback to OpenAI if Claude fails
    if (!success)
        print("Falling back to OpenAI for combat speech");
        responseAI := OpenAICompletion_Thor(context, questions, 1.0, 0.7, 1);
        print("OpenAI Response: "+responseAI);
    endif
    
    // If both APIs fail, return a basic response
    if (!responseAI || responseAI == "Failed to get a response after multiple attempts. Please try again later.")
        return handle_error_combat_AI_speech();
    endif
    
    return StrReplace(responseAI, "\"", " ");
endfunction

function getNPCState(npc)
    var hp_percent := AP_GetVital(npc, HITS) / AP_GetVitalMaximumValue(npc, HITS) * 100;

    if (hp_percent <= 25)
        return "dying";
    elseif (hp_percent <= 50)
        return "hurt";
    elseif (hp_percent <= 75)
        return "injured";
    else
        return "healthy";
    endif
endfunction

function handle_error_combat_AI_speech()
    var responses := array{
        "*Grunhe*",
        "*Rosna*",
        "*Ignora*",
        "*Olha fixamente*",
        "*Range*",
        "*Murmura*"
    };
    return responses[RandomInt(responses.size())+1];
endfunction