# Comp3016CW1

Gameplay Description

Loop
A day is split into four phases (Morning → Day → Evening → Night). You pick one action per turn from a console menu while a HUD renders via SDL:
- Forage (food), Collect water, Gather wood, Explore (random event), Rest, Build Shelter, Eat/Drink, Craft, Save & Quit.
- Time advances after each turn; the day counter only increments when Night wraps to Morning** (full cycle).

- Win Conditions
  1. **Rescue:** survive until **Day 100** (triggered at Night→Morning rollover).
  2. **Raft Escape:** a rare Explore event (~1%) offers escape only if you have ≥30 wood; accept to win.

Game Programming Patterns.
- Stats: Health, Energy, Food, Water, Wood; Shelter flag; simple crafting (Campfire, Rain Collector). All of these can be easily influcnece by the player picking to spesifically get one resource (such as picking forage to get food or resting to restore energy), the only hard resource to fill would be the health as i beleive it adds some challenge to the game that was much needed.
- Weather: Clear/Rain/Storm with biases from an adaptive Director + difficulty curve.
- AI Director: Adjusts tension from scarcity, weather, and outcomes to weight future events, so if the player is having to easy a time it will bump the weather to be worse and increase the chance of bad events and if the player is having a tuff time the Director will lay off a little.
- Events: Loaded from `data/events.tsv`.
- Data-driven difficulty: Curves blend (Easy/Normal/Hard) based on tension; optional overrides via `data/difficulty.tsv`.

Game Mechanics & Implementation Notes
Resources & Needs: Eating/Drinking reset counters; auto-consumption or penalties after 3 ticks without food/water.
Shelter & Crafting: Shelter (5 wood) boosts rest; Campfire (+10 energy daily), Rain Collector (+1 water on rain or storms).
Weather: Very random with some influence from the AI Director
Events: Weighted lottery per Explore; chain IDs to increase follow-up likelihood for two days.
Day Counter: Only increments on Night→Morning to match intuitive “full day survived”.
Win: Day 100 rollover or Raft (≥30 wood + rare event or debug-forced).
Loss: Run out of energy/health


Debug keys in the SDL window
- `D` — toggle debug panel  
- `F1` — **force Raft** offer on next Explore  
- `F2` — jump to **Day 99 Night** (advance time to trigger Day-100 win)


 Dependencies Used
- C++
- SDL3  
- Standard Library headers such as Vector and String 

Example UML

classDiagram
class Game {
  -Player player_
  -WorldState world_
  -Director director_
  -vector<Event> events_
  -mt19937 rng_
  +run()
  -doAction(c:int)
  -applyEvent()
  -advanceWeather()
  -nextDayTick()
  -renderHUD()
}
class Player {
  +int health
  +int energy
  +int food
  +int water
  +int wood
  +bool hasShelter
  +int day
  +int daysSinceEat
  +int daysSinceDrink
  +bool isDead()
  +void clamp()
}
class Director {
  +float tension
  +string lastType
  +void onDayStart(Player,WorldState)
  +void onEventApplied(Event)
  +float weatherBias()
  +float weightMultiplierForType(type)
}
class WorldState {
  +Weather weather
  +DayPhase phase
  +bool hasCampfire
  +bool hasCollector
  +float lightningFlash
}
class Event {
  +string type
  +string description
  +int weight
  +int dEnergy
  +int dFood
  +int dWater
  +int dWood
  +int dHealth
}

Game --> Player
Game --> WorldState
Game --> Director
Game --> "many" Event

Example Images
<img width="981" height="514" alt="Screenshot 2025-11-03 112400" src="https://github.com/user-attachments/assets/ff2626ce-7e63-437d-adb4-eeff9d4d3b52" />
<img width="798" height="530" alt="SDL" src="https://github.com/user-attachments/assets/5b8291c1-38a6-4b9d-841d-dc300fadc65c" />
<img width="225" height="226" alt="debug" src="https://github.com/user-attachments/assets/3dffc147-4ad2-40c6-a8a5-4207edc56e8c" />

Exception Handling & Test Cases

Exception handling present
Fatal file loading (player_init.txt, events.tsv) → catches exception, logs, exits.
Save errors (save.txt) → catches and logs message (no crash).

Functional test cases:
Boot & Data Load: Missing events.tsv should error and exit cleanly.
Save/Load: Perform actions, Save & Quit, relaunch, choose Load → state restored.
Day Rollover: Confirm day++ only when Night→Morning happens.
Starvation/Dehydration: Skip eating/drinking for 3 ticks → penalties apply; with resources available, auto-consume and reset.
Weather Transition: Observe Clear→Rain→Storm over multiple turns; debug panel shows expected biasing.
Event Chains: Trigger an event containing {chain:X}, then Explore within 2 days—related follow-ups become more likely.
Raft Win (debug): Press F1, Explore with ≥30 wood → raft offer → accept → win.
Day-100 Win (debug): Press F2 (Day 99 Night), advance time until rollover → win.
Crafting: Build Campfire/Collector; verify daily bonuses and rainy-day water.


AI Usage
I used AI assistance (ChatGPT) for:
Code scaffolding (HUD drawing, debug UI, event weighting, raft feature).
All generated code was reviewed, tested, and adapted by me.


Brief Evaluation
What went well: Clean separation of systems (Director, weather, events), data-driven content, and a readable HUD without extra libs. The adaptive weighting makes runs feel different while staying fair. The SLD also is very visually appealing with "cool" effects added such as the rain and the flashing during a storm. The game feels mostly balenced with the player usually scavaging for food/water and then when they have enough looking to explore the island for a way to win.
Challenges: Balancing weather/event probabilities, getting day counting to match player intuition, and ensuring a “win path” (Day 100) that remains achievable without cutting the game off to shortly. These issues were adressed by adding a dynamically changing Director that helps handle the weather, changing the day count to only tick after the night phase has passed and adding a additional win path with a low chance.
If doign it again: I’d add automated tests around event weighting and add richer graphics (sprite batching) and sound. I'd also like to make the debug menu more clear and add additional resources that can be collected to expand the crafting system.
