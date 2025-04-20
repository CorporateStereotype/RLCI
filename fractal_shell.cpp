#include "fractalNLPengine.hpp"
#include <iostream>
#include <sstream>
#include <chrono>
#include <random>
#include <uuid/uuid.h>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>

std::string generate_uuid() {
    uuid_t uuid;
    uuid_generate_random(uuid);
    char uuid_str[37];
    uuid_unparse_lower(uuid, uuid_str);
    return std::string(uuid_str);
}

std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%FT%TZ");
    return ss.str();
}

float generate_power_law(float xmin, float xmax, float alpha, std::mt19937& gen) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float u = dist(gen);
    return xmin * std::pow(1.0f - u + u * std::pow(xmax / xmin, 1.0f - alpha), 1.0f / (1.0f - alpha));
}

void process_input(const std::string& prompt, std::vector<Observer>& observers, FractalMemoryCore& memory, SymbolicJournal& journal,
                   std::string& previous_observer_name, float& cumulative_entropy, float& future_entropy, float cross_timeline_entropy,
                   std::mt19937& gen, bool use_timeline1) {
    if (prompt == ":observers") {
        for (const auto& obs : observers) {
            std::cout << obs.name << " (" << obs.symbol << ")\n";
        }
        return;
    }
    if (prompt == ":save_power_law") {
        //journal.savePowerLawAnalysis(use_timeline(MSVC warning: use_timeline1) ? "power_law_analysis_timeline1.txt" : "power_law_analysis_timeline2.txt");
	journal.savePowerLawAnalysis(use_timeline1 ? "power_law_analysis_timeline1.txt" : "power_law_analysis_timeline2.txt");
        return;
    }
    if (prompt == ":save_knowledge_graph") {
        journal.saveKnowledgeGraph(use_timeline1 ? "knowledge_graph_timeline1.png" : "knowledge_graph_timeline2.png");
        return;
    }

    float next_future_entropy = generate_power_law(0.1f, 10.0f, 2.5f, gen);

    std::vector<float> weights(observers.size());
    for (size_t i = 0; i < observers.size(); ++i) {
        weights[i] = 1.0f + observers[i].perceptual_bandwidth * (cumulative_entropy + future_entropy + cross_timeline_entropy);
    }
    std::discrete_distribution<> dis(weights.begin(), weights.end());
    Observer obs = observers[dis(gen)];

    std::string input_lower = prompt;
    std::transform(input_lower.begin(), input_lower.end(), input_lower.begin(), ::tolower);

    DialogueTurn turn;
    turn.input = prompt;
    turn.response = memory.generate_dialogue_response(prompt, obs);
    turn.mode = (obs.name == "Grounded Realist") ? "semantic" : "token";
    turn.sigil = (obs.name == "Grounded Realist") ? "🔧 Calibration Node" :
                 (obs.name == "Fluid Mystic") ? "✶ Collapse Star" : "⨀ Quantum Eye";
    turn.delta_O = 0.9f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 1.0f));
    std::vector<float> entropy_candidates(3);
    for (float& e : entropy_candidates) {
        e = generate_power_law(0.1f, 10.0f, 2.5f, gen);
    }
    turn.entropy = *std::max_element(entropy_candidates.begin(), entropy_candidates.end());
    turn.message = journal.get_message(turn.delta_O, turn.entropy);
    turn.tags = journal.generate_tags(turn, input_lower);
    turn.timestamp = get_timestamp();
    turn.turn_id = generate_uuid();

    cumulative_entropy += turn.entropy;
    future_entropy = next_future_entropy;

    std::string interference = "No interference detected.";
    if (!memory.get_memory().empty()) {
        std::string prev_obs = memory.get_memory().back();
        if (obs.name != prev_obs) {
            interference = "🔄 Interference from [" + prev_obs + "]: ";
            if (obs.name == "Grounded Realist") {
                interference += "Disagreement sensed, recalibrating.";
            } else if (obs.name == "Fluid Mystic") {
                interference += "Resonant alignment detected.";
            } else {
                interference += "Cognitive dissonance—symbolic friction rising.";
            }
        }
    }

    float affinity = memory.get_memory().empty() ? 1.0f : observer_affinity(obs.name, previous_observer_name);
    journal.add_turn(turn, obs, interference, affinity);
    memory.add_memory(obs.name);
    previous_observer_name = obs.name;

    std::cout << "[Timeline " << (use_timeline1 ? "1" : "2") << "] ";
    std::cout << "[" << obs.name << " (" << obs.symbol << ")]: " << turn.response << "\n";
    std::cout << "Message: " << turn.message << "\n";
    std::cout << "Sigil: " << turn.sigil << "\n";
    std::cout << "Tags: ";
    for (const auto& tag : turn.tags) std::cout << tag << " ";
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    std::vector<Observer> observers = {
        {"Grounded Realist", "🧱", 0.2f, 0.22f, 0.9f},
        {"Fluid Mystic", "🌀", 0.85f, 0.57f, 1.5f},
        {"Recursive Oracle", "👁", 0.95f, 1.17f, 0.81f}
    };

    FractalMemoryCore memory_timeline1, memory_timeline2;
    SymbolicJournal journal_timeline1("symbolic_journal_timeline1.json");
    SymbolicJournal journal_timeline2("symbolic_journal_timeline2.json");

    std::string batch_file;
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg == "--batch" && i + 1 < argc) {
            batch_file = argv[++i];
        }
    }

    std::cout << "Fractal NLP Shell (type :quit or :exit to exit, :observers to list observers, :save_power_law to save power-law analysis, :save_knowledge_graph to save knowledge graph)\n";

    std::string previous_observer_name_t1 = "", previous_observer_name_t2 = "";
    std::random_device rd;
    std::mt19937 gen(rd());
    float cumulative_entropy_t1 = 0.0f, cumulative_entropy_t2 = 0.0f;
    float future_entropy_t1 = 0.0f, future_entropy_t2 = 0.0f;
    int turn_count = 0;

    // Process batch inputs if provided
    if (!batch_file.empty()) {
        std::ifstream infile(batch_file);
        if (!infile.is_open()) {
            std::cerr << "Error: Could not open batch file " << batch_file << "\n";
            return 1;
        }

        std::vector<std::string> batch_inputs;
        std::string line;
        while (std::getline(infile, line)) {
            if (!line.empty()) {
                batch_inputs.push_back(line);
            }
        }
        infile.close();

        for (const auto& prompt : batch_inputs) {
            if (prompt == ":quit" || prompt == ":exit") break;

            bool use_timeline1 = (turn_count % 2 == 0);
            FractalMemoryCore& memory = use_timeline1 ? memory_timeline1 : memory_timeline2;
            SymbolicJournal& journal = use_timeline1 ? journal_timeline1 : journal_timeline2;
            std::string& previous_observer_name = use_timeline1 ? previous_observer_name_t1 : previous_observer_name_t2;
            float& cumulative_entropy = use_timeline1 ? cumulative_entropy_t1 : cumulative_entropy_t2;
            float& future_entropy = use_timeline1 ? future_entropy_t1 : future_entropy_t2;
            float cross_timeline_entropy = use_timeline1 ? cumulative_entropy_t2 : cumulative_entropy_t1;

            std::cout << "> " << prompt << "\n";
            process_input(prompt, observers, memory, journal, previous_observer_name, cumulative_entropy, future_entropy, cross_timeline_entropy, gen, use_timeline1);
            turn_count++;
        }
    }

    // Interactive mode
    std::string prompt;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, prompt);

        if (prompt == ":quit" || prompt == ":exit") break;

        bool use_timeline1 = (turn_count % 2 == 0);
        FractalMemoryCore& memory = use_timeline1 ? memory_timeline1 : memory_timeline2;
        SymbolicJournal& journal = use_timeline1 ? journal_timeline1 : journal_timeline2;
        std::string& previous_observer_name = use_timeline1 ? previous_observer_name_t1 : previous_observer_name_t2;
        float& cumulative_entropy = use_timeline1 ? cumulative_entropy_t1 : cumulative_entropy_t2;
        float& future_entropy = use_timeline1 ? future_entropy_t1 : future_entropy_t2;
        float cross_timeline_entropy = use_timeline1 ? cumulative_entropy_t2 : cumulative_entropy_t1;

        process_input(prompt, observers, memory, journal, previous_observer_name, cumulative_entropy, future_entropy, cross_timeline_entropy, gen, use_timeline1);
        turn_count++;
    }

    return 0;
}
