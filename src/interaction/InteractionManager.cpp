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
            return "[E] Bat/Tat den tran (Light Switch)";
        case InteractionType::DeskDrawer: {
            bool open = drawerObject ? drawerObject->isOpen : obj->isOpen;
            return open ? "[E] Dong ngan keo" : "[E] Mo ngan keo (Tim manh giay mat ma)";
        }
        case InteractionType::SecretNote:
            return "[E] Doc manh giay mat ma [PIN: " + correctSafePin + "]";
        case InteractionType::SafeKeypad:
            return safeUnlocked ? "[E] Ket sat / Cua da mo khoa" : "[E] Nhap ma PIN mo cua (Ho tro ban phim 0-9)";
        case InteractionType::SafeDoor: {
            if (!safeUnlocked) return "[E] Ket sat dang KHOA! Bam [E] de nhap ma PIN.";
            bool open = safeDoorObject ? safeDoorObject->isOpen : obj->isOpen;
            return open ? "[E] Dong cua ket sat" : "[E] Mo cua ket sat";
        }
        case InteractionType::GoldenKey:
            return "[E] Nhat Chia Khoa & Manh Sigil thoat hiem!";
        case InteractionType::ExitDoor: {
            if (currentRoomId == 0) {
                if (!hasGoldenKey) return "[E] Cua thoat hiem [KHOA] - Can Chia Khoa Nghi Le tren be tho!";
            } else if (currentRoomId == 1) {
                if (!safeUnlocked && !hasGoldenKey) return "[E] Cua buong giam [KHOA MA SO] - Dung den UV soi mat ma tren tuong!";
            } else if (currentRoomId == 2) {
                if (!mirrorPuzzleSolved && keysCollected < 3) return "[E] Cua Thanh Dia [PHONG AN] - Quan sat anh phan chieu trong guong de giai!";
            }
            bool open = exitDoorObject ? exitDoorObject->isOpen : obj->isOpen;
            return open ? "[E] Buoc qua cua thoat hiem" : "[E] MO CUA THOAT HIEM";
        }
        case InteractionType::Totem: {
            int idx = 0;
            if (obj->id.find("2") != std::string::npos) idx = 1;
            else if (obj->id.find("3") != std::string::npos) idx = 2;
            int angle = static_cast<int>(totemAngles[idx]) % 360;
            return "[E] Xoay Cot Totem #" + std::to_string(idx + 1) + " (Goc hien tai: " + std::to_string(angle) + " do)";
        }
        case InteractionType::FuseItem:
            return "[E] Nhat " + obj->name;
        case InteractionType::BreakerBox: {
            if (powerRestored) return "[E] Tu Cau Chi (Nguon dien 220V da duoc kich hoat)";
            if (fusesCollected < 2) return "[E] Tu Cau Chi (Thieu 2 cau chi, hien co: " + std::to_string(fusesCollected) + "/2)";
            return "[E] LAP 2 CAU CHI KHOI PHUC NGUON DIEN & MO CONG";
        }
        case InteractionType::IsolationGate: {
            if (!powerRestored) return "[E] Cong song sat [KHOA DIEN TU] - Can khoi phuc nguon dien tai Tu Cau Chi!";
            return obj->isOpen ? "[E] Dong cong song sat" : "[E] Mo cong song sat";
        }
        case InteractionType::MirrorBust: {
            int idx = 0;
            if (obj->id.find("2") != std::string::npos) idx = 1;
            else if (obj->id.find("3") != std::string::npos) idx = 2;
            int angle = static_cast<int>(bustAngles[idx]) % 360;
            return "[E] Xoay Tuong Ban Than #" + std::to_string(idx + 1) + " (Goc: " + std::to_string(angle) + " do - Nhin vao guong de so sanh)";
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
            if (audioEngine) audioEngine->playDrawerSlide(glm::vec3(target->getWorldMatrix()[3]));
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
                if (audioEngine) audioEngine->playKeypadBeep(true);
                setMessage("Ket sat dang khoa! Hay go ma PIN 4 chu so (" + correctSafePin + ").", 5.0f);
            } else {
                auto target = safeDoorObject ? safeDoorObject : obj;
                target->isOpen = !target->isOpen;
                if (audioEngine) audioEngine->playDoorCreak(glm::vec3(target->getWorldMatrix()[3]));
                setMessage(target->isOpen ? "Da mo cua ket sat." : "Da dong cua ket sat.");
            }
            break;
        }
        case InteractionType::GoldenKey: {
            if (currentRoomId == 0 && !altarUnlocked && altarLidObject && !altarLidObject->isOpen) {
                setMessage("Nap be tho van dang khoa kin! Hay xoay 3 cot Totem sao cho bong do khop voi co tu.", 5.0f);
                return;
            }
            if (currentRoomId == 1 && !powerRestored) {
                setMessage("Can khoi phuc nguon dien va buoc vao buong giam truoc!", 4.0f);
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
            std::string keyTitle = "CHIA KHOA & MANH SIGIL CUA VANCE [" + std::to_string(keysCollected) + "/3]";
            std::string keyBody = "Ban da thu thap thanh cong Chia Khoa va Manh Co Vat Sigil #" + std::to_string(currentRoomId + 1) + "!\n"
                                  "Co vat toa ra anh sang vang kim chong lai bong toi.\n"
                                  "Tong so co vat hien co: " + std::to_string(keysCollected) + "/3.\n\n"
                                  "Hay tien ve phia Cua Thoat Hiem [EXIT] de tiep tuc hanh trinh!";
            openInspect(InspectType::GoldenKey, keyTitle, keyBody);
            setMessage("DA NHAT DUOC CHIA KHOA & MANH SIGIL [" + std::to_string(keysCollected) + "/3]!", 8.0f);
            break;
        }
        case InteractionType::ExitDoor: {
            if (currentRoomId == 2) {
                // Final room: requires mirror puzzle solved or all 3 keys!
                if (mirrorPuzzleSolved || keysCollected >= 3) {
                    if (exitDoorObject) {
                        exitDoorObject->isOpen = true;
                        exitDoorObject->isLocked = false;
                    }
                    obj->isOpen = true;
                    exitUnlocked = true;
                    gameWon = true;
                    if (audioEngine) audioEngine->playDoorCreak(glm::vec3(obj->getWorldMatrix()[3]));
                    setMessage("CHUC MUNG! Ban da hoa giai thanh cong ao anh Guong Quy va thoat khoi vien tam than!", 12.0f);
                } else {
                    if (audioEngine) audioEngine->playKeypadBeep(false);
                    setMessage("CUA THANH DIA BI PHONG AN! Hay quan sat guong va xoay 3 tuong ban than cho can xung!", 6.0f);
                }
            } else if (currentRoomId == 1) {
                // Room 2: Asylum door opened by safeUnlocked (PIN 4108) or hasGoldenKey
                if (safeUnlocked || hasGoldenKey) {
                    if (exitDoorObject) {
                        exitDoorObject->isOpen = true;
                        exitDoorObject->isLocked = false;
                    }
                    obj->isOpen = true;
                    exitUnlocked = true;
                    requestRoomTransition = true;
                    if (audioEngine) audioEngine->playDoorCreak(glm::vec3(obj->getWorldMatrix()[3]));
                    setMessage("MO KHOA THANH CONG! Dang chuyen sang Can Phong Guong (Room 3)...", 6.0f);
                } else {
                    showKeypadModal = true;
                    enteredPin = "";
                    if (audioEngine) audioEngine->playKeypadBeep(false);
                    setMessage("Cua buong giam khoa bang mat ma so! Bat den pin UV [Chuot phai / F] soi len tuong!", 5.0f);
                }
            } else {
                // Room 0: Occult Ritual room
                if (hasGoldenKey) {
                    if (exitDoorObject) {
                        exitDoorObject->isOpen = true;
                        exitDoorObject->isLocked = false;
                    }
                    obj->isOpen = true;
                    exitUnlocked = true;
                    requestRoomTransition = true;
                    if (audioEngine) audioEngine->playDoorCreak(glm::vec3(obj->getWorldMatrix()[3]));
                    setMessage("MO KHOA THANH CONG! Dang tien vao Benh Vien Tam Than Bo Hoang (Room 2)...", 6.0f);
                } else {
                    if (audioEngine) audioEngine->playKeypadBeep(false);
                    setMessage("Cua thoat hiem dang bi KHOA CHAT! Hay giai cau do chieu bong Totem tren be tho!", 5.0f);
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
                setMessage("BONG DO KHOP VOI CO TU! Nap be tho da truot mo, lo ra Chia Khoa Nghi Le!", 8.0f);
            } else {
                int deg = static_cast<int>(totemAngles[idx]) % 360;
                setMessage("Da xoay Totem #" + std::to_string(idx + 1) + " sang " + std::to_string(deg) + " do. Quan sat bong do len tuong Bac!", 4.0f);
            }
            break;
        }
        case InteractionType::FuseItem: {
            fusesCollected++;
            obj->isVisible = false;
            if (audioEngine) audioEngine->playItemPickup();
            setMessage("Da nhat: " + obj->name + " [" + std::to_string(fusesCollected) + "/2]! Hay dem den Tu Cau Chi o goc tuong.", 6.0f);
            break;
        }
        case InteractionType::BreakerBox: {
            if (powerRestored) {
                setMessage("Tu cau chi da hoat dong. Toan bo he thong dien phong 2 da on dinh.", 4.0f);
                break;
            }
            if (fusesCollected < 2) {
                if (audioEngine) audioEngine->playKeypadBeep(false);
                setMessage("Tu cau chi dang thieu cau chi (" + std::to_string(fusesCollected) + "/2)! Tim tren xe day y te & bon rua.", 5.0f);
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
            setMessage("DA LAP 2 CAU CHI! Dien da sang, khoa tu dong nhung mo! Cong song sat da mo!", 8.0f);
            break;
        }
        case InteractionType::IsolationGate: {
            if (!powerRestored) {
                if (audioEngine) audioEngine->playKeypadBeep(false);
                setMessage("Cong song sat bi KHOA DIEN TU! Can khoi phuc nguon dien tai Tu Cau Chi.", 5.0f);
                break;
            }
            obj->isOpen = !obj->isOpen;
            if (audioEngine) audioEngine->playDoorCreak(glm::vec3(obj->getWorldMatrix()[3]));
            setMessage(obj->isOpen ? "Da mo cong song sat buong giam." : "Da dong cong song sat.", 3.0f);
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
                setMessage("AO ANH TRONG GUONG DA TRUNG KHOP! Phong an giai tru, Cua Thanh Dia da mo toang!", 10.0f);
            } else {
                int deg = static_cast<int>(bustAngles[idx]) % 360;
                setMessage("Da xoay Tuong #" + std::to_string(idx + 1) + " (" + std::to_string(deg) + " do). Hay nhin vao guong kiem tra vi tri!", 4.0f);
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
        if (audioEngine) {
            audioEngine->playKeypadBeep(true);
            audioEngine->playSafeUnlock(safeDoorObject ? glm::vec3(safeDoorObject->getWorldMatrix()[3]) : glm::vec3(3.2f, 1.46f, 0.0f));
        }
        setMessage("DUNG MA PIN! Ket sat da mo khoa va bung cua! Nhat Chia Khoa Vang ben trong!", 8.0f);
        return true;
    } else {
        if (audioEngine) {
            audioEngine->playKeypadBeep(false);
        }
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

    setMessage("Escape Room reset! Kham pha cac phong va pha giai loi nguyen Dr. Vance!", 5.0f);
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
