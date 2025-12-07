function updateGlobalEntropy(amt) 
    // Atualizar entropia global do mundo
    var global_entropy := GetGlobalProperty("GlobalEntropy");
    if (!global_entropy)
        global_entropy := 0;
    endif
    
    SetGlobalProperty("GlobalEntropy", global_entropy + amt);
endfunction