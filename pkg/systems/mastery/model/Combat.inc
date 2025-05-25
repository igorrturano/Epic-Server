include ":mastery:model/Mastery";

const MASTERY_COMBAT := MASTERY + "_COMBAT";

class Combat(Mastery)
    var hits;
    var kills;
	var damage;

    function Combat(this, name, level, xp, hits := 0, kills := 0)
        this.Mastery(name, level, xp);
        this.type := MASTERY_COMBAT;

        this.hits := CInt(hits);
        this.kills := CInt(kills);
    endfunction

    function getHits(this)
        return this.hits;
    endfunction

    function getKills(this)
        return this.kills;
    endfunction

	function getDamage(this)
		return this.damage;
	endfunction

    function setHits(this, value)
        this.hits := CInt(value);
    endfunction

    function setKills(this, value)
        this.kills := CInt(value);
    endfunction

	function setDamage(this, value)
		this.damage := CInt(value);
	endfunction

    function AddHits(this, value)
        this.hits += CInt(value);
    endfunction
    
    function AddKills(this, value)
        this.kills += CInt(value);
    endfunction

	function AddDamage(this, value)
		this.damage += CInt(value);
	endfunction

    function Serialize(this)
        return struct{
            name := this.name,
            type := this.type,
            xp := this.xp,
            level := this.level,
            hits := this.hits,
            kills := this.kills,
            damage := this.damage
        };
    endfunction

    function Deserialize(this, data)
        if (!data)
            return error{ errortext := "Combat data is null or undefined." };
        endif

        this.name := data.name;
        this.type := data.type;
        this.xp := CInt(data.xp);
        this.level := CInt(data.level);
        this.hits := CInt(data.hits);
        this.kills := CInt(data.kills);
        this.damage := CInt(data.damage);
    endfunction

    function toString(this)
        return $"Combat para {this.name} ({this.type}) (Level={this.level}, XP={this.xp}, Hits={this.hits}, Kills={this.kills})";
    endfunction
endclass