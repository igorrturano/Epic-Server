    // 3. Magias count
    var mag_count := magias.size();
    packet.SetInt8(offset, CInt(mag_count / 256)); offset += 1;
    packet.SetInt8(offset, mag_count % 256); offset += 1;
    
    // Write magias data
    foreach item in magias
        // name
        var name_len := len(item.name);
        packet.SetInt8(offset, CInt(name_len / 256)); offset += 1;
        packet.SetInt8(offset, name_len % 256); offset += 1;
        packet.SetString(offset, item.name, 0); offset += name_len;
        
        // circle (as string)
        var circle_str := CStr(item.circle);
        var circle_len := len(circle_str);
        packet.SetInt8(offset, CInt(circle_len / 256)); offset += 1;
        packet.SetInt8(offset, circle_len % 256); offset += 1;
        packet.SetString(offset, circle_str, 0); offset += circle_len;
        
        // desc
        var desc_len := len(item.desc);
        packet.SetInt8(offset, CInt(desc_len / 256)); offset += 1;
        packet.SetInt8(offset, desc_len % 256); offset += 1;
        packet.SetString(offset, item.desc, 0); offset += desc_len;
        
        // iconid
        packet.SetInt8(offset, CInt(item.iconid / 256)); offset += 1;
        packet.SetInt8(offset, item.iconid % 256); offset += 1;
        
        // hue (default 0)
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        
        // mana_cost
        packet.SetInt8(offset, CInt(item.mana_cost / 256)); offset += 1;
        packet.SetInt8(offset, item.mana_cost % 256); offset += 1;
        
        // min_skill (default 0)
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        
        // reagents (empty string for now)
        packet.SetInt8(offset, 0); offset += 1;
        packet.SetInt8(offset, 0); offset += 1;
        
        // spell_id
        packet.SetInt8(offset, CInt(item.spell_id / 256)); offset += 1;
        packet.SetInt8(offset, item.spell_id % 256); offset += 1;
    endforeach
