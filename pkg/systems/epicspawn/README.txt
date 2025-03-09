EPIC BOSS SPAWNER SYSTEM
=======================

This package provides a simplified boss spawning system for epic bosses created with the epicBoss command.

Features:
---------
1. Automatic boss respawning after death
2. Configurable respawn times
3. Spawn range configuration
4. Test spawn functionality
5. Integration with the epicBoss creation system
6. Automatic NPC template prefix handling
7. Form data persistence between pages

How to Use:
-----------
1. Create a boss using the .epicBoss command
   - This will automatically create a spawner at your feet
   - Simply enter the NPC template name without any package prefix
   - The system will automatically try :epicAI:, :brainAI:, and :merchants: prefixes
   - Your form data is saved when switching between pages

2. Configure the spawner by double-clicking it (GM+ only)
   - Set minimum and maximum respawn times (in minutes)
   - Set spawn range (how far from the spawner the boss can appear)
   - Test spawn to see the boss in action

3. The boss will automatically respawn after being killed
   - Respawn timer starts when the boss dies
   - Boss will only spawn when no players are nearby

Technical Details:
-----------------
- The spawner uses the BossStruct property to store the complete boss configuration
- The spawner is invisible by default (graphic 0x1ea7)
- The system uses a datafile to track all spawners (:epicspawn:epicbosses)
- NPC templates are automatically prefixed with the appropriate package
- Form data is stored in memory while navigating between pages

Commands:
---------
.epicBoss - Opens the boss creation interface
  - Creates both a boss and a spawner

Item IDs:
---------
0xA402 - Epic Boss Spawner 