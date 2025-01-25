use uo;
use os;

include ":quedadebraco:leaderboard";

// Estrutura dos títulos e pontos necessários
var titles_config := array{
    struct{ points := 0,    title := "Novato" },
    struct{ points := 50,   title := "Amador" },
    struct{ points := 150,  title := "Profissional" },
    struct{ points := 300,  title := "Veterano" },
    struct{ points := 500,  title := "Mestre" },
    struct{ points := 1000, title := "Grande Mestre" },
    struct{ points := 2000, title := "Lenda" }
};

// Obtém próximo título e pontos necessários
function GetNextTitle(current_points)
    foreach title_data in titles_config
        if (current_points < title_data.points)
            return struct{
                title := title_data.title,
                points_needed := title_data.points - current_points
            };
        endif
        SleepMS(2);
    endforeach
    
    return 0; // Já está no título máximo
endfunction

// Cria gump mostrando progresso do título
function ShowTitleProgress(who)
    var df := GetLeaderboardDataFile();
    var player_data := GetPlayerData(df, who);
    var next_title := GetNextTitle(player_data.points);
    
    var gump := GFCreateGump();
    GFResizePic(gump, 0, 0, 9200, 400, 300);
    GFTextMid(gump, 10, 20, 380, 2100, "Progresso de Títulos");
    
    GFTextLine(gump, 20, 50, 2100, "Título Atual: " + player_data.title);
    GFTextLine(gump, 20, 70, 2100, "Vitórias: " + player_data.wins);
    GFTextLine(gump, 20, 90, 2100, "Derrotas: " + player_data.losses);
    GFTextLine(gump, 20, 110, 2100, "Pontos: " + player_data.points);
    
    if (next_title)
        GFTextLine(gump, 20, 140, 2100, "Próximo Título: " + next_title.title);
        GFTextLine(gump, 20, 160, 2100, "Pontos Necessários: " + next_title.points_needed);
    else
        GFTextLine(gump, 20, 140, 2100, "Título Máximo Alcançado!");
    endif
    
    GFAddButton(gump, 150, 250, 2128, 2129, GF_CLOSE_BTN, 1); 
    GFTextLine(gump, 185, 250, 2100, "Fechar");
    
    return GFSendGump(who, gump);
endfunction

// Função auxiliar para exibir uma lista de todos os títulos disponíveis
function ListAllTitles()
    var list := array{};
    foreach title_config in titles_config
        list.append(struct{
            title := title_config.title,
            points := title_config.points
        });
        sleepms(2);
    endforeach
    return list;
endfunction