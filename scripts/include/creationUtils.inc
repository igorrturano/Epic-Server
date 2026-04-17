use basic;

include ":attributes:attributes";
include ":disguise:disguise";
include ":charactercreation:createchar";
include ":tn:tngumps";
include "include/epicConstants";
include ":gumps:epicGumps";
include ":message:include/model/Message";
include ":spells:SpellRegistry";
//include ":nature:nature";
//include ":charactercreation:include/RPTraits";

const AIGLA_ID := 0x9E7;
const ALURIEL_ID := 0x9E8;
const BADURAN_ID := 0x9E9;
const BJORSKA_ID := 0x9EA;
const GULTHRAK_ID := 0x9EB;
const HAROLOTH_ID := 0x9EC;
const COLHESIANA_ID := 0x9ED;
const LESTE_ID := 0x9EE;

const BOKK_BHARAZ_ID := 0x9EF;
const ELUITHORTH_ID := 0x9F0;
const HALLEUAD_ID := 0x9F1;
const KAI_ID := 0x9F2;
const LORE_ID := 0x9F3;
const USHKRAR_ID := 0x9F4;
const VENERATIO_ID := 0x9F5;

const CUIDADOR_ID := 0x152;
const GOVERNADOR_ID := 0x153;
const HEROI_ID := 0x154;
const BOBO_ID := 0x155;
const AMANTE_ID := 0x156;
const COMUM_ID := 0x157;
const CRIADOR_ID := 0x158;
const MAGO_ID := 0x159;
const EXPLORADOR_ID := 0x15A;
const FORADALEI_ID := 0x15B;
const INOCENTE_ID := 0x15C;
const SABIO_ID := 0x15D;

var BACKGROUNDS := struct{DOMESTIC_WORK := "Mestre do Lar", METAL_WORK := "Ferreiro aprendiz", GATHERING := "Coletor", PICK_POCKETING := "Rato de rua", SNEAK := "Aventureiro", SURVIVAL := "Caçador", MEDICINE := "Curandeiro", WOOD_WORK := "Marceneiro"};
var CULTURAL_BACKGROUNDS := struct{ "Aiglana" := "Legionário", "Har'oloth" := "Prole Abissal", "Colhesiana" := "Armadilheiro", "Alüriel" := "Guardião", "Björska" := "Cavalaria Nórdica", "Gulthrak" := "Flagelo das Areias", "Badûran" := "Filho do progresso", "Povos do Leste" := "Discípulo das Lâminas"};

function getCultureByPictureId(culture_id)
    case (culture_id)
        AIGLA_ID:
            return AIGLANA;
        ALURIEL_ID:
            return ALURIEL;
        BADURAN_ID:
            return BADURAN;
        BJORSKA_ID:
            return BJORSKA;
        GULTHRAK_ID:
            return GULTHRAK;
        HAROLOTH_ID:
            return HAROLOTH;
        COLHESIANA_ID:
            return COLHESIANA;
        LESTE_ID:
            return LESTE;
    endcase
endfunction

function getCraftCultureByPictureId(culture_id)
    case (culture_id)
        AIGLA_ID:
            return "Aiglana";
        ALURIEL_ID:
            return "Aluriel";
        BADURAN_ID:
            return "Baduran";
        BJORSKA_ID:
            return "Bjorska";
        GULTHRAK_ID:
            return "Gulthrak";
        HAROLOTH_ID:
            return "Haroloth";
        COLHESIANA_ID:
            return "Colhesiana";
        LESTE_ID:
            return "Leste";
    endcase
endfunction
var next_btn := array{"9903", "9904"};
var prev_btn := array{"9909", "9910"};
function CultureGump(who, culture_pic)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 17, 31, 0x9E6);

    GFHTMLArea(gump, 346, 56, 192, 40, "<BASEFONT Size=55 Color=#999999 >Escolha sua cultura", 0, 0); //shadow
    GFHTMLArea(gump, 345, 55, 192, 40, "<BASEFONT Size=55 Color=#ffffff >Escolha sua cultura", 0, 0);
    var x := 63;
    var y := 100;
    var i := 0;

    GFHTMLArea(gump, 100, 550, 650, 120, getCulturalDescriptionByPictureId(culture_pic), 1, 1);
    GFAddButton(gump, 543, 300, next_btn[1], next_btn[2], GF_CLOSE_BTN, 201);
    GFAddButton(gump, 273, 300, prev_btn[1], prev_btn[2], GF_CLOSE_BTN, 210);
    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 200);

    GFGumpPic(gump, 298, 100, culture_pic);

    return GFSendGump(who, gump);
endfunction

function getCulturalDescriptionByPictureId(culture_id)
    var textosConfig := ReadConfigFile(":charactercreation:/config/descricoes");
    case (culture_id)
        AIGLA_ID:
            return FindConfigElem(textosConfig, "aigla").descricao;
        ALURIEL_ID:
            return FindConfigElem(textosConfig, "aluriel").descricao;
        BADURAN_ID:
            return FindConfigElem(textosConfig, "baduran").descricao;
        BJORSKA_ID:
            return FindConfigElem(textosConfig, "bjorska").descricao;
        GULTHRAK_ID:
            return FindConfigElem(textosConfig, "gulthrak").descricao;
        HAROLOTH_ID:
            return FindConfigElem(textosConfig, "haroloth").descricao;
        COLHESIANA_ID:
            return FindConfigElem(textosConfig, "colhesiana").descricao;
        LESTE_ID:
            return FindConfigElem(textosConfig, "leste").descricao;
    endcase
endfunction

function EpicRaceGump(who, culture_pic)
    var races := getRaceByCulturalId(culture_pic);
    var gump := GFCreateGump();
    GFClosable(gump, 0);

    var x := 63;
    var y := 100;
    var i := 1;
    showPreviousGumpInformation(gump, who, culture_pic, x, y);
    var raceText := "Escolha sua raça";
    if (races.size() == 1)
        raceText := "Raça selecionada";
    endif
    GFHTMLArea(gump, x+21, y-24, 192, 40, "<BASEFONT Size=55 Color=#999999 > "+raceText, 0, 0); //shadow
    GFHTMLArea(gump, x+20, y-25, 192, 40, "<BASEFONT Size=55 Color=#ffffff > "+raceText, 0, 0);
    

    foreach race in races
        var color := "#000000";
        GFHTMLArea(gump, x, y, 192, 24, "<BASEFONT color=#{}>{}".format(color, race), 1, 0);
        GFTooltipText(gump, "{}".format(displayRacialBonus(race)));
        if (races.size() > 1)
            GFRadioButton(gump, x+201, y, 0x9C4E, 0x9C4F, 0, 200+i);
        endif
        sleepms(2);
        y += 26;
        i++;
    endforeach

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 200);
    GFGumpPic(gump, 298, 100, culture_pic);

    return GFSendGump(who, gump);
endfunction

function getMonthBySign(sign)
    case(sign)
        "Constelação do Cuidador": return 1; // Duwus
        "Constelação do Governador": return 2; // Alorae
        "Constelação do Herói": return 3; // Galgrumirius
        "Constelação do Bobo": return 4; // Kaius
        "Constelação do Amante": return 5; // Idrithae
        "Constelação da Pessoa Comum": return 6; // Lleuadae
        "Constelação do Criador": return 9; // Demiurgus
        "Constelação do Mago": return 8; // Crassus
        "Constelação do Explorador": return 7; // Adarae
        "Constelação do Fora da Lei": return 10; // Phrugum
        "Constelação do Inocente": return 11; // Praetorus
        "Constelação do Sábio": return 12; // Aeternum
        default: return 1;
    endcase
endfunction

function getRaceByCulturalId(culture_pic)
    var races;
    var prefix := "Meio-";
    case (culture_pic)
        HAROLOTH_ID:
            races := {DROW};
        ALURIEL_ID:
            races := {ELFO};
        COLHESIANA_ID:
            races := {PEQUILHO};
        GULTHRAK_ID:
            races := {ORC};
        BADURAN_ID:
            races := {ANAO};
        BJORSKA_ID:
            races := {HUMANO};
        default:
            races := array{ANAO, prefix+DROW, prefix+ELFO, HUMANO, prefix+ORC, PEQUILHO};
    endcase
    return races;
endfunction

function CaracteristicasRPGump(who, byref chardata, culture_pic)
    var display_race := "";
    if (chardata.culture == AIGLANA || chardata.culture == LESTE)
        display_race += "Meio-";
    endif
    display_race += chardata.raca;
    var racial_traits := GetRacialTraits(chardata.raca);
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    
    GFGumpPic(gump, 17, 31, 0x9E6);

    var x_left := 63;
    var x_right := x_left + 500;
    var y := 60;

    // Exibir informações da raça/cultura
    GFHTMLArea(gump, 149, 76, 192, 40, "<BASEFONT Size=55 Color=#999999>Raça", 0, 0);
    GFHTMLArea(gump, 148, 75, 192, 40, "<BASEFONT Size=55 Color=#ffffff>Raça", 0, 0);
    
    GFHTMLArea(gump, 79, 100, 192, 24, "<BASEFONT color=#000000>{}".format(display_race), 1, 0);

    // Exibir imagem da cultura
    GFGumpPic(gump, 298, 100, culture_pic);

    // Título "Características Físicas e Pessoais" 
    GFHTMLArea(gump, 340, 66, 192, 40, "<BASEFONT Size=55 Color=#999999>Minhas Características", 0, 0);
    GFHTMLArea(gump, 339, 65, 192, 40, "<BASEFONT Size=55 Color=#ffffff>Minhas Características", 0, 0);

    var categories := GetPageCategories();
    var required_categories := array{"Altura", "Peso", "Olhos", "Voz"};
    var y_start := 160;
    var category_iter := 0;
    var x := x_left;
    var current_y := y_start;
    var i, option_y, button_id, is_selected;

    foreach category in categories
        if (category_iter == 3)
            x := x_right;
            current_y := y_start;
        endif

        GFHTMLArea(gump, x+1, current_y+1, 192, 40, "<BASEFONT Size=55 Color=#999999>" + racial_traits[category].nome, 0, 0);
        GFHTMLArea(gump, x, current_y, 192, 40, "<BASEFONT Size=55 Color=#ffffff>" + racial_traits[category].nome, 0, 0);
        
        var options := racial_traits[category].opcoes;
        current_y := current_y + 30;
        // option_ids maps each displayed option to a stable enum id
        var option_ids := racial_traits[category].opcoes_ids;
        for (i := 1; i <= options.size(); i := i + 1)
            option_y := current_y + ((i-1) * 25);
            button_id := 100 + (category_iter * 5) + (i - 1);
            // now compare stored enum id (or 0) against the option's enum id
            var caracteristica := chardata.caracteristicas_rp[category];
            is_selected := (caracteristica == option_ids[i] && OT_UNINIT != TypeOfInt(caracteristica));
            GFHTMLArea(gump, x, option_y, 192, 24, "<BASEFONT color=#000000>{}".format(options[i]), 1, 0);
            
            if (category != "Idade Aparente")
                if (is_selected)
                    GFAddButton(gump, x+200, option_y, 0x9C4F, 0x9C4E, GF_CLOSE_BTN, button_id);
                else
                    GFAddButton(gump, x+200, option_y, 0x9C4E, 0x9C4F, GF_CLOSE_BTN, button_id);
                endif
            endif
        endfor
        current_y := current_y + 140;
        category_iter := category_iter + 1;
    endforeach

    // Adicionar caixa de texto para idade escolhida
    current_y += 20;
    GFTextLine(gump, x, current_y, 1153, "Escreva a sua idade:");
    current_y += 20;
    var age_textbox := GFEAddTextBox(gump, x, current_y, 100, 20, COLOR_YELLOW, "", 1000);

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 222);

    var input := GFSendGump(who, gump);

    return input;
endfunction

function GetRaceMaximumAge(race)
    var idade_maxima := 0;
    case (race)
        ANAO:
            idade_maxima := 150;
        PEQUILHO:
            idade_maxima := 150;
        ELFO:
            idade_maxima := 225;
        DROW:
            idade_maxima := 225;
        HUMANO:
            idade_maxima := 75;
        ORC:
            idade_maxima := 75;
        default:
            idade_maxima := 0;
        endcase
    return idade_maxima;
endfunction

function GetPageCategories()
        return array{"Altura", "Peso", "Olhos", "Voz", "Idade Aparente"};
endfunction

function ProcessSelection(byref chardata, input)
        var categories := GetPageCategories();
        var category_index := (input - 100) / 5;
        var option := (input - 100) % 5 + 1;
        
        if (category_index < categories.size())
            var category := categories[category_index + 1];
            // Map the 1-based option index to the enum id for this race/category
            var racial_traits := GetRacialTraits(chardata.raca);
            var option_ids := racial_traits[category].opcoes_ids;
            if (option >= 1 && option <= option_ids.size())
                chardata.caracteristicas_rp[category] := option_ids[option];
            endif
        endif
endfunction

function ClassGump(who, culture_pic)
    var gump := GFCreateGump();
    GFClosable(gump, 0);

    var x := 63;
    var y := 100;
    var i := 1;

    showPreviousGumpInformation(gump, who, culture_pic, x, y);

    GFHTMLArea(gump, x+16, y+40, 192, 40, "<BASEFONT Size=55 Color=#999999 > Escolha seu caminho", 0, 0); //shadow
    GFHTMLArea(gump, x+15, y+39, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Escolha seu caminho", 0, 0);

    var all_classes := array{ 
        "Caminho das Armas",
		"Caminho do Subterfúgio",
		"Caminho da Sabedoria"
    };
    foreach classe in (all_classes)
        GFHTMLArea(gump, x, y+65, 192, 24, "<BASEFONT color=#000000>{}".format(classe), 1, 0);
        GFRadioButton(gump, x+201, y+65, 0x9C4E, 0x9C4F, 0, 200+i);

        sleepms(2);
        y += 26;
        i++;
    endforeach

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 200);
    GFGumpPic(gump, 298, 100, culture_pic);

    return GFSendGump(who, gump);
endfunction

function backgroundGump(who, culture_pic)
    var gump := GFCreateGump();
    GFClosable(gump, 0);

    var x := 63;
    var y := 100;
    var i := 1;

    showPreviousGumpInformation(gump, who, culture_pic, x, y);

    GFHTMLArea(gump, x+16, y+96, 192, 40, "<BASEFONT Size=55 Color=#999999 > Escolha seu passado", 0, 0); //shadow
    GFHTMLArea(gump, x+15, y+95, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Escolha seu passado", 0, 0);

    var all_backgrounds := BACKGROUNDS;
    VAR cultural_bg := backgroundByCulture(who);
    var cfg := ReadConfigFile(":charactercreation:config/backgrounds");
    foreach bg in (all_backgrounds)
        GFHTMLArea(gump, x, y+116, 192, 24, "<BASEFONT color=#ffffff>{}".format(bg), 1, 0);
        GFToolTipText(gump, cfg[bg].Desc);
        GFRadioButton(gump, x+201, y+116, 0x9C4E, 0x9C4F, 0, 200+i);
        sleepms(2);
        y += 26;
        i++;
    endforeach
    GFHTMLArea(gump, x, y+116, 192, 24, "<BASEFONT color=#ffffff>{}".format(cultural_bg), 1, 0);
    GFToolTipText(gump, cfg[cultural_bg].Desc);

    GFRadioButton(gump, x+201, y+116, 0x9C4E, 0x9C4F, 0, 200+i);

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 200);


    return GFSendGump(who, gump);
endfunction

function backgroundByCulture(who)
    var chardata := GetObjProperty(who, "chardata");
    var culture := chardata.culture;

    return CULTURAL_BACKGROUNDS[culture];
endfunction

function traitGump(who, culture_pic, traits)
    var gump := GFCreateGump();
    GFClosable(gump, 0);

    var x := 63;
    var y := 100;
    var i := 1;

    showPreviousGumpInformation(gump, who, culture_pic, x, y);

    GFHTMLArea(gump, x+21, y+136, 192, 40, "<BASEFONT Size=55 Color=#999999 > Escolha sua Característica", 0, 0); //shadow
    GFHTMLArea(gump, x+20, y+135, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Escolha sua Característica", 0, 0);

	foreach trait in traits
        var trait_name := trait.nome;
        var trait_desc := trait.desc;
        var trait_adv := trait.Vantagens;
        
        GFHTMLArea(gump, x, y+151, 192, 24, "<BASEFONT color=#ffffff >{}".format(trait_name), 1, 0);
        GFTooltipText(gump, "{}<br><BASEFONT color=#ffffff>".format(trait_desc));
        GFTooltipText(gump, "{}<br><BASEFONT color=#ffffff>".format(trait_adv));
        GFRadioButton(gump, x+201, y+151, 0x9C4E, 0x9C4F, 0, 200+i);
        sleepms(2);
        y += 26;
        i++;
	endforeach

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 200);

    return GFSendGump(who, gump);
endfunction

function GetSkillValueDesc(skill)

	if (!skill || skill == error)
		return "";
	elseif (skill == 1)
		return "Terciária";
	elseif (skill == 2)
		return "Secundária";
	elseif (skill == 3)
		return "Primária";
	endif

endfunction

function listTraits(who, chardata)

	var cfg := ReadConfigFile(":charactercreation:config/traits");
	var keys := GetConfigStringKeys( cfg );

	var traits := array;
	var selectedtrait := 1;
	foreach key in keys
		var elem  := FindConfigElem(cfg, key);
		var canadd := 1;
		var racas := GetConfigStringArray( elem, "Raca" );
		var culturas := GetConfigStringArray( elem, "Cultura" );
		var classes := GetConfigStringArray( elem, "Classe" );

		if ((racas.size() >= 1) && !(chardata.raca in racas))
			canadd := 0;
		endif
		if ((classes.size()>=1) && !(chardata.classe in classes))
			canadd := 0;
		endif
		if ((culturas.size() >= 1) && !(chardata.culture in culturas))
			canadd := 0;
		endif

		if (canadd)
			var trait  := struct;
			trait.+nome := GetconfigString(elem, "Nome");
			trait.+desc := GetconfigString(elem, "Desc");
			trait.+vantagens := GetConfigString( elem, "Vantagens" );
			trait.+desvantagens := GetConfigString( elem, "Desvantagens" );
			traits.append(trait);
		endif
	endforeach

    return traits;
endfunction

function showPreviousGumpInformation(byref gump, who, culture_pic := 0, x, y)
    GFGumpPic(gump, 17, 31, 0x9E6);

    var chardata := GetObjProperty(who, "chardata");
    chardata.+Primarias;
    var race := chardata.raca;
    var color := "#FFD700";
    var classe := chardata.classe;
    var bg := chardata.background;
    var trait := chardata.trait;
    var skills := chardata.Skills;
    if (race)
        GFHTMLArea(gump, x+86, y-24, 192, 40, "<BASEFONT Size=55 Color=#999999 > Raça", 0, 0); //shadow
        GFHTMLArea(gump, x+85, y-25, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Raça", 0, 0);
        GFHTMLArea(gump, x+16, y, 192, 24, "<BASEFONT color=#{}>{}".format(color, race), 1, 0);
        GFTooltipText(gump, "{}".format(displayRacialBonus(race)));
    endif

    if (culture_pic)
        GFHTMLArea(gump, 346, 66, 192, 40, "<BASEFONT Size=55 Color=#999999 > Cultura selecionada", 0, 0); //shadow
        GFHTMLArea(gump, 345, 65, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Cultura selecionada", 0, 0);
        GFGumpPic(gump, 298, 100, culture_pic);
    endif

    if (classe)
        GFHTMLArea(gump, x+81, y+29, 192, 40, "<BASEFONT Size=55 Color=#999999 > Classe", 0, 0); //shadow
        GFHTMLArea(gump, x+80, y+30, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Classe", 0, 0);
        GFHTMLArea(gump, x+16, y+51, 192, 24, "<BASEFONT color=#{}>{}".format(color, classe), 1, 0);
    endif

    if (bg)
        GFHTMLArea(gump, x+81, y+82, 192, 40, "<BASEFONT Size=55 Color=#999999 > Passado", 0, 0); //shadow
        GFHTMLArea(gump, x+80, y+81, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Passado", 0, 0);
        GFHTMLArea(gump, x+16, y+101, 192, 24, "<BASEFONT color=#{}>{}".format(color, bg), 1, 0);
    endif

    if (trait)
        GFHTMLArea(gump, x+61, y+132, 192, 40, "<BASEFONT Size=55 Color=#999999 > Característica", 0, 0); //shadow
        GFHTMLArea(gump, x+60, y+131, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Característica", 0, 0);
        GFHTMLArea(gump, x+16, y+151, 192, 24, "<BASEFONT color=#{}>{}".format(color, trait), 1, 0);
    endif

if (GetObjProperty(who, "skill") == 1)
    GFHTMLArea(gump, x+81, y+182, 192, 40, "<BASEFONT Size=55 Color=#999999 > Skills", 0, 0); //shadow
    GFHTMLArea(gump, x+80, y+181, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Skills", 0, 0);

    foreach skill in (skills.keys())
        var color := "";
        if (skills[skill] == 3)
            color := "0000CC";
        elseif (skills[skill] == 2)
            color := "007A23";
        elseif (skills[skill] == 1)
            color := "FFE700";
        endif

        GFHTMLArea(gump, x+16, y+201, 192, 24, "<BASEFONT color=#{}>{} | {}".format(color, skill, GetSkillValueDesc(skills[skill])), 1, 0); //shadow
        y += 30;
    endforeach
endif
    
endfunction

function skillGump(who, culture_pic, byref chardata)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    var minus_btn := {0x9F7, 0x9F8};
    var plus_btn := {0x9F9, 0x9FA};
    var pri_allowed := howManyPrimarias(chardata);

    var x := 63;
    var y := 100;
    var i := 1;
    showPreviousGumpInformation(gump, who, culture_pic, x, y);
    x := 555;
    y := 100;
    GFHTMLArea(gump, x+1, y-24, 192, 40, "<BASEFONT Size=55 Color=#999999 > Escolha suas skills", 0, 0); //shadow
    GFHTMLArea(gump, x, y-25, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Escolha suas skills", 0, 0);
    var cfg;
    var elem;

    cfg := ReadConfigFile(":charactercreation:config/classes");
    elem := FindConfigElem(cfg, cstr(chardata.classe));
    var primarias    := GetConfigStringArray(elem, "P");
    var secundarias  := GetConfigStringArray(elem, "S");
    var terciarias   := GetConfigStringArray(elem, "T");
    var quaternarias := GetConfigStringArray(elem, "Q");
    var value := 1;
    var btn_value;
    x+=10;
    for (i := 1; i <= primarias.size()+1; i++)
        var skill := primarias[i];
        btn_value := GetSkillIdFromName(skill);
        if (i == primarias.size()+1)
            if (chardata.bgBonus in primarias)
                i++;
                break;
            endif
            skill := chardata.bgBonus;
            btn_value := GetSkillIdFromName(skill);
         endif

        if (skill == error)
            continue;
        endif
        GFHTMLArea(gump, x, y-4, 135, 24, "<BASEFONT color=#0000CC >{}".format(StrReplace(skill, "_", " ")), 1, 0);
        GFTooltipText(gump, GetSkillTooltipDescriptionFromName(skill));
        if (chardata.Skills[skill] >= 1)
            GFAddButton(gump, x+138, y-2, minus_btn[1], minus_btn[2], GF_CLOSE_BTN, 100+btn_value);
        endif
        if ((chardata.points - value >= 0) && (chardata.Skills[skill] < 3) && ((chardata.Primarias < pri_allowed) || (chardata.Skills[skill] < 2 && chardata.points - value >= 0))) 
            GFAddButton(gump, x+158, y-2, plus_btn[1], plus_btn[2], GF_CLOSE_BTN, 130+btn_value);
        endif
        sleepms(2);
        y += 26;
    endfor
    y+=10;
    value := 2;
    foreach skill in secundarias
        if (skill == chardata.bgBonus)
            continue;
        endif

        btn_value := GetSkillIdFromName(skill);
        GFHTMLArea(gump, x, y-4, 135, 24, "<BASEFONT color=#007A23 >{}".format(StrReplace(skill, "_", " ")), 1, 0);
        GFTooltipText(gump, GetSkillTooltipDescriptionFromName(skill));
        if (chardata.Skills[skill] >= 1)
            GFAddButton(gump, x+138, y-2, minus_btn[1], minus_btn[2], GF_CLOSE_BTN, 200+btn_value);
        endif
        if ((chardata.points - value >= 0) && (chardata.Skills[skill] < 3) && ((chardata.Primarias < pri_allowed) || (chardata.Skills[skill] < 2 && chardata.points - value >= 0))) 
            GFAddButton(gump, x+158, y-2, plus_btn[1], plus_btn[2], GF_CLOSE_BTN, 230+btn_value);
        endif
        sleepms(2);
        y += 26;
        i++;
    endforeach
    // Cavar buraco
    if (chardata.trait == "Cavar Buracos")
        var gathering := GATHERING;

        btn_value := GetSkillIdFromName(gathering);
        GFHTMLArea(gump, x, y-4, 135, 24, "<BASEFONT color=#007A23 >{}".format(StrReplace(gathering, "_", " ")), 1, 0);
        GFTooltipText(gump, GetSkillTooltipDescriptionFromName(gathering));
        if (chardata.Skills[gathering] >= 1)
            GFAddButton(gump, x+138, y-2, minus_btn[1], minus_btn[2], GF_CLOSE_BTN, 200+btn_value);
        endif
        if ((chardata.points - value >= 0) && (chardata.Skills[gathering] < 3) && ((chardata.Primarias < pri_allowed) || (chardata.Skills[gathering] < 2 && chardata.points - value >= 0))) 
            GFAddButton(gump, x+158, y-2, plus_btn[1], plus_btn[2], GF_CLOSE_BTN, 230+btn_value);
        endif
        sleepms(2);
        y += 26;
        i++;

        y+=10;
        value := 3;
    endif
    // End Cavar Buraco
    foreach skill in terciarias
        if (skill == chardata.bgBonus)
            continue;
        endif

        if (chardata.trait == "Cavar Buracos" && skill == GATHERING)
            continue;
        endif

        btn_value := GetSkillIdFromName(skill);
        GFHTMLArea(gump, x, y-4, 135, 24, "<BASEFONT color=#FFE700 >{}".format(StrReplace(skill, "_", " ")), 1, 0);
        GFTooltipText(gump, GetSkillTooltipDescriptionFromName(skill));
        if (chardata.Skills[skill] >= 1)
            GFAddButton(gump, x+138, y-2, minus_btn[1], minus_btn[2], GF_CLOSE_BTN, 300+btn_value);
        endif
        if ((chardata.points - value >= 0) && (chardata.Skills[skill] < 3) && ((chardata.Primarias < pri_allowed) || (chardata.Skills[skill] < 2 && chardata.points - value >= 0))) 
            GFAddButton(gump, x+158, y-2, plus_btn[1], plus_btn[2], GF_CLOSE_BTN, 330+btn_value);
        endif
        sleepms(2);
        y += 26;
        i++;
    endforeach
    y+=10;
    value := 4;
    foreach skill in quaternarias
        if (skill == chardata.bgBonus)
            continue;
        endif

        btn_value := GetSkillIdFromName(skill);
        GFHTMLArea(gump, x, y-4, 135, 24, "<BASEFONT color=#CC0000 >{}".format(StrReplace(skill, "_", " ")), 1, 0);
        GFTooltipText(gump, GetSkillTooltipDescriptionFromName(skill));
        if (chardata.Skills[skill] >= 1)
            GFAddButton(gump, x+138, y-2, minus_btn[1], minus_btn[2], GF_CLOSE_BTN, 400+btn_value);
        endif
        if ((chardata.points - value >= 0) && (chardata.Skills[skill] < 3) && ((chardata.Primarias < pri_allowed) || (chardata.Skills[skill] < 2 && chardata.points - value >= 0))) 
            GFAddButton(gump, x+158, y-2, plus_btn[1], plus_btn[2], GF_CLOSE_BTN, 430+btn_value);
        endif
        sleepms(2);
        y += 26;
        i++;
    endforeach
    var count := 0;
    var skills := chardata.Skills;
    y := 300;
    x := 73;
    var color := "FFFFFF";
    chardata.Primarias := 0;
    foreach skill in (skills.keys())
        if (skills[skill] == 3)
            chardata.Primarias++;
            color := "0000CC";
        elseif (skills[skill] == 2)
            color := "007A23";
        elseif (skills[skill] == 1)
            color := "FFE700";
        endif
        if (skills[skill] > 0)
            GFHTMLArea(gump, x, y, 192, 24, "<BASEFONT color=#{}>{} | {}".format(color, skill, GetSkillValueDesc(skills[skill])), 1, 0); //shadow
            y+=30;
        endif
    endforeach
    
    SetObjProperty(who, "chardata", chardata);
    GFHTMLArea(gump, 345, 515, 192, 40, "<BASEFONT Size=55 Color=#999999 > Pontos restantes", 0, 0);
    GFHTMLArea(gump, 346, 516, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Pontos restantes", 0, 0); //shadow
    GFHTMLArea(gump, 400, 536, 35, 20, "<BASEFONT color={} >{}".format(Color(GENERAL_COLOR::BLACK).GetHtmlValue(), chardata.points), 1, 0);

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 500); // Botão de fechar

    return GFSendGump(who, gump);
endfunction

function howManyPrimarias(chardata)
    if (chardata.trait == "Sabe-Tudo")
        return 1;
    endif

    return 2;
endfunction

function culturalTraitBonus(who, byref chardata)
    var cfg := ReadConfigFile(":charactercreation:config/backgrounds");
    var culturalBg := chardata.background;
    return chardata.+bgBonus := cfg[culturalBg].Bonus;
endfunction

function attributeGump(who, culture_pic, byref chardata)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    var x := 63;
    var y := 100;
    var i := 1;
    var minus_btn := {0x9F7, 0x9F8};
    var plus_btn := {0x9F9, 0x9FA};
    var cost := 5;
    showPreviousGumpInformation(gump, who, culture_pic, x, y);
    GFHTMLArea(gump, 555, y, 192, 40, "<BASEFONT Size=55 Color=#999999 > Escolha seus atributos", 0, 0); //shadow
    GFHTMLArea(gump, 556, y, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Escolha seus atributos", 0, 0);

    GFHTMLArea(gump, 555, y+20, 120, 25, "<BASEFONT color=#FFFFFF >Forca:    {}".format(chardata.str), 1, 0);
    GFTooltipText(gump, displayAttributeDescription(chardata, "Forca"));
    if (chardata.str > 40)
        GFAddButton(gump, 675, y+25, minus_btn[1], minus_btn[2], GF_CLOSE_BTN, 201);
    endif
    if ((chardata.attribute_points - cost >= 0) && (chardata.str <100)) 
        GFAddButton(gump, 695, y+25, plus_btn[1], plus_btn[2], GF_CLOSE_BTN, 200);
    endif

    GFHTMLArea(gump, 555, y+40, 120, 25, "<BASEFONT color=#FFFFFF >Destreza:  {}".format(chardata.dex), 1, 0);
    GFTooltipText(gump, displayAttributeDescription(chardata, "Destreza"));
    if (chardata.dex > 40)
        GFAddButton(gump, 675, y+45, minus_btn[1], minus_btn[2], GF_CLOSE_BTN, 211);
    endif
    if ((chardata.attribute_points - cost >= 0) && (chardata.dex <100)) 
        GFAddButton(gump, 695, y+45, plus_btn[1], plus_btn[2], GF_CLOSE_BTN, 210);
    endif

    GFHTMLArea(gump, 555, y+60, 120, 25, "<BASEFONT color=#FFFFFF >Inteligencia: {}".format(chardata.int), 1, 0);
    GFTooltipText(gump, displayAttributeDescription(chardata, "Inteligencia"));
    if (chardata.int > 40)
        GFAddButton(gump, 675, y+65, minus_btn[1], minus_btn[2], GF_CLOSE_BTN, 221);
    endif
    if ((chardata.attribute_points - cost >= 0) && (chardata.int <100)) 
        GFAddButton(gump, 695, y+65, plus_btn[1], plus_btn[2], GF_CLOSE_BTN, 220);
    endif

    GFHTMLArea(gump, 345, 515, 192, 40, "<BASEFONT Size=55 Color=#999999 > Pontos restantes", 0, 0);
    GFHTMLArea(gump, 346, 516, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Pontos restantes", 0, 0); //shadow
    GFHTMLArea(gump, 400, 536, 35, 20, "<BASEFONT color={} >{}".format(Color(GENERAL_COLOR::BLACK).GetHtmlValue(), chardata.attribute_points), 1, 0);
    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 500); // Botão de fechar

    return GFSendGump(who, gump);
endfunction

function ReligionGump(who, religion_pic)
    var next_btn := array{"9903", "9904"};
    var prev_btn := array{"9909", "9910"};

    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 17, 31, 0x9E6);

    GFHTMLArea(gump, 346, 56, 192, 40, "<BASEFONT Size=55 Color=#999999 >Escolha sua religião", 0, 0); //shadow
    GFHTMLArea(gump, 345, 55, 192, 40, "<BASEFONT Size=55 Color=#ffffff >Escolha sua religião", 0, 0);
    var x := 63;
    var y := 100;
    var i := 0;

    GFHTMLArea(gump, 100, 550, 650, 120, getReligionDescriptionByPictureId(religion_pic), 1, 1);
    GFAddButton(gump, 543, 300, next_btn[1], next_btn[2], GF_CLOSE_BTN, 201);
    GFAddButton(gump, 273, 300, prev_btn[1], prev_btn[2], GF_CLOSE_BTN, 210);
    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 500);

    GFGumpPic(gump, 298, 100, religion_pic);

    return GFSendGump(who, gump);
endfunction

function getReligionByPictureId(religion_id)
    case (religion_id)
        VENERATIO_ID:
            return VENERATIO;
        LORE_ID:
            return LORE;
        BOKK_BHARAZ_ID:
            return BOKK_BHARAZ;
        HALLEUAD_ID:
            return HALLEUAD;
        KAI_ID:
            return KAI;
        ELUITHORTH_ID:
            return ELUITHORTH;
        USHKRAR_ID:
            return USHKRAR;
        default:
            return CETICOS;
    endcase
endfunction

function getReligionDescriptionByPictureId(culture_id)
    var textosConfig := ReadConfigFile(":charactercreation:/config/descricoes");
    case (culture_id)
        VENERATIO_ID:
            return FindConfigElem(textosConfig, "veneratio").descricao;
        LORE_ID:
            return FindConfigElem(textosConfig, "lore").descricao; 
        BOKK_BHARAZ_ID:
            return FindConfigElem(textosConfig, "bokk_bharaz").descricao;
        HALLEUAD_ID:
            return FindConfigElem(textosConfig, "halleuad").descricao;
        KAI_ID:
            return FindConfigElem(textosConfig, "kai").descricao;
        ELUITHORTH_ID:
            return FindConfigElem(textosConfig, "eluithorth").descricao;
        USHKRAR_ID:
            return FindConfigElem(textosConfig, "ushkrar").descricao;
        default:
            return FindConfigElem(textosConfig, "ceticos").descricao;
    endcase
endfunction

function itemInheritance(who)
    var chardata := GetObjProperty(who, "chardata");

    genericItems(who);
    armorsByCulture(who);
    weaponsByClassAndCulture(who);
    workingItems(who);
endfunction

function genericItems(who)
    var chardata := GetObjProperty(who, "chardata");
    var skills := chardata.Skills;
    
    var robe := CreateItemInicial(who, 0x1F03, 1); // Roupa
    EquipItem(who, robe);
    var moedas := "1d4";
    if (chardata.classe == GUERREIRO)
        moedas := "1d8";
    elseif (chardata.classe == LADINO)
        moedas := "1d12";
    endif
	CreateItemInBackpack(who, 0xba63, RandomDiceRoll(moedas)); //moedas de cobre
    if (chardata.culture == BADURAN)
        CreateItemInicial(who, "boots", 1); 
    else
        CreateItemInicial(who, 0x170D, 1); // Sandals
    endif
    CreateItemInicial(who, 0x152e, 1); // short pants
    CreateItemInicial(who, 0x1517, 1); // shirt
    CreateItemInBackpack(who, 3940, 1); // torch
    CreateItemInBackpack(who, 0xC61F, 1); // laterna cintura
    var oleo := CreateItemInicial(who, 0x606f, 2); // Oleo de Polir Equipamentos
    SetObjProperty(oleo, "descmod", "Óleo de polir equipamentos" + "<br>");
endfunction

function weaponsByClassAndCulture(who)
    var chardata := GetObjProperty(who, "chardata");
    var skills := chardata.Skills;

    if (chardata.culture == GULTHRAK)
        CreateItemInicial(who, 0xF9FC, 1); // Adaga

        if (skills[ONEHANDED] >= 1)
            CreateItemInicial(who, 0x26C6, 1); // scepter
        endif
        if (skills[PARRY] >= 1)
            CreateItemInicial(who, 0x1B78, 1); // shield
        endif
        if (skills[TWOHANDED] >= 1)
            if( chardata.magia == "Shaman")
                CreateItemInicial(who, 0x26C4, 1); // scythe
            else
                CreateItemInicial(who, 0x0f45, 1); // Grishnak
            endif
        endif
        if (skills[DUALWIELD] >= 1)
            CreateItemInicial(who, 0x8fd, 1);
        endif
    endif

    if (chardata.culture == AIGLANA)
        CreateItemInicial(who, 0xF99F, 1); // Adaga

        if (skills[ONEHANDED] >= 1)
            CreateItemInicial(who, 0x0f63, 1); // Hasta
        endif
        if (skills[PARRY] >= 1)
            CreateItemInicial(who, 0xD1A1, 1); // Caetra
        endif
        if (skills[TWOHANDED] >= 1)
            CreateItemInicial(who, 0xF9E2, 1); // two handed sword aigla
        endif
        if (skills[DUALWIELD] >= 1)
            CreateItemInicial(who, 0xF9CA, 1);
        endif
    endif

    if (chardata.culture == HAROLOTH)
        CreateItemInicial(who, 0x0f51, 1); // Adaga

        if (skills[ONEHANDED] >= 1)
            CreateItemInicial(who, 0x1401, 1); //kryss
        endif
        if (skills[PARRY] >= 1)
            CreateItemInicial(who, 0x1B73, 1); // buckler
        endif
        if (skills[TWOHANDED] >= 1)
            CreateItemInicial(who, 0x26CB, 1); //crescent
        endif
        if (skills[DUALWIELD] >= 1)
            CreateItemInicial(who, 0xFA4D, 1);
        endif
    endif

    if (chardata.culture == COLHESIANA)
        CreateItemInicial(who, 0xF998, 1); // Adaga

    
        if (skills[ONEHANDED] >= 1)
            CreateItemInicial(who, 0x1440, 1); //cutlass
        endif
        if (skills[PARRY] >= 1)
            CreateItemInicial(who, 0x1B73, 1); // buckler
        endif
        if (skills[TWOHANDED] >= 1)
            CreateItemInicial(who, 0x0e87, 1); //crescent
        endif
        if (skills[DUALWIELD] >= 1)
            CreateItemInicial(who, 0xFA4E, 1);
        endif
    endif

    if (chardata.culture == BJORSKA)
        CreateItemInicial(who, 0xF99B, 1); // Adaga

        if (skills[ONEHANDED] >= 1)
            CreateItemInicial(who, 0x13AF, 1); // waraxe
        endif
        if (skills[PARRY] >= 1)
            CreateItemInicial(who, 0x1B7A, 1); // wooden shield
        endif
        if (skills[TWOHANDED] >= 1)
            CreateItemInicial(who, 0x0f49, 1); // axe
        endif
        if (skills[DUALWIELD] >= 1)
            CreateItemInicial(who, 0xF9F4, 1);
        endif
    endif

    if (chardata.culture == LESTE)
        CreateItemInicial(who, 0xF99A, 1); // Adaga

        if (skills[ONEHANDED] >= 1)
            CreateItemInicial(who, 0x27EF, 1); // wakizashi
        endif
        if (skills[PARRY] >= 1)
            CreateItemInicial(who, 0x1B72, 1); // round shield
        endif
        if (skills[TWOHANDED] >= 1)
            CreateItemInicial(who, 0x27A2, 1); // Katana
        endif
        if (skills[DUALWIELD] >= 1)
            CreateItemInicial(who, 0x27af, 1);
        endif
    endif

    if (chardata.culture == BADURAN)
        CreateItemInicial(who, 0xF999, 1); // Adaga
    
        if (skills[ONEHANDED] >= 1)
            CreateItemInicial(who, 0x143c, 1); // HAMMERPICK
        endif
        if (skills[PARRY] >= 1)
            CreateItemInicial(who, 0xFA5E, 1); // escudo baduran
        endif
        if (skills[TWOHANDED ] >= 1)
            CreateItemInicial(who, 0x1438, 1); // warhammer
        endif
        if (skills[DUALWIELD] >= 1)
            CreateItemInicial(who, 0x8fd, 1);
        endif
    endif

    if (chardata.culture == ALURIEL)
        CreateItemInicial(who, 0xF99D, 1); // Adaga

        if (skills[ONEHANDED] >= 1)
            CreateItemInicial(who, 0x0f5f, 1); // rapiera
        endif
        if (skills[PARRY] >= 1)
            CreateItemInicial(who, 0x1B73, 1); // buckler
        endif
        if (skills[TWOHANDED] >= 1)
            CreateItemInicial(who, 0x26CB, 1); // crescent
        endif
        if (skills[DUALWIELD] >= 1)
            CreateItemInicial(who, 0xF9A0, 1);
        endif
    endif

    if (skills[RANGED] >= 1)
        var ammo_amount := skills[RANGED] * 80;
        CreateItemInicial(who, 0x13B2, 1); // bow
        CreateItemInBackpack(who, 0xf3f, ammo_amount); // arrows

        CreateItemInicial(who, 0x2fb7, 1); // aljava
    endif
endfunction

function armorsByCulture(who)
    var chardata := GetObjProperty(who, "chardata");

    if (chardata.culture == GULTHRAK)
        CreateItemInicial(who, 0x144e, 1); // Armadura de ossos
        CreateItemInicial(who, 0x144f, 1); // Armadura de ossos
        CreateItemInicial(who, 0x1450, 1); // Armadura de ossos
        CreateItemInicial(who, 0x1451, 1); // Armadura de ossos
        CreateItemInicial(who, 0x1452, 1); // Armadura de ossos
    endif

    if (chardata.culture == BADURAN)
        CreateItemInicial(who, 0xF98E, 1); // Armadura de aneis
        CreateItemInicial(who, 0xF987, 1); // Armadura de aneis
        CreateItemInicial(who, 0xF98C, 1); // Armadura de aneis
        CreateItemInicial(who, 0xF98A, 1); // Armadura de aneis
        CreateItemInicial(who, 0xF989, 1); // Armadura de aneis
    endif

    if (chardata.culture == AIGLANA)
        CreateItemInicial(who, 0xc684, 1); // Lorica Squamata
        CreateItemInicial(who, 0xc680, 1); // Lorica Squamata
        CreateItemInicial(who, 0xc681, 1); // Lorica Squamata
        CreateItemInicial(who, 0xc683, 1); // Lorica Squamata
        CreateItemInicial(who, 0xc682, 1); // Lorica Squamata
    endif

    if (chardata.culture == HAROLOTH)
        CreateItemInicial(who, 0x13D6, 1); // Studded Leather
        CreateItemInicial(who, 0x13D5, 1); // Studded Leather
        CreateItemInicial(who, 0x13D4, 1); // Studded Leather
        CreateItemInicial(who, 0x13DA, 1); // Studded Leather
        CreateItemInicial(who, 0x13DB, 1); // Studded Leather
    endif

    if (chardata.culture == COLHESIANA)
        CreateItemInicial(who, 0x13C7, 1); // Armadura de couro simples
        CreateItemInicial(who, 0x13CD, 1); // Armadura de couro simples
        CreateItemInicial(who, 0x1DB9, 1); // Armadura de couro simples
        CreateItemInicial(who, 0x13CB, 1); // Armadura de couro simples
        CreateItemInicial(who, 0x13CC, 1); // Armadura de couro simples
    endif

    if (chardata.culture == ALURIEL)
        CreateItemInicial(who, 0x13D6, 1); // Studded Leather
        CreateItemInicial(who, 0x13D5, 1); // Studded Leather
        CreateItemInicial(who, 0x13D4, 1); // Studded Leather
        CreateItemInicial(who, 0x13DA, 1); // Studded Leather
        CreateItemInicial(who, 0x13DB, 1); // Studded Leather
    endif

    if (chardata.culture == BJORSKA)
        CreateItemInicial(who, 0x13BB, 1); // Cota de malhas
        CreateItemInicial(who, 0x13BE, 1); // Cota de malhas
        CreateItemInicial(who, 0x13BF, 1); // Cota de malhas
    endif

    if (chardata.culture == LESTE)
        CreateItemInicial(who, 0x27c6, 1); // Armadura de fibra
        CreateItemInicial(who, 0x2786, 1); // Armadura de fibra
        CreateItemInicial(who, 0x27c9, 1); // Armadura de fibra
        CreateItemInicial(who, 0x277a, 1); // Armadura de fibra
    endif
endfunction

function workingItems(who)
    var chardata := GetObjProperty(who, "chardata");
    var skills := chardata.Skills;

    if (skills[WOODWORKING] >= 1)
        CreateItemInicial(who, 0x1034, 1); // saw
        CreateItemInicial(who, 0x1022, 1);
    endif
    if (skills[METALWORKING] >= 1)
        CreateItemInicial(who, 0x13E3, 1); // smithhammer
        CreateItemInicial(who, 0x5107, 1); // smelting pot
    endif
    if (skills[GATHERING] >= 1)
        CreateItemInicial(who, 0x0f43, 1); // hatchet
        CreateItemInicial(who, 0x0e85, 1); // pickaxe
    endif
    if (skills[SURVIVAL] >= 1)
        CreateItemInicial(who, 0x13f6, 1); // butcher knife
        CreateItemInicial(who, 0x0ec4, 1); // skinning knife
    endif

    if (skills[MEDICINE] >= 1)
        CreateItemInBackpack(who, 0xe21, (skills[MEDICINE] * 40)); // Bandages
    endif

    if (skills[MEDICINE] == 3)
		CreateItemInBackpack(who, 0xedfe, 1);
		CreateItemInBackpack(who, 0xed2a, 1);
        CreateItemInBackpack(who, 0xf0c8, 1);
        case(randomint(3))//bisturi
            1: CreateItemInBackpack(who, 0xD1CE, 1); 
            2: CreateItemInBackpack(who, 0XD1D1, 1);
            3: CreateItemInBackpack(who, 0XD1CF, 1);
        endcase
        case(randomint(2))// mascara da praga
            1: CreateItemInBackpack(who, 0xF0C6, 1); 
            2: CreateItemInBackpack(who, 0xF0C7, 1);
        endcase
        case(randomint(2))//sobretudo
            1:CreateItemInBackpack(who, 0xD1B9, 1); 
            2:CreateItemInBackpack(who, 0xD1BA, 1);
        endcase
		SendSysMessageEX(who, "Você recebeu itens de medicina por ter Medicine como primária.", SSM_INFO);
	endif

    if (skills[MECHANICS] >= 1)
        CreateItemInicial(who, 0x14FB, (skills[MECHANICS] * 8)); // lockpick
    endif

    

    if (skills[DOMESTIC_WORK] >= 1)
        CreateItemInicial(who, 0x0f9d, 1); // sewingkit
    endif

    if (skills[DOMESTIC_WORK] >= 1 || skills[MEDICINE] >= 1)
        CreateItemInicial(who, 0xf9e, 1); // Scissors
    endif
    
    if (skills[ALCHEMY] >= 1)
        CreateItemInBackpack(who, 0x0e9b, 1); // mortar
        CreateItemInBackpack(who, 0x1832, 1); // balao
        CreateItemInBackpack(who, 0x1830, 1); // beker
        CreateItemInBackpack(who, 0x1849, 1); // aparato
        CreateItemInBackpack(who, 0x1BE3, skills[ALCHEMY] * 10); // vidro pros frascos, porque precisa do material
        var book1 := CreateItemInBackpack(who, "alchemybook");
            book1.newbie := 1;
		    setobjproperty(book1, "serialid", who.serial);
		    setobjproperty(book1, "marked",who.name + " - " + who.acctname);
        var dyebook := CreateItemInBackpack(who, "dyebook");
            setobjproperty(dyebook, "serialid", who.serial);
		    setobjproperty(dyebook, "marked",who.name + " - " + who.acctname);
        var recipe := struct;
        var chosenRecipes := AlchChoices(who);
        var r1 := CreateItemInBackpack(who, "alchemyrecipe1");
        recipe.+recipe := chosenRecipes[1];
        SetObjProperty(r1, "recipe", recipe);
        r1 := CreateItemInBackpack(who, "alchemyrecipe1");
        recipe.recipe := chosenRecipes[2];
        SetObjProperty(r1, "recipe", recipe);
        r1 := CreateItemInBackpack(who, "alchemyrecipe1");
        recipe.recipe := chosenRecipes[3];
        SetObjProperty(r1, "recipe", recipe);
        r1 := CreateItemInBackpack(who, "alchemyrecipe1");
        recipe.recipe := chosenRecipes[4];
        SetObjProperty(r1, "recipe", recipe);
        var i := 0;
        while(i != (skills[ALCHEMY])*2)
            var tinta := randomint(1);
            if(tinta == 0)
                var t1 := CreateItemInBackpack(who, "tintasrecipe1");
            else
                var t1 := CreateItemInBackpack(who, "tintasrecipe2");
            endif
            i := ++i;
        endwhile

        if (skills[ALCHEMY] >= 2)
		    r1 := CreateItemInBackpack(who, "alchemyrecipe1");
            recipe.recipe := chosenRecipes[5];
            SetObjProperty(r1, "recipe", recipe);
            r1 := CreateItemInBackpack(who, "alchemyrecipe1");
            recipe.recipe := chosenRecipes[6];
            SetObjProperty(r1, "recipe", recipe);
            
	        if (skills[ALCHEMY] == 3)
		        var book2:= CreateItemInBackpack(who, "alchemybook");
        		setobjproperty(book2, "serialid",who.serial);
		        setobjproperty(book2, "marked",who.name + " - " + who.acctname);
                r1 := CreateItemInBackpack(who, "alchemyrecipe1");
                recipe.recipe := chosenRecipes[7];
                SetObjProperty(r1, "recipe", recipe);
                r1 := CreateItemInBackpack(who, "alchemyrecipe1");
                recipe.recipe := chosenRecipes[8];
                SetObjProperty(r1, "recipe", recipe);
            endif
        endif
	endif
endfunction

function attributeBonusByRaceAndTraits(who)
    var chardata := GetObjProperty(who, "chardata");
    
    var raca := chardata.raca;
    var final_str := chardata.str;
    var final_dex := chardata.dex;
    var final_int := chardata.int;

    if (raca == ANAO)
        final_str += 10;
    elseif (raca == ELFO)
        final_int += 10;
    elseif (raca == DROW)
        final_dex += 5;
        final_int +=5;
    elseif (raca == ORC)
        final_str += 5;
        final_int +=5;
    elseif (raca == PEQUILHO)
        final_dex += 10;
    endif

    if (chardata.trait == "Corpulento")
        final_int -= 5;
        final_dex -= 5;
        final_str += 10;
	elseif (chardata.trait == "Genio")
        final_str -= 5;
        final_dex -= 5;
        final_int += 10;
	elseif (chardata.trait == "Agil")
        final_str -= 5;
        final_int -= 5;
        final_dex += 10;
	elseif (chardata.trait == "Robusto")
        final_str += 15;
        final_int -= 5;
        final_dex -= 5;
	endif

    AP_SetTrueStat(who, STRENGTH, final_str);
    AP_SetTrueStat(who, DEXTERITY, final_dex);
    AP_SetTrueStat(who, INTELLIGENCE, final_int);
endfunction

function resetCharacter(who)
    resetGender(who);
    if (GetObjProperty(who, "skill") == 1)
        EraseObjProperty(who, "skill");
    endif
    if (GetObjProperty(who, "chardata"))
        EraseObjProperty(who, "chardata");
    endif
    if (GetObjProperty(who, "classe"))
        EraseObjProperty(who, "classe");
    endif
    if (GetObjProperty(who, "advclass"))
        EraseObjProperty(who, "advclass");
    endif
    if (GetObjProperty(who, "raca"))
        EraseObjProperty(who, "raca");
    endif

	if (who.cmdlevel < 2)
		var equiped := listequippeditems(who);
		foreach item in equiped
			if (item != who.backpack)
                DestroyItem(item);
			endif
		endforeach
		equiped := enumerateitemsincontainer(who.backpack);
		foreach item in equiped
			DestroyItem(item);
		endforeach
		equiped := 0;
	endif

	//zera as skills
	foreach attrib in AP_ListAttributesByType()
		AP_SetTrueSkill(who, attrib, 0);
		sleepms(10);
	endforeach

    EraseObjProperty(who, "linguas_conhecidas");
    EraseObjProperty(who, "lingua_escolhida");

    AP_SetVital(who, "Hunger", 0);
	RecalcVitals(who);
endfunction

function resetGender(who)
    if (who.graphic == 1828 || who.graphic == 1830 || who.graphic == 1832 || who.graphic == 1835)
        who.graphic := 400;
    elseif (who.graphic == 1829 || who.graphic == 1831 || who.graphic == 1833 || who.graphic == 1834)
        who.graphic := 401;
    endif
endfunction

function looksGump(who, chardata)
	var disfarce := struct;
    disfarce.+barba := "";
    disfarce.+cabelo := "";
    disfarce.+corcabelo := -1;
    disfarce.+corbarba := -1;
    disfarce.+corpele := -1;

    escolherCabelo(who, disfarce, chardata.raca);
    escolheCorCabelo(who, disfarce, 0, chardata.raca, 1);
    if (chardata.raca != ELFO && chardata.raca != DROW)
        escolherBarba(who, disfarce);
    endif

    SkinGump(who, disfarce, chardata.raca, 1);

    who.color := disfarce.corpele;
    who.truecolor := disfarce.corpele;

    var mybarba := GetEquipmentByLayer(who, LAYER_BEARD);
    var newbarba := CreateItemAtLocation(who.x, who.y, who.z, disfarce.barba, 1, who.realm);

    newbarba.color := 1109;

    newbarba.color := disfarce.corcabelo;
    if (mybarba)
        DestroyItem(mybarba);
    endif
    var result := EquipItem(who, newbarba);

    var myhair := GetEquipmentByLayer(who, LAYER_HAIR);
    var newhair := CreateItemAtLocation(who.x, who.y, who.z, disfarce.cabelo, 1, who.realm);

    newhair.color := 1109;

    newhair.color := disfarce.corcabelo;
    var result2 := DestroyItem(myhair);
    result := EquipItem(who, newhair);

	if (YesNo(who, "Deseja manter esta aparência?" , "Sim.", "Não."))
		return 1;
	else
		looksGump(who, chardata);
	endif
endfunction


function gameModeGump(who, religion_pic)
    var next_btn := array{"9903", "9904"};
    var prev_btn := array{"9909", "9910"};

    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 17, 31, 0x9E6);

    GFHTMLArea(gump, 326, 56, 192, 40, "<BASEFONT Size=55 Color=#999999 >Escolha seu modo de jogo", 0, 0); //shadow
    GFHTMLArea(gump, 325, 55, 192, 40, "<BASEFONT Size=55 Color=#ffffff >Escolha seu modo de jogo", 0, 0);
    var x := 63;
    var y := 100;
    var i := 0;

    GFHTMLArea(gump, 100, 550, 650, 120, gameModeDescriptionByPictureId(religion_pic), 1, 1);
    GFAddButton(gump, 543, 300, next_btn[1], next_btn[2], GF_CLOSE_BTN, 201);
    GFAddButton(gump, 273, 300, prev_btn[1], prev_btn[2], GF_CLOSE_BTN, 210);
    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 200);

    GFGumpPic(gump, 298, 100, religion_pic);

    return GFSendGump(who, gump);
endfunction

function gameModeByPictureId(gameModeId)
    case (gameModeId)
        0x9FC:
            return TRUE_MODE;
        0x9FB:
            return STORY_MODE;
    endcase
endfunction

function gameModeDescriptionByPictureId(gameModeId)
    var textosConfig := ReadConfigFile(":charactercreation:/config/descricoes");
    case (gameModeId)
        0x9FC:
            return FindConfigElem(textosConfig, "trueMode").descricao;
        0x9FB:
            return FindConfigElem(textosConfig, "storyMode").descricao;
    endcase
endfunction

function getWisdomPathsByCulture(who)
    var wisdomPaths := array{"Mago", "Alquimista"};
    var chardata := GetObjProperty(who, "chardata");
    var culture := chardata.culture;
    if (culture == GULTHRAK)
        wisdomPaths.append("Xamã");
    elseif (culture == BADURAN)
        wisdomPaths.append("Martelo Divino");
        wisdomPaths.append("Engenheiro");
    elseif (culture == AIGLANA)
        wisdomPaths.append("Augur");
    elseif (culture == HAROLOTH && who.graphic == 1831)
        wisdomPaths.append("Clériga de Idrith");
    elseif (culture == BJORSKA || culture == LESTE)
        wisdomPaths.append("Clérigo");
    elseif (culture == ALURIEL)
        wisdomPaths.append("Arqueiro Arcano");
    endif
    
    if (culture != HAROLOTH && culture != GULTHRAK)
        wisdomPaths.append("Druida");
    endif
    
    return wisdomPaths;
endfunction

function pathOfWisdomGump(who, culture_pic)
    var gump := GFCreateGump();
    GFClosable(gump, 0);

    var x := 63;
    var y := 100;
    var i := 1;
    showPreviousGumpInformation(gump, who, culture_pic, x, y);
    x := 555;
    y := 100;
    GFHTMLArea(gump, x+1, y-24, 192, 40, "<BASEFONT Size=55 Color=#999999 > Escolha seu caminho", 0, 0); //shadow
    GFHTMLArea(gump, x, y-25, 192, 400, "<BASEFONT Size=55 Color=#ffffff > Escolha seu caminho", 0, 0);

    var wisdomPaths := getWisdomPathsByCulture(who);
    foreach path in wisdomPaths
        var color := "#000000";
        GFHTMLArea(gump, x, y, 192, 24, "<BASEFONT color=#{}>{}".format(color, path), 1, 0);
        GFRadioButton(gump, x+201, y, 0x9C4E, 0x9C4F, 0, 200+i);
        sleepms(2);
        y += 26;
        i++;
    endforeach

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 200); // Botão de fechar
    return GFSendGump(who, gump);
endfunction

function startAdventureGump(who)
    var textosConfig := ReadConfigFile(":charactercreation:/config/descricoes");
    var descricao := FindConfigElem(textosConfig, "adventureStart").descricao;
    var startTime := ReadGameClock();

    var result;
    do
        var gump := GFCreateGump();
        GFClosable(gump, 0);
        GFResizePic(gump, 0, 0, 9270, 760, 625);
        GFHTMLArea(gump, 21, 16, 740, 40, "<BASEFONT Size=55 Color=#000000 >Epic! Beta Test - A Ilha", 0, 0);
        GFHTMLArea(gump, 20, 15, 740, 40, "<BASEFONT Size=55 Color=#ffffff >Epic! Beta Test - A Ilha", 0, 0);
        GFGumpPic(gump, 20, 50, 0xFFFD);
        GFHTMLArea(gump, 20, 470, 720, 110, descricao, 1, 1);
        GFAddButton(gump, 710, 590, 4005, 4007, GF_CLOSE_BTN, 200);
        result := GFSendGump(who, gump);
    dowhile (ReadGameClock() - startTime < 10);

    return result;
endfunction


function languageSelectionGump(who, culture_pic, linguasOpt, known_languages, total_amount)
    var gump := GFCreateGump();
    GFClosable(gump, 0);

    var x := 63;
    var y := 100;
    var i := 1;
    showPreviousGumpInformation(gump, who, culture_pic, x, y);
    x := 555;
    y := 100;
    GFHTMLArea(gump, x+1, y-24, 192, 40, "<BASEFONT Size=55 Color=#999999 > Escolha suas línguas", 0, 0); //shadow
    GFHTMLArea(gump, x, y-25, 192, 400, "<BASEFONT Size=55 Color=#ffffff > Escolha suas línguas", 0, 0);

    var chardata := GetObjProperty(who, "chardata");
    var culture := chardata.culture;
    foreach lang in linguasOpt
        var color := "#000000";
        GFHTMLArea(gump, x, y, 192, 24, "<BASEFONT color=#{}>{}".format(color, lang), 1, 0);
        if (!(lang in known_languages) && (known_languages.size() < total_amount))
            GFCheckBox(gump, x+201, y, 0x9C4E, 0x9C4F, 0, 200+i);
        endif
        sleepms(2);
        y += 26;
        i++;
    endforeach

    GFHTMLArea(gump, 345, 515, 192, 40, "<BASEFONT Size=55 Color=#999999 > Escolha {} línguas".format(Cint(total_amount - known_languages.size())), 0, 0);
    GFHTMLArea(gump, 346, 516, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Escolha {} línguas".format(Cint(total_amount - known_languages.size())), 0, 0); //shadow

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 200); // Botão de fechar
    return GFSendGump(who, gump);
endfunction

function parseLanguageByCulture(who, linguasOpt)
        var culture := GetObjProperty(who, "chardata").culture;

        if (culture == AIGLANA)
            return linguasOpt[1];
        elseif (culture == BADURAN)
            return linguasOpt[2];
        elseif (culture == ALURIEL)
            return linguasOpt[3];
        elseif (culture == LESTE)
            return linguasOpt[4];
        elseif (culture == COLHESIANA)
            return linguasOpt[5];
        elseif (culture == GULTHRAK)
            return linguasOpt[6];
        elseif (culture == BJORSKA)
            return linguasOpt[7];
        elseif (culture == HAROLOTH)
            return linguasOpt[8];
        endif
endfunction

// Adicione estes textos para cada signo

const cuidadorText := "Constelação do Cuidador<br><br>Altruísta e generoso, busca proteger e cuidar dos outros. Preocupa-se com o bem-estar alheio, colocando as necessidades dos outros acima das suas.";

const governadorText := "Constelação do Governador<br><br>Figura autoritária que busca controle, ordem e estabilidade. O Governador acredita que uma liderança firme é essencial para manter a harmonia e a organização.";

const heroiText := "Constelação do Herói<br><br>Determinado e corajoso, busca enfrentar desafios e superar adversidades para proteger os outros. O Herói valoriza a força, a coragem e a superação.";

const boboText := "Constelação do Bobo<br><br>Alegre, irreverente e brincalhão, busca trazer leveza, humor e questionar o status quo. Tem uma atitude despreocupada e divertida diante da vida.";

const amanteText := "Constelação do Amante<br><br>Movido pela paixão e pelo desejo de conexões emocionais profundas. O Amante busca experiências de beleza, amor e prazer em seus relacionamentos e na vida.";

const comumText := "Constelação da Pessoa Comum<br><br>Relaciona-se com todos, representando a igualdade e a simplicidade. Valoriza o pertencimento e a autenticidade, buscando se conectar com os outros de forma genuína.";

const criadorText := "Constelação do Criador<br><br>Focado na inovação, imaginação e criação de algo novo. O Criador é movido pela necessidade de expressar sua visão e trazer algo original ao mundo.";

const magoText := "Constelação do Mago<br><br>Representa o poder de transformação, o conhecimento e o mistério. O Mago é capaz de transformar a realidade ao seu redor, usando sabedoria e intuição.";

const exploradorText := "Constelação do Explorador<br><br>Aventureiro e curioso, busca descobrir o novo e expandir os limites do que é conhecido. Valoriza a liberdade, a autoexpressão e a jornada para descobrir quem realmente é.";

const foradaleiText := "Constelação do Fora da Lei<br><br>Desafiador, quer quebrar regras e sistemas para trazer mudança. Sente-se atraído pela revolução, pela quebra de paradigmas e pela luta contra injustiças.";

const inocenteText := "Constelação do Inocente<br><br>Otimista e sonhador, acredita na bondade inerente das pessoas e da vida. O Inocente busca a felicidade, simplicidade e um mundo sem malícia.";

const sabioText := "Constelação do Sábio<br><br>Motivado pelo desejo de conhecimento e verdade, o Sábio busca entender o mundo profundamente. Valorizado por sua sabedoria, reflexão e discernimento.";

// Adicione esta função para selecionar o signo


function signSelection(who)
    var chardata := GetObjProperty(who, "chardata");
    var sign_pic := CUIDADOR_ID;
    var btn_value;
    while (!GetObjProperty(who, "creation_completed") && who.connected)
        var input := SignGump(who, sign_pic);
        if (!input || input == error)
            sleepms(500);
            continue;
        endif
        btn_value := input[0];

        if (btn_value == 201)
            sign_pic++;
        elseif (btn_value == 210)
            sign_pic--;
        elseif (btn_value == 666)
            break;
        endif

        if (sign_pic < CUIDADOR_ID)
            sign_pic := SABIO_ID;
        elseif (sign_pic > SABIO_ID)
            sign_pic := CUIDADOR_ID;
        endif
    endwhile
    chardata.+sign := getSignByPictureId(sign_pic);
    SetObjProperty(who, "chardata", chardata);

    SendSysMessageEX(who, "Seu signo será "+chardata.sign, SSM_INFO);
    return sign_pic;
endfunction

function getSignByPictureId(sign_pic)
    case(sign_pic)
        CUIDADOR_ID: return "Constelação do Cuidador";
        GOVERNADOR_ID: return "Constelação do Governador";
        HEROI_ID: return "Constelação do Herói";
        BOBO_ID: return "Constelação do Bobo";
        AMANTE_ID: return "Constelação do Amante";
        COMUM_ID: return "Constelação da Pessoa Comum";
        CRIADOR_ID: return "Constelação do Criador";
        MAGO_ID: return "Constelação do Mago";
        EXPLORADOR_ID: return "Constelação do Explorador";
        FORADALEI_ID: return "Constelação do Fora da Lei";
        INOCENTE_ID: return "Constelação do Inocente";
        SABIO_ID: return "Constelação do Sábio";
        default: return "Desconhecido";
    endcase
endfunction

function getSignDescriptionByPictureId(sign_pic)
    case(sign_pic)
        CUIDADOR_ID: return cuidadorText;
        GOVERNADOR_ID: return governadorText;
        HEROI_ID: return heroiText;
        BOBO_ID: return boboText;
        AMANTE_ID: return amanteText;
        COMUM_ID: return comumText;
        CRIADOR_ID: return criadorText;
        MAGO_ID: return magoText;
        EXPLORADOR_ID: return exploradorText;
        FORADALEI_ID: return foradaleiText;
        INOCENTE_ID: return inocenteText;
        SABIO_ID: return sabioText;
        default: return "Descrição não disponível.";
    endcase
endfunction

function SignGump(who, sign_pic)
    var next_btn := array{"9903", "9904"};
    var prev_btn := array{"9909", "9910"};

    var gump := GFCreateGump();
    GFClosable(gump, 0);
    GFGumpPic(gump, 17, 31, 0x9E6);

    GFHTMLArea(gump, 346, 56, 192, 40, "<BASEFONT Size=55 Color=#999999 >Escolha seu Signo", 0, 0); //shadow
    GFHTMLArea(gump, 345, 55, 192, 40, "<BASEFONT Size=55 Color=#ffffff >Escolha seu Signo", 0, 0);
    var x := 63;
    var y := 100;
    var i := 0;

    GFHTMLArea(gump, 100, 550, 650, 120, getSignDescriptionByPictureId(sign_pic), 1, 1);
    GFAddButton(gump, 543, 300, next_btn[1], next_btn[2], GF_CLOSE_BTN, 201);
    GFAddButton(gump, 273, 300, prev_btn[1], prev_btn[2], GF_CLOSE_BTN, 210);
    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 666);

    GFGumpPic(gump, 298, 100, sign_pic);

    return GFSendGump(who, gump);
endfunction

function GetMonths()
    return MONTHS;
endfunction

function GetSignPicture(sign)
    case (sign)
        "Constelação do Cuidador": return CUIDADOR_ID;
        "Constelação do Governador": return GOVERNADOR_ID;
        "Constelação do Herói": return HEROI_ID;
        "Constelação do Bobo": return BOBO_ID;
        "Constelação do Amante": return AMANTE_ID;
        "Constelação da Pessoa Comum": return COMUM_ID;
        "Constelação do Criador": return CRIADOR_ID;
        "Constelação do Mago": return MAGO_ID;
        "Constelação do Explorador": return EXPLORADOR_ID;
        "Constelação do Fora da Lei": return FORADALEI_ID;
        "Constelação do Inocente": return INOCENTE_ID;
        "Constelação do Sábio": return SABIO_ID;
        default: return CUIDADOR_ID; // Imagem padrão caso algo dê errado
    endcase
endfunction

function birthDaySelection(who)
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata.birthdate)
        chardata.birthdate := struct{};
    endif
    var selected_day := 0;
    var month_index := getMonthBySign(chardata.sign);
    var btn_value;
    while (!GetObjProperty(who, "creation_completed") && who.connected)
        var input := BirthDayGump(who, selected_day);
        if (!input || input == error)
            sleepms(500);
            continue;
        endif
        btn_value := input[0];
        if (btn_value == 666 && selected_day != 0)
            break;
        elseif (btn_value >= 1 && btn_value <= MONTHS[month_index].days)
            selected_day := btn_value;
        endif
    endwhile
    if (selected_day > 0)
        chardata.birthdate.day := selected_day;
        chardata.birthdate.month := month_index;
        SetObjProperty(who, "chardata", chardata);
        SendSysMessageEx(who, "Seu dia de nascimento será " + selected_day + " de " + MONTHS[month_index].name, SSM_INFO);
    else
        SendSysMessageEx(who, "Nenhum dia selecionado.", SSM_FAIL);
    endif
    return selected_day;
endfunction

function BirthDayGump(who, selected_day := 0)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    
    var chardata := GetObjProperty(who, "chardata");
    var month_index := getMonthBySign(chardata.sign);
    var month := MONTHS[month_index];
    
    // Fundo principal
    GFGumpPic(gump, 17, 31, 0x9E6);
    
    // Fundo do calendário
    GFResizePic(gump, 65, 163, 83, 223, 201);
    
    // Título
    GFHTMLArea(gump, 315, 55, 400, 40, "<BASEFONT Size=55 Color=#ffffff >Escolha seu Dia de Nascimento", 0, 0);
    
    // Nome do mês
    GFTextLine(gump, 115, 179, 53, month.name);
    
    // Adicionar a imagem do signo escolhido
    var sign_pic := GetSignPicture(chardata.sign);
    GFGumpPic(gump, 298, 100, sign_pic);
    
    // Adicionar os dias do mês
    var day := 1;
    var x, y;
    for i := 0 to 5 // Aumentado para 6 linhas para acomodar 33 dias
        for j := 0 to 6
            if (day <= month.days)
                x := 94 + (j * 24);
                y := 207 + (i * 28);
                var background := (day == selected_day) ? 7762 : 7761;
                GFAddButton(gump, x, y, background, background, GF_CLOSE_BTN, day);
                
                var text_x := x + 8;
                if (day <= 9)
                    text_x -= 2;
                elseif (day >= 10 && day <= 19)
                    text_x -= 3;
                elseif (day >= 20 && day != 21 && day != 31)
                    text_x -= 6;
                elseif (day == 21 || day == 31)
                    text_x -= 3;
                endif
                
                GFTextLine(gump, text_x, y + 2, 2100, CStr(day));
                day += 1;
            endif
        endfor
    endfor

    GFHTMLArea(gump, 60, 550, 650, 120, "Escolha o dia do seu nascimento no calendário acima.", 1, 1);
    
    if (selected_day != 0)
        GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 666);
    else
        GFGumpPic(gump, 653, 680, 4005);
    endif

    return GFSendGump(who, gump);
endfunction

function getDayFromPictureId(day_pic)
    return (day_pic - 0x1000) + 1;
endfunction

function hairSelectionGump(who, byref chardata, currentPage := 1)
    var gump := GfCreateGump();

    GFGumpPic(gump, 17, 31, 0x9E6);
    GFResizePic(gump, 298, 100, 2620, 250, 400);
    GFGumpPic(gump, 328, 120, GetPaperdollBackground(who.graphic));
    GFTextMid(gump, 15, 60, 770, 1153, "Selecione o Cabelo");

    UnloadConfigFile(":fls_core:config/hairtypes");
    var config := ReadConfigFile(":fls_core:config/hairtypes");
    var hair := config["todos"];
    var hairStyles := GetConfigStringArray(hair, "Style");
    
    RenderHairStyles(gump, hairStyles, chardata, currentPage);

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 500); // Close button
    return GFSendGump(who, gump);
endfunction

function RenderHairStyles(byref gump, hairStyles, chardata, currentPage)
    const HAIRS_PER_PAGE := 6;
    const HAIRS_PER_ROW := 2;
    
    var totalPages := Ceil(hairStyles.size() / HAIRS_PER_PAGE);
    var start := (currentPage - 1) * HAIRS_PER_PAGE + 1;
    var end := Min(start + HAIRS_PER_PAGE - 1, hairStyles.size());

    var selectedHair := chardata.hair;

    for i := start to end
        var style := SplitWords(hairStyles[i]);
        var hairname := style[1];
        var hairgraphic := CInt(style[2]);
        var gumppic := CInt(style[3]);
        
        var col := (i - start) % HAIRS_PER_ROW;
        var row := (i - start) / HAIRS_PER_ROW;
        var x_pos := 63 + (col * 119);
        var y_pos := 100 + (row * 141);
        
        GFResizePic(gump, x_pos, y_pos, 83, 100, 100);
        GFTextEntry(gump, x_pos + 10, y_pos + 12, 85, 20, 1153, hairname);
        
        //se houver um cabelo selecionado, verifica se é o mesmo cabelo da iteração e se for, usa o botão de checked ao inves de unchecked
        //ou se este for o primeiro item da iteração e não houver cabelo selecionado, então vai marcar o primeiro item de todos que é o sem barba
        if (selectedHair == gumppic || (i == 1 && selectedHair == ""))
            GFAddButton(gump, x_pos + 34, y_pos + 101, 2153, 2151, GF_CLOSE_BTN, hairgraphic);
        else
            GFAddButton(gump, x_pos + 34, y_pos + 101, 2151, 2153, GF_CLOSE_BTN, hairgraphic);
        endif
        
        GFGumpPic(gump, x_pos-40, y_pos-10, gumppic);
    endfor

    if (chardata.hair != "")
        GFGumpPic(gump, 63+265, 120, chardata.hair);
    endif

    // Add navigation buttons
    if (totalPages > 1)
        if (currentPage < totalPages)
            GFAddButton(gump, 185, 520, 0x15E1, 0x15E5, GF_CLOSE_BTN, 1001); // Next page
        endif
        if (currentPage > 1)
            GFAddButton(gump, 125, 520, 0x15E3, 0x15E7, GF_CLOSE_BTN, 1002); // Previous page
        endif
        GFTextMid(gump, 400, 536, 248, 1153, "Página " + currentPage + " de " + totalPages);
    endif

    return totalPages;
endfunction

function beardSelectionGump(who, byref chardata, currentPage := 1)
    var gump := GfCreateGump();

    GFGumpPic(gump, 17, 31, 0x9E6);
    GFResizePic(gump, 298, 100, 2620, 250, 400);
    GFGumpPic(gump, 328, 120, GetPaperdollBackground(who.graphic));
    GFTextMid(gump, 15, 60, 770, 1153, "Selecione a Barba");

    UnloadConfigFile(":fls_core:config/hairtypes");
    var config := ReadConfigFile(":fls_core:config/hairtypes");
    var beardConfig := (chardata.raca == ANAO) ? config["dwarwen_beard"] : config["beard"];
    var beardStyles := GetConfigStringArray(beardConfig, "Style");
    var hair := GetEquipmentByLayer(who, LAYER_HAIR);
    RenderBeardStyles(gump, beardStyles, chardata, currentPage, hair);

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 500); // Close button
    return GFSendGump(who, gump);
endfunction

function RenderBeardStyles(byref gump, beardStyles, chardata, currentPage, hair)
    const BEARDS_PER_PAGE := 6;
    const BEARDS_PER_ROW := 2;
    
    var totalPages := Ceil(beardStyles.size() / BEARDS_PER_PAGE);
    var start := (currentPage - 1) * BEARDS_PER_PAGE + 1;
    var end := Min(start + BEARDS_PER_PAGE - 1, beardStyles.size());

    var selectedBeard := chardata.beard;

    for i := start to end
        var style := SplitWords(beardStyles[i]);
        var beardname := style[1];
        var beardgraphic := CInt(style[2]);
        var gumppic := CInt(style[3]);
        
        var col := (i - start) % BEARDS_PER_ROW;
        var row := (i - start) / BEARDS_PER_ROW;
        var x_pos := 63 + (col * 119);
        var y_pos := 100 + (row * 141); // Ajustado para ficar um pouco abaixo do cabelo
        
        GFResizePic(gump, x_pos, y_pos, 83, 100, 100);
        GFTextEntry(gump, x_pos + 10, y_pos + 12, 85, 20, 1153, beardname);

        //se houver uma barba selecionada, verifica se é a mesma barba da iteração e se for, usa o botão de checked ao inves de unchecked
        //ou se este for o primeiro item da iteração e não houver barba selecionada, então vai marcar o primeiro item de todos que é o sem barba
        if (selectedBeard == gumppic || (i == 1 && selectedBeard == ""))
            GFAddButton(gump, x_pos + 34, y_pos + 101, 2153, 2151, GF_CLOSE_BTN, beardgraphic);
        else
            GFAddButton(gump, x_pos + 34, y_pos + 101, 2151, 2153, GF_CLOSE_BTN, beardgraphic);
        endif

        GFGumpPic(gump, x_pos-40, y_pos-10, gumppic);
    endfor

    if (chardata.beard != "")
        GFGumpPic(gump, 63+265, 120, chardata.beard); // Ajustado para ficar um pouco abaixo do cabelo
    endif

    if (chardata.hair != "")
        GFGumpPic(gump, 63+265, 120, chardata.hair, hair.color-1);
    endif

    // Add navigation buttons
    if (totalPages > 1)
        if (currentPage < totalPages)
            GFAddButton(gump, 185, 520, 0x15E1, 0x15E5, GF_CLOSE_BTN, 1001); // Next page
        endif
        if (currentPage > 1)
            GFAddButton(gump, 125, 520, 0x15E3, 0x15E7, GF_CLOSE_BTN, 1002); // Previous page
        endif
        GFTextMid(gump, 400, 536, 248, 1153, "Página " + currentPage + " de " + totalPages);
    endif

    return totalPages;
endfunction

function GetBeardGumpGraphic(beardID)
    case (beardID)
        0: return 0; // Sem barba
        8268: return 50905; // Long_Beard_&_Must.
        8254: return 50801; // Long_Beard
        8267: return 50904; // Short_Beard_&_Must.
        8255: return 50802; // Short_Beard
        8269: return 50906; // Vandyke
        8256: return 50800; // Goatee
        8257: return 50803; // Mustache
        41384: return 51451; // Lumberjack
        41385: return 51452; // Handlebar
        41386: return 51453; // Braids
        41387: return 51454; // Costeletas
        51583: return 52129; // BarbaLongaMalFeita
        51584: return 52130; // BarbaMuitoLongaEmaranhada
        51585: return 52131; // BarbaLongaEncaracolada
        51586: return 52132; // BarbaMuitoLonga
        51587: return 52133; // BarbaLongaPontuda
        51588: return 52134; // BarbaRabodeCavalo
        51589: return 52135; // BarbaLongaGrandeTranca
        51627: return 52174; // BigodeLongo
        51628: return 52174; // BigodeCurto
        // Adicione aqui os casos para as barbas anãs
        51590: return 52136; // BarbaMultiplasTrancasEnfeitadas
        51591: return 52137; // BarbaTrancadaBipartida
        51592: return 52138; // BarbaMuitoComprida
        51593: return 52139; // BarbaTrancadaTripartida
        51594: return 52140; // BarbaVolumosa
        51595: return 52141; // BarbaCompridaBaguncada
        51596: return 52142; // BarbaCompridaAtada
        51597: return 52143; // BarbaMultiplasTrancas
        51598: return 52144; // BarbaVolumosaBipartida
        51599: return 52145; // BarbaLongaCheia
        51600: return 52146; // BarbaLongaFrizada
        51601: return 52147; // BarbaLongaPontuda
        51602: return 52148; // BarbaEnfeitadaRevoltosa
        51603: return 52149; // BarbaVolumosaComTrancas
        51604: return 52150; // BarbaComPontasEnfeitadas
        51605: return 52151; // BarbaCompridaAparada
        51606: return 52152; // BarbaMediaAparada
        51607: return 52153; // BarbaArticulada
        51608: return 52154; // BarbichaTrancada
        51609: return 52155; // BarbichaDupla
        51610: return 52156; // BigodeCurvado
        51611: return 52157; // BarbaMediaAparada
        51612: return 52158; // BarbaMediaSemBigode
        51613: return 52159; // BarbaBigodeRevoltoso
        51614: return 52160; // BarbaTrancadaBipartidaSobrancelhas
        51615: return 52161; // BarbaMediaSobrancelhas
        51616: return 52162; // BarbichaTrancaLongaEnfeitada
        51617: return 52163; // BarbichaBipartidaEnfeitada
        51618: return 52164; // BarbaMediaSemBigode
        51619: return 52165; // BarbaBigodeRevoltoso
        51620: return 52166; // BarbaTrancadaBipartidaSobrancelhas
        51621: return 52167; // BarbaMediaSobrancelhas
        51622: return 52168; // BarbichaTrancaLongaEnfeitada
        51623: return 52169; // BarbichaBipartidaEnfeitada
        default: return 50800; // Gráfico padrão (Goatee)
    endcase
endfunction

function GetPaperdollBackground(graphic)
    var result := 0x0E10; // Valor padrão
    case (graphic)
        400:  result := 0xC; // Male
        401:  result := 0xD; // Female
        1828: result := 54562; // Anão F
        1829: result := 54563; // Anão M
        1830: result := 0xE; // Elfo M
        1831: result := 0xF; // Elfo F
        1832: result := 54596; // Orc M
        1833: result := 54595; // Orc F
        1834: result := 54533; // Pequilho F
        1835: result := 54534; // Pequilho M
    endcase
    return result;
endfunction

function GetHairGumpGraphic(hairID)
    case (hairID)
        0: return 0; // Careca
        8252: return 50701; // Long_Hair
        8265: return 50902; // Braids
        8253: return 50702; // Pony_Tail
        8263: return 50900; // Afro
        8261: return 50710; // Pageboy
        8262: return 50712; // Buns
        8251: return 50700; // Short_Hair
        8266: return 50903; // Top_Knot
        8264: return 50901; // Receeding
        8260: return 50703; // Mohawk
        12223: return 50916; // Mid_Long
        12224: return 50917; // Long_Feather
        12225: return 50918; // Short
        12226: return 50919; // Mullet
        12236: return 50890; // Flower
        12238: return 50892; // Big_Knob
        12239: return 50893; // Big_Braid
        12240: return 50894; // Big_Bun
        12241: return 50895; // Spiked
        41380: return 51447; // Bedhead
        41381: return 50900; // Afro (duplicado, usando o mesmo gráfico)
        41382: return 51449; // Monge
        41383: return 51450; // Topete
        41388: return 51455; // LongBraid
        41389: return 51456; // Dread
        41390: return 51457; // Pigtail
        41391: return 51458; // Leia
        41392: return 51459; // Franja
        41393: return 51460; // Spikes
        41394: return 51461; // Longo2
        41395: return 51462; // Longo3
        41396: return 51463; // Side
        41397: return 51464; // Wavy
        41398: return 51465; // Braids2
        41399: return 51466; // NeckWrap
        51629: return 52175; // MoicanoDeLado
        51630: return 52176; // MoicanoAlto
        51631: return 52177; // MoicanoLongo
        51632: return 52178; // TopknotForte
        51633: return 52179; // cabeloplantalongo
        51634: return 52180; // cabeloplantacurto
        51635: return 52181; // cabeloplantagalho
        51636: return 52182; // calvaocrialongo
        51637: return 52183; // calvaocriacurto
        51638: return 52184; // cabeloplantachifverd
        51639: return 54274; // CoqueLongo
        51640: return 52186; // CoqueCurto
        51641: return 54275; // CoqueLivre
        51642: return 54276; // CoqueTrabalhado
        51643: return 54277; // CoqueLateral1
        51644: return 54278; // CoqueLateral2
        51645: return 54279; // CoqueSamurai1
        51646: return 54280; // CoqueSamurai2
        51647: return 54281; // CoqueSamurai3
        51648: return 54282; // CoqueSamurai4
        51649: return 54283; // coque1
        51650: return 54284; // coque2
        51651: return 54285; // coque3
        51652: return 54286; // coque4
        51653: return 54287; // coque5
        51654: return 54288; // coque6
        51655: return 54289; // coque7
        51656: return 54290; // coque8
        51657: return 54291; // coque9
        51658: return 54292; // coque10
        51659: return 54293; // coque11
        51660: return 54294; // CoqueLongo2
        51661: return 54295; // CoqueLongo3
        51662: return 54296; // Coquelongo4
        51663: return 54297; // CoqueLongo5
        51664: return 54298; // CoqueLongo6
        51665: return 54299; // CoqueLongo7
        51666: return 54300; // CoqueLongo8
        51667: return 54301; // CoqueLongo9
        default: return 50700; // Gráfico padrão (Short_Hair)
    endcase
endfunction

function hairAndBeardColorSelectionGump(who, byref chardata, currentPage := 1)
    var gump := GFCreateGump();
    GFPage(gump, 0);

    var beard := GetEquipmentByLayer(who, LAYER_BEARD);
    var hair := GetEquipmentByLayer(who, LAYER_HAIR);
    GFGumpPic(gump, 17, 31, 0x9E6);
    GFResizePic(gump, 298, 100, 2620, 250, 400);

    // Monta o paperdoll
    GFGumpPic(gump, 328, 120, GetPaperdollBackground(who.graphic));
    if (beard)
        GFGumpPic(gump, 63+265, 120, chardata.beard, beard.color-1);
    endif
    if (hair)
        GFGumpPic(gump, 63+265, 120, chardata.hair, hair.color-1);
    endif
    GFTextMid(gump, 15, 60, 770, 1153, "Selecione a Cor do Cabelo e Barba");

    var color_cfg := ReadConfigFile(":hair:config/hairColors");
    var index_elem := GetColorIndexElem(color_cfg, chardata);
    var categories := GetConfigStringArray(index_elem, "Category");

    const COLORS_PER_PAGE := 15;
    var totalColors := 0;
    var allColors := array{};

    foreach category in categories
        var category_elem := color_cfg[category];
        var colors := GetConfigStringArray(category_elem, "Color");
        foreach color in colors
            allColors.append(struct{ "color" := color, "category" := category });
        endforeach
    endforeach

    totalColors := allColors.size();
    var totalPages := Ceil(totalColors / COLORS_PER_PAGE);

    var start := (currentPage - 1) * COLORS_PER_PAGE + 1;
    var end := Min(start + COLORS_PER_PAGE - 1, totalColors);

    var selectedColor := (chardata.hair_item) ? chardata.hair_color : chardata.beard_color;

    var y_pos := 100;
    for i := start to end
        var colorData := allColors[i];
        var color := CInt(colorData.color);
        var category := colorData.category;
        
        //se houver uma cor selecionada, então dá check o botão correto na iteração
        if (selectedColor == color)
            GFAddButton(gump, 257, y_pos, 0x9C4F, 0x9C4E, GF_CLOSE_BTN, color);
        else
            GFAddButton(gump, 257, y_pos, 0x9C4E, 0x9C4F, GF_CLOSE_BTN, color);
        endif
        GFHTMLArea(gump, 135, y_pos + 2, 122, 20, "", 1, 0);
        GFTextLine(gump, 142, y_pos + 4, color-1, category); //hue de cor tem que ter -1
        y_pos += 20;
    endfor

    // Add navigation buttons
    if (totalPages > 1)
        if (currentPage < totalPages)
            GFAddButton(gump, 226, 410, 0x15E1, 0x15E5, GF_CLOSE_BTN, 1001); // Next page
        endif
        if (currentPage > 1)
            GFAddButton(gump, 135, 410, 0x15E3, 0x15E7, GF_CLOSE_BTN, 1002); // Previous page
        endif
        GFTextMid(gump, 400, 536, 248, 1153, "Página " + currentPage + " de " + totalPages);
    endif

    // Adicionar botão de fechar
    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 1003);

    return GFSendGump(who, gump);
endfunction

function GetColorIndexElem(color_cfg, chardata)
    var cultura := chardata.culture;
    var raca := chardata.raca;

    if (cultura == "Björska")
        return color_cfg["Bjorska"];
    elseif (raca == "Orc")
        return color_cfg["Orc"];
    elseif (raca == "Drow")
        return color_cfg["Drow"];
    elseif (raca == "Elfo")
        return color_cfg["Elfo"];
    elseif (raca == "Anão")
        return color_cfg["Anao"];
    elseif (raca == "Qualquer")
        return color_cfg["Index"];
    else
        return color_cfg["Imperial"];
    endif
endfunction

function UpdateCharacterHair(who, chardata)
    if (chardata.hair_item)
        var hair := GetEquipmentByLayer(who, LAYER_HAIR);
        if (hair)
            hair.color := chardata.hair_color;
        else
            var new_hair := CreateItemAtLocation(who.x, who.y, who.z, chardata.hair_item, 1, who.realm);
            new_hair.color := chardata.hair_color;
            EquipItem(who, new_hair);
        endif
    endif

    if (chardata.beard_item)
        var beard := GetEquipmentByLayer(who, LAYER_BEARD);
        if (beard)
            beard.color := chardata.beard_color;
        else
            var new_beard := CreateItemAtLocation(who.x, who.y, who.z, chardata.beard_item, 1, who.realm);
            new_beard.color := chardata.beard_color;
            EquipItem(who, new_beard);
        endif
    endif
endfunction

function skinColorSelectionGump(who, byref chardata, currentPage := 1)
    var gump := GFCreateGump();
    GFPage(gump, 0);

    var beard := GetEquipmentByLayer(who, LAYER_BEARD);
    var hair := GetEquipmentByLayer(who, LAYER_HAIR);

    GFGumpPic(gump, 17, 31, 0x9E6);
    GFResizePic(gump, 298, 100, 2620, 250, 400);

    // Monta o paperdoll
    GFGumpPic(gump, 328, 120, GetPaperdollBackground(who.graphic), chardata.skin_color);
    if (beard)
        GFGumpPic(gump, 63+265, 120, chardata.beard, beard.color-1);
    endif
    if (hair)
        GFGumpPic(gump, 63+265, 120, chardata.hair, hair.color-1);
    endif
    GFTextMid(gump, 15, 60, 770, 1153, "Selecione a Cor da Pele");

    var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
    if (!skin_cfg)
        SendSysMessage(who, "Error: Could not open :fls_core:config/skincolors.cfg -> "+skin_cfg.errortext);
        return 0;
    endif

    var raca := chardata.raca;
    if (raca == ELFO)
        skin_cfg := skin_cfg["elfo"];
    elseif (raca == DROW)
        skin_cfg := skin_cfg["drow"];
    elseif (raca == ORC)
        skin_cfg := skin_cfg["orc"];
    else
        skin_cfg := skin_cfg["list"];
    endif

    var allColors := GetConfigStringArray(skin_cfg, "color");

    var totalColors := allColors.size();
    var totalPages := Ceil(totalColors / COLORS_PER_PAGE);

    var start := (currentPage - 1) * COLORS_PER_PAGE + 1;
    var end := Min(start + COLORS_PER_PAGE, totalColors);
    var skinColor := chardata.skin_color;
    
    var y_pos := 100;
    for i := start to end
        var color := CInt(allColors[i]);

        //se houver uma cor selecionada, então dá check o botão correto na iteração
        if (skinColor == color)
            GFAddButton(gump, 257, y_pos, 0x9C4F, 0x9C4E, GF_CLOSE_BTN, color);
        else
            GFAddButton(gump, 257, y_pos, 0x9C4E, 0x9C4F, GF_CLOSE_BTN, color);
        endif
        GFHTMLArea(gump, 135, y_pos + 2, 122, 20, "", 1, 0);
        GFTextLine(gump, 142, y_pos + 4, color - 1, "Cor " + i); //hue de cor tem que ter -1
        y_pos += 20;
    endfor

    // Add navigation buttons
    if (totalPages > 1)
        if (currentPage < totalPages)
            GFAddButton(gump, 226, 440, 0x15E1, 0x15E5, GF_CLOSE_BTN, 1); // Next page
        endif
        if (currentPage > 1)
            GFAddButton(gump, 135, 440, 0x15E3, 0x15E7, GF_CLOSE_BTN, 2); // Previous page
        endif
        GFTextMid(gump, 400, 536, 248, 1153, "Página " + currentPage + " de " + totalPages);
    endif

    // Adicionar botão de fechar
    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 500);

    return GFSendGump(who, gump);
endfunction

function UpdateCharacterSkin(who, chardata)
    who.color := chardata.skin_color;
    who.truecolor := chardata.skin_color;
endfunction

function spellChoices(who)
    // Get spells by circle from all 4 mage school configs
    var spellsByCircle := dictionary;
    spellsByCircle[1] := array;
    spellsByCircle[2] := array;

    foreach cfg_path in (GetMageSchoolConfigPaths())
        var spellcfg := ReadConfigFile(cfg_path);
        if (!spellcfg)
            continue;
        endif
        var keys := GetConfigIntKeys(spellcfg);
        foreach key in keys
            var elem := FindConfigElem(spellcfg, key);
            var spelllvl := GetConfigInt(elem, "Circle");

            if (spelllvl == 1 || spelllvl == 2)
                var spell := dictionary;
                spell["id"] := key;
                spell["circle"] := spelllvl;
                spell["name"] := GetConfigString(elem, "Name");
                spell["desc"] := GetConfigString(elem, "Desc");
                spell["school"] := GetConfigString(elem, "School");
                spellsByCircle[spelllvl].append(spell);
            endif
        endforeach
    endforeach

    var selected_spells := array;
    var btn_value;
    var total_amount := 3;
    
    while (!GetObjProperty(who, "creation_completed") && who.connected)
        var input := spellSelectionGump(who, spellsByCircle, selected_spells, total_amount);
        if (!input || input == error)
            sleepms(500);
            continue;
        endif

        foreach key in (input.keys)
            btn_value := key;
            if (btn_value > 200 && btn_value <= 200 + spellsByCircle[1].size())
                var spell := spellsByCircle[1][btn_value-200];
                selected_spells.append(spell);
            elseif (btn_value > 300 && btn_value <= 300 + spellsByCircle[2].size())
                var spell := spellsByCircle[2][btn_value-300];
                selected_spells.append(spell);
            endif
        endforeach

        // Validate selection: always 3 spells, max 1 from circle 2
        var circle2_count := 0;
        foreach chosen_spell in (selected_spells)
            if (chosen_spell.circle == 2)
                circle2_count += 1;
            endif
        endforeach

        if (selected_spells.size() != total_amount)
            selected_spells := array;
            SendSysMessageEx(who, "Você deve escolher exatamente " + total_amount + " magias.", SSM_FAIL);
        elseif (circle2_count > 1)
            selected_spells := array;
            SendSysMessageEx(who, "Você pode escolher no máximo 1 magia do círculo 2.", SSM_FAIL);
        else
            break;
        endif
    endwhile

    SendSysMessageEX(who, "Você escolheu as magias: " + selected_spells, SSM_INFO);
    return selected_spells;
endfunction

function spellSelectionGump(who, spellsByCircle, selected_spells, total_amount)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    
    var x := 63;
    var y := 100;
    
    GFGumpPic(gump, 17, 31, 0x9E6);
    
    // Circle 1 header
    GFHTMLArea(gump, x, y, 192, 24, "<BASEFONT color=#6666FF>Magias do Círculo 1:", 1, 0);
    y += 26;
    
    // Circle 1 spells
    var i := 1;
    foreach spell in (spellsByCircle[1])
        var color := spell.id in (selected_spells) ? "#00FF00" : "#000000";
        GFHTMLArea(gump, x, y, 192, 24, "<BASEFONT color=" + color + ">" + spell.name, 1, 0);
        GFTooltipText(gump, spell.desc);
        if (!(spell.id in selected_spells) && (selected_spells.size() < total_amount))
            GFCheckBox(gump, x+201, y, 0x9C4E, 0x9C4F, 0, 200+i);
        endif
        y += 26;
        i += 1;
    endforeach
    
    // Circle 2 spells
    x += 300;
    y := 50;
    GFHTMLArea(gump, x, y, 192, 24, "<BASEFONT color=#6666FF>Magias do Círculo 2:", 1, 0);
    y += 26;

    i := 1;
    foreach spell in (spellsByCircle[2])
        var color := spell.id in (selected_spells) ? "#00FF00" : "#000000";
        GFHTMLArea(gump, x, y, 192, 24, "<BASEFONT color=" + color + ">" + spell.name, 1, 0);
        GFTooltipText(gump, spell.desc);
        if (!(spell.id in selected_spells) && (selected_spells.size() < total_amount))
            GFCheckBox(gump, x+201, y, 0x9C4E, 0x9C4F, 0, 300+i);
        endif
        y += 26;
        i += 1;
    endforeach
    
    var remaining := total_amount - selected_spells.size();
    GFHTMLArea(gump, 115, 645, 380, 20, "<BASEFONT Size=3 Color=#999999>Total: 3 magias (máx. 1 do círculo 2)", 0, 0);
    GFHTMLArea(gump, 116, 646, 380, 20, "<BASEFONT Size=3 Color=#ffffff>Total: 3 magias (máx. 1 do círculo 2)", 0, 0);
    GFHTMLArea(gump, 115, 665, 192, 40, "<BASEFONT Size=5 Color=#999999>Escolha " + remaining + " magia(s)", 0, 0);
    GFHTMLArea(gump, 116, 666, 192, 40, "<BASEFONT Size=5 Color=#ffffff>Escolha " + remaining + " magia(s)", 0, 0);
    
    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 666);
    return GFSendGump(who, gump);
endfunction

function AlchChoices(who)
    var gump := GFCreateGump();
    GFClosable(gump, 0);
    var x := 63;
    var y := 100;
    GFGumpPic(gump, 17, 31, 0x9E6);

    GFHTMLArea(gump, x+21, y-24, 192, 40, "<BASEFONT Size=55 Color=#999999 > "+"Escolha suas receitas", 0, 0); //shadow
    GFHTMLArea(gump, x+20, y-25, 192, 40, "<BASEFONT Size=55 Color=#ffffff > "+"Escolha suas receitas", 0, 0);

    var recipecfg := ReadConfigFile(":alchemy:config/recipes");
    var keys := GetConfigIntKeys(recipecfg);
    var recipes := array;
    var skill := getobjproperty(who, "chardata").skills[ALCHEMY];
    foreach key in keys
        var elem := FindConfigElem(recipecfg, key);
        var recipelvl := GetConfigInt(elem, "init");
            if (!recipelvl || recipelvl == error);
                recipelvl := 4;
            endif
        if (recipelvl <= skill)
            var recipe := dictionary{ 
                "id" -> key,
                "name" -> GetConfigString(elem, "Type"),
                "desc" -> GetConfigString(elem, "Desc")
            };
            recipes.append(recipe);
        endif
    endforeach

    var selected_recipes := array;
    var btn_value;
    while (!GetObjProperty(who, "creation_completed") && who.connected)
        var qtd := (skill*2)+2;
        var input := alchSelectionGump(who, gump, recipes, selected_recipes, qtd, x, y);
        if (!input || input == error)
            sleepms(500);
            continue;
        endif

        foreach key in (input.keys)
            btn_value := key;
            if (btn_value > 200 && btn_value <= 200 + recipes.size())
                var recipe_index := btn_value - 200;
                var recipe_id := recipes[recipe_index].id;
                if (recipe_id in selected_recipes)
                    selected_recipes.Erase(Find(selected_recipes, recipe_id, 1));
                else
                    selected_recipes.append(recipe_id);
                endif
            endif
        endforeach
        if (selected_recipes.size() != qtd)
            selected_recipes := array;
            SendSysMessageEx(who, "Você deve escolher exatamente " + cstr(qtd) + " receitas.", SSM_FAIL);
        else
            break;
        endif
    endwhile

    SendSysMessageEX(who, "Você escolheu as receitas: " + CStr(selected_recipes), SSM_INFO);
    return selected_recipes;
endfunction

function alchSelectionGump(who, gump, alchs, selected_alchs, qtd, x, y)
    GFPage(gump, 1);
    y := 100;
    foreach alch in alchs
        var color := (alch.id in selected_alchs) ? "#00FF00" : "#000000";
        GFHTMLArea(gump, x, y, 192, 24, "<BASEFONT color=#{}>{}".format(color, alch.name), 1, 0);
        GFTooltipText(gump, alch.desc);
        GFCheckBox(gump, x+201, y, 0x9C4E, 0x9C4F, (alch.id in selected_alchs), 200+_alch_iter);
        y += 26;
    endforeach

    GFHTMLArea(gump, 345, 515, 192, 40, "<BASEFONT Size=55 Color=#999999 > Escolha " +cstr(qtd)+ "opções", 0, 0);
    GFHTMLArea(gump, 346, 516, 192, 40, "<BASEFONT Size=55 Color=#ffffff > Escolha " +cstr(qtd)+ " opções", 0, 0); //shadow

    GFAddButton(gump, 653, 680, 4005, 4007, GF_CLOSE_BTN, 666); // Botão de fechar
    return GFSendGump(who, gump);
endfunction

function GetSkillTooltipDescriptionFromName(name)
	var cfg := AP_GetAttributesCfgFile();
	var elem := FindConfigElem(cfg, name);
	if (elem == error)
		return "Descrição não encontrada para: " + name;
	endif

	var tooltip := GetConfigString(elem, "TooltipDesc");
	if (!tooltip || tooltip == error)
		return "Habilidade: " + name;
	endif

	return tooltip;
endfunction

function CreateItemInicial(who, objtype, qtd)
    var item :=  GeraItemComplexo(
                                    who.backpack,
                                    objtype,
                                    qtd,
                                    RandomIntMinMax(1,10) > 7 ? "1>2" : 1,
                                    1, 
                                    array {0x630e}, //exclui ouro
                                    FABRICACAO_MODO_MATERIAL::MENOR_QUE_TIER
                                );

    if(!item)
        SendSysMessageEX(who, "Erro ao criar item inicial {} - {}".format(Hex(objtype), item), SSM_FAIL);
    endif
endfunction

function displayRacialBonus(raca)
    var final_str := 0;
    var final_dex := 0;
    var final_int := 0;
    var meio := "Meio-";
    var desc := "";

    if (raca == ANAO)
        final_str += 10;
    elseif (raca == ELFO || raca == meio+ELFO)
        final_int += 10;
    elseif (raca == DROW || raca == meio+DROW)
        final_dex += 5;
        final_int += 5;
    elseif (raca == ORC || raca == meio+ORC)
        final_str += 5;
        final_int += 5;
    elseif (raca == PEQUILHO)
        final_dex += 10;
    else // Humanos
        desc := "Humanos recebem +2 pontos adicionais para comprar skills.";
    endif

    var bonus_str := "";
    if (final_str > 0)
        bonus_str += "+STR: " + final_str + " ";
    endif
    if (final_dex > 0)
        bonus_str += "+DEX: " + final_dex + " ";
    endif
    if (final_int > 0)
        bonus_str += "+INT: " + final_int + " ";
    endif

    if (desc != "")
        return desc;
    elseif (bonus_str != "")
        return "Bônus racial: " + bonus_str;
    else
        return "Sem bônus racial.";
    endif
endfunction

function displayAttributeDescription(chardata, attribute)
    // calcula o valor e divide por /20 pra dar o dano. e.g. : attribute / 20 a mais de dano
    if (attribute == "Forca")
        attribute := chardata.str;
        return "Aumenta o dano corpo-a-corpo em " + Cint(attribute / 20) + " pontos. Também aumenta a capacidade de carga. E vida";
    elseif (attribute == "Destreza")
        attribute := chardata.dex;
        return "Aumenta a velocidade e dano para armas de longo alcance em " + Cint(attribute / 20) + " pontos. Também aumenta a stamina.";
    elseif (attribute == "Inteligencia")
        attribute := chardata.int;
        return "Aumenta o dano e alcance de magias em " + Cint(attribute / 20) + " pontos. Também aumenta o mana.";
    endif
endfunction