#include "interaction/InteractionManager.hpp"
#include "audio/AudioEngine.hpp"

void InteractionManager::update(float deltaTime) {
    if (messageTimer > 0.0f) {
        messageTimer -= deltaTime;
        if (messageTimer <= 0.0f) {
            statusMessage = "";
        }
    }
}

void InteractionManager::setMessage(const std::string& msg, float duration) {
    statusMessage = msg;
    messageTimer = duration;
}

std::string InteractionManager::getPromptText(const std::shared_ptr<SceneObject>& obj) const {
    if (!obj || !obj->isInteractable()) return "";

    switch (obj->interactType) {
        case InteractionType::LightSwitch:
            return "[E] Toggle Ceiling Light";
        case InteractionType::DeskDrawer: {
            bool open = drawerObject ? drawerObject->isOpen : obj->isOpen;
            return open ? "[E] Close Drawer" : "[E] Open Drawer (Search for Clue)";
        }
        case InteractionType::SecretNote: {
            if (obj->id.find("uv_") != std::string::npos) {
                return "[E] Inspect Fluorescent UV Clue: " + obj->name;
            }
            if (currentRoomId == 0 || currentRoomId == 2) {
                return "[E] Read " + obj->name;
            }
            return "[E] Read Clue Document [PIN: " + correctSafePin + "]";
        }
        case InteractionType::SafeKeypad:
            return safeUnlocked ? "[E] Keypad Unlocked" : "[E] Enter 4-Digit Security PIN (Keyboard 0-9)";
        case InteractionType::SafeDoor: {
            if (!safeUnlocked) return "[E] Vault Safe is LOCKED! Press [E] to enter PIN.";
            bool open = safeDoorObject ? safeDoorObject->isOpen : obj->isOpen;
            return open ? "[E] Close Vault Door" : "[E] Open Vault Door";
        }
        case InteractionType::GoldenKey:
            return "[E] Take Golden Key & Sigil Fragment!";
        case InteractionType::ExitDoor: {
            if (currentRoomId == 0) {
                if (!hasGoldenKey) return "[E] Exit Door [LOCKED] - Requires Ritual Key from the altar!";
            } else if (currentRoomId == 1) {
                if (!safeUnlocked && !hasGoldenKey) return "[E] Isolation Door [PIN LOCKED] - Use UV Blacklight on cell walls!";
            } else if (currentRoomId == 2) {
                if (!mirrorPuzzleSolved && keysCollected < 3) return "[E] Sanctuary Door [SEALED] - Align the 3 busts with the mirror!";
            }
            bool open = exitDoorObject ? exitDoorObject->isOpen : obj->isOpen;
            return open ? "[E] Step Through Exit Door" : "[E] OPEN EXIT DOOR";
        }
        case InteractionType::Totem: {
            int idx = 0;
            if (obj->id.find("2") != std::string::npos) idx = 1;
            else if (obj->id.find("3") != std::string::npos) idx = 2;
            int angle = static_cast<int>(totemAngles[idx]) % 360;
            return "[E] Rotate totem (now " + std::to_string(angle) + " deg)";
        }
        case InteractionType::FuseItem:
            return "[E] Pick up " + obj->name;
        case InteractionType::BreakerBox: {
            if (powerRestored) return "[E] Circuit Breaker (220V Power Online)";
            if (fusesCollected < 2) return "[E] Circuit Breaker (Missing 2 fuses, holding: " + std::to_string(fusesCollected) + "/2)";
            return "[E] INSERT 2 FUSES & RESTORE 220V POWER";
        }
        case InteractionType::IsolationGate: {
            if (!powerRestored) return "[E] Iron Security Gate [POWER OFFLINE] - Restore power at Breaker Box!";
            return obj->isOpen ? "[E] Close Security Gate" : "[E] Open Security Gate";
        }
        case InteractionType::MirrorBust: {
            int idx = 0;
            if (obj->id.find("2") != std::string::npos) idx = 1;
            else if (obj->id.find("3") != std::string::npos) idx = 2;
            int angle = static_cast<int>(bustAngles[idx]) % 360;
            return "[E] Rotate bust (now " + std::to_string(angle) + " deg)";
        }
        default:
            return "";
    }
}

void InteractionManager::interact(std::shared_ptr<SceneObject> obj, PointLight& ceilingLight) {
    if (!obj || !obj->isInteractable()) return;

    switch (obj->interactType) {
        case InteractionType::LightSwitch: {
            ceilingLight.enabled = !ceilingLight.enabled;
            obj->isOpen = ceilingLight.enabled;
            setMessage(ceilingLight.enabled ? "Ceiling light turned ON." : "Ceiling light turned OFF.");
            break;
        }
        case InteractionType::DeskDrawer: {
            auto target = drawerObject ? drawerObject : obj;
            target->isOpen = !target->isOpen;
            if (audioEngine) audioEngine->playDrawerSlide(glm::vec3(target->getWorldMatrix()[3]));
            setMessage(target->isOpen ? "Opened drawer! Paper scrap found: [PIN: " + correctSafePin + "]!" : "Closed desk drawer.", 8.0f);
            break;
        }
        case InteractionType::SecretNote: {
            std::string loreTitle = "";
            std::string loreBody = "";
            if (obj->id.find("uv_cipher_1") != std::string::npos) {
                loreTitle = "UV FLUORESCENT CIPHER [STEP 1/4]";
                loreBody = "First digit of the isolation cell security keypad: [ 4 ]\n\nSearch the remaining 3 corners under UV Blacklight.";
            } else if (obj->id.find("uv_cipher_2") != std::string::npos) {
                loreTitle = "UV FLUORESCENT CIPHER [STEP 2/4]";
                loreBody = "Second digit of the isolation cell security keypad: [ 1 ]\n\nSearch the remaining 2 corners under UV Blacklight.";
            } else if (obj->id.find("uv_cipher_3") != std::string::npos) {
                loreTitle = "UV FLUORESCENT CIPHER [STEP 3/4]";
                loreBody = "Third digit of the isolation cell security keypad: [ 0 ]\n\nSearch the last corner under UV Blacklight.";
            } else if (obj->id.find("uv_cipher_4") != std::string::npos) {
                loreTitle = "UV FLUORESCENT CIPHER [STEP 4/4]";
                loreBody = "Fourth digit of the isolation cell security keypad: [ 8 ]\n\nComplete Keypad PIN Sequence: 4 - 1 - 0 - 8.";
            } else if (currentRoomId == 0) {
                loreTitle = "RITUAL SACRIFICE DIARY OF DR. ELIAS VANCE (1893)";
                loreBody = "November 13, 1893:\n"
                           "Rotate each stone totem until its cast shadow matches the corresponding rune engraved upon the North wall.\n\n"
                           "When all three shadows align with the crimson glyphs, the crypt altar slab shall slide open.\n\n"
                           "        >>> TOTEM ANGLES: 90 / 180 / 270 DEGREES <<<\n\n"
                           "Retrieve the Ritual Key to unlock the southern gateway!";
            } else if (currentRoomId == 1) {
                loreTitle = "PATIENT MEDICAL DOSSIER - SECLUSION WARD";
                loreBody = "Case #4108 - Subject: Elias Vance.\n"
                           "Subject exhibits acute delusions and claims whispers echo from the mirror.\n"
                           "Security lock encoded with 4-digit sequence:\n\n"
                           "        >>> KEYPAD PIN: " + correctSafePin + " <<<\n\n"
                           "Warning: Inspect isolation cell walls under UV Blacklight.";
            } else {
                loreTitle = "CURSE OF THE DEMON MIRROR";
                loreBody = "Dr. Vance retreated into the mirror world.\n"
                           "Observe the spectral guides in the Great Mirror and align each real bust so its golden notch matches the reflection.\n\n"
                           "When all three busts mirror the spectral anomalies, the sanctuary exit seal shall shatter.\n\n"
                           "        >>> TARGET ROTATIONS: 180 / 90 / 270 DEGREES <<<\n";
            }
            openInspect(InspectType::SecretNote, loreTitle, loreBody);
            setMessage("Reading document. Press [E] or [ESC] to close.", 4.0f);
            break;
        }
        case InteractionType::SafeKeypad: {
            if (!safeUnlocked) {
                showKeypadModal = true;
                enteredPin = "";
            } else {
                setMessage("Keypad is already unlocked.");
            }
            break;
        }
        case InteractionType::SafeDoor: {
            if (!safeUnlocked) {
                showKeypadModal = true;
                enteredPin = "";
                if (audioEngine) audioEngine->playKeypadBeep(true);
                setMessage("Vault safe is locked! Enter 4-digit PIN (" + correctSafePin + ").", 5.0f);
            } else {
                auto target = safeDoorObject ? safeDoorObject : obj;
                target->isOpen = !target->isOpen;
                if (audioEngine) audioEngine->playDoorCreak(glm::vec3(target->getWorldMatrix()[3]));
                setMessage(target->isOpen ? "Opened vault safe door." : "Closed vault safe door.");
            }
            break;
        }
        case InteractionType::GoldenKey: {
            if (currentRoomId == 0 && !altarUnlocked && altarLidObject && !altarLidObject->isOpen) {
                setMessage("The altar slab is locked tight! Align the 3 totems with the North wall runes.", 5.0f);
                return;
            }
            if (currentRoomId == 1 && !powerRestored) {
                setMessage("Restore power and step into the isolation cell first!", 4.0f);
                return;
            }
            hasGoldenKey = true;
            if (currentRoomId >= 0 && currentRoomId < 3 && !hasSigil[currentRoomId]) {
                hasSigil[currentRoomId] = true;
                sigilsCollected++;
                keysCollected++;
            }
            if (audioEngine) audioEngine->playItemPickup();
            obj->isVisible = false;
            std::string keyTitle = "KEY & SIGIL FRAGMENT OF VANCE [" + std::to_string(keysCollected) + "/3]";
            std::string keyBody = "You have collected Golden Key and Sigil Fragment #" + std::to_string(currentRoomId + 1) + "!\n"
                                  "The golden relic radiates warmth, pushing back the dark.\n"
                                  "Total relics collected: " + std::to_string(keysCollected) + "/3.\n\n"
                                  "Proceed to the EXIT Door to advance!";
            openInspect(InspectType::GoldenKey, keyTitle, keyBody);
            setMessage("ACQUIRED KEY & SIGIL FRAGMENT [" + std::to_string(keysCollected) + "/3]!", 8.0f);
            break;
        }
        case InteractionType::ExitDoor: {
            if (currentRoomId == 2) {
                if (mirrorPuzzleSolved || keysCollected >= 3) {
                    if (exitDoorObject) {
                        exitDoorObject->isOpen = true;
                        exitDoorObject->isLocked = false;
                    }
                    obj->isOpen = true;
                    exitUnlocked = true;
                    gameWon = true;
                    if (audioEngine) audioEngine->playDoorCreak(glm::vec3(obj->getWorldMatrix()[3]));
                    setMessage("CONGRATULATIONS! You broke the Demon Mirror illusion and escaped the nightmare!", 12.0f);
                } else {
                    if (audioEngine) audioEngine->playKeypadBeep(false);
                    setMessage("SANCTUARY DOOR IS SEALED! Observe the mirror and align the 3 busts symmetrically!", 6.0f);
                }
            } else if (currentRoomId == 1) {
                if (safeUnlocked) {
                    if (exitDoorObject) {
                        exitDoorObject->isOpen = true;
                        exitDoorObject->isLocked = false;
                    }
                    obj->isOpen = true;
                    exitUnlocked = true;
                    if (!hasSigil[1]) {
                        hasSigil[1] = true;
                        sigilsCollected++;
                        keysCollected++;
                    }
                    requestRoomTransition = true;
                    if (audioEngine) audioEngine->playDoorCreak(glm::vec3(obj->getWorldMatrix()[3]));
                    setMessage("UNLOCKED! Sigil Fragment #2 Acquired. Advancing to Haunted Mirror Gallery (Room 3)...", 6.0f);
                } else {
                    showKeypadModal = true;
                    enteredPin = "";
                    if (audioEngine) audioEngine->playKeypadBeep(false);
                    setMessage("Isolation Exit Door is locked! Enter 4-digit PIN on keypad (look for UV markings on cell walls).", 5.0f);
                }
            } else {
                if (hasGoldenKey) {
                    if (exitDoorObject) {
                        exitDoorObject->isOpen = true;
                        exitDoorObject->isLocked = false;
                    }
                    obj->isOpen = true;
                    exitUnlocked = true;
                    requestRoomTransition = true;
                    if (audioEngine) audioEngine->playDoorCreak(glm::vec3(obj->getWorldMatrix()[3]));
                    setMessage("UNLOCKED! Advancing to Abandoned Psychiatric Ward (Room 2)...", 6.0f);
                } else {
                    if (audioEngine) audioEngine->playKeypadBeep(false);
                    setMessage("Exit door is firmly locked! Solve the shadow totem alignment on the altar!", 5.0f);
                }
            }
            break;
        }
        case InteractionType::Totem: {
            int idx = 0;
            if (obj->id.find("2") != std::string::npos) idx = 1;
            else if (obj->id.find("3") != std::string::npos) idx = 2;

            totemAngles[idx] = fmod(totemAngles[idx] + 90.0f, 360.0f);
            obj->setRotation(glm::vec3(0.0f, totemAngles[idx], 0.0f));

            if (audioEngine) {
                audioEngine->playDrawerSlide(glm::vec3(obj->getWorldMatrix()[3]));
            }

            // Check if all 3 totems match target angles
            bool allMatched = true;
            for (int i = 0; i < 3; ++i) {
                float diff = std::abs(totemAngles[i] - targetTotemAngles[i]);
                if (diff > 1.0f && std::abs(diff - 360.0f) > 1.0f) {
                    allMatched = false;
                    break;
                }
            }

            if (allMatched) {
                altarUnlocked = true;
                if (altarLidObject) {
                    altarLidObject->isOpen = true;
                }
                if (audioEngine) {
                    audioEngine->playSafeUnlock(glm::vec3(0.0f, 1.0f, 0.0f));
                }
                setMessage("SHADOWS ALIGNED WITH RUNES! The altar stone slab slides open, revealing the Ritual Key!", 8.0f);
            } else {
                int deg = static_cast<int>(totemAngles[idx]) % 360;
                setMessage("Rotated Totem #" + std::to_string(idx + 1) + " (now " + std::to_string(deg) + " deg). Observe shadow on North wall!", 4.0f);
            }
            break;
        }
        case InteractionType::FuseItem: {
            fusesCollected++;
            obj->isVisible = false;
            if (audioEngine) audioEngine->playItemPickup();
            setMessage("Acquired: " + obj->name + " [" + std::to_string(fusesCollected) + "/2]! Bring to the Circuit Breaker box.", 6.0f);
            break;
        }
        case InteractionType::BreakerBox: {
            if (powerRestored) {
                setMessage("Circuit breaker is online. 220V power grid operating normally.", 4.0f);
                break;
            }
            if (fusesCollected < 2) {
                if (audioEngine) audioEngine->playKeypadBeep(false);
                setMessage("Circuit breaker missing fuses (" + std::to_string(fusesCollected) + "/2)! Follow the UV trail to find them.", 5.0f);
                break;
            }

            // Install fuses & restore power
            powerRestored = true;
            ceilingLight.color = glm::vec3(0.75f, 0.95f, 0.92f); // Fluorescent
            ceilingLight.intensity = 11.0f;
            ceilingLight.flickerSpeed = 4.0f;
            if (isolationGateObject) {
                isolationGateObject->isOpen = true;
            }
            if (audioEngine) {
                audioEngine->playKeypadBeep(true);
                audioEngine->playDoorCreak(glm::vec3(0.0f, 1.5f, 0.0f));
            }
            setMessage("2 FUSES INSERTED! Power restored, electronic security gate opened!", 8.0f);
            break;
        }
        case InteractionType::IsolationGate: {
            if (!powerRestored) {
                if (audioEngine) audioEngine->playKeypadBeep(false);
                setMessage("Security gate is ELECTRONICALLY LOCKED! Restore power at Circuit Breaker.", 5.0f);
                break;
            }
            obj->isOpen = !obj->isOpen;
            if (audioEngine) audioEngine->playDoorCreak(glm::vec3(obj->getWorldMatrix()[3]));
            setMessage(obj->isOpen ? "Opened security gate." : "Closed security gate.", 3.0f);
            break;
        }
        case InteractionType::MirrorBust: {
            int idx = 0;
            if (obj->id.find("2") != std::string::npos) idx = 1;
            else if (obj->id.find("3") != std::string::npos) idx = 2;

            bustAngles[idx] = fmod(bustAngles[idx] + 90.0f, 360.0f);
            obj->setRotation(glm::vec3(0.0f, bustAngles[idx], 0.0f));

            if (audioEngine) {
                audioEngine->playDrawerSlide(glm::vec3(obj->getWorldMatrix()[3]));
            }

            // Check if all 3 busts match the mirror reflection symmetry
            bool allMatched = true;
            for (int i = 0; i < 3; ++i) {
                float diff = std::abs(bustAngles[i] - targetBustAngles[i]);
                if (diff > 1.0f && std::abs(diff - 360.0f) > 1.0f) {
                    allMatched = false;
                    break;
                }
            }

            if (allMatched) {
                mirrorPuzzleSolved = true;
                if (exitDoorObject) {
                    exitDoorObject->isOpen = true;
                    exitDoorObject->isLocked = false;
                }
                exitUnlocked = true;
                gameWon = true;
                if (audioEngine) {
                    audioEngine->playSafeUnlock(glm::vec3(0.0f, 2.0f, 0.0f));
                }
                setMessage("MIRROR ANOMALIES MATCHED! The Sanctuary seal breaks, final Exit Door swings open!", 10.0f);
            } else {
                int deg = static_cast<int>(bustAngles[idx]) % 360;
                setMessage("Rotated Bust #" + std::to_string(idx + 1) + " (now " + std::to_string(deg) + " deg). Look into the mirror to compare!", 4.0f);
            }
            break;
        }
        default:
            break;
    }
}

bool InteractionManager::checkPin(const std::string& pin) {
    if (pin == correctSafePin) {
        safeUnlocked = true;
        showKeypadModal = false;
        if (safeDoorObject) {
            safeDoorObject->isOpen = true;
            safeDoorObject->isLocked = false;
        }
        if (exitDoorObject && currentRoomId == 1) {
            exitDoorObject->isOpen = true;
            exitDoorObject->isLocked = false;
            exitUnlocked = true;
            if (!hasSigil[1]) {
                hasSigil[1] = true;
                sigilsCollected++;
                keysCollected++;
            }
        }
        if (audioEngine) {
            audioEngine->playKeypadBeep(true);
            audioEngine->playSafeUnlock(safeDoorObject ? glm::vec3(safeDoorObject->getWorldMatrix()[3]) : glm::vec3(3.2f, 1.46f, 0.0f));
        }
        setMessage("ACCESS GRANTED! Correct PIN (" + pin + ") entered! Isolation Door Unlocked & Sigil Fragment #2 Acquired.", 8.0f);
        return true;
    } else {
        if (audioEngine) {
            audioEngine->playKeypadBeep(false);
        }
        setMessage("ACCESS DENIED! Incorrect PIN (" + pin + "). Inspect cell walls under UV light!", 5.0f);
        enteredPin.clear();
        return false;
    }
}

void InteractionManager::resetGameState() {
    hasGoldenKey = false;
    safeUnlocked = false;
    exitUnlocked = false;
    gameWon = false;
    requestRoomTransition = false;
    showKeypadModal = false;
    enteredPin = "";

    // Reset Room 1 Totem & Altar state
    totemAngles[0] = totemAngles[1] = totemAngles[2] = 0.0f;
    altarUnlocked = false;
    if (altarLidObject) altarLidObject->isOpen = false;

    // Reset Room 2 Fuse & Power state
    fusesCollected = 0;
    powerRestored = false;
    if (isolationGateObject) isolationGateObject->isOpen = false;

    // Reset Room 3 Mirror Bust state
    bustAngles[0] = bustAngles[1] = bustAngles[2] = 0.0f;
    mirrorPuzzleSolved = false;

    if (drawerObject) drawerObject->isOpen = false;
    if (safeDoorObject) {
        safeDoorObject->isOpen = false;
        safeDoorObject->isLocked = true;
    }
    if (exitDoorObject) {
        exitDoorObject->isOpen = false;
        exitDoorObject->isLocked = true;
    }
    if (keyObject) keyObject->isVisible = true;

    setMessage("Escape Room reset! Explore the chambers and break Vance's curse!", 5.0f);
}

void InteractionManager::openInspect(InspectType type, const std::string& title, const std::string& body) {
    isInspectMode = true;
    currentInspectType = type;
    inspectTitle = title;
    inspectBody = body;
    inspectYaw = 0.0f;
    inspectPitch = 0.0f;
}

void InteractionManager::closeInspect() {
    isInspectMode = false;
    currentInspectType = InspectType::None;
}

void InteractionManager::rotateInspect(float dx, float dy) {
    inspectYaw += dx * 0.4f;
    inspectPitch += dy * 0.4f;
    inspectPitch = std::clamp(inspectPitch, -80.0f, 80.0f);
}
