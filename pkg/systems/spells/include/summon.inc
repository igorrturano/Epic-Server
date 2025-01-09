function summon(template, targ, mage)
    var undead := CreateNpcFromTemplate(template, targ.x, targ.y, targ.z, 0, targ.realm);
    if (mage.acctname)
        undead.SetMaster(mage);
    endif

    var summons := GetObjProperty(mage, "SummonedPets");
    if(!summons)
        summons := array;
    endif
    summons.append(undead.serial);

    SetObjProperty(mage, "SummonedPets", summons);

    SetObjProperty(undead, "GuardKill", 1);
    SetObjProperty(undead, "summoned", 1);
    SetObjProperty(undead, "owner", mage.serial);
    SetObjProperty(undead, "owneri", mage.serial);
    SetObjProperty(undead, "name", targ.name);
    SetObjproperty(undead, "script", targ.script);
    SetObjProperty(undead, "AntigoTreinadorID", mage.serial);

    // Quando o slot for atribuído no summon.src, ele setará essa propriedade
    // Aqui armazenamos o serial do summon no mage usando o slot
    var slot := GetObjProperty(undead, "slot");
    if (slot)
        SetObjProperty(mage, "summon_serial_"+CStr(slot), undead.serial);
    endif

    if (mage.acctname)
        undead.script := ":ghaia:tamed";
        undead.facing := mage.facing;
        RestartScript(undead);
    else
        undead.alignment := 1;
    endif

    if(undead.name == "Globo de Luz")
        var light := CreateItemAtLocation(targ.x, targ.y, targ.z, 0xa22, 1, targ.realm);
        var equip := EquipItem(undead, light.serial);
    endif

    return undead;
endfunction