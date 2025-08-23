include ":gumps:epicGumps";
include ":achievements:model/AchievementCombat";

const MAX_XP := 5000; // XP máximo para a barra de progresso

const MAX_XP := 5000; // XP máximo para a barra de progresso

function ShowAchievementsGump(who)
	var gump := GFECreateGump("Progresso de Conquistas", 680, 400);
	var achievements := who.GetAllAchievementsProgress();

	var tabela := array{};
	foreach achievement in (achievements)
		if (!achievement) continue; endif // Skip if no data available
		
		tabela.append(struct{
			name := achievement.getName(),
			level := achievement.getLevel(),
			xp := achievement.getXP(),
			hits := achievement.getData(ACHIEVDATA_HITS),
			damage := achievement.getData(ACHIEVDATA_DAMAGE_DONE),
			"1000" := "",
			"3000" := "",
			"5000" := ""
		});
	endforeach
	var tabelaCfg := dictionary{
		"name" -> struct{titulo := "Conquista", ordem := 1, largura := 4},
		"level" -> struct{titulo := "Nível", ordem := 2, largura := 0.8},
		"xp" -> struct{titulo := "XP", ordem := 3, largura := 1.5},
		"hits" -> struct{titulo := "Hits", ordem := 4, largura := 1.2},
		"damage" -> struct{titulo := "Dano", ordem := 5, largura := 2},
		"1000" -> struct{titulo := "1000", ordem := 6, largura := 2},
		"3000" -> struct{titulo := "3000", ordem := 7, largura := 2},
		"5000" -> struct{titulo := "5000", ordem := 8, largura := 2}
	};
	CriaTabelaGumpPaginada(30, 80, 500, tabela, gump, dictionary{}, tabelaCfg);
	PositionXPBar(gump, achievements);
	GFSendGump(who, gump);
endfunction

function PositionXPBar(byRef gump, achievements)
	var x_offset := 422;
	var y_offset := 106;

	foreach achievement in (achievements)
		if (!achievement) continue; endif
		var xpbar := GFResizePic(gump, x_offset, y_offset, 9752, CalcFilledXPBar(achievement.xp), 14);
		y_offset += 20;
	endforeach
endfunction

function CalcFilledXPBar(xp)
	xp := CInt(xp); // POL BUGAAAAAAAAAAAAAAAADO

	if (xp > MAX_XP)
		xp := MAX_XP; // Limita o XP máximo para 5000
	endif
	var width := (xp / 1000) * 80;

	// A partir de 1000 começa a completar a barra com 2000
	if (width > 80)
		width := (((xp-1000) / 2000) * 80)+80;
	endif

	return width;
endfunction