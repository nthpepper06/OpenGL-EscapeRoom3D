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
            std::string loreTitle = "";
            std::string loreBody = "";
            if (currentRoomId == 0) {
                loreTitle = "NHAT KY HIEN TE CUA DR. ELIAS VANCE (1893)";
                loreBody = "Ngay 13 thang 11, 1893:\n"
                           "Nhung benh nhan tam than khong phai la ganh nang, ho la vat hien te hoan hao.\n"
                           "Ta da phong an Manh Co Vat dau tien vao chiec ket sat sat ngam.\n"
                           "Ma so giai phong nghi le duoc khac ghi tren buc tuong mau:\n\n"
                           "        >>> MA PIN KET SAT: " + correctSafePin + " <<<\n\n"
                           "Hay mo ket sat de lay Chia Khoa Vang va Manh Sigil!";
            } else if (currentRoomId == 1) {
                loreTitle = "HO SO BENH AN TRICH XUAT - BENH VIEN TAM THAN";
                loreBody = "Benh an #4108 - Doi tuong: Elias Vance.\n"
                           "Doi tuong nghe thay tieng thi tham tu ben kia tam guong.\n"
                           "Doi tuong da giau chia khoa buong giam vao tu ho so thep.\n\n"
                           "        >>> MA PIN KET SAT: " + correctSafePin + " <<<\n\n"
                           "Canh bao: Khong duoc nhin vao tam guong phong so 3!";
            } else {
                loreTitle = "LOI NGUYEN PHONG AN GUONG QUY";
                loreBody = "Tien si Vance da bien mat vao ben trong the gioi guong.\n"
                           "De pha vo vong lap ac mong va mo cua thoat hiem cuoi cung,\n"
                           "nguoi choi can phai so huu du ca 3 Chia Khoa Vang va 3 Manh Sigil!\n\n"
                           "        >>> MA PIN KET SAT CUOI: " + correctSafePin + " <<<\n";
            }
            openInspect(InspectType::SecretNote, loreTitle, loreBody);
            setMessage("Dang doc [Manh Giay Mat Ma]! Nhan [E] hoac [ESC] de dong lai.", 4.0f);
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
            if (currentRoomId >= 0 && currentRoomId < 3 && !hasSigil[currentRoomId]) {
                hasSigil[currentRoomId] = true;
                sigilsCollected++;
                keysCollected++;
            }
            obj->isVisible = false;
            std::string keyTitle = "CHIA KHOA VANG & MANH SIGIL CUA VANCE [" + std::to_string(keysCollected) + "/3]";
            std::string keyBody = "Ban da thu thap thanh cong Chia Khoa Vang va Manh Co Vat Sigil #" + std::to_string(currentRoomId + 1) + "!\n"
                                  "Co vat toa ra anh sang vang kim chong lai bong toi.\n"
                                  "Tong so co vat hien co: " + std::to_string(keysCollected) + "/3.\n\n"
                                  "Hay tien ve phia Cua Thoat Hiem [EXIT] de mo phong tiep theo!";
            openInspect(InspectType::GoldenKey, keyTitle, keyBody);
            setMessage("DA NHAT DUOC CHIA KHOA VANG & MANH SIGIL [" + std::to_string(keysCollected) + "/3]!", 8.0f);
            break;
        }
        case InteractionType::ExitDoor: {
            if (currentRoomId == 2) {
                // Final room: requires all 3 keys!
                if (keysCollected >= 3) {
                    if (exitDoorObject) {
                        exitDoorObject->isOpen = true;
                        exitDoorObject->isLocked = false;
                    }
                    obj->isOpen = true;
                    exitUnlocked = true;
                    gameWon = true;
                    setMessage("CHUC MUNG! Ban da thu thap du 3 Chia Khoa Vang va pha giai toan bo bi an ac mong!", 10.0f);
                } else {
                    setMessage("CUA THOAT HIEM CUOI BI PHONG AN! Can du 3 Chia Khoa Vang de mo (Hien co: " + std::to_string(keysCollected) + "/3).", 6.0f);
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
                    setMessage("MO KHOA THANH CONG! Cua thoat hiem bung mo! Dang chuyen sang can phong tiep theo...", 6.0f);
                } else {
                    setMessage("Cua thoat hiem dang bi KHOA CHAT! Hay tim manh giay & mo ket sat lay Chia Khoa Vang!", 5.0f);
                }
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
