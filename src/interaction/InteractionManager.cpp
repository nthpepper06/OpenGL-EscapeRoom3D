#include "interaction/InteractionManager.hpp"

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
            return "[E] Bat/Tat den tran (Light Switch)";
        case InteractionType::DeskDrawer: {
            bool open = drawerObject ? drawerObject->isOpen : obj->isOpen;
            return open ? "[E] Dong ngan keo" : "[E] Mo ngan keo (Tim manh giay mat ma)";
        }
        case InteractionType::SecretNote:
            return "[E] Doc manh giay mat ma [PIN: " + correctSafePin + "]";
        case InteractionType::SafeKeypad:
            return safeUnlocked ? "[E] Ket sat da mo khoa" : "[E] Nhap ma PIN ket sat (Ho tro ban phim 0-9)";
        case InteractionType::SafeDoor: {
            if (!safeUnlocked) return "[E] Ket sat dang KHOA! Bam [E] de nhap ma PIN.";
            bool open = safeDoorObject ? safeDoorObject->isOpen : obj->isOpen;
            return open ? "[E] Dong cua ket sat" : "[E] Mo cua ket sat";
        }
        case InteractionType::GoldenKey:
            if (!safeUnlocked && safeDoorObject && !safeDoorObject->isOpen) return "";
            return "[E] Nhat Chia Khoa Vang thoat hiem!";
        case InteractionType::ExitDoor: {
            if (!hasGoldenKey) return "[E] Cua thoat hiem [KHOA] - Can Chia Khoa Vang trong ket sat!";
            bool open = exitDoorObject ? exitDoorObject->isOpen : obj->isOpen;
            return open ? "[E] Thoat sang phong tiep theo" : "[E] DUNG CHIA KHOA VANG MO CUA THOAT HIEM";
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
            setMessage(ceilingLight.enabled ? "Da BAT den tran." : "Da TAT den tran.");
            break;
        }
        case InteractionType::DeskDrawer: {
            auto target = drawerObject ? drawerObject : obj;
            target->isOpen = !target->isOpen;
            setMessage(target->isOpen ? "Da mo ngan keo! Ben trong co manh giay ghi: [MA PIN KET SAT: " + correctSafePin + "]!" : "Da dong ngan keo ban.", 8.0f);
            break;
        }
        case InteractionType::SecretNote: {
            setMessage("Manh giay mat ma ghi ro: [MA PIN KET SAT LA " + correctSafePin + "] -> Hay den ket sat goc phong de nhap!", 8.0f);
            break;
        }
        case InteractionType::SafeKeypad: {
            if (!safeUnlocked) {
                showKeypadModal = true;
                enteredPin = "";
            } else {
                setMessage("Ket sat da mo khoa san roi.");
            }
            break;
        }
        case InteractionType::SafeDoor: {
            if (!safeUnlocked) {
                showKeypadModal = true;
                enteredPin = "";
                setMessage("Ket sat dang khoa! Hay go ma PIN 4 chu so (" + correctSafePin + ").", 5.0f);
            } else {
                auto target = safeDoorObject ? safeDoorObject : obj;
                target->isOpen = !target->isOpen;
                setMessage(target->isOpen ? "Da mo cua ket sat." : "Da dong cua ket sat.");
            }
            break;
        }
        case InteractionType::GoldenKey: {
            if (!safeUnlocked && safeDoorObject && !safeDoorObject->isOpen) {
                setMessage("Ket sat van dang khoa! Hay nhap ma PIN de mo ket truoc.", 4.0f);
                return;
            }
            hasGoldenKey = true;
            obj->isVisible = false;
            setMessage("DA NHAT DUOC CHIA KHOA VANG! Hay quay ra sau lung den Cua Thoat Hiem [EXIT] de mo cua!", 8.0f);
            break;
        }
        case InteractionType::ExitDoor: {
            if (hasGoldenKey) {
                if (exitDoorObject) {
                    exitDoorObject->isOpen = true;
                    exitDoorObject->isLocked = false;
                }
                obj->isOpen = true;
                exitUnlocked = true;
                requestRoomTransition = true;
                setMessage("MO KHOA THANH CONG! Cua thoat hiem bung mo! Dang chuyen sang can phong tiep theo...", 6.0f);
            } else {
                setMessage("Cua thoat hiem dang bi KHOA CHAT! Hay tim manh giay & mo ket sat lay Chia Khoa Vang!", 5.0f);
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
        setMessage("DUNG MA PIN! Ket sat da mo khoa va bung cua! Nhat Chia Khoa Vang ben trong!", 8.0f);
        return true;
    } else {
        setMessage("SAI MA PIN! Manh giay trong ngan keo ghi: " + correctSafePin, 5.0f);
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

    setMessage("Escape Room reset! Find the clue, unlock the safe, and escape!", 5.0f);
}
