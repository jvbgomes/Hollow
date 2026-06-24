#pragma once
#include <string>
#include <vector>

struct DialogueLine {
    std::string speaker;
    std::string text;
    bool        isPlayer = false;
};

using DialogueSequence = std::vector<DialogueLine>;

struct DialogueMenuOption {
    int         index;
    std::string label;
    bool        seen = false;
};
