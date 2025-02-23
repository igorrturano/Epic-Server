//Author: Deiro || Rafael Santos

function PegaComponenteTierDesc(tier)
	case (tier)
		1 : return struct {desc := "Mediocre" , valor := -1}; break;
		2 : return struct {desc := "Comum" , valor := 0}; break;
		3 : return struct {desc := "Notável" , valor := 1}; break;
		4 : return struct {desc := "Superior" , valor := 2}; break;
		5 : return struct {desc := "Excepcional" , valor := 3}; break;
		default:
			return struct {desc := "Erro!" , valor := 0}; break;
	endcase
endfunction