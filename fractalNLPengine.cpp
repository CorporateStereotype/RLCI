#include "fractalNLPengine.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <random>
#include <cmath>
#include <uuid/uuid.h>

PowerLawAnalyzer::PowerLawAnalyzer() : alpha_(0.0) {}

void PowerLawAnalyzer::logEvent(double size) {
    event_sizes_.push_back(size);
}

void PowerLawAnalyzer::buildHistogram() {
    histogram_.clear();
    for (double size : event_sizes_) {
        int bin = static_cast<int>(size * 100); // Scale for finer bins
        histogram_[bin]++;
    }
}

void PowerLawAnalyzer::fitPowerLaw() {
    int n = event_sizes_.size();
    if (n < 2) {
        alpha_ = 0.0;
        return;
    }
    double sum_log = 0.0;
    double min_size = *std::min_element(event_sizes_.begin(), event_sizes_.end());
    if (min_size <= 0) min_size = 0.01; // Avoid log(0)
    for (double size : event_sizes_) {
        if (size > 0) sum_log += std::log(size / min_size);
    }
    alpha_ = 1.0 + n / sum_log;
}

void PowerLawAnalyzer::saveAnalysis(const std::string& filename) {
    buildHistogram();
    fitPowerLaw();
    std::ofstream out(filename);
    out << "# Event Size Distribution\n";
    out << "# Power-law exponent (alpha): " << alpha_ << "\n";
    out << "# Size\tFrequency\tPowerLawFit\n";
    for (const auto& [size, freq] : histogram_) {
        double scaled_size = size / 100.0;
        double fit = std::pow(scaled_size, -alpha_);
        out << scaled_size << "\t" << freq << "\t" << fit << "\n";
    }
    out.close();
    std::cout << "Power-law analysis saved to " << filename << ". Estimated alpha: " << alpha_ << std::endl;
}

double PowerLawAnalyzer::getAlpha() const { return alpha_; }

KnowledgeGraph::KnowledgeGraph() : edge_count_(0) {
    gvc_ = gvContext();
    graph_ = agopen(const_cast<char*>("FractalNLPGraph"), Agdirected, nullptr);
}

KnowledgeGraph::~KnowledgeGraph() {
    agclose(graph_);
    gvFreeContext(gvc_);
}

void KnowledgeGraph::addSymbol(const std::string& symbol) {
    if (nodes_.find(symbol) == nodes_.end()) {
        Agnode_t* node = agnode(graph_, const_cast<char*>(symbol.c_str()), 1);
        nodes_[symbol] = node;
    }
}

void KnowledgeGraph::addRelationship(const std::string& from, const std::string& to) {
    addSymbol(from);
    addSymbol(to);
    Agedge_t* edge = agedge(graph_, nodes_[from], nodes_[to], nullptr, 1);
    char edge_id[32];
    snprintf(edge_id, sizeof(edge_id), "e%d", edge_count_++);
    agsafeset(edge, const_cast<char*>("label"), edge_id, const_cast<char*>(""));
}

void KnowledgeGraph::saveGraph(const std::string& filename) {
    gvLayout(gvc_, graph_, "dot");
    gvRenderFilename(gvc_, graph_, "png", const_cast<char*>(filename.c_str()));
    gvFreeLayout(gvc_, graph_);
    std::cout << "Knowledge graph saved to " << filename << std::endl;
}

void FractalMemoryCore::add_memory(const std::string& entry) {
    memory_.push_back(entry);
}

std::vector<std::string> FractalMemoryCore::get_memory() const {
    return memory_;
}

std::string FractalMemoryCore::generate_dialogue_response(const std::string& input, const Observer& observer) const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 2);

    std::string response = "Generated: ";
    if (observer.name == "Grounded Realist") {
        int choice = dis(gen);
        if (choice == 0) response += "observer signal signal";
        else if (choice == 1) response += "calibration in progress";
        else response += "semantic anchor detected";
    } else if (observer.name == "Fluid Mystic") {
        int choice = dis(gen);
        if (choice == 0) response += "entropy signal signal";
        else if (choice == 1) response += "flow state activated";
        else response += "resonance pulse emitted";
    } else if (observer.name == "Recursive Oracle") {
        int choice = dis(gen);
        if (choice == 0) response += "coherence observer coherence";
        else if (choice == 1) response += "recursive loop initiated";
        else response += "quantum eye scanning";
    }
    return response;
}

SymbolicJournal::SymbolicJournal(const std::string& filename) : filename_(filename) {}

std::string SymbolicJournal::get_message(float delta_O, float entropy) const {
    if (delta_O > 1.5f) {
        return "⚠️ Divergence detected. Semantic clarity unraveling.";
    }
    return "🔄 Meaning fluctuation within acceptable thresholds.";
}

std::vector<std::string> SymbolicJournal::generate_tags(const DialogueTurn& turn, const std::string& input_lower) {
    std::vector<std::string> tags;
    if (turn.entropy > 1.0f) tags.push_back("#entropy-spike");
    if (turn.delta_O > 1.5f) tags.push_back("#high-decoherence");
    if (input_lower.find("happy") != std::string::npos || input_lower.find("birth day") != std::string::npos) {
        tags.push_back("#celebration");
    }
    if (input_lower.find("uncertainty") != std::string::npos || input_lower.find("doubt") != std::string::npos) {
        tags.push_back("#introspection");
    }
    if (input_lower.find("quantum eye") != std::string::npos) {
        tags.push_back("#symbol-request");
    }
    if (turn.delta_O > 1.5f && turn.response.find("paradox") != std::string::npos) {
        tags.push_back("#paradox");
    }
    if (input_lower.find("recursive oracle") != std::string::npos || input_lower.find("fluid mystic") != std::string::npos ||
        input_lower.find("grounded realist") != std::string::npos || input_lower.find("recusive") != std::string::npos) {
        tags.push_back("#observer-interaction");
    }
    return tags;
}

void SymbolicJournal::add_turn(const DialogueTurn& turn, const Observer& observer,
                              const std::string& interference, float affinity) {
    static std::string previous_sigil;

    json j;
    std::ifstream file(filename_);
    if (file.good()) file >> j;
    else j["journal"] = json::array();

    json turn_entry;
    turn_entry["context"]["affinity"] = affinity;
    turn_entry["context"]["interference"] = interference;
    turn_entry["context"]["previous_observer"] = observer.name;
    turn_entry["input"] = turn.input;
    turn_entry["message"] = turn.message;
    turn_entry["metrics"]["delta_O"] = turn.delta_O;
    turn_entry["metrics"]["entropy"] = turn.entropy;
    turn_entry["mode"] = turn.mode;
    turn_entry["observer"]["belief_field"] = observer.belief_field;
    turn_entry["observer"]["modulation_strength"] = observer.modulation_strength;
    turn_entry["observer"]["name"] = observer.name;
    turn_entry["observer"]["perceptual_bandwidth"] = observer.perceptual_bandwidth;
    turn_entry["observer"]["symbol"] = observer.symbol;
    turn_entry["response"] = turn.response;
    turn_entry["sigil"] = turn.sigil;
    turn_entry["tags"] = turn.tags;
    turn_entry["timestamp"] = turn.timestamp;
    turn_entry["turn_id"] = turn.turn_id;

    j["journal"].push_back(turn_entry);
    std::ofstream out(filename_);
    out << j.dump(4);

    power_law_analyzer_.logEvent(turn.entropy);
    knowledge_graph_.addSymbol(turn.sigil);
    knowledge_graph_.addSymbol(observer.symbol);
    knowledge_graph_.addRelationship(observer.symbol, turn.sigil);
    if (!previous_sigil.empty()) {
        knowledge_graph_.addRelationship(previous_sigil, turn.sigil);
    }
    previous_sigil = turn.sigil;
}

void SymbolicJournal::savePowerLawAnalysis(const std::string& filename) {
    power_law_analyzer_.saveAnalysis(filename);
}

void SymbolicJournal::saveKnowledgeGraph(const std::string& filename) {
    knowledge_graph_.saveGraph(filename);
}

float observer_affinity(const std::string& current_observer, const std::string& previous_observer) {
    if (current_observer == previous_observer) return 1.0f;
    if ((current_observer == "Grounded Realist" && previous_observer == "Fluid Mystic") ||
        (current_observer == "Fluid Mystic" && previous_observer == "Grounded Realist")) {
        return 0.7f;
    }
    if ((current_observer == "Recursive Oracle" && previous_observer == "Fluid Mystic") ||
        (current_observer == "Fluid Mystic" && previous_observer == "Recursive Oracle")) {
        return 0.85f;
    }
    if ((current_observer == "Grounded Realist" && previous_observer == "Recursive Oracle") ||
        (current_observer == "Recursive Oracle" && previous_observer == "Grounded Realist")) {
        return 0.6f;
    }
    return 0.5f;
}
