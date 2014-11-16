namespace ai
{
    namespace monster
    {
        enum { M_NONE = 0, M_SEARCH, M_HOME, M_ATTACKING, M_PAIN, M_SLEEP, M_AIMING };  // monster states
       
        struct MonsterAiInfo
        {
            int monsterstate;                   // one of M_*, M_NONE means human
            int tag;                            // Tag for triggers and so on
            fpsent *enemy;                      // monster wants to kill this entity
            float targetyaw;                    // monster wants to look in this direction
            int trigger;                        // millis at which transition to another monsterstate takes place
            vec attacktarget;                   // delayed attacks
            int anger;                          // how many times already hit by fellow monster
            physent *stacked;
            vec stackpos;
            bool counts;                        // does count for guts
            int lastshot;

            MonsterAiInfo() :   monsterstate(M_SEARCH), tag(-1), enemy(NULL), targetyaw(0), trigger(0), attacktarget(0, 0, 0),
                                anger(0), stacked(NULL), stackpos(0, 0, 0), counts(true), lastshot(0)
            {}

            // monster AI is sequenced using transitions: they are in a particular state where
            // they execute a particular behaviour until the trigger time is hit, and then they
            // reevaluate their situation based on the current state, the environment etc., and
            // transition to the next state. Transition timeframes are parametrized by difficulty
            // level (skill), faster transitions means quicker decision making means tougher AI.

            void transition(fpsent *d, int _state, int _moving, int n, int r); // n = at skill 0, n/2 = at skill 10, r = added random factor
        };

        // Convert networked skill (same as bots use: 1-101), to local level 1-4
        inline int aiSkillToLevel(int skill)
        {
            return (skill-1)/25+1;
        }
        // And the other way around
        inline int aiLevelToSkill(int skill)
        {
            return (skill-1)*25;
        }

        struct MonsterAi : AiInterface
        {
            fpsent *bestenemy;
            bool monsterhurt, monsterwashurt;
            vec monsterhurtpos;
            MonsterAi() : bestenemy(NULL), monsterhurt(false), monsterhurtpos(0, 0, 0) {}
            void destroy(fpsent *d);
            void create(fpsent *d);

            void collide(fpsent *d, physent *o, const vec &dir);

            void hit(fpsent *d, fpsent *actor, int damage, const vec &vel, int gun);
            void spawned(fpsent *d);
            void killed(fpsent *d, fpsent *e);

            void startThink();
            void think(fpsent *d, bool run);
            void endThink();

            void clear();
            void render(fpsent *d, float i);
        };
    }
}