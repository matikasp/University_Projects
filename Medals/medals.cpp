#include <iostream>
#include <unordered_map>
#include <regex>
#include <algorithm>
#include <array>
#include <inttypes.h>
#include <vector>


// in the task there are 3 possible medals
constexpr int MEDAL_TYPE_COUNT = 3;

// alias for the type of hashmap used in the task
using MedalMap = std::unordered_map<std::string, std::array<std::size_t, MEDAL_TYPE_COUNT>>;


// function declarations

// main function for parsing data
void parseData(void);

// function that parses the earning of a medal by a country
void parseEarningMedal(std::size_t lineCounter, const std::string &line,
                       MedalMap &medals);

// helper function to determine what action to take
void determineAction(std::size_t lineCounter, std::string &line,
                     MedalMap &medals);

// function that parses the revoking of a medal by a country
void parseRevokingMedal(std::size_t lineCounter, const std::string &line,
                        MedalMap &medals);

// function that outputs the current ranking
void listRankings(std::size_t lineCounter, const std::string &line,
                  const MedalMap &medals);

// function to output errors to std::out
void handleError(std::size_t lineCounter);


int main() {
    parseData();
    return 0;
}

void parseData(void) {
    // variable to store the current line
    std::string line;

    // in this unordered_map we store how many medals each country currently has
    MedalMap medals;

    std::size_t lineCounter = 1;

    // Reading subsequent lines from the input text
    while(std::getline(std::cin, line)) {

        // an empty line is treated as invalid input
        if (line.empty()) {
            handleError(lineCounter);
        }
        else {
            determineAction(lineCounter, line, medals);
        }
        lineCounter++;
    }

}

void determineAction(std::size_t lineCounter, std::string &line,
                     MedalMap &medals) {
    // a '-' at the beginning of the line means a medal is being revoked
    if (line[0] == '-') {
        parseRevokingMedal(lineCounter, line, medals);
    }
    // a '=' at the beginning of the line means we want to print the ranking
    else if (line[0] == '=') {
        listRankings(lineCounter, line, medals);
    }
    // otherwise, we assume we want to parse a medal
    // (input data correctness is checked later)
    else {
        parseEarningMedal(lineCounter, line, medals);
    }
}

void parseEarningMedal(std::size_t lineCounter, const std::string &line,
                       MedalMap &medals) {

    // regex to check the correctness of the line
    // one uppercase letter A-Z, followed by 0 or more lowercase letters and spaces
    // the country name ends with a letter
    // then there's a space followed by a digit 0-3
    const std::regex countryRegex("^([A-Z][A-Za-z ]*[A-Za-z]) ([0-3])$");

    // to store matches and split the line into the country name and medal
    std::smatch country_medal;

    // we check if we have a match - if not, it means invalid input data
    if (!std::regex_match(line, country_medal, countryRegex)) {
        handleError(lineCounter);
        return;
    }

    std::string country = country_medal[1];
    const int32_t medal = std::stoi(country_medal[2]);

    // we check in the medals hashmap whether the country already exists in it
    // if not, we add it
    auto it = medals.find(country);
    if (it == medals.end()) {
        it = medals.emplace(country, std::array<std::size_t, 
                            MEDAL_TYPE_COUNT>{0}).first;
    }
    // if the medal is not 0 (which means no medal), we increase the medal count of the corresponding type by 1
    if (medal > 0) {
        it->second[medal - 1]++;
    }
}

void parseRevokingMedal(std::size_t lineCounter, const std::string &line,
                        MedalMap &medals) {
    // regex to check the correctness of the line
    // analogous to the regex from parseEarningMedal,
    // but it starts with '-' and ends with a digit 1-3
    const std::regex countryRegex("^-([A-Z][A-Za-z ]*[A-Za-z]) ([1-3])$");

    // to store matches and split the line into the country name and medal
    std::smatch country_medal;

    // we check if we have a match - if not, it means invalid input data
    if (!std::regex_match(line, country_medal, countryRegex)) {
        handleError(lineCounter);
        return;
    }

    std::string country = country_medal[1];
    const int32_t medal = std::stoi(country_medal[2]);

    // we check in the medals hashmap whether the country already existed in it
    // or if the country has 0 medals of that type
    // if one of these conditions is met, we output an error
    const auto it = medals.find(country);
    if (it == medals.end() || it->second[medal - 1] == 0) {
        handleError(lineCounter);
        return;
    }

    // we decrease the number of medals of that type by 1
    it->second[medal - 1]--;
    
}

void listRankings(std::size_t lineCounter, const std::string &line,
                  const MedalMap &medals) {
    
    // regex to check the correctness of the input data
    // the line starts with '='
    // followed by 3 integers in the range [1,999999] separated by spaces
    // numbers cannot have leading zeros
    const std::regex rankRegex("^=([1-9][0-9]{0,5}) ([1-9][0-9]{0,5}) ([1-9][0-9]{0,5})$");

    // medal weights
    std::smatch weighs;

    if (std::regex_match(line, weighs, rankRegex)) {
        
        std::array<int32_t, MEDAL_TYPE_COUNT> weighsArr;

        for (int32_t idx = 0; idx < MEDAL_TYPE_COUNT; idx++) {
            weighsArr[idx] = std::stoi(weighs[idx + 1].str());
        }

        std::vector<std::pair<std::size_t, std::string>> rank;
        // here we store pairs (score, country) and then sort by scores

        for (const auto& medal: medals) {
            // count each type of medal and then calculate the score
            std::array<int32_t, MEDAL_TYPE_COUNT> noOfMedals;

            for (int idx = 0; idx < MEDAL_TYPE_COUNT; idx++) {
                noOfMedals[idx] = medal.second[idx];
            }

            std::string country = medal.first;

            std::size_t score = 0;

            for (int idx = 0; idx < MEDAL_TYPE_COUNT; idx++) {
                score += noOfMedals[idx]*weighsArr[idx];
            }

            rank.push_back(std::make_pair(score, country));
        }

        // we sort rank in descending order by score and ascending by names
        sort(rank.begin(), rank.end(), [](const auto& a, const auto& b) {
            return std::tie(a.first, b.second) > std::tie(b.first, a.second);
        });
    
        // here we output the current ranking
        std::size_t position = 1;
        std::size_t prev_score = 0;
        for (std::size_t idx = 0; idx < rank.size(); idx++) {
            std::size_t curr_score = rank[idx].first;
            if (curr_score != prev_score) {
                position = idx + 1;
            }
            prev_score = curr_score;

            std::cout << position << ". " << rank[idx].second << std::endl;
            
        }
    }
    else {
        // if we don't have a match, we output an error message
        handleError(lineCounter);
    }
    
}


void handleError(const std::size_t lineCounter) {
    std::cerr << "ERROR " << lineCounter << std::endl;
}
