include ":gumps:epicGumps";

function ShowMasteryGump(byRef gump, who)
	var prop := who.GetProp("Masteries");

	if (!prop || !prop.size())
		GFTextMid(gump, 25, 120, 380, COLOR_RED, "Nenhuma maestria disponível!");
		return;
	endif

	var tabela := array{};
	foreach mastery in (prop)
		tabela.append(struct{
			maestria := mastery.name,
			level := mastery.level,
			xp := mastery.xp,
			hits := mastery.hits,
			damage := mastery.damage
		});
	endforeach
	var tabelaCfg := dictionary{
		"maestria" -> struct{titulo := "Maestria", ordem := 1, largura := 5},
		"level" -> struct{titulo := "Nível", ordem := 2, largura := 2},
		"xp" -> struct{titulo := "XP", ordem := 3, largura := 2},
		"hits" -> struct{titulo := "Hits", ordem := 4, largura := 2},
		"damage" -> struct{titulo := "Dano", ordem := 5, largura := 2}
	};

	CriaTabelaGump(25,120,380,tabela,gump,0,dictionary{},tabelaCfg);
endfunction