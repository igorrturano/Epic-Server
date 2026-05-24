// MockMulti integration for levers.
// Depends on mockmultiUtils (included via lever.inc before this file).

function setMockMulti(who, item)
   var opts := item.GetLeverOpts();

   if (opts.mockmulti)
      var choice := RadioGump(who, 260, 0, "Gerenciar MockMulti", array{
         "Reconfigurar Z e Velocidade",
         "Criar/Recriar Corda de Subir",
         "Criar/Recriar Corda de Volta",
         "Deletar Cordas",
         "Remover Link"
      });
      if (!choice) return; endif

      if (choice == "Reconfigurar Z e Velocidade")
         var mm := opts.mockmulti;
         var input_list := array{};
         input_list.append(struct{ "title" := "Z de SUBIDA (lever ON):", "marked" := ""+mm.z_up });
         input_list.append(struct{ "title" := "Z de DESCIDA (lever OFF):", "marked" := ""+mm.z_down });
         input_list.append(struct{ "title" := "Velocidade (0=Lento 1=Normal 2=Rapido):", "marked" := ""+mm.vel });
         var output := QuestionsGump(who, input_list);
         if (!output) return; endif
         opts.mockmulti.z_up   := CInt(output[1]);
         opts.mockmulti.z_down := CInt(output[2]);
         opts.mockmulti.vel    := CInt(output[3]);
         item.setLeverOpts(opts);
         SendSysMessage(who, "Configuracao atualizada.");

      elseif (choice == "Criar/Recriar Corda de Subir")
         _criarCordaLever(who, item, opts, 0);

      elseif (choice == "Criar/Recriar Corda de Volta")
         _criarCordaLever(who, item, opts, 1);

      elseif (choice == "Deletar Cordas")
         _deletarCordasLever(who, item, opts);

      elseif (choice == "Remover Link")
         _deletarCordasLever(who, item, opts);
         opts := item.GetLeverOpts();
         opts.-"mockmulti";
         item.setLeverOpts(opts);
         SendSysMessage(who, "Link com MockMulti removido.");
      endif
      return;
   endif

   SendSysMessage(who, "Clique em um tile do MockMulti a controlar.");
   var trg := Target(who);
   if (!trg) return; endif

   var groupId := GetObjProperty(trg, "mockmultiGroup");
   if (!groupId || groupId.errortext)
      SendSysMessage(who, "Este item nao pertence a um MockMulti.");
      return;
   endif

   var gdata := GetGlobalProperty("mmGroup_" + groupId);
   if (!gdata || gdata.errortext)
      SendSysMessage(who, "Grupo #" + groupId + " nao encontrado no registro global.");
      return;
   endif

   var input_list := array{};
   input_list.append(struct{ "title" := "Z de SUBIDA (lever ON):", "marked" := "0" });
   input_list.append(struct{ "title" := "Z de DESCIDA (lever OFF):", "marked" := ""+gdata.az });
   input_list.append(struct{ "title" := "Velocidade (0=Lento 1=Normal 2=Rapido):", "marked" := "1" });
   var output := QuestionsGump(who, input_list);
   if (!output) return; endif

   opts.mockmulti := struct{
      groupId     := groupId,
      z_up        := CInt(output[1]),
      z_down      := CInt(output[2]),
      vel         := CInt(output[3]),
      corda_subir := 0,
      corda_volta := 0
   };
   item.setLeverOpts(opts);
   SendSysMessage(who, "MockMulti '" + gdata.nome + "' linkado. ON->Z=" + output[1] + " | OFF->Z=" + output[2] + ".");

   var rope_choice := CheckBoxGump(who, 200, 150, "Adicionar Cordas?", array{"Corda de Subir", "Corda de Volta"});
   foreach r in rope_choice
      sleepms(2);
      opts := item.GetLeverOpts();
      if (r == "Corda de Subir")
         _criarCordaLever(who, item, opts, 0);
      elseif (r == "Corda de Volta")
         _criarCordaLever(who, item, opts, 1);
      endif
   endforeach
endfunction

function _criarCordaLever(who, item, byRef opts, volta)
   var existing := volta ? opts.mockmulti.corda_volta : opts.mockmulti.corda_subir;
   if (existing)
      var old := SystemFindObjectBySerial(existing);
      if (old) DestroyItem(old); endif
   endif

   var resultado := _CriarCordaNoMundo(who, volta);
   if (!resultado) return; endif

   if (volta)
      opts.mockmulti.corda_volta := resultado.corda.serial;
   else
      opts.mockmulti.corda_subir := resultado.corda.serial;
   endif
   item.setLeverOpts(opts);
endfunction

function _deletarCordasLever(who, item, byRef opts)
   var n := 0;
   if (opts.mockmulti.corda_subir)
      var cSubir := SystemFindObjectBySerial(opts.mockmulti.corda_subir);
      if (cSubir) DestroyItem(cSubir); endif
      opts.mockmulti.corda_subir := 0;
      n += 1;
   endif
   if (opts.mockmulti.corda_volta)
      var cVolta := SystemFindObjectBySerial(opts.mockmulti.corda_volta);
      if (cVolta) DestroyItem(cVolta); endif
      opts.mockmulti.corda_volta := 0;
      n += 1;
   endif
   item.setLeverOpts(opts);
   SendSysMessage(who, n + " corda(s) deletada(s).");
endfunction
