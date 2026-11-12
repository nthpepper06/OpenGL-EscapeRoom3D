#ifndef MATCH_MATCHMANAGER_HPP
#define MATCH_MATCHMANAGER_HPP

#include <string>
#include "../Core/StateMachine.hpp"
#include "../Core/EventBus.hpp"

namespace Match {

enum class MatchState {
    SPLASH,
    LOBBY,
    MATCHMAKING,
    DEPLOYING,
    PLAYING,
    VICTORY,
    ELIMINATED,
    RESULTS
};

struct MatchStateChangedEvent : public Core::IEvent {
    MatchState oldState;
    MatchState newState;
    MatchStateChangedEvent(MatchState oldS, MatchState newS) : oldState(oldS), newState(newS) {}
};

class MatchManager {
public:
    MatchState currentState = MatchState::SPLASH;
    float stateTimer = 0.0f;

    int totalPlayers = 100;
    int alivePlayers = 100;
    int playerKills = 0;
    int totalDamageDealt = 0;
    float matchDuration = 0.0f;
    int playerRank = 1;

    // Progression XP & Gold rewards
    int earnedXP = 0;
    int earnedCredits = 0;

    void setState(MatchState newState) {
        if (currentState == newState) return;
        MatchState oldState = currentState;
        currentState = newState;
        stateTimer = 0.0f;

        MatchStateChangedEvent evt(oldState, newState);
        Core::EventBus::instance().publish(evt);
    }

    void startMatch() {
        totalPlayers = 100;
        alivePlayers = 100;
        playerKills = 0;
        totalDamageDealt = 0;
        matchDuration = 0.0f;
        playerRank = 1;
        earnedXP = 0;
        earnedCredits = 0;
        setState(MatchState::DEPLOYING);
    }

    void update(float dt) {
        stateTimer += dt;

        if (currentState == MatchState::MATCHMAKING) {
            // Simulate 2.5s matchmaking countdown
            if (stateTimer >= 2.5f) {
                startMatch();
            }
        } else if (currentState == MatchState::PLAYING || currentState == MatchState::DEPLOYING) {
            matchDuration += dt;
        } else if (currentState == MatchState::VICTORY || currentState == MatchState::ELIMINATED) {
            // Auto transition to detailed Results after 4.0s
            if (stateTimer >= 4.0f) {
                calculateResults();
                setState(MatchState::RESULTS);
            }
        }
    }

    void onPlayerKilledTarget(int damage) {
        playerKills++;
        totalDamageDealt += damage;
    }

    void onEntityEliminated(bool isPlayer) {
        if (alivePlayers > 0) alivePlayers--;

        if (isPlayer) {
            playerRank = alivePlayers + 1;
            setState(MatchState::ELIMINATED);
        } else if (alivePlayers <= 1) {
            playerRank = 1;
            setState(MatchState::VICTORY);
        }
    }

    void calculateResults() {
        // Compute Vanguard XP & Credits based on rank, kills and survival time for 100 players
        int rankScore = (playerRank == 1) ? 800 : std::max(50, 600 - playerRank * 6);
        int killScore = playerKills * 120;
        int timeScore = (int)(matchDuration * 3.0f);

        earnedXP = rankScore + killScore + timeScore;
        earnedCredits = (int)(earnedXP * 0.5f);
    }
};

} // namespace Match

#endif // MATCH_MATCHMANAGER_HPP
