const ACHIEVEMENT := "ACHIEVEMENT";
// OTHERS
// TODO: Peixes pescados
// TODO: Peixes pescados com isca
// TODO: Quantas magias castadas 
// TODO: Qual magia mais castada
// TODO: Quanto recurso recolhido
// TODO: Quantos corpos abertos
// TODO: Quantos mobs rastreados
// TODO: Quantos mobs descobertos com awareness
// TODO: Quantos boss mortos
// TODO: Qual tier mais alto voce ja matou/ajudou a matar
// TODO: Top ganho de XP num dia
// TODO: Quantas vezes ja se disfarçou
// TODO: Quantos golpes acertados consecutivos sem errar em criaturas red
// TODO: Quantos aliados voce ja curou
// TODO: Quanto voce ja curou seus aliados
// TODO: Quantas vezes utilizou suas habilidades
// TODO: Qual habilidade mais utilizada
class Achievement()
	var playerRef; // Readonly
	var criteriaRef; // Readonly
	var name; // Readonly

	var level;
	var xp;

	function Achievement(this, playerRef, criteriaRef, name := false, level := 0, xp := 0)
		this.playerRef := playerRef;
		this.criteriaRef := criteriaRef;
		this.name := !name ? CStr(criteriaRef) : CStr(name);

		this.level := CInt(level);
		this.xp := CInt(xp);
	endfunction

	function GetData(this, key) endfunction
	function GetDataKeys(this) endfunction
	function SetData(this, key, value) endfunction

	function GetPlayerRef(this)
		return this.playerRef;
	endfunction

	function GetCriteriaRef(this)
		return this.criteriaRef;
	endfunction

	function GetName(this)
		return this.name;
	endfunction

	function GetLevel(this)
		return this.level;
	endfunction

	function GetXP(this)
		return this.xp;
	endfunction

	function _SetName(this, value) // Feito com _ porque SetName é reservado ao compilador
		this.name := value;
	endfunction

	function SetLevel(this, value)
		this.level := value;
	endfunction

	function SetXP(this, value)
		this.xp := value;
	endfunction

	function IncrementXP(this, value)
		this.xp += CInt(value);
	endfunction

	function IncrementLevel(this)
		this.level++;
	endfunction

	function IncrementData(this, key, value) endfunction

	function Serialize(this)
		return struct{
			playerRef := this.playerRef,
			criteriaRef := this.criteriaRef,
			name := this.name,

			level := this.level,
			xp := this.xp
		};
	endfunction

	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif
		this.playerRef := data.playerRef;
		this.criteriaRef := data.criteriaRef;
		this.name := CStr(data.name);

		this.level := CInt(data.level);
		this.xp := CInt(data.xp);
	endfunction

	function toString(this)
		return $"Achievement para {this.name} (playerRef={this.playerRef}, criteriaRef={this.criteriaRef}, xp={this.xp}, level={this.level})";
	endfunction
endclass