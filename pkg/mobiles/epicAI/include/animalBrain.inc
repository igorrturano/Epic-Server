use npc;

function AnimalBrain()
    var ev;
    var wanders := 0;
    // EnableMainEvents();
    Set_Event_Queue_Size(50);

    while (me)
        ev := os::wait_for_event(200);
        if (ev)
            // case (ev.type)
            //     // Animal-specific event handling
            // endcase
        endif

        // Animal-specific behavior
        // AnimalLookAround();
        // AnimalWander();
    endwhile
endfunction

// Add animal-specific helper functions here