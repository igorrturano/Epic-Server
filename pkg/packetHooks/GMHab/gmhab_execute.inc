
function ExecuteCommand(who, cmd_index)
	// Array with all commands (must match ShowComandosPage)
	var comandos := array{};
	
	// Comandos Gerais
	comandos.append(".ficha");
	comandos.append(".point");
	comandos.append(".pularcerca");
	comandos.append(".r");
	comandos.append(".revistar");
	comandos.append(".senha");
	comandos.append(".showdmg");
	
	// Magias
	comandos.append(".c");
	comandos.append(".cast");
	comandos.append(".d");
	comandos.append(".m");
	comandos.append(".p");
	comandos.append(".t");
	comandos.append(".arcoelemental");
	comandos.append(".arremessar");
	comandos.append(".assumirforma");
	comandos.append(".casting");
	comandos.append(".pilotar");
	comandos.append(".rituais");
	comandos.append(".voltar");
	
	// Combate
	comandos.append(".golpe");
	comandos.append(".showequip");
	
	// Skills
	comandos.append(".camuflar");
	comandos.append(".acalmar");
	comandos.append(".adestrar");
	
	// Roleplay
	comandos.append(".actions");
	comandos.append(".arrastar");
	comandos.append(".erguer");
	comandos.append(".ficharp");
	comandos.append(".lang");
	comandos.append(".sounds");
	
	// Pets
	comandos.append(".pe");
	comandos.append(".pet");
	
	// Outros
	comandos.append(".conquistas");
	comandos.append(".fome");
	comandos.append(".meditar");
	comandos.append(".pontosrp");
	comandos.append(".postura");
	comandos.append(".removerdisfarce");
	comandos.append(".rps");
	comandos.append(".soltar");
	comandos.append(".soltargemas");
	comandos.append(".translation");
	
	if (cmd_index < 1 || cmd_index > comandos.size())
		SendSysMessageEx(who, "Indice de comando invalido!", SSM_FAIL);
		return;
	endif
	
	var cmd := comandos[cmd_index];
	
	// Send the command as if the player typed it
	SendSysMessageEx(who, "Executando: " + cmd, SSM_INFO);
	SendTextCommand(who, cmd);
endfunction
