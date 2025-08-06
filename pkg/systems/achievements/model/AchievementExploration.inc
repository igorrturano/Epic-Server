include ":achievements:model/Achievement";

const ACHIEVEMENT_EXPLORATION := ACHIEVEMENT + "_EXPLORATION";

// TODO: Dungeons exploradas
// TODO: Itens de lore encontrados
// TODO: Quantos baús abertos
// TODO: Quantas armadilhas desativadas
// TODO: Quantos máximos passos em sneak consecutivo
// TODO: Dungeons completadas
// TODO: Bosses únicos derrotados
// TODO: Áreas secretas descobertas
// TODO: NPCs únicos encontrados

class AchievementExploration(Achievement)
	var locations;
	var discoveries;
	var distance;

	function AchievementExploration(this, key)
		Super(key);
		this.type := ACHIEVEMENT_EXPLORATION;
		this.locations := array{};
		this.discoveries := 0;
		this.distance := 0;
	endfunction

	function getLocations(this)
		return this.locations;
	endfunction

	function getDiscoveries(this)
		return this.discoveries;
	endfunction

	function getDistance(this)
		return this.distance;
	endfunction

	function setLocations(this, value)
		this.locations := value;
	endfunction

	function setDiscoveries(this, value)
		this.discoveries := value;
	endfunction

	function setDistance(this, value)
		this.distance := value;
	endfunction

	function AddLocations(this, value)
		this.locations.append(value);
	endfunction

	function AddDiscoveries(this, value)
		this.discoveries += CInt(value);
	endfunction

	function AddDistance(this, value)
		this.distance += CInt(value);
	endfunction

	function Serialize(this)
		var base_data := Achievement::Serialize(this);
		base_data.+locations := this.locations;
		base_data.+discoveries := this.discoveries;
		base_data.+distance := this.distance;

		return base_data;
	endfunction

	function Deserialize(this, data)
		if (!data) return error{ errortext := "Invalid data for Achievement deserialization." }; endif
		Achievement::Deserialize(this, data);
		this.locations := CInt(data.locations);
		this.discoveries := CInt(data.discoveries);
		this.distance := CInt(data.distance);
	endfunction

	function toString(this)
		return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level}, locations={this.locations}, discoveries={this.discoveries}, distance={this.distance})";
	endfunction
endclass