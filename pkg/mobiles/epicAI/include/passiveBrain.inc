/*
 * PassiveBrain - Does absolutely nothing.
 * Used for golems and other constructs that are meant to be piloted/controlled,
 * not act autonomously.
 */
function PassiveBrain()
    while (me)
        var ev := os::wait_for_event(120);

        if (ShouldGoDormant(polcore().systime))
            return;
        endif
    endwhile
endfunction
