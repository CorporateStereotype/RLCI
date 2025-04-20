#ifndef FRACTAL_NLP_ENGINE_HPP
#define FRACTAL_NLP_ENGINE_HPP

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <gvc.h>
#include <cgraph.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Observer {
    std::string name;
    std::string symbol;
    float belief_field;
    float modulation_strength;
    float perceptual_bandwidth;
};

struct DialogueTurn {
    std::string input;
    std::string response;
    std::string message;
    std::string mode;
    std::string sigil;
    float delta_O;
    float entropy;
    std::vector<std::string> tags;
    std::string timestamp;
    std::string turn_id;
};

class PowerLawAnalyzer {
private:
    std::vector<double> event_sizes_;
    std::map<int, int> histogram_;
    double alpha_;
public:
    PowerLawAnalyzer();
    void logEvent(double size);
    void buildHistogram();
    void fitPowerLaw();
    void saveAnalysis(const std::string& filename);
    double getAlpha() const;
};

class KnowledgeGraph {
private:
    std::unordered_map<std::string, Agnode_t*> nodes_;
    Agraph_t* graph_;
    GVC_t* gvc_;
    int edge_count_;
public:
    KnowledgeGraph();
    ~KnowledgeGraph();
    void addSymbol(const std::string& symbol);
    void addRelationship(const std::string& from, const std::string& to);
    void saveGraph(const std::string& filename);
};

class FractalMemoryCore {
private:
    std::vector<std::string> memory_;
public:
    void add_memory(const std::string& entry);
    std::vector<std::string> get_memory() const;
    std::string generate_dialogue_response(const std::string& input, const Observer& observer) const;
};

class SymbolicJournal {
private:
    std::string filename_;
    PowerLawAnalyzer power_law_analyzer_;
    KnowledgeGraph knowledge_graph_;
public:
    SymbolicJournal(const std::string& filename);
    void add_turn(const DialogueTurn& turn, const Observer& observer, const std::string& interference, float affinity);
    void savePowerLawAnalysis(const std::string& filename);
    void saveKnowledgeGraph(const std::string& filename);
    std::string get_message(float delta_O, float entropy) const;
    std::vector<std::string> generate_tags(const DialogueTurn& turn, const std::string& input_lower);
};

float observer_affinity(const std::string& current_observer, const std::string& previous_observer);

#endif
