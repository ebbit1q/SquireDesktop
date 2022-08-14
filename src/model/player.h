#pragma once
#include <string>
#include <squire_core/squire_core.h>

class Player
{
public:
    Player();
    Player(squire_core::sc_PlayerId pid, squire_core::sc_TournamentId tid);
    Player(const Player &p);
    ~Player();
    std::string name();
    std::string gameName();
    squire_core::sc_PlayerStatus status();
    std::string statusAsStr();
    squire_core::sc_PlayerId id();
    squire_core::sc_TournamentId tourn_id();
    bool matches(std::string query);
private:
    squire_core::sc_PlayerId pid;
    squire_core::sc_TournamentId tid;
};
