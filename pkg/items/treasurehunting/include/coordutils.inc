use uo;
use math;

/*
 * ToSextant(x, y)
 * Converte coordenadas x,y do jogo para formato de coordenadas náuticas
 * Ex: 30o 10'N, 45o 20'W
 */
function ToSextant(x, y)
    var latitude_direction, longitude_direction;
    var latitude_degrees, longitude_degrees;
    var latitude_minutes, longitude_minutes;
    
    latitude_degrees := (y-1624)*0.087890625;
    longitude_degrees := (x-1323)*0.0703125;
    
    if (latitude_degrees < 0)
        latitude_direction := "N";
    else
        latitude_direction := "S";
    endif
    
    if (longitude_degrees < 0)
        longitude_direction := "W";
    else
        longitude_direction := "E";
    endif
    
    latitude_degrees := Abs(latitude_degrees);
    longitude_degrees := Abs(longitude_degrees);
    latitude_minutes := CInt((CInt((latitude_degrees-Floor(latitude_degrees))*100)*60)/100);
    longitude_minutes := CInt((CInt((longitude_degrees-Floor(longitude_degrees))*100)*60)/100);
    
    return (Floor(latitude_degrees) + "o " + latitude_minutes + "'" + latitude_direction + ", " + 
            Floor(longitude_degrees) + "o " + longitude_minutes + "'" + longitude_direction);
endfunction

/*
 * FromSextant(coord_string)
 * Converte string de coordenadas náuticas para x,y do jogo
 * Ex: "30o 10'N, 45o 20'W" -> struct{"x" := X, "y" := Y}
 */
function FromSextant(coord_string)
    // TODO: Implementar conversão inversa se necessário
    return struct{
        "x" := 0,
        "y" := 0
    };
endfunction

/*
 * FormatCoord(x, y)
 * Retorna coordenadas formatadas para display
 */
function FormatCoord(x, y)
    return "*" + x + "," + y + "* (" + ToSextant(x, y) + ")";
endfunction

/*
 * ValidateCoordinates(x, y, realm := "britannia")
 * Verifica se as coordenadas são válidas para o realm
 */
function ValidateCoordinates(x, y, realm := "britannia")
    case (realm)
        "britannia":
            if (x > 5120 && y < 2304)
                return 0; // Coordenadas inválidas
            endif
            if (x < 0 || y < 0 || x > 6144 || y > 4096)
                return 0;
            endif
            return 1;
            
        default:
            return 0;
    endcase
endfunction