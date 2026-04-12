// ============================================================================
// merchantRaceInit.inc
// Configuração aleatória de raça, gênero e nome para mercantes.
// Compartilhado entre pcmerchant/init.src e merchantfaccao/include/init.src
// ============================================================================

function MerchantSetupRace(npc)
	var result;
	case (RandomInt(10))
		0: // Humano Macho
			npc.gender := 0;
			npc.graphic := 400;
			result := NPC_SetupName(npc, "Human_Male");
		1: // Humana Fêmea
			npc.gender := 1;
			npc.graphic := 401;
			result := NPC_SetupName(npc, "Human_Female");
		2: // Anão Macho
			npc.gender := 2;
			npc.graphic := 1828;
			result := NPC_SetupName(npc, "Human_Male");
		3: // Anã Fêmea
			npc.gender := 3;
			npc.graphic := 1829;
			result := NPC_SetupName(npc, "Human_Female");
		4: // Elfo Macho
			npc.gender := 4;
			npc.graphic := 1830;
			result := NPC_SetupName(npc, "Human_Male");
		5: // Elfa Fêmea
			npc.gender := 5;
			npc.graphic := 1831;
			result := NPC_SetupName(npc, "Human_Female");
		6: // Orc Macho
			npc.gender := 6;
			npc.graphic := 1832;
			result := NPC_SetupName(npc, "Orc");
		7: // Orc Fêmea
			npc.gender := 7;
			npc.graphic := 1833;
			result := NPC_SetupName(npc, "Orc");
		8: // Wolfkin Macho
			npc.gender := 8;
			npc.graphic := 1834;
			result := NPC_SetupName(npc, "Human_Male");
		9: // Wolfkin Fêmea
			npc.gender := 9;
			npc.graphic := 1835;
			result := NPC_SetupName(npc, "Human_Female");
	endcase
	return result;
endfunction
