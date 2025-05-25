include ":achievements:model/Achievement";

const ACHIEVEMENT_EXPLORATION := ACHIEVEMENT + "_EXPLORATION";

class Exploration(Achievement)
    var locations := array;
    var discoveries;
    var distance;

    function Exploration(this, name, xp, level, locations, discoveries, distance)
        Super(name, xp, level);
        this.type := ACHIEVEMENT_EXPLORATION;
        this.locations := CInt(locations);
        this.discoveries := CInt(discoveries);
        this.distance := CInt(distance);
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
        this.locations += CInt(value);
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
        var result := Achievement::Deserialize(this, data);
        if (result == error)
            return result;
        endif

        this.locations := CInt(data.locations);
        this.discoveries := CInt(data.discoveries);
        this.distance := CInt(data.distance);
    endfunction

    function toString(this)
        return $"Achievement para {this.name} ({this.type}) (xp={this.xp}, level={this.level}, locations={this.locations}, discoveries={this.discoveries}, distance={this.distance})";
    endfunction
endclass